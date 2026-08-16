// PowerEditor/src/linuxmain.cpp — Punto de entrada Linux para Notepad++ (Qt6)
// Parte del plan de migración Notepad++ → Linux nativo
//
// Reemplaza winmain.cpp en Linux. Gestiona:
//   - Inicialización de la aplicación Qt6 (QApplication)
//   - Instancia única via QLocalServer/QLocalSocket
//   - Parseo de argumentos de línea de comandos
//   - Arranque de la ventana principal Notepad++ en Qt6
//
// Copyright (C) Notepad++ contributors. GPL v3+

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformString.h"
#include "Platform/PlatformMsg.h"

#include <QApplication>
#include <QMainWindow>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QFileInfo>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <string>
#include <vector>
#include <iostream>

// ─── Forward declarations ──────────────────────────────────────────────────── 
// Declaradas aquí; la implementación estará en NotepadPlusApp_linux.cpp (Fase 3b)
void nppLinuxInit(QApplication* app, const std::vector<NppString>& filesToOpen);
void nppLinuxShutdown();

// ─── Application constants ──────────────────────────────────────────────────
static constexpr const char* NPP_APP_NAME    = "Notepad++";
static constexpr const char* NPP_ORG_NAME    = "notepad-plus-plus";
static constexpr const char* NPP_SOCKET_NAME = "org.notepad-plus-plus.Notepad";

// ─── Contexto de activación ───────────────────────────────────────────────────
struct AppContext {
    std::vector<NppString>  openFiles;         // archivos pedidos en args
    bool                    newInstance = false; // -multiInst
    bool                    noPlugin    = false; // -noPlugin
    NppString               language;           // -l <lang>
    int                     lineNumber  = -1;   // -n <line>
    int                     colNumber   = -1;   // -c <col>
};

// ─── Instancia única via QLocalServer ────────────────────────────────────────
// Si ya existe una instancia corriendo, le enviamos los archivos a abrir y salimos.
// Equivalente a la lógica de mutex + WM_COPYDATA en winmain.cpp.

static bool tryConnectToExistingInstance(const AppContext& ctx) {
    QLocalSocket socket;
    socket.connectToServer(NPP_SOCKET_NAME);
    if (!socket.waitForConnected(500)) {
        return false; // No hay instancia corriendo
    }

    // Enviar archivos como líneas separadas por '\n'
    QByteArray data;
    for (const auto& f : ctx.openFiles) {
        data.append(QByteArray::fromStdString(f));
        data.append('\n');
    }
    socket.write(data);
    socket.waitForBytesWritten(1000);
    socket.disconnectFromServer();
    return true; // Conexión exitosa, salir
}

static QLocalServer* startLocalServer() {
    auto* server = new QLocalServer();
    // Eliminar socket viejo si quedó de un crash
    QLocalServer::removeServer(NPP_SOCKET_NAME);
    if (!server->listen(NPP_SOCKET_NAME)) {
        qWarning("No se pudo iniciar QLocalServer: %s",
                 qPrintable(server->errorString()));
        delete server;
        return nullptr;
    }

    // Cuando otra instancia se conecta, leer los archivos y enviarlos al bus
    QObject::connect(server, &QLocalServer::newConnection, [server]() {
        QLocalSocket* client = server->nextPendingConnection();
        if (!client) return;
        QObject::connect(client, &QLocalSocket::readyRead, [client]() {
            QByteArray data = client->readAll();
            QStringList paths = QString::fromUtf8(data).split('\n',
                Qt::SkipEmptyParts);
            for (const auto& p : paths) {
                std::string path = p.toStdString();
                NppMsgBus::instance().post(NppMsg::FILE_OPEN,
                    0, reinterpret_cast<NppLparam>(&path));
            }
            client->deleteLater();
        });
    });

    return server;
}

// ─── main() ───────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    // 1. Crear QApplication (debe ser primero para poder usar QCommandLineParser)
    QApplication app(argc, argv);
    app.setApplicationName(NPP_APP_NAME);
    app.setOrganizationName(NPP_ORG_NAME);
    app.setApplicationVersion("9.0-linux"); // versión del port

    // 2. Parsear argumentos
    QCommandLineParser parser;
    parser.setApplicationDescription("Notepad++ — Editor de código para Linux");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption multiInstOpt("multiInst", "Abrir nueva instancia");
    QCommandLineOption noPluginOpt("noPlugin", "No cargar plugins");
    QCommandLineOption langOpt(QStringList() << "l" << "lang",
        "Lenguaje de sintaxis", "language");
    QCommandLineOption lineOpt(QStringList() << "n" << "line",
        "Ir a número de línea", "lineNumber");
    QCommandLineOption colOpt(QStringList() << "c" << "col",
        "Ir a número de columna", "colNumber");
    parser.addOption(multiInstOpt);
    parser.addOption(noPluginOpt);
    parser.addOption(langOpt);
    parser.addOption(lineOpt);
    parser.addOption(colOpt);
    parser.addPositionalArgument("files", "Archivos a abrir", "[archivo...]");

    parser.process(app);

    AppContext ctx;
    ctx.newInstance = parser.isSet(multiInstOpt);
    ctx.noPlugin   = parser.isSet(noPluginOpt);
    if (parser.isSet(langOpt)) ctx.language = parser.value(langOpt).toStdString();
    if (parser.isSet(lineOpt)) ctx.lineNumber = parser.value(lineOpt).toInt();
    if (parser.isSet(colOpt))  ctx.colNumber  = parser.value(colOpt).toInt();

    // Resolver rutas de archivos a rutas absolutas
    for (const auto& arg : parser.positionalArguments()) {
        QFileInfo fi(arg);
        ctx.openFiles.push_back(fi.absoluteFilePath().toStdString());
    }

    // 3. Instancia única (a menos que se pida -multiInst)
    if (!ctx.newInstance) {
        if (tryConnectToExistingInstance(ctx)) {
            // Otra instancia ya corre; le pasamos los archivos y salimos
            return 0;
        }
    }

    // 4. Iniciar servidor local (para recibir archivos de futuras instancias)
    QLocalServer* server = nullptr;
    if (!ctx.newInstance) {
        server = startLocalServer();
    }

    // 5. Inicializar la ventana principal Notepad++
    nppLinuxInit(&app, ctx.openFiles);

    // 6. Event loop
    int status = app.exec();

    // 7. Cleanup
    nppLinuxShutdown();
    if (server) {
        server->close();
        delete server;
    }

    return status;
}

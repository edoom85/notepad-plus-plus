// PowerEditor/src/NotepadPlusApp_linux.cpp — Inicialización de Notepad++ en Linux
// Conecta linuxmain.cpp con NotepadPlusWindowQt
//
// Copyright (C) Notepad++ contributors. GPL v3+

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include "WinControls/Qt/NotepadPlusWindowQt.h"

#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QFontDatabase>
#include <QStyle>
#include <QString>

#include "Platform/PlatformSingleInstance.h"

#include <iostream>
#include <vector>
#include <string>

#include "Platform/PlatformCrashHandler.h"
#include "Platform/PlatformDesktopIntegration.h"

static NotepadPlusWindowQt* g_mainWindow = nullptr;
static PlatformSingleInstance* g_singleInstance = nullptr;

/// Aplica tema oscuro global a toda la aplicación Qt6.
static void applyGlobalDarkTheme(QApplication* app) {
    app->setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::WindowText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Base,            QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::AlternateBase,   QColor(0x25, 0x25, 0x26));
    dark.setColor(QPalette::ToolTipBase,     QColor(0x25, 0x25, 0x26));
    dark.setColor(QPalette::ToolTipText,     QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Text,            QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Button,          QColor(0x33, 0x33, 0x33));
    dark.setColor(QPalette::ButtonText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::BrightText,      QColor(0xFF, 0x44, 0x44));
    dark.setColor(QPalette::Link,            QColor(0x56, 0x9C, 0xD6));
    dark.setColor(QPalette::Highlight,       QColor(0x26, 0x4F, 0x78));
    dark.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
    dark.setColor(QPalette::PlaceholderText, QColor(0x6C, 0x6C, 0x6C));
    dark.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x6C, 0x6C, 0x6C));
    dark.setColor(QPalette::Disabled, QPalette::Text,       QColor(0x6C, 0x6C, 0x6C));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x6C, 0x6C, 0x6C));

    app->setPalette(dark);

    // Fuente por defecto: intentar fuentes modernas
    QFont appFont("Inter", 10);
    if (!QFontDatabase::hasFamily("Inter")) {
        appFont = QFont("Noto Sans", 10);
        if (!QFontDatabase::hasFamily("Noto Sans")) {
            appFont = QFont("Sans", 10);
        }
    }
    app->setFont(appFont);
}

/// Inicializa la ventana principal de Notepad++.
/// Llamada desde linuxmain.cpp después de crear QApplication.
void nppLinuxInit(QApplication* app, const std::vector<NppString>& filesToOpen) {
    // -1. Instalar Crash Handler para generar informes de depuración en caso de SIGSEGV
    NppCrashHandler::install();

    // 0. Instancia Única: si ya hay una instancia corriendo, enviarle los archivos e ingresar al proceso existente
    QStringList filesList;
    for (const auto& path : filesToOpen) {
        filesList.append(QString::fromStdString(path));
    }

    if (PlatformSingleInstance::sendToExistingInstance(filesList)) {
        std::cout << "[Notepad++] Archivos enviados a la instancia activa de Notepad++." << std::endl;
        ::exit(0);
    }

    // 1. Icono global de la aplicación nativa (Dock / Alt+Tab / Wayland / X11)
    QIcon appIcon;
    appIcon.addFile(":/Platform/npp_icon.png");
    appIcon.addFile(":/icons/npp_256.ico");
    app->setQuitOnLastWindowClosed(true);
    app->setWindowIcon(appIcon);
    app->setDesktopFileName("notepadplusplus");


    // 2. Tema oscuro global
    applyGlobalDarkTheme(app);

    // 3. Crear ventana principal
    g_mainWindow = new NotepadPlusWindowQt();
    g_mainWindow->show();

    // 3b. Integración de escritorio y TrayIcon
    auto* desktopInt = new NppDesktopIntegration(g_mainWindow, app);
    desktopInt->createTrayIcon(appIcon);


    // 4. Iniciar servidor de instancia única
    g_singleInstance = new PlatformSingleInstance(g_mainWindow);
    QObject::connect(g_singleInstance, &PlatformSingleInstance::openFilesRequested, [](const QStringList& files) {
        if (!g_mainWindow) return;
        for (const QString& file : files) {
            if (!file.trimmed().isEmpty()) {
                g_mainWindow->openFile(file.toStdString());
            }
        }
        g_mainWindow->show();
        g_mainWindow->raise();
        g_mainWindow->activateWindow();
    });

    // 5. Abrir archivos pasados por línea de comandos
    for (const auto& path : filesToOpen) {
        g_mainWindow->openFile(path);
    }
}


/// Limpia recursos al cerrar la aplicación.
void nppLinuxShutdown() {
    delete g_mainWindow;
    g_mainWindow = nullptr;
}

#endif // NPP_PLATFORM_LINUX

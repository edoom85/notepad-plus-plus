// Platform/PlatformSingleInstance.h — Gestión de Instancia Única en Linux/Qt6
// Reemplaza el manejo de instancia única Win32 (WM_COPYDATA / Mutex)
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QLocalServer>
#include <QLocalSocket>
#include <QStringList>
#include <QObject>
#include <QApplication>

/// Gestiona el modo de instancia única en Notepad++ Linux utilizando IPC local socket.
class PlatformSingleInstance : public QObject {
    Q_OBJECT

public:
    static constexpr const char* SOCKET_NAME = "notepadplusplus_single_instance_ipc";

    /// Verifica si ya existe una instancia principal ejecutándose.
    /// Si existe, le envía los archivos a abrir y retorna true.
    static bool sendToExistingInstance(const QStringList& files) {
        QLocalSocket socket;
        socket.connectToServer(SOCKET_NAME);
        if (socket.waitForConnected(500)) {
            QString payload = files.join("\n");
            socket.write(payload.toUtf8());
            socket.waitForBytesWritten(500);
            socket.disconnectFromServer();
            return true;
        }
        return false;
    }

    explicit PlatformSingleInstance(QObject* parent = nullptr) : QObject(parent) {
        QLocalServer::removeServer(SOCKET_NAME);
        _server = new QLocalServer(this);
        if (_server->listen(SOCKET_NAME)) {
            connect(_server, &QLocalServer::newConnection, this, &PlatformSingleInstance::onNewConnection);
        }
    }

    ~PlatformSingleInstance() override {
        if (_server) {
            _server->close();
            QLocalServer::removeServer(SOCKET_NAME);
        }
    }

signals:
    void openFilesRequested(const QStringList& files);

private slots:
    void onNewConnection() {
        QLocalSocket* socket = _server->nextPendingConnection();
        if (!socket) return;
        connect(socket, &QLocalSocket::readyRead, [this, socket]() {
            QByteArray data = socket->readAll();
            QString payload = QString::fromUtf8(data);
            QStringList files = payload.split('\n', Qt::SkipEmptyParts);
            emit openFilesRequested(files);
            socket->deleteLater();
        });
    }

private:
    QLocalServer* _server = nullptr;
};

#endif // NPP_PLATFORM_LINUX

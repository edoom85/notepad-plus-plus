// Platform/PlatformFileMonitor.h — Monitor de cambios en archivos (Linux/Qt6)
// Reemplaza ReadDirectoryChangesW en Linux
//
// En Windows: ReadDirectoryChangesW + OVERLAPPED I/O + WaitForMultipleObjects
// En Linux:   QFileSystemWatcher (Qt6) — usa inotify internamente
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformString.h"
#include "PlatformMsg.h"

#ifdef NPP_PLATFORM_LINUX

#include <QFileSystemWatcher>
#include <QObject>
#include <QSet>
#include <QString>
#include <functional>

/// Monitor de cambios en archivos y directorios para Notepad++ Linux.
/// Equivalente portable a ReadDirectoryChangesW.
/// Usa QFileSystemWatcher que internamente usa inotify en Linux.
class NppFileMonitor : public QObject {
    Q_OBJECT

public:
    /// Tipo de cambio detectado.
    enum class ChangeType {
        Modified,   // El contenido del archivo cambió
        Deleted,    // El archivo fue eliminado
        Created     // Un nuevo archivo apareció en un directorio monitoreado
    };

    /// Callback cuando se detecta un cambio.
    using ChangeCallback = std::function<void(const NppString& path, ChangeType type)>;

    explicit NppFileMonitor(QObject* parent = nullptr)
        : QObject(parent)
        , _watcher(new QFileSystemWatcher(this))
    {
        connect(_watcher, &QFileSystemWatcher::fileChanged,
                this, &NppFileMonitor::onFileChanged);
        connect(_watcher, &QFileSystemWatcher::directoryChanged,
                this, &NppFileMonitor::onDirectoryChanged);
    }

    /// Monitorea un archivo específico.
    bool watchFile(const NppString& path) {
        QString qpath = QString::fromStdString(path);
        return _watcher->addPath(qpath);
    }

    /// Monitorea un directorio (detecta creación/eliminación de archivos).
    bool watchDirectory(const NppString& path) {
        QString qpath = QString::fromStdString(path);
        return _watcher->addPath(qpath);
    }

    /// Deja de monitorear un archivo o directorio.
    bool unwatch(const NppString& path) {
        QString qpath = QString::fromStdString(path);
        return _watcher->removePath(qpath);
    }

    /// Deja de monitorear todo.
    void unwatchAll() {
        if (!_watcher->files().isEmpty())
            _watcher->removePaths(_watcher->files());
        if (!_watcher->directories().isEmpty())
            _watcher->removePaths(_watcher->directories());
    }

    /// Establece el callback de cambio.
    void setCallback(ChangeCallback cb) { _callback = std::move(cb); }

    /// Archivos actualmente monitoreados.
    QStringList watchedFiles() const { return _watcher->files(); }

    /// Directorios actualmente monitoreados.
    QStringList watchedDirs() const { return _watcher->directories(); }

signals:
    /// Emitida cuando un archivo monitoreado cambia.
    void fileModified(const QString& path);

    /// Emitida cuando un archivo monitoreado es eliminado.
    void fileDeleted(const QString& path);

    /// Emitida cuando un directorio monitoreado cambia.
    void directoryChanged(const QString& path);

private slots:
    void onFileChanged(const QString& path) {
        NppString nppPath = path.toStdString();

        // QFileSystemWatcher deja de monitorear archivos eliminados
        // Verificar si el archivo aún existe
        if (QFile::exists(path)) {
            emit fileModified(path);
            if (_callback) _callback(nppPath, ChangeType::Modified);

            // Notificar al bus de mensajes
            NppMsgBus::instance().post(NppMsg::FILE_RELOAD,
                0, reinterpret_cast<NppLparam>(&nppPath));
        } else {
            emit fileDeleted(path);
            if (_callback) _callback(nppPath, ChangeType::Deleted);
        }
    }

    void onDirectoryChanged(const QString& path) {
        emit directoryChanged(path);
        NppString nppPath = path.toStdString();
        if (_callback) _callback(nppPath, ChangeType::Created);
    }

private:
    QFileSystemWatcher* _watcher = nullptr;
    ChangeCallback      _callback;
};

#endif // NPP_PLATFORM_LINUX

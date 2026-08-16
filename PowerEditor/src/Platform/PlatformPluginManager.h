// Platform/PlatformPluginManager.h — Gestor de Plugins nativos para Linux (.so)
// Reemplaza PowerEditor/src/PluginsManager.cpp en la versión Linux
//
// En Windows: Carga de DLLs via LoadLibrary, comunicación MSG via HWND
// En Linux:   Carga de .so via PlatformPlugin / dlopen, comunicación directa C++ / Qt
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformString.h"
#include "PlatformPlugin.h"

#ifdef NPP_PLATFORM_LINUX

#include <vector>
#include <memory>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

/// Gestor de carga, descubrimiento y ejecución de plugins de Notepad++ en Linux.
class NppPluginManager {
public:
    static NppPluginManager& instance() {
        static NppPluginManager s_instance;
        return s_instance;
    }

    /// Descubre y carga todos los plugins (.so) en las rutas estándar:
    ///   1. ~/.config/notepad-plus-plus/plugins/
    ///   2. /usr/lib/notepad-plus-plus/plugins/ (o la ruta de instalación)
    void loadPlugins(QWidget* mainWindow) {
        _plugins.clear();

        QStringList searchPaths = {
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/plugins",
            "/usr/lib/notepad-plus-plus/plugins",
            "/usr/local/lib/notepad-plus-plus/plugins"
        };

        NppData nppData;
        nppData._nppHandle = mainWindow;
        nppData._scintillaMainHandle = nullptr;
        nppData._scintillaSecondHandle = nullptr;

        for (const auto& path : searchPaths) {
            QDir dir(path);
            if (!dir.exists()) continue;

            QStringList files = dir.entryList({"*.so", "*.so.*"}, QDir::Files);
            for (const auto& fileName : files) {
                QString fullPath = dir.absoluteFilePath(fileName);
                auto loader = std::make_unique<NppPluginLoader>();

                if (loader->load(fullPath.toStdString())) {
                    loader->setInfo(nppData);
                    qDebug() << "✅ Plugin cargado exitosamente:" << loader->name().c_str() << "desde" << fullPath;
                    _plugins.push_back(std::move(loader));
                } else {
                    qWarning() << "⚠️ No se pudo cargar el plugin:" << fullPath;
                }
            }
        }
    }

    /// Descarga todos los plugins activos.
    void unloadPlugins() {
        for (auto& plugin : _plugins) {
            plugin->unload();
        }
        _plugins.clear();
    }

    /// Notifica a todos los plugins de una acción (Shortcut / Command).
    void notifyPlugins(int cmdId) {
        for (auto& plugin : _plugins) {
            // Invocar comando de plugin si coincide
            (void)cmdId;
        }
    }

    /// Cantidad de plugins cargados.
    size_t pluginCount() const { return _plugins.size(); }

    /// Obtiene un plugin por su índice.
    NppPluginLoader* getPlugin(size_t index) {
        if (index < _plugins.size()) return _plugins[index].get();
        return nullptr;
    }

private:
    NppPluginManager() = default;
    std::vector<std::unique_ptr<NppPluginLoader>> _plugins;
};

#endif // NPP_PLATFORM_LINUX

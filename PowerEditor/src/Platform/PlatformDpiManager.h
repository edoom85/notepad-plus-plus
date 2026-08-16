// Platform/PlatformDpiManager.h — Gestión de DPI Scaling Qt6 (Linux)
// Reemplaza PowerEditor/src/dpiManagerV2.cpp/.h en la versión Linux
//
// En Windows: dpiManagerV2 usa GetDpiForWindow / EnableNonClientDpiScaling
// En Linux:   Qt6 gestiona High-DPI de forma nativa mediante QGuiApplication
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#include <qglobal.h>

/// Gestor de escalado High-DPI para Notepad++ Linux.
/// En Qt6 el escalado por ppp (DPI) se maneja automáticamente por el framework.
class NppDpiManager {
public:
    /// Configura el escalado High-DPI al inicio de la aplicación.
    static void initDpiScaling() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6 habilita el escalado High DPI por defecto.
        // Habilitamos redondeo uniforme de escala para evitar distorsión de píxeles.
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    }

    /// Obtiene el factor de escala actual de la pantalla de un widget.
    static double scaleFactor(const QWidget* widget = nullptr) {
        if (widget && widget->screen()) {
            return widget->screen()->devicePixelRatio();
        }
        if (QGuiApplication::primaryScreen()) {
            return QGuiApplication::primaryScreen()->devicePixelRatio();
        }
        return 1.0;
    }

    /// Escala un valor de píxeles según el DPI actual.
    static int scale(int px, const QWidget* widget = nullptr) {
        return static_cast<int>(px * scaleFactor(widget));
    }
};

#endif // NPP_PLATFORM_LINUX

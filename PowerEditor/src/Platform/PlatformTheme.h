// Platform/PlatformTheme.h — Detección de tema oscuro/claro del sistema (Linux/Qt6)
// Reemplaza DarkMode/ (IatHook, UAHMenuBar, uxtheme) en Linux
//
// En Windows: DarkMode usa hooks internos de uxtheme.dll (no documentados)
// En Linux:   Se consulta la preferencia del sistema via:
//   - Portal XDG (org.freedesktop.appearance)
//   - QStyleHints::colorScheme() (Qt 6.5+)
//   - Fallback: heurística basada en luminosidad de QPalette
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QStyleHints>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariant>

/// Detección y gestión de temas dark/light en Notepad++ Linux.
/// Reemplaza el subsistema DarkMode/ completo de Win32
/// (IatHook.h, UAHMenuBar.h, DarkMode.h, uxtheme hacks).
class NppTheme {
public:
    enum class ColorScheme {
        Light,
        Dark,
        Unknown
    };

    /// Detecta si el sistema está en modo oscuro.
    /// Prioridad:
    ///   1. Portal XDG (Freedesktop) via D-Bus
    ///   2. Qt 6.5+ QStyleHints::colorScheme()
    ///   3. Heurística de luminosidad de QPalette::Window
    static ColorScheme detectSystemTheme() {
        // 1. Portal XDG via D-Bus (funciona en GNOME, KDE, etc.)
        ColorScheme xdgResult = queryXdgPortal();
        if (xdgResult != ColorScheme::Unknown)
            return xdgResult;

        // 2. Qt 6.5+ API directa
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        auto* hints = QApplication::styleHints();
        if (hints) {
            auto scheme = hints->colorScheme();
            if (scheme == Qt::ColorScheme::Dark)  return ColorScheme::Dark;
            if (scheme == Qt::ColorScheme::Light) return ColorScheme::Light;
        }
#endif

        // 3. Heurística de luminosidad
        return detectByLuminosity();
    }

    /// ¿El sistema está en modo oscuro?
    static bool isDarkMode() {
        return detectSystemTheme() == ColorScheme::Dark;
    }

    /// Aplica el tema oscuro VS Code a toda la aplicación Qt.
    static void applyDarkTheme(QApplication* app) {
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
    }

    /// Aplica el tema claro estándar.
    static void applyLightTheme(QApplication* app) {
        app->setStyle("Fusion");
        app->setPalette(app->style()->standardPalette());
    }

    /// Aplica el tema correcto según la preferencia del sistema.
    static void applySystemTheme(QApplication* app) {
        if (isDarkMode())
            applyDarkTheme(app);
        else
            applyLightTheme(app);
    }

private:
    /// Consulta la preferencia de color via XDG Portal D-Bus.
    /// org.freedesktop.appearance → color-scheme:
    ///   0 = no preference, 1 = dark, 2 = light
    static ColorScheme queryXdgPortal() {
        QDBusInterface iface(
            "org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.Settings",
            QDBusConnection::sessionBus());

        if (!iface.isValid()) return ColorScheme::Unknown;

        QDBusReply<QVariant> reply = iface.call(
            "ReadOne",
            "org.freedesktop.appearance",
            "color-scheme");

        if (!reply.isValid()) return ColorScheme::Unknown;

        uint value = reply.value().toUInt();
        switch (value) {
            case 1: return ColorScheme::Dark;
            case 2: return ColorScheme::Light;
            default: return ColorScheme::Unknown;
        }
    }

    /// Heurística: si el color de fondo de la ventana es oscuro, asumimos dark mode.
    static ColorScheme detectByLuminosity() {
        QColor windowColor = QApplication::palette().color(QPalette::Window);
        // Fórmula de luminosidad percibida (ITU-R BT.601)
        double luminosity = 0.299 * windowColor.redF()
                          + 0.587 * windowColor.greenF()
                          + 0.114 * windowColor.blueF();

        return (luminosity < 0.5) ? ColorScheme::Dark : ColorScheme::Light;
    }
};

#endif // NPP_PLATFORM_LINUX

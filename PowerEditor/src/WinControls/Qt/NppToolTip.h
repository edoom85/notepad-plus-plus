// WinControls/Qt/NppToolTip.h — ToolTip enriquecido portable Qt6
// Reemplaza WinControls/ToolTip/ToolTip.cpp/.h en Linux
//
// En Windows: HWND TOOLTIPS_CLASS Win32 + TTM_ADDTOOL
// En Linux:   QToolTip + QWidget::setToolTip con formateo HTML/QSS
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QToolTip>
#include <QWidget>
#include <QPoint>
#include <QString>
#include <QFont>

/// Tooltip enriquecido para Notepad++ Linux.
/// Reemplaza la clase ToolTip de Win32.
class NppToolTip {
public:
    /// Muestra un tooltip flotante en la posición global dada.
    static void showText(const QPoint& pos, const QString& text, QWidget* w = nullptr) {
        QToolTip::showText(pos, text, w);
    }

    /// Muestra un tooltip con formato HTML enriquecido (VS Code style).
    static void showRichToolTip(const QPoint& pos, const QString& title, const QString& body, QWidget* w = nullptr) {
        QString html = QString(
            "<div style='background-color: #252526; color: #CCCCCC; border: 1px solid #454545; "
            "padding: 6px; border-radius: 3px; font-family: sans-serif; font-size: 10pt;'>"
            "<b style='color: #569CD6;'>%1</b><br/>"
            "<span style='color: #D4D4D4;'>%2</span>"
            "</div>").arg(title).arg(body);
        QToolTip::showText(pos, html, w);
    }

    /// Oculta el tooltip actual.
    static void hide() {
        QToolTip::hideText();
    }
};

#endif // NPP_PLATFORM_LINUX

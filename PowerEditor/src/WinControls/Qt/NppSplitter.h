// WinControls/Qt/NppSplitter.h — Splitter portable Qt6
// Reemplaza WinControls/SplitterContainer/Splitter.cpp/.h en Linux
//
// En Windows: SplitterContainer usa controles custom Win32 con manejo de WM_MOUSEMOVE
// En Linux:   QSplitter hace todo esto nativamente
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QSplitter>
#include <QWidget>
#include <QList>

/// Splitter portable Qt6 para Notepad++ Linux.
/// Reemplaza SplitterContainer + Splitter de Win32.
/// Soporta split horizontal (side-by-side) y vertical (top-bottom).
class NppSplitter : public QSplitter {
    Q_OBJECT

public:
    explicit NppSplitter(Qt::Orientation orientation = Qt::Horizontal,
                         QWidget* parent = nullptr)
        : QSplitter(orientation, parent)
    {
        setHandleWidth(3);
        setChildrenCollapsible(false); // no permitir colapsar paneles a 0px

        // Estilo oscuro del handle de arrastre
        setStyleSheet(
            "QSplitter::handle {"
            "  background: #3C3C3C;"
            "}"
            "QSplitter::handle:hover {"
            "  background: #007ACC;"
            "}"
        );
    }

    /// Agrega un widget (editor) al splitter.
    void addPanel(QWidget* widget) {
        addWidget(widget);
    }

    /// Establece la proporción de tamaño entre paneles.
    /// ratio: 0.0-1.0, porcentaje que ocupa el primer panel.
    void setRatio(double ratio) {
        int total = (orientation() == Qt::Horizontal) ? width() : height();
        int first = static_cast<int>(total * ratio);
        setSizes({first, total - first});
    }

    /// Alterna entre split horizontal y vertical.
    void toggleOrientation() {
        setOrientation(
            orientation() == Qt::Horizontal
                ? Qt::Vertical
                : Qt::Horizontal);
    }

    /// ¿Está en modo split (tiene 2+ paneles visibles)?
    bool isSplit() const {
        int visible = 0;
        for (int i = 0; i < count(); ++i) {
            if (widget(i)->isVisible()) ++visible;
        }
        return visible >= 2;
    }
};

#endif // NPP_PLATFORM_LINUX

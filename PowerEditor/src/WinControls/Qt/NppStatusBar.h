// WinControls/Qt/NppStatusBar.h — StatusBar portable Qt6
// Reemplaza WinControls/StatusBar/StatusBar.cpp/.h en Linux
//
// En Windows: StatusBar usa controles nativos Win32 (STATUSCLASSNAME, SB_SETTEXT, etc.)
// En Linux:   Usamos QStatusBar con QLabel secciones (estilo Notepad++ con 5 partes)
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QStatusBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QString>
#include <array>

/// StatusBar portable Qt6 para Notepad++ Linux.
/// Mantiene la misma interfaz conceptual que StatusBar de Win32:
///   - Hasta 5 partes (secciones)
///   - setText(partIndex, text) para actualizar cada parte
///   - Indicadores de tipo de documento, encoding, posición, etc.
class NppStatusBar : public QStatusBar {
    Q_OBJECT

public:
    static constexpr int MAX_PARTS = 5;

    explicit NppStatusBar(QWidget* parent = nullptr)
        : QStatusBar(parent)
    {
        setSizeGripEnabled(true);

        // Crear las 5 secciones como QLabel
        for (int i = 0; i < MAX_PARTS; ++i) {
            _parts[i] = new QLabel(this);
            _parts[i]->setFrameShape(QFrame::NoFrame);
            _parts[i]->setStyleSheet(
                "QLabel { padding: 0 8px; color: #D4D4D4; }");
            if (i == 0) {
                // La primera parte se expande (como en Npp original)
                addWidget(_parts[i], 1);
            } else {
                addPermanentWidget(_parts[i], 0);
            }
        }

        // Estilo oscuro por defecto
        setStyleSheet(
            "QStatusBar { background: #007ACC; color: #FFFFFF; }"
            "QStatusBar::item { border: none; }"
        );
    }

    /// Establece el texto usando const char* directamente.
    void setText(int partIndex, const char* text) {
        if (partIndex < 0 || partIndex >= MAX_PARTS) return;
        _parts[partIndex]->setText(QString::fromUtf8(text));
    }

    /// Establece el texto del índice indicado (0-based).
    void setText(int partIndex, const NppString& text) {
        if (partIndex < 0 || partIndex >= MAX_PARTS) return;
        _parts[partIndex]->setText(QString::fromStdString(text));
    }

    /// Establece el texto usando QString directamente.
    void setText(int partIndex, const QString& text) {
        if (partIndex < 0 || partIndex >= MAX_PARTS) return;
        _parts[partIndex]->setText(text);
    }

    /// Obtiene el QStatusBar subyacente.
    QStatusBar* statusBar() { return this; }

    /// Muestra un mensaje temporal en la barra (desaparece después de ms).
    void showTemporaryMessage(const QString& msg, int timeoutMs = 3000) {
        showMessage(msg, timeoutMs);
    }

private:
    std::array<QLabel*, MAX_PARTS>     _parts = {};
};


#endif // NPP_PLATFORM_LINUX

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
class NppStatusBar : public QWidget {
    Q_OBJECT

public:
    static constexpr int MAX_PARTS = 5;

    explicit NppStatusBar(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        _statusBar = new QStatusBar(this);
        _statusBar->setSizeGripEnabled(true);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(_statusBar);

        // Crear las 5 secciones como QLabel
        for (int i = 0; i < MAX_PARTS; ++i) {
            _parts[i] = new QLabel(this);
            _parts[i]->setFrameShape(QFrame::NoFrame);
            _parts[i]->setStyleSheet(
                "QLabel { padding: 0 8px; color: #D4D4D4; }");
            if (i == 0) {
                // La primera parte se expande (como en Npp original)
                _statusBar->addWidget(_parts[i], 1);
            } else {
                _statusBar->addPermanentWidget(_parts[i], 0);
            }
        }

        // Estilo oscuro por defecto
        _statusBar->setStyleSheet(
            "QStatusBar { background: #007ACC; color: #FFFFFF; }"
            "QStatusBar::item { border: none; }"
        );
    }

    /// Establece el texto de una parte (0-based, como Win32 SB_SETTEXT).
    void setText(int partIndex, const NppString& text) {
        if (partIndex < 0 || partIndex >= MAX_PARTS) return;
        _parts[partIndex]->setText(QString::fromStdString(text));
    }

    /// Establece el texto usando QString directamente.
    void setText(int partIndex, const QString& text) {
        if (partIndex < 0 || partIndex >= MAX_PARTS) return;
        _parts[partIndex]->setText(text);
    }

    /// Obtiene el QStatusBar subyacente (para agregar widgets personalizados).
    QStatusBar* statusBar() const { return _statusBar; }

    /// Muestra un mensaje temporal en la barra (desaparece después de ms).
    void showTemporaryMessage(const QString& msg, int timeoutMs = 3000) {
        _statusBar->showMessage(msg, timeoutMs);
    }

private:
    QStatusBar*                        _statusBar = nullptr;
    std::array<QLabel*, MAX_PARTS>     _parts = {};
};

#endif // NPP_PLATFORM_LINUX

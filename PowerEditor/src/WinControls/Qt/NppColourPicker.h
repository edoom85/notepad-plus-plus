// WinControls/Qt/NppColourPicker.h — Selector de color portable Qt6
// Reemplaza WinControls/ColourPicker/ColourPicker.cpp/.h en Linux
//
// En Windows: CreateWindowEx + WM_PAINT custom + ChooseColor (comdlg32)
// En Linux:   QPushButton con color de fondo + QColorDialog
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QColor>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>

/// Selector de color portable Qt6 para Notepad++ Linux.
/// Muestra una muestra del color actual y abre QColorDialog al hacer clic.
class NppColourPicker : public QWidget {
    Q_OBJECT

public:
    explicit NppColourPicker(QWidget* parent = nullptr, const QColor& initial = Qt::white)
        : QWidget(parent)
        , _color(initial)
    {
        setFixedSize(28, 28);
        setCursor(Qt::PointingHandCursor);
        setToolTip("Clic para elegir color");
    }

    /// Obtiene el color actual.
    QColor color() const { return _color; }

    /// Establece el color programáticamente.
    void setColor(const QColor& c) {
        if (_color != c) {
            _color = c;
            update();
            emit colorChanged(_color);
        }
    }

signals:
    /// Emitida cuando el usuario elige un nuevo color.
    void colorChanged(const QColor& newColor);

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Borde
        p.setPen(QPen(QColor(0x55, 0x55, 0x55), 1));
        p.setBrush(_color);
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 3, 3);

        // Indicador de selección (pequeño triángulo en esquina)
        if (hasFocus()) {
            p.setPen(QPen(QColor(0x00, 0x7A, 0xCC), 2));
            p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 3, 3);
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            QColor chosen = QColorDialog::getColor(_color, this, "Seleccionar color",
                QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
            if (chosen.isValid()) {
                setColor(chosen);
            }
        }
        QWidget::mousePressEvent(event);
    }

private:
    QColor _color;
};

#endif // NPP_PLATFORM_LINUX

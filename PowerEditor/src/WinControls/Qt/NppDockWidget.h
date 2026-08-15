// WinControls/Qt/NppDockWidget.h — DockWidget portable Qt6
// Reemplaza WinControls/DockingWnd/ (DockingCont, DockingManager, DockingSplitter, Gripper) en Linux
//
// En Windows: el sistema de docking es completamente custom con Win32
//             (MoveWindow, SetWindowPos, track mouse, etc.)
// En Linux:   QDockWidget con QMainWindow::addDockWidget()
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QDockWidget>
#include <QWidget>
#include <QMainWindow>
#include <QString>
#include <QAction>

/// Panel dockable portable Qt6 para Notepad++ Linux.
/// Reemplaza el sistema DockingWnd completo (DockingCont, DockingManager,
/// DockingSplitter, Gripper) con un solo QDockWidget.
///
/// Los paneles de Notepad++ que usan docking:
///   - FunctionList
///   - FileBrowser  
///   - ProjectPanel
///   - DocumentMap
///   - ClipboardHistory
///   - AnsiCharPanel
///
/// Cada uno crea un NppDockWidget y lo registra con el QMainWindow.
class NppDockWidget : public QDockWidget {
    Q_OBJECT

public:
    /// Posición de docking (equivalente a DockingDlgInterface::_dockingData::_iContType)
    enum class DockPosition {
        Left   = 0,   // Qt::LeftDockWidgetArea
        Right  = 1,   // Qt::RightDockWidgetArea
        Top    = 2,   // Qt::TopDockWidgetArea
        Bottom = 3    // Qt::BottomDockWidgetArea
    };

    explicit NppDockWidget(const QString& title, QWidget* parent = nullptr,
                           DockPosition pos = DockPosition::Right)
        : QDockWidget(title, parent)
        , _defaultPos(pos)
    {
        setAllowedAreas(Qt::AllDockWidgetAreas);
        setFeatures(QDockWidget::DockWidgetClosable |
                    QDockWidget::DockWidgetMovable |
                    QDockWidget::DockWidgetFloatable);

        // Estilo oscuro
        setStyleSheet(
            "QDockWidget {"
            "  color: #D4D4D4;"
            "  titlebar-close-icon: none;"
            "}"
            "QDockWidget::title {"
            "  background: #252526;"
            "  padding: 6px;"
            "  border-bottom: 1px solid #3C3C3C;"
            "  text-align: left;"
            "}"
            "QDockWidget::close-button, QDockWidget::float-button {"
            "  background: transparent;"
            "  border: none;"
            "  padding: 2px;"
            "}"
            "QDockWidget::close-button:hover, QDockWidget::float-button:hover {"
            "  background: #505050;"
            "}"
        );
    }

    /// Registra este dock widget con un QMainWindow.
    void registerWith(QMainWindow* mainWindow) {
        Qt::DockWidgetArea area;
        switch (_defaultPos) {
            case DockPosition::Left:   area = Qt::LeftDockWidgetArea;   break;
            case DockPosition::Right:  area = Qt::RightDockWidgetArea;  break;
            case DockPosition::Top:    area = Qt::TopDockWidgetArea;    break;
            case DockPosition::Bottom: area = Qt::BottomDockWidgetArea; break;
        }
        mainWindow->addDockWidget(area, this);

        // Añadir toggle action al menú View (si existe)
        QAction* toggle = toggleViewAction();
        toggle->setText(windowTitle());
    }

    /// Muestra u oculta el panel.
    void display(bool toShow = true) {
        setVisible(toShow);
        if (toShow) raise();
    }

    /// Establece el widget central del panel dock.
    void setContent(QWidget* widget) {
        setWidget(widget);
    }

    /// Obtiene la posición actual de docking.
    DockPosition dockPosition() const { return _defaultPos; }

private:
    DockPosition _defaultPos;
};

#endif // NPP_PLATFORM_LINUX

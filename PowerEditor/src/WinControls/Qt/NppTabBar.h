// WinControls/Qt/NppTabBar.h — TabBar portable Qt6
// Reemplaza WinControls/TabBar/TabBar.cpp/.h en Linux
//
// En Windows: TabBar usa controles nativos Win32 (TCM_INSERTITEM, TCN_SELCHANGE, etc.)
// En Linux:   Usamos QTabWidget / QTabBar con soporte para:
//   - Pestañas arrastrables (drag & drop reorder)
//   - Botón de cerrar en cada pestaña
//   - Color de pestaña para indicar estado (modificado, read-only)
//   - Context menu en las pestañas
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "../Platform/PlatformMsg.h"

#ifdef NPP_PLATFORM_LINUX

#include <QTabWidget>
#include <QTabBar>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QString>
#include <QColor>
#include <QIcon>
#include <functional>

/// TabBar personalizado con drag-to-reorder y context menu.
class NppTabBar : public QTabBar {
    Q_OBJECT

public:
    explicit NppTabBar(QWidget* parent = nullptr)
        : QTabBar(parent)
    {
        setMovable(true);             // drag & drop para reordenar
        setTabsClosable(true);        // botón X en cada pestaña
        setExpanding(false);           // no expandir pestañas al ancho
        setUsesScrollButtons(true);    // scroll cuando hay muchas pestañas
        setElideMode(Qt::ElideRight);  // truncar nombres largos con "..."
        setDocumentMode(true);         // estilo integrado (sin bordes)

        // Estilo oscuro
        setStyleSheet(
            "QTabBar::tab {"
            "  background: #2D2D2D;"
            "  color: #969696;"
            "  padding: 6px 12px;"
            "  border: none;"
            "  border-bottom: 2px solid transparent;"
            "  min-width: 80px;"
            "  max-width: 200px;"
            "}"
            "QTabBar::tab:selected {"
            "  background: #1E1E1E;"
            "  color: #FFFFFF;"
            "  border-bottom: 2px solid #007ACC;"
            "}"
            "QTabBar::tab:hover:!selected {"
            "  background: #383838;"
            "  color: #D4D4D4;"
            "}"
            "QTabBar::close-button {"
            "  image: none;"
            "  subcontrol-position: right;"
            "}"
        );
    }

signals:
    /// Emitida cuando se solicita cerrar una pestaña.
    void tabCloseRequested(int index);

    /// Emitida cuando se solicita el context menu en una pestaña.
    void tabContextMenuRequested(int index, const QPoint& globalPos);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::MiddleButton) {
            int idx = tabAt(event->pos());
            if (idx >= 0) {
                emit tabCloseRequested(idx);
                return;
            }
        }
        QTabBar::mousePressEvent(event);
    }

    void contextMenuEvent(QContextMenuEvent* event) override {
        int idx = tabAt(event->pos());
        if (idx >= 0) {
            emit tabContextMenuRequested(idx, event->globalPos());
        }
    }
};

/// Contenedor de pestañas que aloja widgets (editores Scintilla).
/// Equivalente a la combinación TabBar + ControlsTab de Win32 Notepad++.
class NppTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit NppTabWidget(QWidget* parent = nullptr)
        : QTabWidget(parent)
    {
        // Usar nuestro TabBar personalizado
        auto* tabBar = new NppTabBar(this);
        setTabBar(tabBar);

        // Redirigir señales del TabBar
        connect(tabBar, &NppTabBar::tabCloseRequested,
                this, &NppTabWidget::onTabCloseRequest);
        connect(tabBar, &NppTabBar::tabContextMenuRequested,
                this, &NppTabWidget::onTabContextMenu);

        // Sin bordes
        setDocumentMode(true);
        setStyleSheet("QTabWidget::pane { border: none; }");
    }

    /// Añadir una pestaña con nombre y widget editor.
    int addTab(QWidget* editor, const NppString& name) {
        return QTabWidget::addTab(editor, QString::fromStdString(name));
    }

    /// Marcar pestaña como modificada (agrega * al nombre).
    void setTabModified(int index, bool modified) {
        QString name = tabText(index);
        if (modified && !name.startsWith("*")) {
            setTabText(index, "*" + name);
        } else if (!modified && name.startsWith("*")) {
            setTabText(index, name.mid(1));
        }
    }

    /// Obtener ruta de archivo asociada a una pestaña (almacenada como data).
    NppString tabFilePath(int index) const {
        return tabBar()->tabData(index).toString().toStdString();
    }

    /// Asignar ruta de archivo a una pestaña.
    void setTabFilePath(int index, const NppString& path) {
        tabBar()->setTabData(index, QString::fromStdString(path));
    }

signals:
    /// Emitida cuando el usuario quiere cerrar una pestaña.
    void closeTabRequested(int index);

    /// Emitida para context menu de pestaña.
    void contextMenuOnTab(int index, const QPoint& globalPos);

private slots:
    void onTabCloseRequest(int index) {
        emit closeTabRequested(index);
        // Notificar al bus de mensajes
        NppMsgBus::instance().post(NppMsg::FILE_CLOSE,
            static_cast<NppWparam>(index), 0);
    }

    void onTabContextMenu(int index, const QPoint& pos) {
        emit contextMenuOnTab(index, pos);
    }
};

#endif // NPP_PLATFORM_LINUX

// WinControls/Qt/NppToolBar.h — ToolBar portable Qt6
// Reemplaza WinControls/ToolBar/ToolBar.cpp/.h en Linux
//
// En Windows: ToolBar usa TBSTYLE_FLAT + TB_ADDBUTTONS (controles comunes)
// En Linux:   QToolBar con QAction y soporte para iconos SVG/PNG
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QWidget>
#include <QSize>
#include <QString>
#include <QStyle>
#include <vector>
#include <functional>

/// Barra de herramientas portable Qt6 para Notepad++ Linux.
/// Los botones se crean con addButton() y cada uno tiene un ID de comando
/// que corresponde al mismo command ID que en la versión Windows.
class NppToolBar : public QToolBar {
    Q_OBJECT

public:
    explicit NppToolBar(const QString& title = "Main Toolbar", QWidget* parent = nullptr)
        : QToolBar(title, parent)
    {
        setMovable(false);
        setIconSize(QSize(16, 16));
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setFloatable(false);

        // Estilo oscuro
        setStyleSheet(
            "QToolBar {"
            "  background: #333333;"
            "  border: none;"
            "  spacing: 2px;"
            "  padding: 2px;"
            "}"
            "QToolButton {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "  padding: 4px;"
            "}"
            "QToolButton:hover {"
            "  background: #505050;"
            "}"
            "QToolButton:pressed {"
            "  background: #404040;"
            "}"
        );
    }

    /// Estructura para definir un botón de toolbar.
    struct ButtonDef {
        int         cmdId;       // ID de comando (debe coincidir con Win32 IDM_*)
        QString     tooltip;     // Texto del tooltip
        QIcon       icon;        // Icono del botón
        bool        isSeparator = false;
    };

    /// Añade un botón a la toolbar. Retorna la QAction creada.
    QAction* addButton(const ButtonDef& def) {
        if (def.isSeparator) {
            addSeparator();
            return nullptr;
        }
        QAction* action = addAction(def.icon, def.tooltip);
        action->setData(def.cmdId);
        action->setToolTip(def.tooltip);

        // Conectar a la señal commandTriggered con el cmdId
        connect(action, &QAction::triggered, [this, cmdId = def.cmdId]() {
            emit commandTriggered(cmdId);
        });

        _actions.push_back(action);
        return action;
    }

    /// Añade un separador.
    void addToolBarSeparator() { addSeparator(); }

    /// Habilita/deshabilita un botón por su command ID.
    void enableButton(int cmdId, bool enable) {
        for (auto* a : _actions) {
            if (a->data().toInt() == cmdId) {
                a->setEnabled(enable);
                return;
            }
        }
    }

    /// Marca/desmarca un botón como "checked" (toggle) por su command ID.
    void checkButton(int cmdId, bool checked) {
        for (auto* a : _actions) {
            if (a->data().toInt() == cmdId) {
                a->setCheckable(true);
                a->setChecked(checked);
                return;
            }
        }
    }

    /// Cambia el tamaño de iconos (16, 24, 32).
    void setIconSizePreset(int pixels) {
        setIconSize(QSize(pixels, pixels));
    }

    /// Crea botones estándar de Notepad++ con iconos del theme.
    /// TODO: Llamar esto con iconos reales desde el recurso de imágenes
    void addStandardButtons() {
        QStyle* style = this->style();

        addButton({1, "Nuevo",   style->standardIcon(QStyle::SP_FileIcon)});
        addButton({2, "Abrir",   style->standardIcon(QStyle::SP_DialogOpenButton)});
        addButton({3, "Guardar", style->standardIcon(QStyle::SP_DialogSaveButton)});
        addToolBarSeparator();
        addButton({4, "Deshacer",style->standardIcon(QStyle::SP_ArrowBack)});
        addButton({5, "Rehacer", style->standardIcon(QStyle::SP_ArrowForward)});
        addToolBarSeparator();
        addButton({6, "Buscar",  style->standardIcon(QStyle::SP_FileDialogContentsView)});
    }

signals:
    /// Emitida cuando se hace clic en un botón (con su command ID).
    void commandTriggered(int cmdId);

private:
    std::vector<QAction*> _actions;
};

#endif // NPP_PLATFORM_LINUX

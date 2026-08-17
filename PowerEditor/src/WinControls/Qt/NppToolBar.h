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
#include "../../Platform/PlatformIconProvider.h"


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

    /// Refresca dinámicamente los iconos de todos los botones cargando el estilo .ico activo.
    void refreshIcons() {
        for (auto* action : _actions) {
            int cmdId = action->data().toInt();
            NppIconProvider::IconType type;
            switch (cmdId) {
                case 1:  type = NppIconProvider::IconType::New; break;
                case 2:  type = NppIconProvider::IconType::Open; break;
                case 3:  type = NppIconProvider::IconType::Save; break;
                case 4:  type = NppIconProvider::IconType::SaveAll; break;
                case 5:  type = NppIconProvider::IconType::Close; break;
                case 6:  type = NppIconProvider::IconType::Undo; break;
                case 7:  type = NppIconProvider::IconType::Redo; break;
                case 8:  type = NppIconProvider::IconType::Cut; break;
                case 9:  type = NppIconProvider::IconType::Copy; break;
                case 10: type = NppIconProvider::IconType::Paste; break;
                case 11: type = NppIconProvider::IconType::Find; break;
                case 12: type = NppIconProvider::IconType::Replace; break;
                case 13: type = NppIconProvider::IconType::ZoomIn; break;
                case 14: type = NppIconProvider::IconType::ZoomOut; break;
                case 15: type = NppIconProvider::IconType::FileBrowser; break;
                case 16: type = NppIconProvider::IconType::FunctionList; break;
                case 17: type = NppIconProvider::IconType::ProjectPanel; break;
                case 18: type = NppIconProvider::IconType::ClipboardHistory; break;
                case 19: type = NppIconProvider::IconType::Plugins; break;
                case 20: type = NppIconProvider::IconType::Settings; break;
                case 21: type = NppIconProvider::IconType::About; break;
                default: continue;
            }
            action->setIcon(NppIconProvider::get(type));
        }
    }

    /// Cambia el estilo y tamaño de iconos de la barra de herramientas.
    void setIconSizePreset(int presetIndex, bool isDarkMode = true) {
        int pixels = (presetIndex == 1 || presetIndex == 3) ? 32 : 16;
        setIconSize(QSize(pixels, pixels));
        NppIconProvider::setToolbarStyle(presetIndex, isDarkMode);
        refreshIcons();
    }


    /// Crea la barra de herramientas completa de Notepad++ con iconos garantizados.
    void addStandardButtons() {
        addButton({1,  "Nuevo documento",                  NppIconProvider::get(NppIconProvider::IconType::New)});
        addButton({2,  "Abrir archivo...",                 NppIconProvider::get(NppIconProvider::IconType::Open)});
        addButton({3,  "Guardar",                          NppIconProvider::get(NppIconProvider::IconType::Save)});
        addButton({4,  "Guardar todo",                     NppIconProvider::get(NppIconProvider::IconType::SaveAll)});
        addButton({5,  "Cerrar documento actual",          NppIconProvider::get(NppIconProvider::IconType::Close)});
        addToolBarSeparator();
        addButton({6,  "Deshacer",                         NppIconProvider::get(NppIconProvider::IconType::Undo)});
        addButton({7,  "Rehacer",                          NppIconProvider::get(NppIconProvider::IconType::Redo)});
        addToolBarSeparator();
        addButton({8,  "Cortar",                           NppIconProvider::get(NppIconProvider::IconType::Cut)});
        addButton({9,  "Copiar",                           NppIconProvider::get(NppIconProvider::IconType::Copy)});
        addButton({10, "Pegar",                            NppIconProvider::get(NppIconProvider::IconType::Paste)});
        addToolBarSeparator();
        addButton({11, "Buscar...",                        NppIconProvider::get(NppIconProvider::IconType::Find)});
        addButton({12, "Reemplazar...",                    NppIconProvider::get(NppIconProvider::IconType::Replace)});
        addToolBarSeparator();
        addButton({13, "Acercar Zoom",                     NppIconProvider::get(NppIconProvider::IconType::ZoomIn)});
        addButton({14, "Alejar Zoom",                      NppIconProvider::get(NppIconProvider::IconType::ZoomOut)});
        addToolBarSeparator();
        addButton({15, "Explorador de Archivos",           NppIconProvider::get(NppIconProvider::IconType::FileBrowser)});
        addButton({16, "Lista de Funciones",               NppIconProvider::get(NppIconProvider::IconType::FunctionList)});
        addButton({17, "Panel de Proyectos",               NppIconProvider::get(NppIconProvider::IconType::ProjectPanel)});
        addButton({18, "Historial del Portapapeles",       NppIconProvider::get(NppIconProvider::IconType::ClipboardHistory)});
        addButton({19, "Administrador de Plugins",         NppIconProvider::get(NppIconProvider::IconType::Plugins)});
        addToolBarSeparator();
        addButton({20, "Preferencias...",                  NppIconProvider::get(NppIconProvider::IconType::Settings)});
        addButton({21, "Acerca de Notepad++",              NppIconProvider::get(NppIconProvider::IconType::About)});
    }


signals:
    /// Emitida cuando se hace clic en un botón (con su command ID).
    void commandTriggered(int cmdId);

private:
    std::vector<QAction*> _actions;
};

#endif // NPP_PLATFORM_LINUX

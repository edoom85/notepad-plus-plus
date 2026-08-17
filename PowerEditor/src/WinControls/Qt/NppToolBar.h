// WinControls/Qt/NppToolBar.h — ToolBar portable Qt6 con paridad 1:1 de botones de Windows
// Reemplaza WinControls/ToolBar/ToolBar.cpp/.h en Linux
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

/// Barra de herramientas portable Qt6 para Notepad++ Linux con paridad 1:1 de botones.
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

    struct ButtonDef {
        int         cmdId;       
        QString     tooltip;     
        QIcon       icon;        
        bool        isSeparator = false;
    };

    QAction* addButton(const ButtonDef& def) {
        if (def.isSeparator) {
            addSeparator();
            return nullptr;
        }
        QAction* action = addAction(def.icon, def.tooltip);
        action->setData(def.cmdId);
        action->setToolTip(def.tooltip);

        connect(action, &QAction::triggered, [this, cmdId = def.cmdId]() {
            emit commandTriggered(cmdId);
        });

        _actions.push_back(action);
        return action;
    }

    void addToolBarSeparator() { addSeparator(); }

    void enableButton(int cmdId, bool enable) {
        for (auto* a : _actions) {
            if (a->data().toInt() == cmdId) {
                a->setEnabled(enable);
                return;
            }
        }
    }

    void checkButton(int cmdId, bool checked) {
        for (auto* a : _actions) {
            if (a->data().toInt() == cmdId) {
                a->setCheckable(true);
                a->setChecked(checked);
                return;
            }
        }
    }

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
                case 30: type = NppIconProvider::IconType::CloseAll; break;
                case 31: type = NppIconProvider::IconType::Print; break;

                case 8:  type = NppIconProvider::IconType::Cut; break;
                case 9:  type = NppIconProvider::IconType::Copy; break;
                case 10: type = NppIconProvider::IconType::Paste; break;

                case 6:  type = NppIconProvider::IconType::Undo; break;
                case 7:  type = NppIconProvider::IconType::Redo; break;

                case 11: type = NppIconProvider::IconType::Find; break;
                case 12: type = NppIconProvider::IconType::Replace; break;

                case 13: type = NppIconProvider::IconType::ZoomIn; break;
                case 14: type = NppIconProvider::IconType::ZoomOut; break;

                case 32: type = NppIconProvider::IconType::SyncV; break;
                case 33: type = NppIconProvider::IconType::SyncH; break;

                case 34: type = NppIconProvider::IconType::UDL; break;
                case 35: type = NppIconProvider::IconType::DocMap; break;
                case 36: type = NppIconProvider::IconType::DocList; break;
                case 16: type = NppIconProvider::IconType::FunctionList; break;
                case 15: type = NppIconProvider::IconType::FileBrowser; break;
                case 37: type = NppIconProvider::IconType::Monitoring; break;

                case 38: type = NppIconProvider::IconType::AllChars; break;
                case 39: type = NppIconProvider::IconType::IndentGuide; break;
                case 40: type = NppIconProvider::IconType::Wrap; break;

                case 41: type = NppIconProvider::IconType::StartRecord; break;
                case 42: type = NppIconProvider::IconType::StopRecord; break;
                case 43: type = NppIconProvider::IconType::PlayRecord; break;
                case 44: type = NppIconProvider::IconType::SaveRecord; break;
                case 45: type = NppIconProvider::IconType::PlayRecordM; break;
                default: continue;
            }
            action->setIcon(NppIconProvider::get(type));
        }
    }

    void setIconSizePreset(int presetIndex, bool isDarkMode = true) {
        int pixels = (presetIndex == 1 || presetIndex == 3) ? 32 : 16;
        setIconSize(QSize(pixels, pixels));
        NppIconProvider::setToolbarStyle(presetIndex, isDarkMode);
        refreshIcons();
    }

    /// Crea la barra de herramientas completa 1:1 de Notepad++ Windows.
    void addStandardButtons() {
        addButton({1,  "Nuevo documento",                  NppIconProvider::get(NppIconProvider::IconType::New)});
        addButton({2,  "Abrir archivo...",                 NppIconProvider::get(NppIconProvider::IconType::Open)});
        addButton({3,  "Guardar",                          NppIconProvider::get(NppIconProvider::IconType::Save)});
        addButton({4,  "Guardar todo",                     NppIconProvider::get(NppIconProvider::IconType::SaveAll)});
        addButton({5,  "Cerrar documento actual",          NppIconProvider::get(NppIconProvider::IconType::Close)});
        addButton({30, "Cerrar todo",                      NppIconProvider::get(NppIconProvider::IconType::CloseAll)});
        addButton({31, "Imprimir...",                      NppIconProvider::get(NppIconProvider::IconType::Print)});
        addToolBarSeparator();

        addButton({8,  "Cortar",                           NppIconProvider::get(NppIconProvider::IconType::Cut)});
        addButton({9,  "Copiar",                           NppIconProvider::get(NppIconProvider::IconType::Copy)});
        addButton({10, "Pegar",                            NppIconProvider::get(NppIconProvider::IconType::Paste)});
        addToolBarSeparator();

        addButton({6,  "Deshacer",                         NppIconProvider::get(NppIconProvider::IconType::Undo)});
        addButton({7,  "Rehacer",                          NppIconProvider::get(NppIconProvider::IconType::Redo)});
        addToolBarSeparator();

        addButton({11, "Buscar...",                        NppIconProvider::get(NppIconProvider::IconType::Find)});
        addButton({12, "Reemplazar...",                    NppIconProvider::get(NppIconProvider::IconType::Replace)});
        addToolBarSeparator();

        addButton({13, "Acercar Zoom",                     NppIconProvider::get(NppIconProvider::IconType::ZoomIn)});
        addButton({14, "Alejar Zoom",                      NppIconProvider::get(NppIconProvider::IconType::ZoomOut)});
        addToolBarSeparator();

        addButton({32, "Sincronización Vertical",           NppIconProvider::get(NppIconProvider::IconType::SyncV)});
        addButton({33, "Sincronización Horizontal",         NppIconProvider::get(NppIconProvider::IconType::SyncH)});
        addToolBarSeparator();

        addButton({34, "Lenguaje del usuario (UDL)",       NppIconProvider::get(NppIconProvider::IconType::UDL)});
        addButton({35, "Mapa del Documento",              NppIconProvider::get(NppIconProvider::IconType::DocMap)});
        addButton({36, "Lista de Documentos",              NppIconProvider::get(NppIconProvider::IconType::DocList)});
        addButton({16, "Lista de Funciones",               NppIconProvider::get(NppIconProvider::IconType::FunctionList)});
        addButton({15, "Explorador de Archivos",           NppIconProvider::get(NppIconProvider::IconType::FileBrowser)});
        addButton({37, "Monitorización (tail -f)",         NppIconProvider::get(NppIconProvider::IconType::Monitoring)});
        addToolBarSeparator();

        addButton({38, "Mostrar todos los caracteres",      NppIconProvider::get(NppIconProvider::IconType::AllChars)});
        addButton({39, "Guías de sangría",                 NppIconProvider::get(NppIconProvider::IconType::IndentGuide)});
        addButton({40, "Ajuste de línea (Word Wrap)",      NppIconProvider::get(NppIconProvider::IconType::Wrap)});
        addToolBarSeparator();

        addButton({41, "Iniciar grabación de macro",      NppIconProvider::get(NppIconProvider::IconType::StartRecord)});
        addButton({42, "Detener grabación de macro",      NppIconProvider::get(NppIconProvider::IconType::StopRecord)});
        addButton({43, "Reproducir macro",                 NppIconProvider::get(NppIconProvider::IconType::PlayRecord)});
        addButton({44, "Guardar macro grabada",           NppIconProvider::get(NppIconProvider::IconType::SaveRecord)});
        addButton({45, "Ejecutar macro N veces...",        NppIconProvider::get(NppIconProvider::IconType::PlayRecordM)});
    }

signals:
    void commandTriggered(int cmdId);

private:
    std::vector<QAction*> _actions;
};

#endif // NPP_PLATFORM_LINUX

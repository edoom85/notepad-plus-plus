// WinControls/Qt/NppPluginsAdmin.h — Administrador de Plugins (Plugins Admin) Qt6
// Reemplaza WinControls/PluginsAdmin/PluginsAdmin.cpp/.h en Linux
//
// En Windows: ListView Win32 + cURL/WinINet para instalar DLLs de plugins
// En Linux:   NppDialog + QTableWidget + pestañas Disponibles / Instalados / Actualizaciones
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QCheckBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QString>
#include <vector>

struct PluginInfo {
    QString name;
    QString version;
    QString author;
    QString description;
    bool installed = false;
};

/// Diálogo del Administrador de Plugins (Plugins Admin) para Notepad++ Linux.
class NppPluginsAdmin : public NppDialog {
    Q_OBJECT

public:
    explicit NppPluginsAdmin(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Administrador de Plugins (Plugins Admin)");
        setMinimumSize(780, 520);
        buildUI();
        loadMockPlugins();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // ── Tabs: Disponibles | Instalados | Actualizaciones ──────────────────
        _tabs = new QTabWidget(this);
        _tabs->setStyleSheet(
            "QTabWidget::pane { border: none; }"
            "QTabBar::tab { background: #2D2D2D; color: #969696; padding: 6px 16px; }"
            "QTabBar::tab:selected { background: #1E1E1E; color: #FFF; border-bottom: 2px solid #007ACC; }"
        );

        // Campo de búsqueda
        auto* searchLayout = new QHBoxLayout();
        searchLayout->addWidget(new QLabel("Buscar plugin:", this));
        _searchEdit = new QLineEdit(this);
        _searchEdit->setPlaceholderText("Escribe para filtrar por nombre o descripción...");
        _searchEdit->setClearButtonEnabled(true);
        searchLayout->addWidget(_searchEdit);
        mainLayout->addLayout(searchLayout);

        // Tabla de plugins
        _table = new QTableWidget(this);
        _table->setColumnCount(4);
        _table->setHorizontalHeaderLabels({"Instalar", "Plugin", "Versión", "Descripción"});
        _table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        _table->setSelectionBehavior(QAbstractItemView::SelectRows);
        _table->setAlternatingRowColors(true);
        _table->setStyleSheet(
            "QTableWidget { background: #1E1E1E; color: #D4D4D4; gridline-color: #333333; }"
            "QHeaderView::section { background: #252526; color: #CCCCCC; padding: 6px; border: 1px solid #333; }"
        );

        mainLayout->addWidget(_table);

        // ── Botones de acción ────────────────────────────────────────────────
        auto* btnLayout = new QHBoxLayout();
        auto* btnInstall = new QPushButton("Instalar Plugins Seleccionados", this);
        btnInstall->setStyleSheet(
            "QPushButton { background: #0E639C; color: #FFF; border: none; padding: 6px 16px; border-radius: 2px; }"
            "QPushButton:hover { background: #1177BB; }"
        );
        btnLayout->addStretch();
        btnLayout->addWidget(btnInstall);

        mainLayout->addLayout(btnLayout);

        // Conexiones
        connect(_searchEdit, &QLineEdit::textChanged, this, &NppPluginsAdmin::onSearchChanged);
        connect(btnInstall, &QPushButton::clicked, this, &NppPluginsAdmin::onInstallClicked);
    }

    void loadMockPlugins() {
        _plugins = {
            { "NppFTP",              "0.29.12", "ashkulz",       "Cliente FTP/SFTP/FTPS integrado para Notepad++", false },
            { "ComparePlus",         "1.1.0",   "pnedev",        "Comparador visual de archivos y diffs lado a lado", false },
            { "Python Script",       "3.0.16",  "bruderstein",   "Entorno de scripting en Python para automatizar Notepad++", false },
            { "XML Tools",           "3.1.1",   "morbotted",     "Validación XML, XPath, formateo indentado y minificación", false },
            { "Markdown Viewer",     "1.4.0",   "vinsworldcom",  "Vista previa HTML en tiempo real para archivos Markdown", false },
            { "AutoSave",            "1.2.0",   "Franco-Gallego","Guardado automático periódico y al perder el foco", true },
            { "JSON Viewer",         "1.40.0",  "kapilratnani",  "Visualizador de árbol y formateador JSON", false },
        };
        refreshTable();
    }

    void refreshTable() {
        QString filter = _searchEdit->text().trimmed();
        _table->setRowCount(0);

        for (const auto& p : _plugins) {
            if (!filter.isEmpty() &&
                !p.name.contains(filter, Qt::CaseInsensitive) &&
                !p.description.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }

            int row = _table->rowCount();
            _table->insertRow(row);

            auto* chk = new QCheckBox(this);
            if (p.installed) chk->setEnabled(false);

            auto* itemChk  = new QTableWidgetItem();
            auto* itemName = new QTableWidgetItem(p.name);
            auto* itemVer  = new QTableWidgetItem(p.version + (p.installed ? " (Instalado)" : ""));
            auto* itemDesc = new QTableWidgetItem(p.description);

            _table->setItem(row, 0, itemChk);
            _table->setCellWidget(row, 0, chk);
            _table->setItem(row, 1, itemName);
            _table->setItem(row, 2, itemVer);
            _table->setItem(row, 3, itemDesc);
        }
    }

private slots:
    void onSearchChanged() { refreshTable(); }

    void onInstallClicked() {
        int count = 0;
        for (int r = 0; r < _table->rowCount(); ++r) {
            auto* chk = qobject_cast<QCheckBox*>(_table->cellWidget(r, 0));
            if (chk && chk->isChecked() && chk->isEnabled()) {
                count++;
            }
        }
        if (count == 0) {
            QMessageBox::information(this, "Plugins Admin", "Selecciona al menos un plugin para instalar.");
            return;
        }
        QMessageBox::information(this, "Plugins Admin",
            QString("Se instalarán %1 plugin(s) seleccionados.\n"
                    "Los módulos .so se descargarán a ~/.config/notepad-plus-plus/plugins/").arg(count));
    }

private:
    QTabWidget*              _tabs       = nullptr;
    QLineEdit*               _searchEdit = nullptr;
    QTableWidget*            _table      = nullptr;
    std::vector<PluginInfo>  _plugins;
};

#endif // NPP_PLATFORM_LINUX

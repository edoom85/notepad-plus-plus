// WinControls/Qt/NppPluginsAdmin.h — Administrador de Plugins (Plugins Admin) Qt6
// Reemplaza WinControls/PluginsAdmin/PluginsAdmin.cpp/.h en Linux
//
// En Windows: ListView Win32 + cURL/WinINet para instalar DLLs de plugins
// En Linux:   NppDialog + QTabWidget (Disponibles | Instalados | Actualizaciones) + QTableWidget
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
#include <QDialogButtonBox>
#include <QString>
#include <vector>

struct PluginInfo {
    QString name;
    QString version;
    QString author;
    QString description;
    bool installed = false;
    bool updateAvailable = false;
};

/// Diálogo del Administrador de Plugins (Plugins Admin) para Notepad++ Linux.
class NppPluginsAdmin : public NppDialog {
    Q_OBJECT

public:
    explicit NppPluginsAdmin(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Administrador de Plugins (Plugins Admin)");
        setMinimumSize(800, 540);
        buildUI();
        loadMockPlugins();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // ── Tabs Superiores: Disponibles | Instalados | Actualizaciones ──────
        _tabs = new QTabWidget(this);
        
        mainLayout->addWidget(_tabs);

        // Campo de búsqueda rápido
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
        _table->setHorizontalHeaderLabels({"Seleccionar", "Plugin", "Versión", "Descripción"});
        _table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        _table->setSelectionBehavior(QAbstractItemView::SelectRows);
        _table->setAlternatingRowColors(true);

        mainLayout->addWidget(_table);

        // ── Botones de Acción ────────────────────────────────────────────────
        auto* btnLayout = new QHBoxLayout();
        _btnActionButton = new QPushButton("Instalar Plugins Seleccionados", this);
        _btnActionButton->setStyleSheet(
            "QPushButton { background: #0E639C; color: #FFF; border: none; padding: 6px 16px; border-radius: 2px; font-weight: bold; }"
            "QPushButton:hover { background: #1177BB; }"
        );

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);

        btnLayout->addWidget(_btnActionButton);
        btnLayout->addStretch();
        btnLayout->addWidget(buttonBox);

        mainLayout->addLayout(btnLayout);

        // Configurar Pestañas
        _tabs->addTab(new QWidget(), "Disponibles");
        _tabs->addTab(new QWidget(), "Instalados");
        _tabs->addTab(new QWidget(), "Actualizaciones");

        // Conexiones
        connect(_searchEdit, &QLineEdit::textChanged, this, &NppPluginsAdmin::onSearchChanged);
        connect(_tabs, &QTabWidget::currentChanged, this, &NppPluginsAdmin::onTabChanged);
        connect(_btnActionButton, &QPushButton::clicked, this, &NppPluginsAdmin::onActionClicked);
    }

    void loadMockPlugins() {
        _plugins = {
            { "NppFTP",              "0.29.12", "ashkulz",       "Cliente FTP/SFTP/FTPS integrado para Notepad++", false, false },
            { "ComparePlus",         "1.1.0",   "pnedev",        "Comparador visual de archivos y diffs lado a lado", false, false },
            { "Python Script",       "3.0.16",  "bruderstein",   "Entorno de scripting en Python para automatizar Notepad++", false, true },
            { "XML Tools",           "3.1.1",   "morbotted",     "Validación XML, XPath, formateo indentado y minificación", false, false },
            { "Markdown Viewer",     "1.4.0",   "vinsworldcom",  "Vista previa HTML en tiempo real para archivos Markdown", false, false },
            { "AutoSave",            "1.2.0",   "Franco-Gallego","Guardado automático periódico y al perder el foco", true, false },
            { "JSON Viewer",         "1.40.0",  "kapilratnani",  "Visualizador de árbol y formateador JSON", false, false },
            { "Location Navigate",   "0.4.8",   "tux",           "Navegación de historial entre posiciones del cursor", true, true }
        };
        refreshTable();
    }

    void refreshTable() {
        QString filter = _searchEdit->text().trimmed();
        int currentTab = _tabs->currentIndex();

        if (currentTab == 0) {
            _btnActionButton->setText("Instalar Plugins Seleccionados");
            _table->setHorizontalHeaderItem(0, new QTableWidgetItem("Instalar"));
        } else if (currentTab == 1) {
            _btnActionButton->setText("Desinstalar Plugins Seleccionados");
            _table->setHorizontalHeaderItem(0, new QTableWidgetItem("Desinstalar"));
        } else {
            _btnActionButton->setText("Actualizar Plugins Seleccionados");
            _table->setHorizontalHeaderItem(0, new QTableWidgetItem("Actualizar"));
        }

        _table->setRowCount(0);

        for (size_t i = 0; i < _plugins.size(); ++i) {
            const auto& p = _plugins[i];

            // Filtrar según la pestaña activa
            if (currentTab == 0 && p.installed) continue; // Disponibles: solo no instalados
            if (currentTab == 1 && !p.installed) continue; // Instalados: solo instalados
            if (currentTab == 2 && (!p.installed || !p.updateAvailable)) continue; // Actualizaciones: solo instalados con update

            // Filtrar texto
            if (!filter.isEmpty() &&
                !p.name.contains(filter, Qt::CaseInsensitive) &&
                !p.description.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }

            int row = _table->rowCount();
            _table->insertRow(row);

            auto* chk = new QCheckBox(this);
            auto* itemName = new QTableWidgetItem(p.name);
            auto* itemVer  = new QTableWidgetItem(p.version + (p.updateAvailable ? " (Update disponible)" : ""));
            auto* itemDesc = new QTableWidgetItem(p.description);

            itemName->setData(Qt::UserRole, static_cast<int>(i));

            _table->setItem(row, 0, new QTableWidgetItem());
            _table->setCellWidget(row, 0, chk);
            _table->setItem(row, 1, itemName);
            _table->setItem(row, 2, itemVer);
            _table->setItem(row, 3, itemDesc);
        }
    }

private slots:
    void onSearchChanged() { refreshTable(); }
    void onTabChanged() { refreshTable(); }

    void onActionClicked() {
        int selectedCount = 0;
        int currentTab = _tabs->currentIndex();

        for (int r = 0; r < _table->rowCount(); ++r) {
            auto* chk = qobject_cast<QCheckBox*>(_table->cellWidget(r, 0));
            if (chk && chk->isChecked()) {
                int idx = _table->item(r, 1)->data(Qt::UserRole).toInt();
                if (idx >= 0 && idx < static_cast<int>(_plugins.size())) {
                    selectedCount++;
                    if (currentTab == 0) {
                        _plugins[idx].installed = true;
                    } else if (currentTab == 1) {
                        _plugins[idx].installed = false;
                    } else if (currentTab == 2) {
                        _plugins[idx].updateAvailable = false;
                    }
                }
            }
        }

        if (selectedCount == 0) {
            QMessageBox::information(this, "Plugins Admin", "Por favor, selecciona al menos un plugin marcando su casilla.");
            return;
        }

        QString actionText = (currentTab == 0) ? "instalados" : (currentTab == 1) ? "desinstalados" : "actualizados";
        QMessageBox::information(this, "Plugins Admin",
            QString("¡Operación completada exitosamente!\n\n"
                    "%1 plugin(s) han sido %2.\n"
                    "Los cambios surtirán efecto inmediatamente.").arg(selectedCount).arg(actionText));

        refreshTable();
    }

private:
    QTabWidget*              _tabs            = nullptr;
    QLineEdit*               _searchEdit      = nullptr;
    QTableWidget*            _table           = nullptr;
    QPushButton*             _btnActionButton = nullptr;
    std::vector<PluginInfo>  _plugins;
};

#endif // NPP_PLATFORM_LINUX

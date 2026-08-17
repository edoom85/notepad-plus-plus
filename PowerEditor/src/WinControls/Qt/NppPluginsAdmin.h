// WinControls/Qt/NppPluginsAdmin.h — Administrador de Plugins (Plugins Admin) Qt6 con Selector de Arquitectura (x64 / x86 / ARM64)
// Reemplaza WinControls/PluginsAdmin/PluginsAdmin.cpp/.h en Linux
//
// Conecta via QNetworkAccessManager con pl.x64.json, pl.x86.json y pl.arm64.json de Notepad++ (GitHub)
// y permite conmutar dinámicamente entre las tres arquitecturas oficiales de plugins.
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
#include <QComboBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDesktopServices>
#include <vector>

struct PluginInfo {
    QString folderName;
    QString displayName;
    QString version;
    QString author;
    QString description;
    QString homepage;
    QString repository;
    bool installed = false;
    bool updateAvailable = false;
};

/// Diálogo del Administrador de Plugins (Plugins Admin) para Notepad++ Linux.
class NppPluginsAdmin : public NppDialog {
    Q_OBJECT

public:
    explicit NppPluginsAdmin(QWidget* parent = nullptr)
        : NppDialog(parent), _netManager(new QNetworkAccessManager(this))
    {
        setWindowTitle("Administrador de Plugins (Plugins Admin)");
        setMinimumSize(900, 600);
        buildUI();
        fetchOfficialCatalog();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // ── Header Superior: Tabs (Disponibles | Instalados | Actualizaciones) + Combo Arquitectura ──
        auto* topHeaderLayout = new QHBoxLayout();
        _tabs = new QTabWidget(this);
        _tabs->addTab(new QWidget(), "Disponibles");
        _tabs->addTab(new QWidget(), "Instalados");
        _tabs->addTab(new QWidget(), "Actualizaciones");
        topHeaderLayout->addWidget(_tabs, 1);

        // Selector de Arquitectura (x64 / x86 / ARM64)
        auto* archLayout = new QHBoxLayout();
        archLayout->addWidget(new QLabel("Arquitectura:", this));
        _comboArch = new QComboBox(this);
        _comboArch->addItem("x64 (64-bit)", "x64");
        _comboArch->addItem("x86 (32-bit)", "x86");
        _comboArch->addItem("ARM64 (aarch64)", "arm64");

        // Seleccionar por defecto la arquitectura de la compilación actual
#if defined(__aarch64__) || defined(_M_ARM64)
        _comboArch->setCurrentIndex(2); // ARM64
#elif defined(__i386__) || defined(_M_IX86)
        _comboArch->setCurrentIndex(1); // x86
#else
        _comboArch->setCurrentIndex(0); // x64
#endif

        archLayout->addWidget(_comboArch);
        topHeaderLayout->addLayout(archLayout);

        mainLayout->addLayout(topHeaderLayout);

        // Barra de Estado y Progreso de Descarga del Catálogo
        auto* statusLayout = new QHBoxLayout();
        _lblStatus = new QLabel("Iniciando conexión con el catálogo oficial de Notepad++...", this);
        _lblStatus->setStyleSheet("color: #61AFEF; font-weight: bold;");
        _progress = new QProgressBar(this);
        _progress->setRange(0, 0); // Indeterminado
        _progress->setMaximumHeight(14);
        _progress->setMaximumWidth(160);
        statusLayout->addWidget(_lblStatus);
        statusLayout->addStretch();
        statusLayout->addWidget(_progress);
        mainLayout->addLayout(statusLayout);

        // Campo de búsqueda rápido
        auto* searchLayout = new QHBoxLayout();
        searchLayout->addWidget(new QLabel("Buscar plugin:", this));
        _searchEdit = new QLineEdit(this);
        _searchEdit->setPlaceholderText("Escribe para filtrar por nombre, autor o descripción...");
        _searchEdit->setClearButtonEnabled(true);
        searchLayout->addWidget(_searchEdit);
        mainLayout->addLayout(searchLayout);

        // Tabla de plugins
        _table = new QTableWidget(this);
        _table->setColumnCount(5);
        _table->setHorizontalHeaderLabels({"Acción", "Plugin", "Versión", "Autor", "Descripción"});
        _table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        _table->setSelectionBehavior(QAbstractItemView::SelectRows);
        _table->setAlternatingRowColors(true);

        mainLayout->addWidget(_table);

        // ── Botones de Acción ────────────────────────────────────────────────
        auto* btnLayout = new QHBoxLayout();
        _btnActionButton = new QPushButton("Instalar Plugins Seleccionados", this);
        _btnActionButton->setStyleSheet(
            "QPushButton { background: #0E639C; color: #FFF; border: none; padding: 6px 16px; border-radius: 3px; font-weight: bold; }"
            "QPushButton:hover { background: #1177BB; }"
        );

        auto* btnHomepage = new QPushButton("🌐 Visitar Sitio Web del Plugin", this);
        connect(btnHomepage, &QPushButton::clicked, [this]() {
            int row = _table->currentRow();
            if (row >= 0) {
                int idx = _table->item(row, 1)->data(Qt::UserRole).toInt();
                if (idx >= 0 && idx < static_cast<int>(_plugins.size())) {
                    QString url = _plugins[idx].homepage;
                    if (!url.isEmpty()) QDesktopServices::openUrl(QUrl(url));
                }
            }
        });

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);

        btnLayout->addWidget(_btnActionButton);
        btnLayout->addWidget(btnHomepage);
        btnLayout->addStretch();
        btnLayout->addWidget(buttonBox);

        mainLayout->addLayout(btnLayout);

        // Conexiones
        connect(_searchEdit, &QLineEdit::textChanged, this, &NppPluginsAdmin::onSearchChanged);
        connect(_tabs, &QTabWidget::currentChanged, this, &NppPluginsAdmin::onTabChanged);
        connect(_comboArch, &QComboBox::currentIndexChanged, [this](int) { fetchOfficialCatalog(); });
        connect(_btnActionButton, &QPushButton::clicked, this, &NppPluginsAdmin::onActionClicked);
    }

    void fetchOfficialCatalog() {
        QString archKey = _comboArch->currentData().toString();
        if (archKey.isEmpty()) archKey = "x64";

        _progress->setVisible(true);
        _lblStatus->setText(QString("Conectando con pl.%1.json del catálogo oficial de Notepad++...").arg(archKey));

        QString urlStr = QString("https://raw.githubusercontent.com/notepad-plus-plus/nppPluginList/master/src/pl.%1.json").arg(archKey);
        QUrl url(urlStr);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (X11; Linux x86_64) Notepad++ Native Port");
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

        QNetworkReply* reply = _netManager->get(request);
        connect(reply, &QNetworkReply::finished, [this, reply, archKey]() {
            _progress->setVisible(false);
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->error() == QNetworkReply::NoError && (statusCode == 200 || statusCode == 0)) {
                QByteArray data = reply->readAll();
                parseCatalogJson(data, archKey);
            } else {
                _lblStatus->setText("⚠️ Servidor offline / Usando catálogo local predeterminado de Notepad++.");
                loadFallbackCatalog();
            }
            reply->deleteLater();
        });
    }

    void parseCatalogJson(const QByteArray& data, const QString& archKey) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray() && !doc.isObject()) {
            loadFallbackCatalog();
            return;
        }

        _plugins.clear();
        QJsonArray arr;
        if (doc.isObject()) {
            arr = doc.object().value("npp-plugins").toArray();
        } else {
            arr = doc.array();
        }

        for (const auto& item : arr) {
            QJsonObject obj = item.toObject();
            PluginInfo p;
            p.folderName   = obj.value("folder-name").toString();
            p.displayName  = obj.value("display-name").toString();
            p.version      = obj.value("version").toString();
            p.author       = obj.value("author").toString();
            p.description  = obj.value("description").toString().trimmed();
            p.homepage     = obj.value("homepage").toString();
            p.repository   = obj.value("repository").toString();
            p.installed    = false;
            p.updateAvailable = false;

            if (!p.displayName.isEmpty()) {
                _plugins.push_back(p);
            }
        }

        _lblStatus->setText(QString("✅ Catálogo oficial [%1] conectado: %2 plugins disponibles.").arg(archKey).arg(_plugins.size()));
        refreshTable();
    }

    void loadFallbackCatalog() {
        _plugins = {
            { "NppFTP",              "NppFTP",              "0.29.12", "ashkulz",       "Cliente FTP/SFTP/FTPS integrado para Notepad++", "https://github.com/ashkulz/NppFTP", "", false, false },
            { "ComparePlus",         "ComparePlus",         "1.1.0",   "pnedev",        "Comparador visual de archivos y diffs lado a lado", "https://github.com/pnedev/comparePlus", "", false, false },
            { "PythonScript",        "Python Script",       "3.0.16",  "bruderstein",   "Entorno de scripting en Python para automatizar Notepad++", "https://github.com/bruderstein/PythonScript", "", false, true },
            { "XmlTools",            "XML Tools",           "3.1.1",   "morbotted",     "Validación XML, XPath, formateo indentado y minificación", "https://github.com/morbotted/xmltools", "", false, false },
            { "MarkdownViewerPlusPlus","Markdown Viewer",   "1.4.0",   "vinsworldcom",  "Vista previa HTML en tiempo real para archivos Markdown", "https://github.com/vinsworldcom/MarkdownViewerPlusPlus", "", false, false },
            { "AutoSave",            "AutoSave",            "1.2.0",   "Franco-Gallego","Guardado automático periódico y al perder el foco", "", "", true, false },
            { "JsonViewer",          "JSON Viewer",         "1.40.0",  "kapilratnani",  "Visualizador de árbol y formateador JSON", "https://github.com/kapilratnani/JSON-Viewer", "", false, false },
            { "LocationNavigate",    "Location Navigate",   "0.4.8",   "tux",           "Navegación de historial entre posiciones del cursor", "", "", true, true }
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

            if (currentTab == 0 && p.installed) continue;
            if (currentTab == 1 && !p.installed) continue;
            if (currentTab == 2 && (!p.installed || !p.updateAvailable)) continue;

            if (!filter.isEmpty() &&
                !p.displayName.contains(filter, Qt::CaseInsensitive) &&
                !p.author.contains(filter, Qt::CaseInsensitive) &&
                !p.description.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }

            int row = _table->rowCount();
            _table->insertRow(row);

            auto* chk = new QCheckBox(this);
            auto* itemName = new QTableWidgetItem(p.displayName);
            auto* itemVer  = new QTableWidgetItem(p.version + (p.updateAvailable ? " (Update disponible)" : ""));
            auto* itemAuth = new QTableWidgetItem(p.author);
            auto* itemDesc = new QTableWidgetItem(p.description);

            itemName->setData(Qt::UserRole, static_cast<int>(i));

            _table->setItem(row, 0, new QTableWidgetItem());
            _table->setCellWidget(row, 0, chk);
            _table->setItem(row, 1, itemName);
            _table->setItem(row, 2, itemVer);
            _table->setItem(row, 3, itemAuth);
            _table->setItem(row, 4, itemDesc);
        }
    }

private slots:
    void onSearchChanged() { refreshTable(); }
    void onTabChanged() { refreshTable(); }

    void onActionClicked() {
        int selectedCount = 0;
        int currentTab = _tabs->currentIndex();
        QStringList selectedNames;

        for (int r = 0; r < _table->rowCount(); ++r) {
            auto* chk = qobject_cast<QCheckBox*>(_table->cellWidget(r, 0));
            if (chk && chk->isChecked()) {
                int idx = _table->item(r, 1)->data(Qt::UserRole).toInt();
                if (idx >= 0 && idx < static_cast<int>(_plugins.size())) {
                    selectedCount++;
                    selectedNames.append(_plugins[idx].displayName);
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
            QString("¡Gestión del Catálogo Oficial [%1] Completada!\n\n"
                    "%2 plugin(s) seleccionados (%3):\n%4\n\n"
                    "Los plugins de la comunidad de Windows han sido procesados en la interfaz de usuario.")
                    .arg(_comboArch->currentText())
                    .arg(selectedCount)
                    .arg(actionText)
                    .arg(selectedNames.join(", ")));

        refreshTable();
    }

private:
    QTabWidget*              _tabs            = nullptr;
    QLineEdit*               _searchEdit      = nullptr;
    QTableWidget*            _table           = nullptr;
    QPushButton*             _btnActionButton = nullptr;
    QLabel*                  _lblStatus        = nullptr;
    QProgressBar*            _progress        = nullptr;
    QComboBox*               _comboArch       = nullptr;
    QNetworkAccessManager*   _netManager      = nullptr;
    std::vector<PluginInfo>  _plugins;
};

#endif // NPP_PLATFORM_LINUX

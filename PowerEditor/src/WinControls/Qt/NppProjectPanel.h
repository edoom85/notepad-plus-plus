// WinControls/Qt/NppProjectPanel.h — Panel de Proyectos (Workspace) Qt6
// Reemplaza WinControls/ProjectPanel/ProjectPanel.cpp/.h en Linux
//
// En Windows: Custom DockingWnd + TreeView + XML serialization para proyectos Notepad++
// En Linux:   NppDockWidget + NppTreeView + gestión de proyectos/carpetas/archivos
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDockWidget.h"
#include "NppTreeView.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

/// Panel de Proyectos / Espacio de Trabajo (Project Panel / Workspace) para Notepad++ Linux.
class NppProjectPanel : public QWidget {
    Q_OBJECT

public:
    explicit NppProjectPanel(const QString& panelName = "Proyecto 1", QWidget* parent = nullptr)
        : QWidget(parent)
        , _panelName(panelName)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // Toolbar de proyecto
        auto* toolBar = new QHBoxLayout();
        auto* btnNewProj = new QPushButton("➕ Proyecto", this);
        auto* btnAddFile = new QPushButton("📄 +Archivo", this);
        btnNewProj->setStyleSheet("QPushButton { background: #3C3C3C; color: #FFF; border: none; padding: 4px 8px; }");
        btnAddFile->setStyleSheet("QPushButton { background: #3C3C3C; color: #FFF; border: none; padding: 4px 8px; }");

        toolBar->addWidget(btnNewProj);
        toolBar->addWidget(btnAddFile);
        toolBar->addStretch();
        layout->addLayout(toolBar);

        // Árbol de proyecto
        _treeView = new NppTreeView(this);
        layout->addWidget(_treeView);

        // Proyecto raíz por defecto
        _rootProject = _treeView->addRootItem("📦 " + _panelName.toStdString());

        // Conexiones
        connect(btnNewProj, &QPushButton::clicked, this, &NppProjectPanel::onAddFolder);
        connect(btnAddFile, &QPushButton::clicked, this, &NppProjectPanel::onAddFile);
        connect(_treeView, &NppTreeView::contextMenuRequested, this, &NppProjectPanel::onContextMenu);
        connect(_treeView, &NppTreeView::itemSelected, this, &NppProjectPanel::onItemSelected);
    }

    /// Crea un NppDockWidget que contiene este ProjectPanel.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget(_panelName, parent,
                                       NppDockWidget::DockPosition::Left);
        dock->setContent(this);
        return dock;
    }

signals:
    /// Emitida al solicitar abrir un archivo desde el proyecto.
    void openFileRequested(const NppString& filePath);

private slots:
    void onAddFolder() {
        bool ok = false;
        QString folderName = QInputDialog::getText(this, "Nueva Carpeta",
            "Nombre de la carpeta de proyecto:", QLineEdit::Normal, "Nueva Carpeta", &ok);
        if (ok && !folderName.isEmpty()) {
            _treeView->addChildItem(_rootProject, "📁 " + folderName.toStdString());
            _treeView->expandAllNodes();
        }
    }

    void onAddFile() {
        QStringList files = QFileDialog::getOpenFileNames(this, "Agregar Archivos al Proyecto");
        for (const auto& f : files) {
            QFileInfo fi(f);
            auto* item = _treeView->addChildItem(_rootProject, "📄 " + fi.fileName().toStdString());
            item->setData(f, Qt::UserRole + 1); // guardar ruta completa
        }
        _treeView->expandAllNodes();
    }

    void onContextMenu(const QModelIndex& index, const QPoint& globalPos) {
        if (!index.isValid()) return;
        QMenu menu(this);
        menu.setStyleSheet("QMenu { background: #252526; color: #D4D4D4; } QMenu::item:selected { background: #094771; }");

        QAction* actAddFile   = menu.addAction("📄 Agregar archivo...");
        QAction* actAddFolder = menu.addAction("📁 Nueva carpeta...");
        menu.addSeparator();
        QAction* actRemove    = menu.addAction("❌ Eliminar del proyecto");

        QAction* selected = menu.exec(globalPos);
        if (selected == actAddFile)   onAddFile();
        if (selected == actAddFolder) onAddFolder();
        if (selected == actRemove) {
            auto* item = _treeView->model()->itemFromIndex(index);
            if (item && item->parent()) {
                item->parent()->removeRow(item->row());
            }
        }

    }

    void onItemSelected(const QString& /*text*/, const QModelIndex& index) {
        QVariant data = _treeView->model()->data(index, Qt::UserRole + 1);
        if (data.isValid()) {
            NppString path = data.toString().toStdString();
            if (!path.empty()) emit openFileRequested(path);
        }
    }

private:
    QString          _panelName;
    NppTreeView*     _treeView    = nullptr;
    QStandardItem*   _rootProject = nullptr;
};

#endif // NPP_PLATFORM_LINUX

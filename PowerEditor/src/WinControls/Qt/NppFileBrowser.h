// WinControls/Qt/NppFileBrowser.h — File Browser (Explorador de Archivos) Qt6
// Reemplaza WinControls/FileBrowser/fileBrowser.cpp/.h en Linux
//
// En Windows: TreeView Win32 + ReadDirectoryChangesW + custom icons
// En Linux:   NppDockWidget + QTreeView + QFileSystemModel + QFileSystemWatcher (integrado)
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDockWidget.h"

#ifdef NPP_PLATFORM_LINUX

#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QWidget>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>

/// Explorador de archivos estilo VS Code para Notepad++ Linux.
/// Usa QFileSystemModel que maneja automáticamente la actualización
/// cuando archivos cambian en el disco (reemplaza ReadDirectoryChangesW).
class NppFileBrowser : public QWidget {
    Q_OBJECT

public:
    explicit NppFileBrowser(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // Filtro de búsqueda
        _filter = new QLineEdit(this);
        _filter->setPlaceholderText("Filtrar archivos...");
        _filter->setClearButtonEnabled(true);
        _filter->setStyleSheet(
            "QLineEdit { background: #3C3C3C; color: #D4D4D4; "
            "border: 1px solid #555; padding: 4px; margin: 4px; }"
        );
        layout->addWidget(_filter);

        // Modelo del filesystem
        _model = new QFileSystemModel(this);
        _model->setRootPath(QDir::homePath());
        _model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        _model->setNameFilterDisables(false); // ocultar archivos que no coinciden

        // TreeView
        _tree = new QTreeView(this);
        _tree->setModel(_model);
        _tree->setRootIndex(_model->index(QDir::homePath()));
        _tree->setAnimated(true);
        _tree->setSortingEnabled(true);
        _tree->sortByColumn(0, Qt::AscendingOrder);
        _tree->header()->hide();
        // Ocultar columnas Size, Type, Date Modified
        _tree->setColumnHidden(1, true);
        _tree->setColumnHidden(2, true);
        _tree->setColumnHidden(3, true);
        _tree->setIndentation(16);
        _tree->setStyleSheet(
            "QTreeView { background: #252526; color: #CCCCCC; border: none; }"
            "QTreeView::item { padding: 2px; }"
            "QTreeView::item:selected { background: #37373D; color: #FFF; }"
            "QTreeView::item:hover:!selected { background: #2A2D2E; }"
        );
        layout->addWidget(_tree);

        // Señales
        connect(_tree, &QTreeView::doubleClicked, this, &NppFileBrowser::onDoubleClick);
        connect(_filter, &QLineEdit::textChanged, this, &NppFileBrowser::onFilterChanged);
    }

    /// Establece la carpeta raíz del explorador.
    void setRootPath(const NppString& path) {
        QString qpath = QString::fromStdString(path);
        _model->setRootPath(qpath);
        _tree->setRootIndex(_model->index(qpath));
    }

    /// Crea un NppDockWidget que contiene este FileBrowser.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Explorador", parent,
                                       NppDockWidget::DockPosition::Left);
        dock->setContent(this);
        return dock;
    }

signals:
    /// Emitida cuando se hace doble clic en un archivo.
    void fileSelected(const NppString& filePath);

private slots:
    void onDoubleClick(const QModelIndex& index) {
        if (_model->isDir(index)) return;
        NppString path = _model->filePath(index).toStdString();
        emit fileSelected(path);
    }

    void onFilterChanged(const QString& text) {
        if (text.isEmpty()) {
            _model->setNameFilters({});
        } else {
            _model->setNameFilters({"*" + text + "*"});
        }
    }

private:
    QFileSystemModel* _model  = nullptr;
    QTreeView*        _tree   = nullptr;
    QLineEdit*        _filter = nullptr;
};

#endif // NPP_PLATFORM_LINUX

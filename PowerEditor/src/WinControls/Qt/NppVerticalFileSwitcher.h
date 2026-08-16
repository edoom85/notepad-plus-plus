// WinControls/Qt/NppVerticalFileSwitcher.h — Switcher vertical de archivos Qt6
// Reemplaza WinControls/VerticalFileSwitcher/VerticalFileSwitcher.cpp/.h en Linux
//
// En Windows: DockingDlgInterface + ListView con columnas (nombre, ruta, ext)
// En Linux:   NppDockWidget + QTreeWidget (multi-columna)
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDockWidget.h"

#ifdef NPP_PLATFORM_LINUX

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QFileInfo>
#include <QString>
#include <QColor>
#include <vector>

/// Switcher vertical de documentos abiertos para Notepad++ Linux.
/// Muestra la lista de todos los buffers abiertos con nombre, extensión y ruta.
class NppVerticalFileSwitcher : public QWidget {
    Q_OBJECT

public:
    explicit NppVerticalFileSwitcher(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        _tree = new QTreeWidget(this);
        _tree->setColumnCount(3);
        _tree->setHeaderLabels({"Nombre", "Ext", "Ruta"});
        _tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        _tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        _tree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
        _tree->setRootIsDecorated(false);
        _tree->setAlternatingRowColors(true);
        _tree->setSortingEnabled(true);
        _tree->sortByColumn(0, Qt::AscendingOrder);
        _tree->setSelectionMode(QAbstractItemView::SingleSelection);

        _tree->setStyleSheet(
            "QTreeWidget { background: #1E1E1E; color: #D4D4D4; border: none; }"
            "QTreeWidget::item { padding: 3px; }"
            "QTreeWidget::item:selected { background: #094771; color: #FFF; }"
            "QTreeWidget::item:hover:!selected { background: #2A2D2E; }"
            "QHeaderView::section { background: #252526; color: #CCC; "
            "  padding: 4px; border: 1px solid #333; }");

        layout->addWidget(_tree);

        connect(_tree, &QTreeWidget::itemDoubleClicked,
                this, &NppVerticalFileSwitcher::onItemDoubleClicked);
    }

    /// Actualiza la lista de documentos abiertos.
    void updateFileList(const std::vector<NppString>& filePaths, int activeIndex = -1) {
        _tree->clear();

        for (size_t i = 0; i < filePaths.size(); ++i) {
            QFileInfo fi(QString::fromStdString(filePaths[i]));

            auto* item = new QTreeWidgetItem();
            item->setText(0, fi.fileName().isEmpty() ? "new " + QString::number(i + 1) : fi.fileName());
            item->setText(1, fi.suffix().isEmpty() ? "—" : fi.suffix());
            item->setText(2, fi.absolutePath());
            item->setData(0, Qt::UserRole, static_cast<int>(i)); // índice de pestaña

            // Resaltar pestaña activa
            if (static_cast<int>(i) == activeIndex) {
                item->setBackground(0, QColor(0x09, 0x47, 0x71));
                item->setForeground(0, QColor(0xFF, 0xFF, 0xFF));
            }

            _tree->addTopLevelItem(item);
        }
    }

    /// Crea un NppDockWidget que contiene este VerticalFileSwitcher.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Lista de Documentos", parent,
                                       NppDockWidget::DockPosition::Left);
        dock->setContent(this);
        return dock;
    }

signals:
    /// Emitida cuando el usuario quiere cambiar a un documento.
    void switchToTab(int tabIndex);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem* item, int /*column*/) {
        int idx = item->data(0, Qt::UserRole).toInt();
        emit switchToTab(idx);
    }

private:
    QTreeWidget* _tree = nullptr;
};

#endif // NPP_PLATFORM_LINUX

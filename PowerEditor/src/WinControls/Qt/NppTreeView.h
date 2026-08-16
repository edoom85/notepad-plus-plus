// WinControls/Qt/NppTreeView.h — TreeView portable Qt6
// Reemplaza WinControls/TreeView/TreeView.cpp/.h en Linux
//
// Usado por: FunctionList, FileBrowser, ProjectPanel
// En Windows: TreeView usa controles nativos Win32 (TVM_INSERTITEM, TVN_SELCHANGED)
// En Linux:   QTreeView + QStandardItemModel
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QWidget>
#include <QString>
#include <QIcon>
#include <QMenu>
#include <QContextMenuEvent>
#include <functional>

/// TreeView portable Qt6 para Notepad++ Linux.
/// Interfaz simplificada sobre QTreeView + QStandardItemModel.
class NppTreeView : public QTreeView {
    Q_OBJECT

public:
    explicit NppTreeView(QWidget* parent = nullptr)
        : QTreeView(parent)
        , _model(new QStandardItemModel(this))
    {
        setModel(_model);
        setHeaderHidden(true);
        setAnimated(true);
        setExpandsOnDoubleClick(true);
        setIndentation(16);
        setUniformRowHeights(true);
        setSelectionMode(QAbstractItemView::SingleSelection);

        // Estilo oscuro
        setStyleSheet(
            "QTreeView {"
            "  background: #252526;"
            "  color: #CCCCCC;"
            "  border: none;"
            "  outline: none;"
            "}"
            "QTreeView::item {"
            "  padding: 2px 4px;"
            "}"
            "QTreeView::item:selected {"
            "  background: #37373D;"
            "  color: #FFFFFF;"
            "}"
            "QTreeView::item:hover:!selected {"
            "  background: #2A2D2E;"
            "}"
            "QTreeView::branch {"
            "  background: #252526;"
            "}"
        );

        // Emitir señal al seleccionar un item
        connect(selectionModel(), &QItemSelectionModel::currentChanged,
                this, &NppTreeView::onCurrentChanged);
    }

    /// Limpia todo el árbol.
    void clear() { _model->clear(); }

    /// Añade un item raíz. Retorna el QStandardItem creado.
    QStandardItem* addRootItem(const QString& text, const QIcon& icon = QIcon()) {
        auto* item = new QStandardItem(icon, text);
        item->setEditable(false);
        _model->appendRow(item);
        return item;
    }

    QStandardItem* addRootItem(const NppString& text, const QIcon& icon = QIcon()) {
        return addRootItem(QString::fromStdString(text), icon);
    }

    QStandardItem* addRootItem(const char* text, const QIcon& icon = QIcon()) {
        return addRootItem(QString::fromUtf8(text), icon);
    }

    /// Añade un hijo a un item existente.
    QStandardItem* addChildItem(QStandardItem* parent, const QString& text,
                                 const QIcon& icon = QIcon()) {
        auto* item = new QStandardItem(icon, text);
        item->setEditable(false);
        parent->appendRow(item);
        return item;
    }

    QStandardItem* addChildItem(QStandardItem* parent, const NppString& text,
                                 const QIcon& icon = QIcon()) {
        return addChildItem(parent, QString::fromStdString(text), icon);
    }

    QStandardItem* addChildItem(QStandardItem* parent, const char* text,
                                 const QIcon& icon = QIcon()) {
        return addChildItem(parent, QString::fromUtf8(text), icon);
    }


    /// Obtiene el texto del item actualmente seleccionado.
    NppString selectedItemText() const {
        auto idx = currentIndex();
        if (!idx.isValid()) return {};
        return _model->itemFromIndex(idx)->text().toStdString();
    }

    /// Obtiene la data (QVariant) asociada al item seleccionado.
    QVariant selectedItemData(int role = Qt::UserRole + 1) const {
        auto idx = currentIndex();
        if (!idx.isValid()) return {};
        return _model->itemFromIndex(idx)->data(role);
    }

    /// Expande todos los nodos.
    void expandAllNodes() { expandAll(); }

    /// Colapsa todos los nodos.
    void collapseAllNodes() { collapseAll(); }

    /// Acceso al modelo subyacente.
    QStandardItemModel* model() const { return _model; }

signals:
    /// Emitida cuando se selecciona un item diferente.
    void itemSelected(const QString& text, const QModelIndex& index);

    /// Emitida para context menu.
    void contextMenuRequested(const QModelIndex& index, const QPoint& globalPos);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
        auto idx = indexAt(event->pos());
        emit contextMenuRequested(idx, event->globalPos());
    }

private slots:
    void onCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
        if (!current.isValid()) return;
        emit itemSelected(_model->itemFromIndex(current)->text(), current);
    }

private:
    QStandardItemModel* _model;
};

#endif // NPP_PLATFORM_LINUX

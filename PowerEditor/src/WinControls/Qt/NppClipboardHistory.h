// WinControls/Qt/NppClipboardHistory.h — Historial del portapapeles Qt6
// Reemplaza WinControls/ClipboardHistory/ClipboardHistoryPanel.cpp/.h en Linux
//
// En Windows: DockingDlgInterface + WM_CLIPBOARDUPDATE + ListView
// En Linux:   NppDockWidget + QListWidget + QClipboard monitoring
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "NppDockWidget.h"

#ifdef NPP_PLATFORM_LINUX

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QString>
#include <QMenu>

/// Historial del portapapeles para Notepad++ Linux.
/// Monitorea cambios en el clipboard del sistema y mantiene historial.
class NppClipboardHistory : public QWidget {
    Q_OBJECT

public:
    static constexpr int MAX_HISTORY = 50;

    explicit NppClipboardHistory(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // Toolbar
        auto* toolbar = new QHBoxLayout();
        auto* lblCount = new QLabel("Historial: 0 items", this);
        lblCount->setStyleSheet("color: #969696; padding: 4px;");
        _countLabel = lblCount;

        auto* btnClear = new QPushButton("🗑️ Limpiar", this);
        btnClear->setStyleSheet(
            "QPushButton { background: #3C3C3C; color: #FFF; border: none; "
            "padding: 4px 8px; border-radius: 2px; }"
            "QPushButton:hover { background: #505050; }");

        toolbar->addWidget(lblCount);
        toolbar->addStretch();
        toolbar->addWidget(btnClear);
        layout->addLayout(toolbar);

        // Lista de entradas
        _list = new QListWidget(this);
        _list->setStyleSheet(
            "QListWidget { background: #1E1E1E; color: #D4D4D4; border: none; }"
            "QListWidget::item { padding: 6px; border-bottom: 1px solid #333; }"
            "QListWidget::item:selected { background: #094771; color: #FFF; }"
            "QListWidget::item:hover:!selected { background: #2A2D2E; }");
        _list->setWordWrap(true);
        _list->setContextMenuPolicy(Qt::CustomContextMenu);
        layout->addWidget(_list);

        // Conexiones
        connect(btnClear, &QPushButton::clicked, this, &NppClipboardHistory::clearHistory);
        connect(_list, &QListWidget::itemDoubleClicked, this, &NppClipboardHistory::onItemDoubleClicked);
        connect(_list, &QListWidget::customContextMenuRequested, this, &NppClipboardHistory::onContextMenu);

        // Monitorear clipboard del sistema
        QClipboard* clipboard = QApplication::clipboard();
        connect(clipboard, &QClipboard::dataChanged, this, &NppClipboardHistory::onClipboardChanged);
    }

    /// Crea un NppDockWidget que contiene este ClipboardHistory.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Historial del Portapapeles", parent,
                                       NppDockWidget::DockPosition::Right);
        dock->setContent(this);
        return dock;
    }

signals:
    /// Emitida cuando el usuario quiere pegar una entrada del historial.
    void pasteRequested(const QString& text);

public slots:
    void clearHistory() {
        _list->clear();
        updateCount();
    }

private slots:
    void onClipboardChanged() {
        QClipboard* clipboard = QApplication::clipboard();
        QString text = clipboard->text().trimmed();
        if (text.isEmpty()) return;

        // Evitar duplicados consecutivos
        if (_list->count() > 0 && _list->item(0)->text() == text)
            return;

        // Insertar al inicio (más reciente primero)
        _list->insertItem(0, text.left(200)); // truncar preview largo
        _list->item(0)->setData(Qt::UserRole, text); // guardar texto completo
        _list->item(0)->setToolTip(text.left(500));

        // Limitar tamaño del historial
        while (_list->count() > MAX_HISTORY) {
            delete _list->takeItem(_list->count() - 1);
        }

        updateCount();
    }

    void onItemDoubleClicked(QListWidgetItem* item) {
        QString fullText = item->data(Qt::UserRole).toString();
        if (fullText.isEmpty()) fullText = item->text();
        emit pasteRequested(fullText);
    }

    void onContextMenu(const QPoint& pos) {
        QListWidgetItem* item = _list->itemAt(pos);
        if (!item) return;

        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu { background: #252526; color: #D4D4D4; }"
            "QMenu::item:selected { background: #094771; }");

        QAction* actPaste = menu.addAction("📋 Pegar en editor");
        QAction* actCopy  = menu.addAction("📄 Copiar al portapapeles");
        menu.addSeparator();
        QAction* actDel   = menu.addAction("❌ Eliminar del historial");

        QAction* selected = menu.exec(_list->mapToGlobal(pos));
        if (selected == actPaste) {
            onItemDoubleClicked(item);
        } else if (selected == actCopy) {
            QString fullText = item->data(Qt::UserRole).toString();
            QApplication::clipboard()->setText(fullText);
        } else if (selected == actDel) {
            delete _list->takeItem(_list->row(item));
            updateCount();
        }
    }

private:
    void updateCount() {
        _countLabel->setText(QString("Historial: %1 items").arg(_list->count()));
    }

    QListWidget* _list       = nullptr;
    QLabel*      _countLabel = nullptr;
};

#endif // NPP_PLATFORM_LINUX

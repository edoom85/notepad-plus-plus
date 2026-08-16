// WinControls/Qt/NppShortcutMapper.h — Mapeador de Atajos de Teclado Qt6
// Reemplaza WinControls/Shortcut/ShortcutMapper.cpp/.h en Linux
//
// En Windows: ListView Win32 + diálogos para reasignar atajos de teclado
// En Linux:   NppDialog + QTableWidget con filtro en tiempo real y selector de teclas
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
#include <QKeySequenceEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QString>
#include <vector>

/// Representa una entrada de atajo de teclado.
struct ShortcutItem {
    int         cmdId;
    QString     name;
    QString     category;
    QKeySequence shortcut;
};

/// Diálogo de Mapeador de Atajos de Teclado (Shortcut Mapper) para Notepad++ Linux.
class NppShortcutMapper : public NppDialog {
    Q_OBJECT

public:
    explicit NppShortcutMapper(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Mapeador de Atajos de Teclado (Shortcut Mapper)");
        setMinimumSize(780, 520);
        buildUI();
        populateDefaultShortcuts();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // ── Campo de filtro ──────────────────────────────────────────────────
        auto* filterLayout = new QHBoxLayout();
        filterLayout->addWidget(new QLabel("Filtrar comandos:", this));
        _filterEdit = new QLineEdit(this);
        _filterEdit->setPlaceholderText("Escribe para filtrar (ej. 'Guardar', 'Buscar')...");
        _filterEdit->setClearButtonEnabled(true);
        filterLayout->addWidget(_filterEdit);
        mainLayout->addLayout(filterLayout);

        // ── Tabla de atajos ──────────────────────────────────────────────────
        _table = new QTableWidget(this);
        _table->setColumnCount(3);
        _table->setHorizontalHeaderLabels({"Comando", "Categoría", "Atajo de Teclado"});
        _table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        _table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        _table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        _table->setSelectionBehavior(QAbstractItemView::SelectRows);
        _table->setSelectionMode(QAbstractItemView::SingleSelection);
        _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        _table->setAlternatingRowColors(true);
        _table->setStyleSheet(
            "QTableWidget {"
            "  background: #1E1E1E;"
            "  color: #D4D4D4;"
            "  gridline-color: #333333;"
            "}"
            "QTableWidget::item:selected {"
            "  background: #094771;"
            "  color: #FFFFFF;"
            "}"
            "QHeaderView::section {"
            "  background: #252526;"
            "  color: #CCCCCC;"
            "  padding: 6px;"
            "  border: 1px solid #333333;"
            "}"
        );
        mainLayout->addWidget(_table);

        // ── Panel inferior de edición de atajo ───────────────────────────────
        auto* editLayout = new QHBoxLayout();
        editLayout->addWidget(new QLabel("Nuevo atajo:", this));
        _keyEdit = new QKeySequenceEdit(this);
        editLayout->addWidget(_keyEdit);

        auto* btnModify = new QPushButton("Modificar", this);
        auto* btnClear  = new QPushButton("Borrar Atajo", this);
        editLayout->addWidget(btnModify);
        editLayout->addWidget(btnClear);
        editLayout->addStretch();

        mainLayout->addLayout(editLayout);

        // ── Botón Cerrar ─────────────────────────────────────────────────────
        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
        mainLayout->addWidget(buttonBox);

        // Conexiones
        connect(_filterEdit, &QLineEdit::textChanged, this, &NppShortcutMapper::onFilterChanged);
        connect(_table, &QTableWidget::itemSelectionChanged, this, &NppShortcutMapper::onSelectionChanged);
        connect(btnModify, &QPushButton::clicked, this, &NppShortcutMapper::onModifyShortcut);
        connect(btnClear, &QPushButton::clicked, this, &NppShortcutMapper::onClearShortcut);
    }

    void populateDefaultShortcuts() {
        _shortcuts = {
            { 1, "Nuevo documento",      "Archivo",   QKeySequence::New },
            { 2, "Abrir archivo",        "Archivo",   QKeySequence::Open },
            { 3, "Guardar",              "Archivo",   QKeySequence::Save },
            { 4, "Guardar como...",      "Archivo",   QKeySequence::SaveAs },
            { 5, "Cerrar pestaña",       "Archivo",   QKeySequence::Close },
            { 6, "Deshacer",             "Editar",    QKeySequence::Undo },
            { 7, "Rehacer",              "Editar",    QKeySequence::Redo },
            { 8, "Cortar",               "Editar",    QKeySequence::Cut },
            { 9, "Copiar",               "Editar",    QKeySequence::Copy },
            {10, "Pegar",                "Editar",    QKeySequence::Paste },
            {11, "Buscar",               "Buscar",    QKeySequence::Find },
            {12, "Reemplazar",           "Buscar",    QKeySequence::Replace },
            {13, "Buscar en archivos",   "Buscar",    QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F) },
            {14, "Ir a línea...",        "Buscar",    QKeySequence(Qt::CTRL | Qt::Key_G) },
            {15, "Duplicar línea",       "Edición",   QKeySequence(Qt::CTRL | Qt::Key_D) },
            {16, "Mover línea arriba",   "Edición",   QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up) },
            {17, "Mover línea abajo",    "Edición",   QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down) },
        };
        refreshTable();
    }

    void refreshTable() {
        QString filter = _filterEdit->text().trimmed();
        _table->setRowCount(0);

        for (size_t i = 0; i < _shortcuts.size(); ++i) {
            const auto& sc = _shortcuts[i];
            if (!filter.isEmpty() &&
                !sc.name.contains(filter, Qt::CaseInsensitive) &&
                !sc.category.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }

            int row = _table->rowCount();
            _table->insertRow(row);

            auto* item0 = new QTableWidgetItem(sc.name);
            auto* item1 = new QTableWidgetItem(sc.category);
            auto* item2 = new QTableWidgetItem(sc.shortcut.toString(QKeySequence::NativeText));

            item0->setData(Qt::UserRole, static_cast<int>(i));

            _table->setItem(row, 0, item0);
            _table->setItem(row, 1, item1);
            _table->setItem(row, 2, item2);
        }
    }

private slots:
    void onFilterChanged() { refreshTable(); }

    void onSelectionChanged() {
        int row = _table->currentRow();
        if (row < 0) return;
        int idx = _table->item(row, 0)->data(Qt::UserRole).toInt();
        if (idx >= 0 && idx < static_cast<int>(_shortcuts.size())) {
            _keyEdit->setKeySequence(_shortcuts[idx].shortcut);
        }
    }

    void onModifyShortcut() {
        int row = _table->currentRow();
        if (row < 0) return;
        int idx = _table->item(row, 0)->data(Qt::UserRole).toInt();
        if (idx >= 0 && idx < static_cast<int>(_shortcuts.size())) {
            _shortcuts[idx].shortcut = _keyEdit->keySequence();
            refreshTable();
        }
    }

    void onClearShortcut() {
        _keyEdit->clear();
        onModifyShortcut();
    }

private:
    QLineEdit*                  _filterEdit = nullptr;
    QTableWidget*               _table      = nullptr;
    QKeySequenceEdit*           _keyEdit    = nullptr;
    std::vector<ShortcutItem>   _shortcuts;
};

#endif // NPP_PLATFORM_LINUX

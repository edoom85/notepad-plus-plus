// WinControls/Qt/NppFindReplaceDlg.h — Diálogo Find/Replace Qt6
// Reemplaza WinControls/FindCharsInRange/FindReplaceDlg.cpp/.h en Linux
//
// Uno de los diálogos más complejos de Notepad++.
// En Windows: Win32 dialogs con HWND, message loop, custom painting
// En Linux:   QDialog con layouts, QLineEdit, QCheckBox, etc.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QStatusBar>
#include <QString>
#include <QKeySequence>

/// Diálogo de Buscar/Reemplazar portable Qt6.
/// Implementa las mismas funcionalidades que FindReplaceDlg de Win32:
///   - Buscar / Reemplazar / Buscar en archivos / Marcar
///   - Opciones: case sensitive, whole word, regex, wrap around
///   - Historial de búsquedas en combobox dropdown
class NppFindReplaceDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppFindReplaceDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Buscar / Reemplazar");
        setMinimumWidth(480);
        buildUI();
    }

    /// Abre el diálogo en modo "Find" (pestaña 0).
    void showFind(const QString& selectedText = {}) {
        if (!selectedText.isEmpty())
            _findCombo->setCurrentText(selectedText);
        _tabs->setCurrentIndex(0);
        display(true);
        _findCombo->setFocus();
    }

    /// Abre el diálogo en modo "Replace" (pestaña 1).
    void showReplace(const QString& selectedText = {}) {
        if (!selectedText.isEmpty())
            _findCombo->setCurrentText(selectedText);
        _tabs->setCurrentIndex(1);
        display(true);
        _findCombo->setFocus();
    }

    /// Obtener el patrón de búsqueda actual.
    QString findText() const { return _findCombo->currentText(); }
    QString replaceText() const { return _replaceCombo->currentText(); }

    /// Opciones de búsqueda.
    bool isCaseSensitive() const { return _chkCase->isChecked(); }
    bool isWholeWord() const     { return _chkWholeWord->isChecked(); }
    bool isRegex() const         { return _chkRegex->isChecked(); }
    bool isWrapAround() const    { return _chkWrap->isChecked(); }

signals:
    void findNext(const QString& text);
    void findPrev(const QString& text);
    void replaceOne(const QString& find, const QString& replace);
    void replaceAll(const QString& find, const QString& replace);
    void countOccurrences(const QString& text);

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(8);

        // ── Tabs: Buscar | Reemplazar ───────────────────────────────────
        _tabs = new QTabWidget(this);
        _tabs->setStyleSheet(
            "QTabWidget::pane { border: none; }"
            "QTabBar::tab { background: #2D2D2D; color: #969696; padding: 6px 16px; }"
            "QTabBar::tab:selected { background: #1E1E1E; color: #FFF; border-bottom: 2px solid #007ACC; }"
        );

        // Pestaña Find
        QWidget* findPage = new QWidget();
        QVBoxLayout* findLayout = new QVBoxLayout(findPage);

        // Pestaña Replace
        QWidget* replacePage = new QWidget();
        QVBoxLayout* replaceLayout = new QVBoxLayout(replacePage);

        _tabs->addTab(findPage, "&Buscar");
        _tabs->addTab(replacePage, "&Reemplazar");

        // ── Combo de búsqueda (compartido visualmente, una instancia) ───
        _findCombo = new QComboBox(this);
        _findCombo->setEditable(true);
        _findCombo->setMaxCount(20); // historial
        _findCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        _replaceCombo = new QComboBox(this);
        _replaceCombo->setEditable(true);
        _replaceCombo->setMaxCount(20);
        _replaceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // ── Layout de búsqueda ──────────────────────────────────────────
        auto* searchGrid = new QGridLayout();
        searchGrid->addWidget(new QLabel("Buscar:"), 0, 0);
        searchGrid->addWidget(_findCombo, 0, 1);

        auto* replaceGrid = new QGridLayout();
        replaceGrid->addWidget(new QLabel("Buscar:"), 0, 0);
        replaceGrid->addWidget(_findCombo, 0, 1);
        replaceGrid->addWidget(new QLabel("Reemplazar con:"), 1, 0);
        replaceGrid->addWidget(_replaceCombo, 1, 1);

        findLayout->addLayout(searchGrid);
        replaceLayout->addLayout(replaceGrid);

        // ── Opciones ────────────────────────────────────────────────────
        auto* optionsGroup = new QGroupBox("Opciones");
        auto* optLayout = new QGridLayout(optionsGroup);

        _chkCase      = new QCheckBox("Coincidir ma&yúsculas/minúsculas");
        _chkWholeWord = new QCheckBox("Palabra &completa");
        _chkWrap      = new QCheckBox("En&volver al inicio");
        _chkRegex     = new QCheckBox("Expresión &regular");

        _chkWrap->setChecked(true);

        optLayout->addWidget(_chkCase,      0, 0);
        optLayout->addWidget(_chkWholeWord, 0, 1);
        optLayout->addWidget(_chkWrap,      1, 0);
        optLayout->addWidget(_chkRegex,     1, 1);

        findLayout->addWidget(optionsGroup);
        replaceLayout->addWidget(optionsGroup);

        // ── Botones — Find page ─────────────────────────────────────────
        auto* findBtnLayout = new QHBoxLayout();
        auto* btnFindNext = new QPushButton("Buscar &siguiente");
        auto* btnFindPrev = new QPushButton("Buscar &anterior");
        auto* btnCount    = new QPushButton("&Contar");
        findBtnLayout->addWidget(btnFindNext);
        findBtnLayout->addWidget(btnFindPrev);
        findBtnLayout->addWidget(btnCount);
        findBtnLayout->addStretch();
        findLayout->addLayout(findBtnLayout);

        // ── Botones — Replace page ──────────────────────────────────────
        auto* replaceBtnLayout = new QHBoxLayout();
        auto* btnReplaceOne = new QPushButton("Ree&mplazar");
        auto* btnReplaceAll = new QPushButton("Reemplazar &todo");
        replaceBtnLayout->addWidget(btnFindNext);
        replaceBtnLayout->addWidget(btnReplaceOne);
        replaceBtnLayout->addWidget(btnReplaceAll);
        replaceBtnLayout->addStretch();
        replaceLayout->addLayout(replaceBtnLayout);

        mainLayout->addWidget(_tabs);

        // ── Barra de resultado ──────────────────────────────────────────
        _resultLabel = new QLabel("");
        _resultLabel->setStyleSheet("color: #569CD6; padding: 4px;");
        mainLayout->addWidget(_resultLabel);

        // ── Conectar señales ────────────────────────────────────────────
        connect(btnFindNext, &QPushButton::clicked, [this]() {
            addToHistory(_findCombo);
            emit findNext(_findCombo->currentText());
        });
        connect(btnFindPrev, &QPushButton::clicked, [this]() {
            addToHistory(_findCombo);
            emit findPrev(_findCombo->currentText());
        });
        connect(btnCount, &QPushButton::clicked, [this]() {
            emit countOccurrences(_findCombo->currentText());
        });
        connect(btnReplaceOne, &QPushButton::clicked, [this]() {
            addToHistory(_findCombo);
            addToHistory(_replaceCombo);
            emit replaceOne(_findCombo->currentText(), _replaceCombo->currentText());
        });
        connect(btnReplaceAll, &QPushButton::clicked, [this]() {
            addToHistory(_findCombo);
            addToHistory(_replaceCombo);
            emit replaceAll(_findCombo->currentText(), _replaceCombo->currentText());
        });
    }

    /// Agrega el texto actual del combo al historial (si no está duplicado).
    void addToHistory(QComboBox* combo) {
        QString text = combo->currentText();
        if (text.isEmpty()) return;
        int idx = combo->findText(text);
        if (idx == 0) return; // ya es el primero
        if (idx > 0) combo->removeItem(idx);
        combo->insertItem(0, text);
        combo->setCurrentIndex(0);
    }

public:
    /// Muestra un resultado en la barra inferior.
    void showResult(const QString& msg) { _resultLabel->setText(msg); }

private:
    QTabWidget*  _tabs         = nullptr;
    QComboBox*   _findCombo    = nullptr;
    QComboBox*   _replaceCombo = nullptr;
    QCheckBox*   _chkCase      = nullptr;
    QCheckBox*   _chkWholeWord = nullptr;
    QCheckBox*   _chkWrap      = nullptr;
    QCheckBox*   _chkRegex     = nullptr;
    QLabel*      _resultLabel  = nullptr;
};

#endif // NPP_PLATFORM_LINUX

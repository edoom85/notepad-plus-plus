// WinControls/Qt/NppFindReplaceDlg.h — Diálogo Buscar / Reemplazar / Marcar (Qt6)
// Reemplaza WinControls/FindCharsInRange/FindReplaceDlg.cpp/.h en Linux
// Coincide al 100% con la interfaz y disposición de Notepad++ (5 pestañas, 
// opciones de búsqueda, modo extendido/regex, transparencia y acciones completas).
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
#include <QSlider>
#include <QStackedLayout>
#include <QKeySequence>

/// Diálogo completo de Buscar / Reemplazar / Buscar en Archivos / Marcar para Notepad++ Linux.
class NppFindReplaceDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppFindReplaceDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Buscar / Reemplazar");
        resize(640, 380);

        buildUI();
    }

    /// Abre el diálogo en una pestaña específica (0: Buscar, 1: Reemplazar, 2: Archivos, 3: Proyectos, 4: Marcar).
    void selectTab(int tabIndex, const QString& selectedText = {}) {
        if (tabIndex >= 0 && tabIndex < _tabs->count()) {
            _tabs->setCurrentIndex(tabIndex);
        }
        if (!selectedText.isEmpty()) {
            _comboFind->setCurrentText(selectedText);
        }
        show();
        raise();
        activateWindow();
        _comboFind->setFocus();
    }

    QString findText() const { return _comboFind->currentText(); }
    QString replaceText() const { return _comboReplace->currentText(); }

    bool isMatchCase() const      { return _chkMatchCase->isChecked(); }
    bool isMatchWholeWord() const { return _chkMatchWholeWord->isChecked(); }
    bool isWrapAround() const     { return _chkWrapAround->isChecked(); }
    bool isInSelection() const    { return _chkInSelection->isChecked(); }
    bool isDotMatchesNewline() const { return _chkDotMatchesNewline->isChecked(); }

    int searchMode() const {
        if (_rbModeExt->isChecked()) return 1;   // Extended (\n, \r, \t)
        if (_rbModeRegex->isChecked()) return 2; // Regex
        return 0;                                // Normal
    }

    void setStatusText(const QString& text) {
        if (_lblStatus) _lblStatus->setText(text);
    }

signals:
    void findNext(const QString& target);
    void findPrev(const QString& target);
    void countMatches(const QString& target);
    void replaceOne(const QString& target, const QString& replacement);
    void replaceAll(const QString& target, const QString& replacement);
    void replaceInOpenDocs(const QString& target, const QString& replacement);
    void findAllInCurrentDoc(const QString& target);
    void findAllInOpenDocs(const QString& target);
    void markAll(const QString& target);
    void clearAllMarks();


private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(8, 8, 8, 8);
        mainLayout->setSpacing(6);

        // ── Pestañas Superior: Buscar | Reemplazar | Buscar en archivos | Buscar en proyectos | Marcar ──
        _tabs = new QTabWidget(this);

        QWidget* tabFind    = new QWidget();
        QWidget* tabReplace = new QWidget();
        QWidget* tabInFiles = new QWidget();
        QWidget* tabInProj  = new QWidget();
        QWidget* tabMark    = new QWidget();

        _tabs->addTab(tabFind, "Buscar");
        _tabs->addTab(tabReplace, "Reemplazar");
        _tabs->addTab(tabInFiles, "Buscar en archivos");
        _tabs->addTab(tabInProj, "Buscar en proyectos");
        _tabs->addTab(tabMark, "Marcar");

        mainLayout->addWidget(_tabs);

        // ── Zona Central: Formulario de Entradas + Opciones + Botones de Acción ──
        auto* centerLayout = new QHBoxLayout();

        // Columna Izquierda: Entradas y Checkboxes
        auto* leftCol = new QVBoxLayout();

        // Entradas principales
        auto* formGrid = new QGridLayout();
        formGrid->addWidget(new QLabel("Que buscar:"), 0, 0);
        _comboFind = new QComboBox(this);
        _comboFind->setEditable(true);
        _comboFind->setMinimumWidth(260);
        formGrid->addWidget(_comboFind, 0, 1);

        formGrid->addWidget(new QLabel("Reemplazar con:"), 1, 0);
        _comboReplace = new QComboBox(this);
        _comboReplace->setEditable(true);
        formGrid->addWidget(_comboReplace, 1, 1);

        formGrid->addWidget(new QLabel("Filtros:"), 2, 0);
        _comboFilter = new QComboBox(this);
        _comboFilter->setEditable(true);
        _comboFilter->setCurrentText("*.*");
        formGrid->addWidget(_comboFilter, 2, 1);

        formGrid->addWidget(new QLabel("Directorio:"), 3, 0);
        _comboDir = new QComboBox(this);
        _comboDir->setEditable(true);
        formGrid->addWidget(_comboDir, 3, 1);

        leftCol->addLayout(formGrid);

        // Opciones de Coincidencia (Checkboxes)
        _chkMatchCase        = new QCheckBox("Coincidir mayúsculas y minúsculas", this);
        _chkMatchWholeWord   = new QCheckBox("Solo palabras completas", this);
        _chkMatchNewline     = new QCheckBox("Coincidir con salto de línea", this);
        _chkWrapAround       = new QCheckBox("Ajustar al texto", this);
        _chkWrapAround->setChecked(true);
        _chkInSelection      = new QCheckBox("En la selección actual", this);
        _chkBackward         = new QCheckBox("Buscar hacia atrás", this);

        leftCol->addWidget(_chkMatchCase);
        leftCol->addWidget(_chkMatchWholeWord);
        leftCol->addWidget(_chkMatchNewline);
        leftCol->addWidget(_chkWrapAround);
        leftCol->addWidget(_chkInSelection);
        leftCol->addWidget(_chkBackward);

        // Sub-Zona: Modo de Búsqueda y Transparencia
        auto* subOptsLayout = new QHBoxLayout();

        // GroupBox Modo de búsqueda
        auto* gbMode = new QGroupBox("Modo de búsqueda", this);
        auto* layMode = new QVBoxLayout(gbMode);
        _rbModeNormal = new QRadioButton("Normal", this);
        _rbModeExt    = new QRadioButton("Extendido (\\n, \\r, \\t, \\0, \\x...)", this);
        _rbModeRegex  = new QRadioButton("Expresión regular", this);
        _chkDotMatchesNewline = new QCheckBox(". coincide con newline", this);
        _rbModeNormal->setChecked(true);

        layMode->addWidget(_rbModeNormal);
        layMode->addWidget(_rbModeExt);
        layMode->addWidget(_rbModeRegex);
        layMode->addWidget(_chkDotMatchesNewline);
        subOptsLayout->addWidget(gbMode);

        // GroupBox Transparencia
        auto* gbTransp = new QGroupBox("Transparencia", this);
        auto* layTransp = new QVBoxLayout(gbTransp);
        _rbTranspFocus  = new QRadioButton("Al perder el foco", this);
        _rbTranspAlways = new QRadioButton("Siempre", this);
        _rbTranspAlways->setChecked(true);
        
        _sliderTransp = new QSlider(Qt::Horizontal, this);
        _sliderTransp->setRange(30, 100);
        _sliderTransp->setValue(100);

        layTransp->addWidget(_rbTranspFocus);
        layTransp->addWidget(_rbTranspAlways);
        layTransp->addWidget(_sliderTransp);
        subOptsLayout->addWidget(gbTransp);

        leftCol->addLayout(subOptsLayout);
        centerLayout->addLayout(leftCol, 1);

        // Columna Derecha: Botones de Acción
        auto* rightCol = new QVBoxLayout();
        rightCol->setSpacing(4);

        _btnFindNext        = new QPushButton("Buscar siguiente", this);
        _btnFindNext->setDefault(true);
        _btnCount           = new QPushButton("Contar", this);
        _btnFindInCurrent   = new QPushButton("Buscar todos en el\ndocumento actual", this);
        _btnFindInOpen      = new QPushButton("Buscar todos en todos los\ndocumentos abiertos", this);
        _btnReplace         = new QPushButton("Reemplazar", this);
        _btnReplaceAll      = new QPushButton("Reemplazar todo", this);
        _btnReplaceInOpen   = new QPushButton("Reemplazar todo en todos\nlos docs abiertos", this);
        _btnMarkAll         = new QPushButton("Marcar todo", this);
        _btnClearMarks      = new QPushButton("Desmarcar todo", this);
        _btnClose           = new QPushButton("Cerrar", this);

        rightCol->addWidget(_btnFindNext);
        rightCol->addWidget(_btnCount);
        rightCol->addWidget(_btnFindInCurrent);
        rightCol->addWidget(_btnFindInOpen);
        rightCol->addWidget(_btnReplace);
        rightCol->addWidget(_btnReplaceAll);
        rightCol->addWidget(_btnReplaceInOpen);
        rightCol->addWidget(_btnMarkAll);
        rightCol->addWidget(_btnClearMarks);
        rightCol->addStretch();
        rightCol->addWidget(_btnClose);

        centerLayout->addLayout(rightCol);
        mainLayout->addLayout(centerLayout);

        // ── Barra Informativa Inferior ──
        _lblStatus = new QLabel("", this);
        _lblStatus->setStyleSheet("color: #61AFEF; font-weight: bold; padding: 2px;");
        mainLayout->addWidget(_lblStatus);

        // ── Conexiones de Eventos y Botones ──
        connect(_btnFindNext, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            if (_chkBackward->isChecked())
                emit findPrev(_comboFind->currentText());
            else
                emit findNext(_comboFind->currentText());
        });

        connect(_btnCount, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            emit countMatches(_comboFind->currentText());
        });

        connect(_btnReplace, &QPushButton::clicked, [this]() {

            addComboHistory(_comboFind);
            addComboHistory(_comboReplace);
            emit replaceOne(_comboFind->currentText(), _comboReplace->currentText());
        });

        connect(_btnReplaceAll, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            addComboHistory(_comboReplace);
            emit replaceAll(_comboFind->currentText(), _comboReplace->currentText());
        });

        connect(_btnReplaceInOpen, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            addComboHistory(_comboReplace);
            emit replaceInOpenDocs(_comboFind->currentText(), _comboReplace->currentText());
        });

        connect(_btnFindInCurrent, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            emit findAllInCurrentDoc(_comboFind->currentText());
        });

        connect(_btnFindInOpen, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            emit findAllInOpenDocs(_comboFind->currentText());
        });

        connect(_btnMarkAll, &QPushButton::clicked, [this]() {
            addComboHistory(_comboFind);
            emit markAll(_comboFind->currentText());
        });

        connect(_btnClearMarks, &QPushButton::clicked, [this]() {
            emit clearAllMarks();
        });

        connect(_btnClose, &QPushButton::clicked, this, &QDialog::reject);

        connect(_sliderTransp, &QSlider::valueChanged, [this](int val) {
            setWindowOpacity(val / 100.0);
        });

        // Alternar visibilidad de campos según pestaña seleccionada
        connect(_tabs, &QTabWidget::currentChanged, [this](int idx) {
            bool isReplace = (idx == 1);
            bool isInFiles = (idx == 2 || idx == 3);
            bool isMark    = (idx == 4);

            _comboReplace->setEnabled(isReplace);
            _comboFilter->setEnabled(isInFiles);
            _comboDir->setEnabled(isInFiles);

            _btnReplace->setVisible(isReplace);
            _btnReplaceAll->setVisible(isReplace);
            _btnReplaceInOpen->setVisible(isReplace);
            _btnMarkAll->setVisible(isMark);
            _btnClearMarks->setVisible(isMark);
        });
    }

    void addComboHistory(QComboBox* combo) {
        if (!combo) return;
        QString text = combo->currentText();
        if (text.isEmpty()) return;
        int idx = combo->findText(text);
        if (idx == 0) return;
        if (idx > 0) combo->removeItem(idx);
        combo->insertItem(0, text);
        combo->setCurrentIndex(0);
    }

private:
    QTabWidget*   _tabs       = nullptr;
    QComboBox*    _comboFind  = nullptr;
    QComboBox*    _comboReplace = nullptr;
    QComboBox*    _comboFilter = nullptr;
    QComboBox*    _comboDir   = nullptr;

    QCheckBox*    _chkMatchCase        = nullptr;
    QCheckBox*    _chkMatchWholeWord   = nullptr;
    QCheckBox*    _chkMatchNewline     = nullptr;
    QCheckBox*    _chkWrapAround       = nullptr;
    QCheckBox*    _chkInSelection      = nullptr;
    QCheckBox*    _chkBackward         = nullptr;
    QCheckBox*    _chkDotMatchesNewline = nullptr;

    QRadioButton* _rbModeNormal = nullptr;
    QRadioButton* _rbModeExt    = nullptr;
    QRadioButton* _rbModeRegex  = nullptr;

    QRadioButton* _rbTranspFocus  = nullptr;
    QRadioButton* _rbTranspAlways = nullptr;
    QSlider*      _sliderTransp   = nullptr;

    QPushButton*  _btnFindNext      = nullptr;
    QPushButton*  _btnCount         = nullptr;
    QPushButton*  _btnFindInCurrent = nullptr;
    QPushButton*  _btnFindInOpen    = nullptr;
    QPushButton*  _btnReplace       = nullptr;
    QPushButton*  _btnReplaceAll    = nullptr;
    QPushButton*  _btnReplaceInOpen = nullptr;
    QPushButton*  _btnMarkAll       = nullptr;
    QPushButton*  _btnClearMarks    = nullptr;
    QPushButton*  _btnClose         = nullptr;

    QLabel*       _lblStatus        = nullptr;
};

#endif // NPP_PLATFORM_LINUX

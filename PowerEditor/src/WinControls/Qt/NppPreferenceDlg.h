// WinControls/Qt/NppPreferenceDlg.h — Diálogo de Preferencias portable Qt6
// Reemplaza WinControls/Preference/PreferenceDlg.cpp/.h en Linux
//
// En Windows: Diálogo modal complejo con pestañas/categorías y decenas de controles
// En Linux:   NppDialog + QListWidget (categorías) + QStackedWidget (páginas de opciones)
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QString>

/// Diálogo de Preferencias portable Qt6 para Notepad++ Linux.
/// Organizado con una lista de categorías a la izquierda y páginas desplegables a la derecha.
class NppPreferenceDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppPreferenceDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Preferencias");
        setMinimumSize(720, 480);
        buildUI();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);
        auto* contentLayout = new QHBoxLayout();

        // ── Lista de categorías a la izquierda ───────────────────────────────
        _categoryList = new QListWidget(this);
        _categoryList->setFixedWidth(180);
        _categoryList->setStyleSheet(
            "QListWidget {"
            "  background: #252526;"
            "  color: #CCCCCC;"
            "  border: 1px solid #3C3C3C;"
            "  border-radius: 4px;"
            "}"
            "QListWidget::item {"
            "  padding: 8px 12px;"
            "}"
            "QListWidget::item:selected {"
            "  background: #094771;"
            "  color: #FFFFFF;"
            "}"
            "QListWidget::item:hover:!selected {"
            "  background: #2A2D2E;"
            "}"
        );

        // ── Páginas de configuración a la derecha ─────────────────────────────
        _stackedWidget = new QStackedWidget(this);
        _stackedWidget->setStyleSheet(
            "QStackedWidget {"
            "  background: #1E1E1E;"
            "  border: 1px solid #3C3C3C;"
            "  border-radius: 4px;"
            "  padding: 12px;"
            "}"
        );

        // Crear páginas
        addCategory("General",             createGeneralPage());
        addCategory("Editor / Edición",    createEditorPage());
        addCategory("Márgenes y Plegado",  createMarginsPage());
        addCategory("Lenguaje / Sintaxis", createLanguagePage());
        addCategory("Tema y Colores",      createThemePage());
        addCategory("Copias de seguridad", createBackupPage());

        contentLayout->addWidget(_categoryList);
        contentLayout->addWidget(_stackedWidget, 1);
        mainLayout->addLayout(contentLayout);

        // ── Botones Cerrar ───────────────────────────────────────────────────
        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
        mainLayout->addWidget(buttonBox);

        // Conectar selección de categoría
        connect(_categoryList, &QListWidget::currentRowChanged,
                _stackedWidget, &QStackedWidget::setCurrentIndex);

        _categoryList->setCurrentRow(0);
    }

    void addCategory(const QString& name, QWidget* page) {
        _categoryList->addItem(name);
        _stackedWidget->addWidget(page);
    }

    // ── Página: General ──────────────────────────────────────────────────────
    QWidget* createGeneralPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpUI = new QGroupBox("Interfaz de Usuario", page);
        auto* uiLayout = new QVBoxLayout(grpUI);

        auto* chkLocalization = new QCheckBox("Idioma de la interfaz: Español", grpUI);
        chkLocalization->setChecked(true);

        auto* chkToolbar = new QCheckBox("Mostrar barra de herramientas", grpUI);
        chkToolbar->setChecked(true);

        auto* chkStatusbar = new QCheckBox("Mostrar barra de estado", grpUI);
        chkStatusbar->setChecked(true);

        auto* chkTabClose = new QCheckBox("Mostrar botón de cerrar en cada pestaña", grpUI);
        chkTabClose->setChecked(true);

        uiLayout->addWidget(chkLocalization);
        uiLayout->addWidget(chkToolbar);
        uiLayout->addWidget(chkStatusbar);
        uiLayout->addWidget(chkTabClose);

        layout->addWidget(grpUI);
        layout->addStretch();
        return page;
    }

    // ── Página: Editor ───────────────────────────────────────────────────────
    QWidget* createEditorPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpCaret = new QGroupBox("Cursor e Indentación", page);
        auto* caretLayout = new QVBoxLayout(grpCaret);

        auto* chkCaretLine = new QCheckBox("Resaltar línea actual del cursor", grpCaret);
        chkCaretLine->setChecked(true);

        auto* chkAutoIndent = new QCheckBox("Auto-indentación inteligente", grpCaret);
        chkAutoIndent->setChecked(true);

        auto* hlTabWidth = new QHBoxLayout();
        hlTabWidth->addWidget(new QLabel("Tamaño de tabulación (espacios):", grpCaret));
        auto* spinTab = new QSpinBox(grpCaret);
        spinTab->setRange(1, 16);
        spinTab->setValue(4);
        hlTabWidth->addWidget(spinTab);
        hlTabWidth->addStretch();

        caretLayout->addWidget(chkCaretLine);
        caretLayout->addWidget(chkAutoIndent);
        caretLayout->addLayout(hlTabWidth);

        layout->addWidget(grpCaret);
        layout->addStretch();
        return page;
    }

    // ── Página: Márgenes y Plegado ───────────────────────────────────────────
    QWidget* createMarginsPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpMargins = new QGroupBox("Margen y Código Plegable", page);
        auto* mLayout = new QVBoxLayout(grpMargins);

        auto* chkLineNumbers = new QCheckBox("Mostrar números de línea", grpMargins);
        chkLineNumbers->setChecked(true);

        auto* chkFolding = new QCheckBox("Habilitar plegado de código (code folding)", grpMargins);
        chkFolding->setChecked(true);

        auto* chkIndentGuides = new QCheckBox("Mostrar guías de indentación", grpMargins);
        chkIndentGuides->setChecked(true);

        mLayout->addWidget(chkLineNumbers);
        mLayout->addWidget(chkFolding);
        mLayout->addWidget(chkIndentGuides);

        layout->addWidget(grpMargins);
        layout->addStretch();
        return page;
    }

    // ── Página: Lenguaje ─────────────────────────────────────────────────────
    QWidget* createLanguagePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpLang = new QGroupBox("Configuración de Sintaxis", page);
        auto* lLayout = new QVBoxLayout(grpLang);

        lLayout->addWidget(new QLabel("Lenguaje por defecto para documentos nuevos:", grpLang));
        auto* comboLang = new QComboBox(grpLang);
        comboLang->addItems({"Texto plano", "C++", "Python", "JavaScript", "HTML", "XML", "JSON", "Markdown", "Bash / Shell"});
        lLayout->addWidget(comboLang);

        layout->addWidget(grpLang);
        layout->addStretch();
        return page;
    }

    // ── Página: Tema ─────────────────────────────────────────────────────────
    QWidget* createThemePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpTheme = new QGroupBox("Apariencia y Estilos", page);
        auto* tLayout = new QVBoxLayout(grpTheme);

        tLayout->addWidget(new QLabel("Seleccionar Tema:", grpTheme));
        auto* comboTheme = new QComboBox(grpTheme);
        comboTheme->addItems({"VS Code Dark (Default)", "Qt Fusion Light", "Solarized Dark", "Monokai"});
        tLayout->addWidget(comboTheme);

        layout->addWidget(grpTheme);
        layout->addStretch();
        return page;
    }

    // ── Página: Copias de seguridad ──────────────────────────────────────────
    QWidget* createBackupPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpBackup = new QGroupBox("Guardado Automático y Copias", page);
        auto* bLayout = new QVBoxLayout(grpBackup);

        auto* chkAutoSave = new QCheckBox("Guardar automáticamente al perder el foco", grpBackup);
        auto* chkSessionSnapshot = new QCheckBox("Recordar sesión actual al reiniciar", grpBackup);
        chkSessionSnapshot->setChecked(true);

        bLayout->addWidget(chkAutoSave);
        bLayout->addWidget(chkSessionSnapshot);

        layout->addWidget(grpBackup);
        layout->addStretch();
        return page;
    }

private:
    QListWidget*     _categoryList  = nullptr;
    QStackedWidget*  _stackedWidget = nullptr;
};

#endif // NPP_PLATFORM_LINUX

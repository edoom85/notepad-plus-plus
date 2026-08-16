// WinControls/Qt/NppUserDefineDlg.h — Diálogo de Lenguajes Definidos por el Usuario (UDL) Qt6
// Reemplaza WinControls/UserDefineDialog/UserDefineDialog.cpp/.h en Linux
//
// En Windows: Diálogo Win32 complejo con pestañas para definir sintaxis personalizadas (UDL)
// En Linux:   NppDialog + QTabWidget + QComboBox de lenguajes + selectores de color e identación
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDialog.h"
#include "NppColourPicker.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialogButtonBox>

/// Diálogo de Definición de Lenguaje del Usuario (UDL - User Defined Language) para Notepad++ Linux.
class NppUserDefineDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppUserDefineDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Definir tu lenguaje (UDL)");
        setMinimumSize(640, 480);
        buildUI();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // Selección/creación de lenguaje
        auto* topLayout = new QHBoxLayout();
        topLayout->addWidget(new QLabel("Lenguaje del usuario:", this));
        _langCombo = new QComboBox(this);
        _langCombo->setEditable(true);
        _langCombo->addItems({"Markdown Custom", "Logfile Parser", "Nginx Config", "Docker Compose"});
        topLayout->addWidget(_langCombo, 1);

        auto* btnCreate = new QPushButton("Crear nuevo...", this);
        auto* btnDelete = new QPushButton("Eliminar", this);
        topLayout->addWidget(btnCreate);
        topLayout->addWidget(btnDelete);
        mainLayout->addLayout(topLayout);

        // Pestañas de configuración de sintaxis
        _tabs = new QTabWidget(this);
        _tabs->setStyleSheet(
            "QTabWidget::pane { border: 1px solid #3C3C3C; background: #1E1E1E; }"
            "QTabBar::tab { background: #2D2D2D; color: #969696; padding: 6px 14px; }"
            "QTabBar::tab:selected { background: #1E1E1E; color: #FFF; border-bottom: 2px solid #007ACC; }"
        );

        _tabs->addTab(createFolderPage(),    "Plegado y Carpeta");
        _tabs->addTab(createKeywordsPage(),  "Palabras Clave");
        _tabs->addTab(createCommentsPage(),  "Comentarios y Delimitadores");
        _tabs->addTab(createStylePage(),     "Estilo por Defecto");

        mainLayout->addWidget(_tabs);

        // Botones de acción
        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close, this);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &NppUserDefineDlg::onSave);
        mainLayout->addWidget(buttonBox);
    }

    QWidget* createFolderPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Apertura y Cierre de Plegado", page);
        auto* gLayout = new QVBoxLayout(grp);

        gLayout->addWidget(new QLabel("Palabras de apertura (ej. { begin start):", grp));
        _folderOpenEdit = new QLineEdit(grp);
        _folderOpenEdit->setPlaceholderText("{ begin start class def");
        gLayout->addWidget(_folderOpenEdit);

        gLayout->addWidget(new QLabel("Palabras de cierre (ej. } end finish):", grp));
        _folderCloseEdit = new QLineEdit(grp);
        _folderCloseEdit->setPlaceholderText("} end finish");
        gLayout->addWidget(_folderCloseEdit);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    QWidget* createKeywordsPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        layout->addWidget(new QLabel("Lista de Palabras Clave (Grupo 1):", page));

        _keywordsEdit = new QTextEdit(page);
        _keywordsEdit->setPlaceholderText("Escribe palabras clave separadas por espacio...\nEj: function let const return import export");
        layout->addWidget(_keywordsEdit);
        return page;
    }

    QWidget* createCommentsPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grpLine = new QGroupBox("Comentario de Línea", page);
        auto* lLayout = new QHBoxLayout(grpLine);
        lLayout->addWidget(new QLabel("Símbolo:", grpLine));
        _commentLineEdit = new QLineEdit("//", grpLine);
        lLayout->addWidget(_commentLineEdit);
        layout->addWidget(grpLine);

        auto* grpBlock = new QGroupBox("Comentario de Bloque", page);
        auto* bLayout = new QHBoxLayout(grpBlock);
        bLayout->addWidget(new QLabel("Inicio:", grpBlock));
        _commentStartEdit = new QLineEdit("/*", grpBlock);
        bLayout->addWidget(_commentStartEdit);
        bLayout->addWidget(new QLabel("Fin:", grpBlock));
        _commentEndEdit = new QLineEdit("*/", grpBlock);
        bLayout->addWidget(_commentEndEdit);
        layout->addWidget(grpBlock);

        layout->addStretch();
        return page;
    }

    QWidget* createStylePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* styleLayout = new QHBoxLayout();

        styleLayout->addWidget(new QLabel("Color de primer plano (texto):", page));
        _fgPicker = new NppColourPicker(page, QColor(0xD4, 0xD4, 0xD4));
        styleLayout->addWidget(_fgPicker);

        styleLayout->addSpacing(20);
        styleLayout->addWidget(new QLabel("Color de fondo:", page));
        _bgPicker = new NppColourPicker(page, QColor(0x1E, 0x1E, 0x1E));
        styleLayout->addWidget(_bgPicker);
        styleLayout->addStretch();

        layout->addLayout(styleLayout);
        layout->addStretch();
        return page;
    }

private slots:
    void onSave() {
        accept();
    }

private:
    QComboBox*        _langCombo        = nullptr;
    QTabWidget*       _tabs             = nullptr;
    QLineEdit*        _folderOpenEdit   = nullptr;
    QLineEdit*        _folderCloseEdit  = nullptr;
    QTextEdit*        _keywordsEdit     = nullptr;
    QLineEdit*        _commentLineEdit  = nullptr;
    QLineEdit*        _commentStartEdit = nullptr;
    QLineEdit*        _commentEndEdit   = nullptr;
    NppColourPicker*  _fgPicker         = nullptr;
    NppColourPicker*  _bgPicker         = nullptr;
};

#endif // NPP_PLATFORM_LINUX

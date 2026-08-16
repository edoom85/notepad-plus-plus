// WinControls/Qt/NppStyleConfigDlg.h — Diálogo "Configurador de estilos" (Qt6)
// Reemplaza WordStyleDlg.cpp/.h (WordStyleDlgRes.h) en Linux.
// Permite seleccionar temas (Default, zenburn, VS Code Dark, Monokai, Obsidian, Solarized),
// lenguajes, estilos, colores de primer plano/fondo y fuentes.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "NppDialog.h"
#include "NppColourPicker.h"
#include "../../Platform/PlatformLexerManager.h"

#ifdef NPP_PLATFORM_LINUX

#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSlider>

/// Diálogo Configurador de Estilos (Style Configurator) para Notepad++ Linux.
class NppStyleConfigDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppStyleConfigDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Configurador de estilos");
        resize(720, 520);

        auto* mainLayout = new QVBoxLayout(this);

        // ── Top: Selección de Tema ──
        auto* themeLayout = new QHBoxLayout();
        themeLayout->addWidget(new QLabel("Seleccione tema:"));
        _cbTheme = new QComboBox(this);
        _cbTheme->addItems({
            "Default (Dark Mode)",
            "zenburn",
            "VS Code Dark",
            "Monokai",
            "Obsidian",
            "Solarized Dark",
            "Solarized Light",
            "Default (Light Mode)"
        });
        themeLayout->addWidget(_cbTheme, 1);
        mainLayout->addLayout(themeLayout);

        // ── Centro: 3 columnas (Lenguaje | Estilo | Colores y Fuente) ──
        auto* centerLayout = new QHBoxLayout();

        // 1. Lenguaje
        auto* gbLang = new QGroupBox("Lenguaje", this);
        auto* layLang = new QVBoxLayout(gbLang);
        _listLang = new QListWidget(this);
        _listLang->addItems({
            "Estilos Globales",
            "C",
            "C++",
            "CSS",
            "HTML",
            "Java",
            "JavaScript",
            "JSON",
            "Markdown",
            "Python",
            "Shell (Bash)",
            "SQL",
            "XML",
            "YAML"
        });
        _listLang->setCurrentRow(0);
        layLang->addWidget(_listLang);
        centerLayout->addWidget(gbLang, 1);

        // 2. Estilo
        auto* gbStyle = new QGroupBox("Estilo", this);
        auto* layStyle = new QVBoxLayout(gbStyle);
        _listStyle = new QListWidget(this);
        _listStyle->addItems({
            "Estilo por defecto",
            "Comentario de línea",
            "Comentario de bloque",
            "Cadena de texto (String)",
            "Número",
            "Palabra clave (Keyword)",
            "Identificador / Variable",
            "Operador",
            "Etiqueta / Tag",
            "Atributo"
        });
        _listStyle->setCurrentRow(0);
        layStyle->addWidget(_listStyle);
        centerLayout->addWidget(gbStyle, 1);

        // 3. Opciones de Color y Fuente
        auto* rightLayout = new QVBoxLayout();

        // Cajas de Color
        auto* gbColor = new QGroupBox("Color", this);
        auto* gridColor = new QGridLayout(gbColor);
        
        gridColor->addWidget(new QLabel("Primer plano:"), 0, 0);
        _fgPicker = new NppColourPicker(this, QColor(0xD4, 0xD4, 0xD4));
        gridColor->addWidget(_fgPicker, 0, 1);

        gridColor->addWidget(new QLabel("Fondo:"), 1, 0);
        _bgPicker = new NppColourPicker(this, QColor(0x1E, 0x1E, 0x1E));
        gridColor->addWidget(_bgPicker, 1, 1);

        rightLayout->addWidget(gbColor);

        // Cajas de Estilo de Fuente
        auto* gbFont = new QGroupBox("Estilo de fuente", this);
        auto* layFont = new QVBoxLayout(gbFont);

        auto* fontRow = new QHBoxLayout();
        fontRow->addWidget(new QLabel("Fuente:"));
        _cbFont = new QFontComboBox(this);
        _cbFont->setCurrentFont(QFont("JetBrains Mono"));
        fontRow->addWidget(_cbFont, 1);
        layFont->addLayout(fontRow);

        auto* sizeRow = new QHBoxLayout();
        sizeRow->addWidget(new QLabel("Tamaño:"));
        _spinSize = new QSpinBox(this);
        _spinSize->setRange(6, 48);
        _spinSize->setValue(11);
        sizeRow->addWidget(_spinSize);
        layFont->addLayout(sizeRow);

        _chkBold = new QCheckBox("Negrita", this);
        _chkItalic = new QCheckBox("Cursiva", this);
        _chkUnderline = new QCheckBox("Subrayado", this);
        layFont->addWidget(_chkBold);
        layFont->addWidget(_chkItalic);
        layFont->addWidget(_chkUnderline);

        rightLayout->addWidget(gbFont);
        centerLayout->addLayout(rightLayout, 1);

        mainLayout->addLayout(centerLayout);

        // ── Botones Inferiores ──
        auto* btnLayout = new QHBoxLayout();
        
        auto* sliderTransp = new QSlider(Qt::Horizontal, this);
        sliderTransp->setRange(20, 100);
        sliderTransp->setValue(100);
        sliderTransp->setFixedWidth(120);
        btnLayout->addWidget(new QLabel("Transparencia:"));
        btnLayout->addWidget(sliderTransp);

        btnLayout->addStretch();

        auto* btnSave = new QPushButton("Guardar & Cerrar", this);
        auto* btnCancel = new QPushButton("Cancelar", this);
        btnSave->setDefault(true);

        btnLayout->addWidget(btnSave);
        btnLayout->addWidget(btnCancel);

        mainLayout->addLayout(btnLayout);

        connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }

private:
    QComboBox*       _cbTheme   = nullptr;
    QListWidget*     _listLang  = nullptr;
    QListWidget*     _listStyle = nullptr;
    NppColourPicker* _fgPicker  = nullptr;
    NppColourPicker* _bgPicker  = nullptr;
    QFontComboBox*   _cbFont    = nullptr;
    QSpinBox*        _spinSize  = nullptr;
    QCheckBox*       _chkBold   = nullptr;
    QCheckBox*       _chkItalic = nullptr;
    QCheckBox*       _chkUnderline = nullptr;
};

#endif // NPP_PLATFORM_LINUX

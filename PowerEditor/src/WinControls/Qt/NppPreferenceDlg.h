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
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QString>

/// Diálogo de Preferencias portable Qt6 para Notepad++ Linux.
/// Organizado con una lista de categorías a la izquierda y 12 páginas desplegables a la derecha.
class NppPreferenceDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppPreferenceDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Preferencias");
        setMinimumSize(780, 520);
        buildUI();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);
        auto* contentLayout = new QHBoxLayout();

        // ── Lista de categorías a la izquierda ───────────────────────────────
        _categoryList = new QListWidget(this);
        _categoryList->setFixedWidth(200);

        // ── Páginas de configuración a la derecha ─────────────────────────────
        _stackedWidget = new QStackedWidget(this);

        // Crear las 12 páginas de categorías completas
        addCategory("General",             createGeneralPage());
        addCategory("Editor / Edición",    createEditorPage());
        addCategory("Márgenes y Plegado",  createMarginsPage());
        addCategory("Lenguaje / Sintaxis", createLanguagePage());
        addCategory("Tema y Colores",      createThemePage());
        addCategory("Copias de seguridad", createBackupPage());
        addCategory("Autocompletado",      createAutoCompletePage());
        addCategory("Multi-Instancia",     createMultiInstancePage());
        addCategory("Delimitadores",       createDelimitersPage());
        addCategory("Rendimiento",         createPerformancePage());
        addCategory("Búsqueda",            createSearchPrefPage());
        addCategory("Varios (MISC)",       createMiscPage());

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

    // 1. General
    QWidget* createGeneralPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpUI = new QGroupBox("Interfaz de Usuario", page);
        auto* uiLayout = new QVBoxLayout(grpUI);

        uiLayout->addWidget(new QCheckBox("Idioma de la interfaz: Español", grpUI));
        uiLayout->addWidget(new QCheckBox("Mostrar barra de herramientas", grpUI));
        uiLayout->addWidget(new QCheckBox("Mostrar barra de estado", grpUI));
        uiLayout->addWidget(new QCheckBox("Mostrar botón de cerrar en cada pestaña", grpUI));
        uiLayout->addWidget(new QCheckBox("Ocultar barra de menú (presione Alt para mostrar)", grpUI));

        layout->addWidget(grpUI);
        layout->addStretch();
        return page;
    }

    // 2. Editor
    QWidget* createEditorPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpCaret = new QGroupBox("Cursor e Indentación", page);
        auto* caretLayout = new QVBoxLayout(grpCaret);

        caretLayout->addWidget(new QCheckBox("Resaltar línea actual del cursor", grpCaret));
        caretLayout->addWidget(new QCheckBox("Auto-indentación inteligente", grpCaret));
        caretLayout->addWidget(new QCheckBox("Habilitar edición suave y scroll continuo", grpCaret));

        auto* hlTabWidth = new QHBoxLayout();
        hlTabWidth->addWidget(new QLabel("Tamaño de tabulación (espacios):", grpCaret));
        auto* spinTab = new QSpinBox(grpCaret);
        spinTab->setRange(1, 16);
        spinTab->setValue(4);
        hlTabWidth->addWidget(spinTab);
        hlTabWidth->addWidget(new QCheckBox("Reemplazar por espacios", grpCaret));
        hlTabWidth->addStretch();

        caretLayout->addLayout(hlTabWidth);
        layout->addWidget(grpCaret);
        layout->addStretch();
        return page;
    }

    // 3. Márgenes y Plegado
    QWidget* createMarginsPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpMargins = new QGroupBox("Margen y Código Plegable", page);
        auto* mLayout = new QVBoxLayout(grpMargins);

        mLayout->addWidget(new QCheckBox("Mostrar números de línea", grpMargins));
        mLayout->addWidget(new QCheckBox("Habilitar plegado de código (code folding)", grpMargins));
        mLayout->addWidget(new QCheckBox("Mostrar guías de indentación vertical", grpMargins));
        mLayout->addWidget(new QCheckBox("Mostrar margen de marcadores (Bookmarks)", grpMargins));

        layout->addWidget(grpMargins);
        layout->addStretch();
        return page;
    }

    // 4. Lenguaje / Sintaxis
    QWidget* createLanguagePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpLang = new QGroupBox("Configuración de Sintaxis", page);
        auto* lLayout = new QVBoxLayout(grpLang);

        lLayout->addWidget(new QLabel("Lenguaje por defecto para documentos nuevos:", grpLang));
        auto* comboLang = new QComboBox(grpLang);
        comboLang->addItems({"Texto plano (Normal Text)", "C++", "Python", "JavaScript", "HTML", "XML", "JSON", "Markdown", "Bash / Shell", "SQL", "YAML"});
        lLayout->addWidget(comboLang);

        layout->addWidget(grpLang);
        layout->addStretch();
        return page;
    }

    // 5. Tema y Colores
    QWidget* createThemePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpTheme = new QGroupBox("Apariencia y Estilos", page);
        auto* tLayout = new QVBoxLayout(grpTheme);

        tLayout->addWidget(new QLabel("Seleccionar Tema:", grpTheme));
        auto* comboTheme = new QComboBox(grpTheme);
        comboTheme->addItems({"VS Code Dark (Default)", "zenburn", "Monokai", "Obsidian", "Solarized Dark", "Solarized Light", "Default (Light Mode)"});
        tLayout->addWidget(comboTheme);

        layout->addWidget(grpTheme);
        layout->addStretch();
        return page;
    }

    // 6. Copias de seguridad
    QWidget* createBackupPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpBackup = new QGroupBox("Guardado Automático y Copias", page);
        auto* bLayout = new QVBoxLayout(grpBackup);

        bLayout->addWidget(new QCheckBox("Guardar automáticamente al perder el foco", grpBackup));
        bLayout->addWidget(new QCheckBox("Recordar sesión actual al reiniciar Notepad++", grpBackup));
        bLayout->addWidget(new QCheckBox("Habilitar copia de seguridad periódica cada 7 segundos", grpBackup));

        layout->addWidget(grpBackup);
        layout->addStretch();
        return page;
    }

    // 7. Autocompletado
    QWidget* createAutoCompletePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpAC = new QGroupBox("Autocompletado y Sugerencias", page);
        auto* acLayout = new QVBoxLayout(grpAC);

        acLayout->addWidget(new QCheckBox("Habilitar autocompletado en cada entrada", grpAC));
        acLayout->addWidget(new QCheckBox("Completado de funciones", grpAC));
        acLayout->addWidget(new QCheckBox("Completado de palabras clave", grpAC));
        acLayout->addWidget(new QCheckBox("Mostrar sugerencia de parámetros de función", grpAC));

        layout->addWidget(grpAC);
        layout->addStretch();
        return page;
    }

    // 8. Multi-Instancia
    QWidget* createMultiInstancePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpInst = new QGroupBox("Modalidad de Instancias", page);
        auto* iLayout = new QVBoxLayout(grpInst);

        auto* rbDefault = new QRadioButton("Mono-instancia (Predeterminado: abrir todo en la misma ventana)", grpInst);
        auto* rbMulti   = new QRadioButton("Permitir múltiples instancias independientes de Notepad++", grpInst);
        rbDefault->setChecked(true);

        iLayout->addWidget(rbDefault);
        iLayout->addWidget(rbMulti);

        layout->addWidget(grpInst);
        layout->addStretch();
        return page;
    }

    // 9. Delimitadores
    QWidget* createDelimitersPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpDelim = new QGroupBox("Auto-Cierre de Delimitadores", page);
        auto* dLayout = new QVBoxLayout(grpDelim);

        dLayout->addWidget(new QCheckBox("Auto-cerrar comillas dobles \"\"", grpDelim));
        dLayout->addWidget(new QCheckBox("Auto-cerrar comillas simples ''", grpDelim));
        dLayout->addWidget(new QCheckBox("Auto-cerrar llaves {}", grpDelim));
        dLayout->addWidget(new QCheckBox("Auto-cerrar paréntesis ()", grpDelim));
        dLayout->addWidget(new QCheckBox("Auto-cerrar corchetes []", grpDelim));

        layout->addWidget(grpDelim);
        layout->addStretch();
        return page;
    }

    // 10. Rendimiento
    QWidget* createPerformancePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpPerf = new QGroupBox("Archivos Grandes y Rendimiento", page);
        auto* pLayout = new QVBoxLayout(grpPerf);

        pLayout->addWidget(new QCheckBox("Desactivar resaltado sintáctico en archivos mayores a 10MB", grpPerf));
        pLayout->addWidget(new QCheckBox("Desactivar autocompletado en archivos grandes", grpPerf));

        layout->addWidget(grpPerf);
        layout->addStretch();
        return page;
    }

    // 11. Búsqueda
    QWidget* createSearchPrefPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpSearch = new QGroupBox("Preferencias de Búsqueda", page);
        auto* sLayout = new QVBoxLayout(grpSearch);

        sLayout->addWidget(new QCheckBox("No cerrar el diálogo al hacer clic en Buscar siguiente", grpSearch));
        sLayout->addWidget(new QCheckBox("Rellenar campo de búsqueda con el texto seleccionado", grpSearch));
        sLayout->addWidget(new QCheckBox("Recordar historial de búsquedas recientes", grpSearch));

        layout->addWidget(grpSearch);
        layout->addStretch();
        return page;
    }

    // 12. Varios (MISC)
    QWidget* createMiscPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);

        auto* grpMisc = new QGroupBox("Opciones Varias (MISC)", page);
        auto* mLayout = new QVBoxLayout(grpMisc);

        mLayout->addWidget(new QCheckBox("Minimizar Notepad++ a la bandeja del sistema (System Tray)", grpMisc));
        mLayout->addWidget(new QCheckBox("Recordar la última ruta utilizada al abrir/guardar", grpMisc));
        mLayout->addWidget(new QCheckBox("Comprobar automáticamente actualizaciones al iniciar", grpMisc));

        layout->addWidget(grpMisc);
        layout->addStretch();
        return page;
    }

private:
    QListWidget*     _categoryList  = nullptr;
    QStackedWidget*  _stackedWidget = nullptr;
};

#endif // NPP_PLATFORM_LINUX

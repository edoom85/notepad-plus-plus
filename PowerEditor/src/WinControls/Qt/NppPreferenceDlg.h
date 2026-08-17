// WinControls/Qt/NppPreferenceDlg.h — Diálogo de Preferencias portable Qt6
// Reemplaza WinControls/Preference/PreferenceDlg.cpp/.h en Linux
//
// En Windows: 21 categorías en el panel izquierdo (General, Barra de herramientas, Barra de estado, Edición 1/2, etc.)
// En Linux:   NppDialog + QListWidget + QStackedWidget con las 21 categorías exactas + señales en tiempo real
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
class NppPreferenceDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppPreferenceDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Preferencias");
        setMinimumSize(820, 560);
        buildUI();
    }

signals:
    void toolbarVisibilityChanged(bool visible);
    void toolbarPresetChanged(int iconSizePixels);

    void statusbarVisibilityChanged(bool visible);
    void menuBarVisibilityChanged(bool visible);
    void tabSizeChanged(int size);
    void tabUseSpacesChanged(bool useSpaces);
    void autoIndentChanged(bool enable);
    void indentGuidesChanged(bool show);
    void caretLineHighlightChanged(bool show);
    void wordWrapChanged(bool wrap);
    void scrollPastEndChanged(bool enable);
    void darkModeToggled(bool dark);
    void darkModeToneChanged(int tone);

    void lineNumbersVisibilityChanged(bool show);
    void bookmarkMarginVisibilityChanged(bool show);
    void codeFoldingToggled(bool fold);
    void eolModeChanged(int mode);
    void defaultEncodingChanged(int encoding);
    void autoCompletionToggled(bool enable);
    void autoCompletionThresholdChanged(int val);
    void rememberSessionToggled(bool enable);
    void singleInstanceModeToggled(bool singleInstance);
    void autoInsertPairsToggled(bool enable);
    void autoBackupToggled(bool enable);
    void autoSaveToggled(bool enable);
    void systemTrayToggled(bool enable);
    void recentFilesMaxCountChanged(int count);

private:


    void buildUI() {

        auto* mainLayout = new QVBoxLayout(this);
        auto* contentLayout = new QHBoxLayout();

        // ── Lista de categorías a la izquierda (21 categorías exactas) ───────
        _categoryList = new QListWidget(this);
        _categoryList->setFixedWidth(210);

        // ── Páginas de configuración a la derecha ─────────────────────────────
        _stackedWidget = new QStackedWidget(this);

        // Crear las 21 categorías exactas de Notepad++ Windows
        addCategory("General",                   createGeneralPage());
        addCategory("Barra de herramientas",     createToolbarPage());
        addCategory("Barra de estado",           createStatusbarPage());
        addCategory("Edición 1",                 createEditing1Page());
        addCategory("Edición 2",                 createEditing2Page());
        addCategory("Modo oscuro",               createDarkModePage());
        addCategory("Margen, fuente y línea",    createMarginPage());
        addCategory("Nuevo documento",           createNewDocPage());
        addCategory("Carpeta predeterminada",    createDefaultDirPage());
        addCategory("Archivos recientes",        createRecentFilesPage());
        addCategory("Asociación de archivos",    createFileAssocPage());
        addCategory("Lenguaje",                  createLanguagePage());
        addCategory("Impresión",                 createPrintPage());
        addCategory("Búsqueda",                  createSearchPage());
        addCategory("Copias de seguridad",       createBackupPage());
        addCategory("Autocompletado",            createAutoCompletePage());
        addCategory("Multi-instancia & Fecha",   createMultiInstancePage());
        addCategory("Delimitador",               createDelimiterPage());
        addCategory("Rendimiento",               createPerformancePage());
        addCategory("Lista de paneles",          createPanelListPage());
        addCategory("Varios (MISC)",             createMiscPage());

        connect(_categoryList, &QListWidget::currentRowChanged, _stackedWidget, &QStackedWidget::setCurrentIndex);
        _categoryList->setCurrentRow(0);

        contentLayout->addWidget(_categoryList);
        contentLayout->addWidget(_stackedWidget, 1);
        mainLayout->addLayout(contentLayout);

        // ── Botón Cerrar ─────────────────────────────────────────────────────
        auto* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        auto* btnClose = new QPushButton("Cerrar", this);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
        btnLayout->addWidget(btnClose);
        mainLayout->addLayout(btnLayout);
    }

    void addCategory(const QString& name, QWidget* page) {
        _categoryList->addItem(name);
        _stackedWidget->addWidget(page);
    }

    // 1. General
    QWidget* createGeneralPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Interfaz de Usuario", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("Idioma de la interfaz: Español", grp));
        v->addWidget(new QCheckBox("Mostrar botón de cerrar en cada pestaña", grp));
        v->addWidget(new QCheckBox("Doble clic para cerrar pestaña", grp));
        
        auto* chkHideMenu = new QCheckBox("Ocultar barra de menú (presione Alt para mostrar)", grp);
        connect(chkHideMenu, &QCheckBox::toggled, [this](bool checked) {
            emit menuBarVisibilityChanged(!checked);
        });
        v->addWidget(chkHideMenu);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 2. Barra de herramientas
    QWidget* createToolbarPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Opciones de la Barra de Herramientas", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkHide = new QCheckBox("Ocultar barra de herramientas", grp);
        connect(chkHide, &QCheckBox::toggled, [this](bool checked) {
            emit toolbarVisibilityChanged(!checked);
        });
        v->addWidget(chkHide);

        auto* r1 = new QRadioButton("Iconos pequeños sin relleno", grp);
        auto* r2 = new QRadioButton("Iconos grandes sin relleno", grp);
        auto* r3 = new QRadioButton("Iconos pequeños con relleno", grp);
        auto* r4 = new QRadioButton("Iconos grandes con relleno", grp);
        auto* r5 = new QRadioButton("Iconos pequeños predeterminados", grp);
        r5->setChecked(true);

        connect(r1, &QRadioButton::toggled, [this](bool checked) { if (checked) emit toolbarPresetChanged(0); });
        connect(r2, &QRadioButton::toggled, [this](bool checked) { if (checked) emit toolbarPresetChanged(1); });
        connect(r3, &QRadioButton::toggled, [this](bool checked) { if (checked) emit toolbarPresetChanged(2); });
        connect(r4, &QRadioButton::toggled, [this](bool checked) { if (checked) emit toolbarPresetChanged(3); });
        connect(r5, &QRadioButton::toggled, [this](bool checked) { if (checked) emit toolbarPresetChanged(4); });


        v->addWidget(r1); v->addWidget(r2); v->addWidget(r3); v->addWidget(r4); v->addWidget(r5);
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 3. Barra de estado
    QWidget* createStatusbarPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Barra de Estado", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chk = new QCheckBox("Mostrar barra de estado", grp);
        chk->setChecked(true);
        connect(chk, &QCheckBox::toggled, [this](bool checked) {
            emit statusbarVisibilityChanged(checked);
        });
        v->addWidget(chk);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 4. Edición 1
    QWidget* createEditing1Page() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Configuración de Tabulación y Cursor", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* h = new QHBoxLayout();
        h->addWidget(new QLabel("Tamaño de tabulación (espacios):", grp));
        auto* spin = new QSpinBox(grp);
        spin->setRange(1, 16); spin->setValue(4);
        connect(spin, &QSpinBox::valueChanged, [this](int val) {
            emit tabSizeChanged(val);
        });
        h->addWidget(spin);

        auto* chkSpace = new QCheckBox("Reemplazar por espacios", grp);
        connect(chkSpace, &QCheckBox::toggled, [this](bool checked) {
            emit tabUseSpacesChanged(checked);
        });
        h->addWidget(chkSpace);
        h->addStretch();
        v->addLayout(h);

        auto* chkSmart = new QCheckBox("Habilitar sangría automática inteligente", grp);
        chkSmart->setChecked(true);
        connect(chkSmart, &QCheckBox::toggled, [this](bool checked) {
            emit autoIndentChanged(checked);
        });
        v->addWidget(chkSmart);
        
        auto* chkGuide = new QCheckBox("Mostrar guía de sangría vertical", grp);
        connect(chkGuide, &QCheckBox::toggled, [this](bool checked) {
            emit indentGuidesChanged(checked);
        });
        v->addWidget(chkGuide);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 5. Edición 2
    QWidget* createEditing2Page() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Ajustes Avanzados de Edición", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkCaret = new QCheckBox("Resaltar línea actual del cursor", grp);
        connect(chkCaret, &QCheckBox::toggled, [this](bool checked) {
            emit caretLineHighlightChanged(checked);
        });
        v->addWidget(chkCaret);

        auto* chkWrap = new QCheckBox("Activar ajuste de línea automático (Word Wrap)", grp);
        connect(chkWrap, &QCheckBox::toggled, [this](bool checked) {
            emit wordWrapChanged(checked);
        });
        v->addWidget(chkWrap);

        auto* chkEnd = new QCheckBox("Habilitar desplazamiento continuo más allá del final", grp);
        connect(chkEnd, &QCheckBox::toggled, [this](bool checked) {
            emit scrollPastEndChanged(checked);
        });
        v->addWidget(chkEnd);
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 6. Modo oscuro
    QWidget* createDarkModePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Configuración de Modo Oscuro", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkDark = new QCheckBox("Habilitar modo oscuro nativo", grp);
        chkDark->setChecked(true);
        connect(chkDark, &QCheckBox::toggled, [this](bool checked) {
            emit darkModeToggled(checked);
        });
        v->addWidget(chkDark);

        auto* rCharcoal = new QRadioButton("Tono oscuro estándar (Dark Charcoal)", grp);
        auto* rOled     = new QRadioButton("Tono negro profundo (OLED Black)", grp);
        rCharcoal->setChecked(true);

        connect(rCharcoal, &QRadioButton::toggled, [this](bool checked) {
            if (checked) emit darkModeToneChanged(0);
        });
        connect(rOled, &QRadioButton::toggled, [this](bool checked) {
            if (checked) emit darkModeToneChanged(1);
        });

        v->addWidget(rCharcoal);
        v->addWidget(rOled);
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }


    // 7. Margen, fuente y línea
    QWidget* createMarginPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Márgenes y Estilos de Plegado", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkLines = new QCheckBox("Mostrar margen de números de línea", grp);
        chkLines->setChecked(true);
        connect(chkLines, &QCheckBox::toggled, [this](bool checked) {
            emit lineNumbersVisibilityChanged(checked);
        });
        v->addWidget(chkLines);

        auto* chkBm = new QCheckBox("Mostrar margen de marcadores (Bookmarks)", grp);
        chkBm->setChecked(true);
        connect(chkBm, &QCheckBox::toggled, [this](bool checked) {
            emit bookmarkMarginVisibilityChanged(checked);
        });
        v->addWidget(chkBm);

        auto* chkFold = new QCheckBox("Habilitar árbol de plegado de código", grp);
        chkFold->setChecked(true);
        connect(chkFold, &QCheckBox::toggled, [this](bool checked) {
            emit codeFoldingToggled(checked);
        });
        v->addWidget(chkFold);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 8. Nuevo documento
    QWidget* createNewDocPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Formato de Nuevo Documento", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* hEol = new QHBoxLayout();
        hEol->addWidget(new QLabel("Fin de línea (EOL):", grp));
        auto* cbEol = new QComboBox(grp);
        cbEol->addItems({"Unix (LF)", "Windows (CR LF)", "Macintosh (CR)"});
        connect(cbEol, &QComboBox::currentIndexChanged, [this](int idx) {
            emit eolModeChanged(idx);
        });
        hEol->addWidget(cbEol);
        hEol->addStretch();
        v->addLayout(hEol);

        auto* hEnc = new QHBoxLayout();
        hEnc->addWidget(new QLabel("Codificación predeterminada:", grp));
        auto* cbEnc = new QComboBox(grp);
        cbEnc->addItems({"UTF-8 sin BOM", "UTF-8 con BOM", "ANSI"});
        connect(cbEnc, &QComboBox::currentIndexChanged, [this](int idx) {
            emit defaultEncodingChanged(idx);
        });
        hEnc->addWidget(cbEnc);
        hEnc->addStretch();
        v->addLayout(hEnc);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 9. Carpeta predeterminada
    QWidget* createDefaultDirPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Directorio de Apertura/Guardado", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QRadioButton("Seguir el directorio del archivo actual", grp));
        v->addWidget(new QRadioButton("Recordar el último directorio utilizado", grp));
        v->addWidget(new QRadioButton("Ruta de directorio personalizada:", grp));
        v->addWidget(new QLineEdit("/home", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 10. Archivos recientes
    QWidget* createRecentFilesPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Historial de Archivos Recientes", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* h = new QHBoxLayout();
        h->addWidget(new QLabel("Número máximo de entradas en el historial:", grp));
        auto* spin = new QSpinBox(grp);
        spin->setRange(1, 30); spin->setValue(15);
        connect(spin, &QSpinBox::valueChanged, [this](int val) {
            emit recentFilesMaxCountChanged(val);
        });
        h->addWidget(spin);
        h->addStretch();
        v->addLayout(h);

        v->addWidget(new QCheckBox("Mostrar solo nombre de archivo en submenú", grp));
        v->addWidget(new QCheckBox("Mostrar ruta completa en submenú", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }


    // 11. Asociación de archivos
    QWidget* createFileAssocPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Asociaciones de Extensión en el Sistema", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QLabel("Extensiones registradas para Notepad++:", grp));
        v->addWidget(new QCheckBox(".txt - Archivos de texto plano", grp));
        v->addWidget(new QCheckBox(".cpp / .h - Código C / C++", grp));
        v->addWidget(new QCheckBox(".py - Scripts en Python", grp));
        v->addWidget(new QCheckBox(".json / .xml / .yaml - Documentos estructurados", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 12. Lenguaje
    QWidget* createLanguagePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Menú y Resaltado de Lenguaje", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("Ignorar mayúsculas y minúsculas en palabras clave", grp));
        v->addWidget(new QCheckBox("Habilitar resaltado de sintaxis global", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 13. Impresión
    QWidget* createPrintPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Opciones de Impresión de Documentos", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("Imprimir números de línea", grp));
        v->addWidget(new QCheckBox("Imprimir en blanco y negro (ahorro de tinta)", grp));
        v->addWidget(new QCheckBox("Incluir encabezado y pie de página", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 14. Búsqueda
    QWidget* createSearchPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Opciones de Búsqueda y Reemplazo", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("No cerrar diálogo de búsqueda tras buscar", grp));
        v->addWidget(new QCheckBox("Resaltar todas las coincidencias automáticamente", grp));
        v->addWidget(new QCheckBox("Habilitar expresiones regulares de 16 bits por defecto", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 15. Copias de seguridad
    QWidget* createBackupPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Respaldo y Auto-Guardado", page);
        auto* v = new QVBoxLayout(grp);
        auto* chkAutoSave = new QCheckBox("Activar guardado automático en segundo plano", grp);
        auto* chkBackup   = new QCheckBox("Crear copia de seguridad al guardar (Backup)", grp);

        connect(chkAutoSave, &QCheckBox::toggled, [this](bool checked) {
            emit autoSaveToggled(checked);
        });
        connect(chkBackup, &QCheckBox::toggled, [this](bool checked) {
            emit autoBackupToggled(checked);
        });

        v->addWidget(chkAutoSave);
        v->addWidget(chkBackup);
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 16. Autocompletado
    QWidget* createAutoCompletePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Auto-completado de Código", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkAC = new QCheckBox("Habilitar autocompletado en cada entrada de texto", grp);
        chkAC->setChecked(true);
        connect(chkAC, &QCheckBox::toggled, [this](bool checked) {
            emit autoCompletionToggled(checked);
        });
        v->addWidget(chkAC);

        auto* chkQuotes  = new QCheckBox("Cierre automático de comillas \"\" y ''", grp);
        auto* chkBrackets = new QCheckBox("Cierre automático de paréntesis () y corchetes []", grp);
        chkQuotes->setChecked(true);
        chkBrackets->setChecked(true);

        connect(chkQuotes, &QCheckBox::toggled, [this](bool checked) {
            emit autoInsertPairsToggled(checked);
        });
        connect(chkBrackets, &QCheckBox::toggled, [this](bool checked) {
            emit autoInsertPairsToggled(checked);
        });

        v->addWidget(chkQuotes);
        v->addWidget(chkBrackets);
        v->addWidget(new QCheckBox("Cierre automático de etiquetas HTML/XML </>", grp));


        auto* h = new QHBoxLayout();
        h->addWidget(new QLabel("Escribir caracteres desde:", grp));
        auto* spin = new QSpinBox(grp);
        spin->setRange(1, 9); spin->setValue(2);
        connect(spin, &QSpinBox::valueChanged, [this](int val) {
            emit autoCompletionThresholdChanged(val);
        });
        h->addWidget(spin);
        h->addStretch();
        v->addLayout(h);

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 17. Multi-instancia & Fecha
    QWidget* createMultiInstancePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Modo Multi-Instancia y Formato de Fecha", page);
        auto* v = new QVBoxLayout(grp);
        auto* rbSingle = new QRadioButton("Modo instancia única (Predeterminado)", grp);
        auto* rbMulti  = new QRadioButton("Permitir múltiples instancias independientes", grp);

        rbSingle->setChecked(true);
        v->addWidget(rbSingle);
        v->addWidget(rbMulti);

        connect(rbSingle, &QRadioButton::toggled, [this](bool checked) {
            emit singleInstanceModeToggled(checked);
        });

        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }


    // 18. Delimitador
    QWidget* createDelimiterPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Selección por Delimitadores", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("Selección en bloque mediante doble clic", grp));
        v->addWidget(new QCheckBox("Incluir delimitadores personalizados en la selección", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 19. Rendimiento
    QWidget* createPerformancePage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Optimización de Archivos Grandes", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* h = new QHBoxLayout();
        h->addWidget(new QLabel("Límite de tamaño para modo rendimiento (MB):", grp));
        auto* spin = new QSpinBox(grp);
        spin->setRange(1, 2000); spin->setValue(200);
        h->addWidget(spin);
        h->addStretch();
        v->addLayout(h);

        v->addWidget(new QCheckBox("Desactivar resaltado sintáctico en archivos gigantes", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 20. Lista de paneles
    QWidget* createPanelListPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Paneles Laterales e Integración", page);
        auto* v = new QVBoxLayout(grp);
        v->addWidget(new QCheckBox("Mostrar icono en la barra de pestañas laterales", grp));
        v->addWidget(new QCheckBox("Recordar estado de paneles acoplados al salir", grp));
        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }

    // 21. Varios (MISC)
    QWidget* createMiscPage() {
        auto* page = new QWidget();
        auto* layout = new QVBoxLayout(page);
        auto* grp = new QGroupBox("Opciones Varias (MISC)", page);
        auto* v = new QVBoxLayout(grp);
        
        auto* chkTray = new QCheckBox("Minimizar a la bandeja del sistema (System Tray)", grp);
        connect(chkTray, &QCheckBox::toggled, [this](bool checked) {
            emit systemTrayToggled(checked);
        });
        v->addWidget(chkTray);

        v->addWidget(new QCheckBox("Comprobar actualizaciones automáticamente al iniciar", grp));

        auto* chkSess = new QCheckBox("Recordar sesión actual de archivos al reiniciar", grp);
        chkSess->setChecked(true);
        connect(chkSess, &QCheckBox::toggled, [this](bool checked) {
            emit rememberSessionToggled(checked);
        });
        v->addWidget(chkSess);


        layout->addWidget(grp);
        layout->addStretch();
        return page;
    }


private:
    QListWidget*    _categoryList  = nullptr;
    QStackedWidget* _stackedWidget = nullptr;
};

#endif // NPP_PLATFORM_LINUX

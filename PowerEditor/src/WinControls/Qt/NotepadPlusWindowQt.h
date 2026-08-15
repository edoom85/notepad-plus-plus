// WinControls/Qt/NotepadPlusWindowQt.h — Ventana principal de Notepad++ en Qt6
// Reemplaza Notepad_plus_Window.h/.cpp en Linux
//
// Ensambla todos los componentes Qt6:
//   QMainWindow + NppTabWidget + QsciScintilla + NppToolBar +
//   NppStatusBar + NppSplitter + NppDockWidget
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "../Platform/PlatformMsg.h"

#ifdef NPP_PLATFORM_LINUX

#include "NppStatusBar.h"
#include "NppTabBar.h"
#include "NppToolBar.h"
#include "NppSplitter.h"
#include "NppDockWidget.h"
#include "NppTreeView.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QFileInfo>
#include <QTextStream>
#include <QFile>
#include <QScreen>
#include <QShortcut>
#include <QString>

// Forward declaration de QScintilla
class QsciScintilla;
class QsciLexerCPP;

/// Ventana principal de Notepad++ en Qt6.
/// Equivale a Notepad_plus_Window + Notepad_plus en la versión Win32.
class NotepadPlusWindowQt : public QMainWindow {
    Q_OBJECT

public:
    explicit NotepadPlusWindowQt(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Notepad++ [Linux Port]");
        setMinimumSize(640, 400);

        // ── 1. Menu Bar ─────────────────────────────────────────────────────
        createMenus();

        // ── 2. Tool Bar ─────────────────────────────────────────────────────
        _toolbar = new NppToolBar("Main Toolbar", this);
        _toolbar->addStandardButtons();
        addToolBar(Qt::TopToolBarArea, _toolbar);

        connect(_toolbar, &NppToolBar::commandTriggered,
                this, &NotepadPlusWindowQt::onToolbarCommand);

        // ── 3. Splitter principal con dos TabWidgets ─────────────────────────
        _splitter = new NppSplitter(Qt::Horizontal, this);
        _mainTabs = new NppTabWidget(this);
        _subTabs  = new NppTabWidget(this);
        _splitter->addPanel(_mainTabs);
        _splitter->addPanel(_subTabs);
        _subTabs->hide(); // sub-view oculta por defecto (como en Npp)
        setCentralWidget(_splitter);

        connect(_mainTabs, &NppTabWidget::closeTabRequested,
                this, &NotepadPlusWindowQt::onCloseTab);
        connect(_mainTabs, &QTabWidget::currentChanged,
                this, &NotepadPlusWindowQt::onTabChanged);

        // ── 4. Status Bar ───────────────────────────────────────────────────
        _statusBar = new NppStatusBar(this);
        setStatusBar(_statusBar->statusBar());
        updateStatusBar();

        // ── 5. Crear pestaña vacía por defecto ──────────────────────────────
        newDocument();

        // ── 6. Restaurar geometría ──────────────────────────────────────────
        restoreWindowState();

        // ── 7. Conectar bus de mensajes ─────────────────────────────────────
        NppMsgBus::instance().subscribe(NppMsg::FILE_OPEN,
            [this](NppWparam /*wp*/, NppLparam lp) -> NppLresult {
                auto* pathPtr = reinterpret_cast<std::string*>(lp);
                if (pathPtr) openFile(*pathPtr);
                return 0;
            });
    }

    // ── Operaciones de archivo ──────────────────────────────────────────────

    /// Abre un archivo en una nueva pestaña.
    void openFile(const NppString& filePath) {
        QFile file(QString::fromStdString(filePath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error",
                "No se pudo abrir: " + QString::fromStdString(filePath));
            return;
        }
        QTextStream stream(&file);
        QString content = stream.readAll();
        file.close();

        QFileInfo fi(QString::fromStdString(filePath));

        // Crear editor (placeholder hasta QScintilla esté integrado)
        auto* editor = createEditor();
        // TODO: editor->setText(content); cuando QScintilla esté integrado

        int idx = _mainTabs->addTab(editor, fi.fileName().toStdString());
        _mainTabs->setTabFilePath(idx, filePath);
        _mainTabs->setCurrentIndex(idx);

        updateStatusBar();
        NppMsgBus::instance().post(NppMsg::BUFFER_SWITCH,
            static_cast<NppWparam>(idx), 0);
    }

    /// Crea un nuevo documento vacío.
    void newDocument() {
        static int newCount = 1;
        auto* editor = createEditor();
        NppString name = "new " + std::to_string(newCount++);
        int idx = _mainTabs->addTab(editor, name);
        _mainTabs->setCurrentIndex(idx);
        updateStatusBar();
    }

protected:
    void closeEvent(QCloseEvent* event) override {
        saveWindowState();
        NppMsgBus::instance().send(NppMsg::SHUTDOWN);
        event->accept();
    }

private:
    // ── Crear menús ─────────────────────────────────────────────────────────
    void createMenus() {
        // Archivo
        QMenu* fileMenu = menuBar()->addMenu("&Archivo");
        fileMenu->addAction("&Nuevo",    this, &NotepadPlusWindowQt::newDocument,
                           QKeySequence::New);
        fileMenu->addAction("&Abrir...", this, &NotepadPlusWindowQt::onOpen,
                           QKeySequence::Open);
        fileMenu->addSeparator();
        fileMenu->addAction("&Salir",    qApp, &QApplication::quit,
                           QKeySequence::Quit);

        // Editar
        QMenu* editMenu = menuBar()->addMenu("&Editar");
        editMenu->addAction("&Deshacer", [](){}, QKeySequence::Undo);
        editMenu->addAction("&Rehacer",  [](){}, QKeySequence::Redo);
        editMenu->addSeparator();
        editMenu->addAction("&Cortar",   [](){}, QKeySequence::Cut);
        editMenu->addAction("Co&piar",   [](){}, QKeySequence::Copy);
        editMenu->addAction("&Pegar",    [](){}, QKeySequence::Paste);

        // Buscar
        QMenu* searchMenu = menuBar()->addMenu("&Buscar");
        searchMenu->addAction("&Buscar...", [](){}, QKeySequence::Find);
        searchMenu->addAction("&Reemplazar...", [](){}, QKeySequence::Replace);

        // Vista
        QMenu* viewMenu = menuBar()->addMenu("&Vista");
        viewMenu->addAction("Dividir &Horizontalmente", [this]() {
            _subTabs->setVisible(!_subTabs->isVisible());
            if (_subTabs->isVisible()) _splitter->setRatio(0.5);
        });

        // Lenguaje
        menuBar()->addMenu("&Lenguaje");

        // Plugins
        menuBar()->addMenu("&Plugins");

        // Ayuda
        QMenu* helpMenu = menuBar()->addMenu("A&yuda");
        helpMenu->addAction("&Acerca de...", [this]() {
            QMessageBox::about(this, "Acerca de Notepad++ Linux Port",
                "Notepad++ Linux Port\n"
                "Basado en Notepad++ por Don Ho\n"
                "Port a Linux con Qt6 + QScintilla\n"
                "Licencia: GPL v3");
        });

        // Estilo oscuro para menús
        menuBar()->setStyleSheet(
            "QMenuBar {"
            "  background: #333333;"
            "  color: #D4D4D4;"
            "}"
            "QMenuBar::item:selected {"
            "  background: #505050;"
            "}"
            "QMenu {"
            "  background: #252526;"
            "  color: #D4D4D4;"
            "  border: 1px solid #3C3C3C;"
            "}"
            "QMenu::item:selected {"
            "  background: #094771;"
            "}"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: #3C3C3C;"
            "}"
        );
    }

    // ── Crear editor (placeholder; reemplazar con QsciScintilla) ────────────
    QWidget* createEditor() {
        // TODO: Reemplazar con QsciScintilla cuando esté integrado
        auto* widget = new QWidget(this);
        widget->setStyleSheet("background: #1E1E1E;");
        return widget;
    }

    // ── Actualizar barra de estado ──────────────────────────────────────────
    void updateStatusBar() {
        int idx = _mainTabs->currentIndex();
        if (idx < 0) return;

        NppString path = _mainTabs->tabFilePath(idx);
        _statusBar->setText(0, path.empty() ? "Sin guardar" : path);
        _statusBar->setText(1, "Ln: 1  Col: 1");
        _statusBar->setText(2, "UTF-8");
        _statusBar->setText(3, "Unix (LF)");
        _statusBar->setText(4, "INS");
    }

    // ── Guardar/restaurar estado de ventana ─────────────────────────────────
    void saveWindowState() {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }

    void restoreWindowState() {
        QSettings settings;
        if (settings.contains("geometry")) {
            restoreGeometry(settings.value("geometry").toByteArray());
            restoreState(settings.value("windowState").toByteArray());
        } else {
            // Primera ejecución: centrar ventana
            QSize screen = QApplication::primaryScreen()->size();
            resize(1200, 800);
            move((screen.width() - 1200) / 2, (screen.height() - 800) / 2);
        }
    }

private slots:
    void onOpen() {
        QStringList paths = QFileDialog::getOpenFileNames(this, "Abrir archivo");
        for (const auto& p : paths) {
            openFile(p.toStdString());
        }
    }

    void onCloseTab(int index) {
        // TODO: verificar si hay cambios sin guardar
        QWidget* w = _mainTabs->widget(index);
        _mainTabs->removeTab(index);
        delete w;

        if (_mainTabs->count() == 0) newDocument();
        updateStatusBar();
    }

    void onTabChanged(int /*index*/) {
        updateStatusBar();
    }

    void onToolbarCommand(int cmdId) {
        switch (cmdId) {
            case 1: newDocument(); break;
            case 2: onOpen(); break;
            case 3: /* TODO: guardar */ break;
            case 6: /* TODO: buscar */ break;
        }
    }

    // ── Componentes ─────────────────────────────────────────────────────────
private:
    NppToolBar*    _toolbar   = nullptr;
    NppSplitter*   _splitter  = nullptr;
    NppTabWidget*  _mainTabs  = nullptr;
    NppTabWidget*  _subTabs   = nullptr;
    NppStatusBar*  _statusBar = nullptr;
};

#endif // NPP_PLATFORM_LINUX

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

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

#include "../../Platform/PlatformIconProvider.h"
#include "../../Platform/PlatformLexerManager.h"
#include "../../Platform/PlatformTheme.h"
#include "NppAboutDlg.h"

#include "NppFindReplaceDlg.h"
#include "NppPreferenceDlg.h"
#include "NppShortcutMapper.h"
#include "NppGoToLineDlg.h"
#include "NppRunDlg.h"
#include "NppPluginsAdmin.h"
#include "NppFileBrowser.h"
#include "NppFunctionList.h"
#include "NppProjectPanel.h"
#include "NppClipboardHistory.h"
#include "NppUserDefineDlg.h"

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

        auto* widget = createEditor();
        auto* editor = qobject_cast<QsciScintilla*>(widget);
        if (editor) {
            editor->setText(content);
            // Auto-detectar extensión (.json, .py, .cpp, .html, .xml, etc.) y aplicar lexer
            NppLexerManager::Language detectedLang = NppLexerManager::detectFromExtension(QString::fromStdString(filePath));
            NppLexerManager::applyLanguage(editor, detectedLang);
        }


        int idx = _mainTabs->addTab(widget, fi.fileName().toStdString());
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
    /// Obtiene el widget QsciScintilla activo actualmente.
    QsciScintilla* activeEditor() const {
        QWidget* w = _mainTabs->currentWidget();
        return qobject_cast<QsciScintilla*>(w);
    }

private:
    // ── Crear menús ─────────────────────────────────────────────────────────
    void createMenus() {
        // Desactivar el menú global nativo del sistema operativo (KDE/GNOME DBus AppMenu)
        // para renderizar el menú directamente dentro de la ventana de Notepad++ y garantizar
        // respuesta inmediata al clic sin interferencias externas de DBus.
        menuBar()->setNativeMenuBar(false);

        // ── Archivo ──
        QMenu* fileMenu = menuBar()->addMenu("&Archivo");

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::New),  "&Nuevo",          QKeySequence::New,  this, &NotepadPlusWindowQt::newDocument);
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Open), "&Abrir...",       QKeySequence::Open, this, &NotepadPlusWindowQt::onOpen);
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Save), "&Guardar",        QKeySequence::Save, [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0 && activeEditor()) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (path.empty()) {
                    QString selected = QFileDialog::getSaveFileName(this, "Guardar como");
                    if (!selected.isEmpty()) {
                        path = selected.toStdString();
                        _mainTabs->setTabFilePath(idx, path);
                        _mainTabs->setTabText(idx, QFileInfo(selected).fileName());
                    }
                }
                if (!path.empty()) {
                    QFile file(QString::fromStdString(path));
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&file);
                        out << activeEditor()->text();
                        file.close();
                    }
                }
            }
        });
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "Cerrar pestaña", QKeySequence::Close, [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) onCloseTab(idx);
        });
        fileMenu->addSeparator();
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "&Salir",        QKeySequence::Quit, qApp, &QApplication::quit);

        // ── Editar ──
        QMenu* editMenu = menuBar()->addMenu("&Editar");
        editMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Undo), "&Deshacer", QKeySequence::Undo, [this]() {
            if (auto* ed = activeEditor()) ed->undo();
        });
        editMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Redo), "&Rehacer",  QKeySequence::Redo, [this]() {
            if (auto* ed = activeEditor()) ed->redo();
        });
        editMenu->addSeparator();
        editMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Cut),   "&Cortar",   QKeySequence::Cut, [this]() {
            if (auto* ed = activeEditor()) ed->cut();
        });
        editMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Copy),  "Co&piar",   QKeySequence::Copy, [this]() {
            if (auto* ed = activeEditor()) ed->copy();
        });
        editMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Paste), "&Pegar",    QKeySequence::Paste, [this]() {
            if (auto* ed = activeEditor()) ed->paste();
        });
        editMenu->addSeparator();
        editMenu->addAction("Seleccionar &todo", QKeySequence::SelectAll, [this]() {
            if (auto* ed = activeEditor()) ed->selectAll();
        });

        // ── Buscar ──
        QMenu* searchMenu = menuBar()->addMenu("&Buscar");
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Find),    "&Buscar...",     QKeySequence::Find, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Replace), "&Reemplazar...", QKeySequence::Replace, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction("Ir a &línea...", QKeySequence(Qt::CTRL | Qt::Key_G), [this]() {
            auto* dlg = new NppGoToLineDlg(this);
            if (activeEditor()) dlg->setInfo(activeEditor()->firstVisibleLine() + 1, activeEditor()->lines());
            connect(dlg, &NppGoToLineDlg::goToLine, [this](int line) {
                if (auto* ed = activeEditor()) ed->setCursorPosition(line - 1, 0);
            });
            dlg->show();
        });

        // ── Vista ──
        QMenu* viewMenu = menuBar()->addMenu("&Vista");
        viewMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::ZoomIn),  "Acercar &Zoom", QKeySequence::ZoomIn, [this]() {
            if (auto* ed = activeEditor()) ed->zoomIn();
        });
        viewMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::ZoomOut), "Alejar Z&oom",  QKeySequence::ZoomOut, [this]() {
            if (auto* ed = activeEditor()) ed->zoomOut();
        });
        viewMenu->addSeparator();
        viewMenu->addAction("Dividir &Horizontalmente", [this]() {
            _subTabs->setVisible(!_subTabs->isVisible());
            if (_subTabs->isVisible()) _splitter->setRatio(0.5);
        });

        // ── Lenguaje ──
        QMenu* langMenu = menuBar()->addMenu("&Lenguaje");
        langMenu->addAction("Texto plano (Normal Text)", [this]() {
            if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText);
        });
        langMenu->addSeparator();

        // Submenú C
        QMenu* menuC = langMenu->addMenu("C");
        menuC->addAction("C",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C++", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("CSS", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CSS); });

        // Submenú H
        QMenu* menuH = langMenu->addMenu("H");
        menuH->addAction("HTML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::HTML); });

        // Submenú J
        QMenu* menuJ = langMenu->addMenu("J");
        menuJ->addAction("Java",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Java); });
        menuJ->addAction("JavaScript", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JavaScript); });
        menuJ->addAction("JSON",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JSON); });

        // Submenú M
        QMenu* menuM = langMenu->addMenu("M");
        menuM->addAction("Markdown", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });

        // Submenú P
        QMenu* menuP = langMenu->addMenu("P");
        menuP->addAction("Python", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Python); });

        // Submenú S
        QMenu* menuS = langMenu->addMenu("S");
        menuS->addAction("Shell (Bash)", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuS->addAction("SQL",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::SQL); });

        // Submenú X
        QMenu* menuX = langMenu->addMenu("X");
        menuX->addAction("XML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::XML); });

        // Submenú Y
        QMenu* menuY = langMenu->addMenu("Y");
        menuY->addAction("YAML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::YAML); });

        langMenu->addSeparator();
        langMenu->addAction("Definir tu lenguaje (UDL)...", [this]() {
            auto* dlg = new NppUserDefineDlg(this);
            dlg->show();
        });

        // ── Ejecutar / Herramientas ──
        QMenu* runMenu = menuBar()->addMenu("&Ejecutar");
        runMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Run), "&Ejecutar comando...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R), [this]() {
            auto* dlg = new NppRunDlg(this);
            dlg->show();
        });


        // ── Plugins ──
        QMenu* pluginsMenu = menuBar()->addMenu("&Plugins");
        pluginsMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Plugins), "Administrador de &Plugins...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });

        // ── Configuración ──
        QMenu* configMenu = menuBar()->addMenu("&Configuración");
        configMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Settings), "&Preferencias...", [this]() {
            auto* dlg = new NppPreferenceDlg(this);
            dlg->show();
        });
        configMenu->addAction("Mapeador de &Atajos...", [this]() {
            auto* dlg = new NppShortcutMapper(this);
            dlg->show();
        });

        // ── Ayuda ──
        QMenu* helpMenu = menuBar()->addMenu("A&yuda");
        helpMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::About), "&Acerca de Notepad++...", [this]() {
            auto* dlg = new NppAboutDlg(this);
            dlg->exec();
        });

        // Estilo adaptativo para menús (Modo Oscuro / Modo Claro)
        if (NppTheme::isDarkMode()) {
            menuBar()->setStyleSheet(
                "QMenuBar { background: #333333; color: #D4D4D4; }"
                "QMenuBar::item:selected { background: #505050; }"
                "QMenu { background: #252526; color: #D4D4D4; border: 1px solid #3C3C3C; }"
                "QMenu::item:selected { background: #094771; }"
                "QMenu::separator { height: 1px; background: #3C3C3C; }"
            );
        } else {
            menuBar()->setStyleSheet(
                "QMenuBar { background: #F0F0F0; color: #000000; }"
                "QMenuBar::item:selected { background: #E0E0E0; }"
                "QMenu { background: #FFFFFF; color: #000000; border: 1px solid #CCCCCC; }"
                "QMenu::item:selected { background: #007ACC; color: #FFFFFF; }"
                "QMenu::separator { height: 1px; background: #E0E0E0; }"
            );
        }
    }



    // ── Crear editor QsciScintilla real ─────────────────────────────────────
    QWidget* createEditor() {
        auto* editor = new QsciScintilla(this);

        // Fuente monoespaciada
        QFont font("JetBrains Mono", 11);
        if (!font.exactMatch()) font = QFont("Monospace", 11);
        editor->setFont(font);

        // Colores de margen y papel adaptativos (Modo Oscuro / Modo Claro)
        QColor marginBg = NppTheme::marginBackgroundColor();
        QColor marginFg = NppTheme::marginForegroundColor();
        QColor paperBg  = NppTheme::paperColor();
        QColor textFg   = NppTheme::textColor();

        // Márgenes — números de línea y plegado de código (folding)
        editor->setMarginType(0, QsciScintilla::NumberMargin);
        editor->setMarginWidth(0, "00000");
        editor->setMarginsForegroundColor(marginFg);
        editor->setMarginsBackgroundColor(marginBg);
        editor->setMarginsFont(font);

        // Ocultar margen de marcadores 1 y configurar color adaptativo para el margen 2 de plegado
        editor->setMarginWidth(1, 0);
        editor->setFoldMarginColors(marginBg, marginBg);

        // Colores del editor (dinámico según modo oscuro / claro)
        editor->setPaper(paperBg);
        editor->setColor(textFg);

        editor->setCaretForegroundColor(QColor(0xFF, 0xFF, 0xFF));
        editor->setSelectionBackgroundColor(QColor(0x26, 0x4F, 0x78));
        editor->setSelectionForegroundColor(QColor(0xFF, 0xFF, 0xFF));
        editor->setCaretLineVisible(true);
        editor->setCaretLineBackgroundColor(QColor(0x28, 0x28, 0x28));

        // Indentación
        editor->setTabWidth(4);
        editor->setIndentationsUseTabs(false);
        editor->setAutoIndent(true);
        editor->setIndentationGuides(true);
        editor->setIndentationGuidesBackgroundColor(QColor(0x40, 0x40, 0x40));
        editor->setIndentationGuidesForegroundColor(QColor(0x40, 0x40, 0x40));

        // Brackets matching & folding
        editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
        editor->setMatchedBraceBackgroundColor(QColor(0x3A, 0x3A, 0x3A));
        editor->setMatchedBraceForegroundColor(QColor(0xFF, 0xD7, 0x00));
        editor->setFolding(QsciScintilla::BoxedTreeFoldStyle, 2);

        // Lexer C++ por defecto
        auto* lexer = new QsciLexerCPP(editor);
        lexer->setFont(font);
        lexer->setColor(QColor(0xD4, 0xD4, 0xD4), QsciLexerCPP::Default);
        lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::Comment);
        lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::CommentLine);
        lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::CommentDoc);
        lexer->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerCPP::Number);
        lexer->setColor(QColor(0xD6, 0xD8, 0x85), QsciLexerCPP::DoubleQuotedString);
        lexer->setColor(QColor(0xD6, 0x9D, 0x85), QsciLexerCPP::SingleQuotedString);
        lexer->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerCPP::Keyword);
        lexer->setColor(QColor(0x9B, 0x9B, 0x9B), QsciLexerCPP::PreProcessor);
        lexer->setColor(QColor(0x4E, 0xC9, 0xB0), QsciLexerCPP::Identifier);

        for (int i = 0; i <= QsciLexerCPP::TaskMarker; ++i)
            lexer->setPaper(QColor(0x1E, 0x1E, 0x1E), i);

        editor->setLexer(lexer);
        return editor;
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
            case 1:  newDocument(); break;
            case 2:  onOpen(); break;
            case 3:  {
                int idx = _mainTabs->currentIndex();
                if (idx >= 0 && activeEditor()) {
                    NppString path = _mainTabs->tabFilePath(idx);
                    if (path.empty()) {
                        QString selected = QFileDialog::getSaveFileName(this, "Guardar como");
                        if (!selected.isEmpty()) {
                            path = selected.toStdString();
                            _mainTabs->setTabFilePath(idx, path);
                            _mainTabs->setTabText(idx, QFileInfo(selected).fileName());
                        }
                    }
                    if (!path.empty()) {
                        QFile file(QString::fromStdString(path));
                        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&file);
                            out << activeEditor()->text();
                            file.close();
                        }
                    }
                }
                break;
            }
            case 4:  /* Save All */ break;
            case 5:  {
                int idx = _mainTabs->currentIndex();
                if (idx >= 0) onCloseTab(idx);
                break;
            }
            case 6:  if (auto* ed = activeEditor()) ed->undo(); break;
            case 7:  if (auto* ed = activeEditor()) ed->redo(); break;
            case 8:  if (auto* ed = activeEditor()) ed->cut(); break;
            case 9:  if (auto* ed = activeEditor()) ed->copy(); break;
            case 10: if (auto* ed = activeEditor()) ed->paste(); break;
            case 11:
            case 12: (new NppFindReplaceDlg(this))->show(); break;
            case 13: if (auto* ed = activeEditor()) ed->zoomIn(); break;
            case 14: if (auto* ed = activeEditor()) ed->zoomOut(); break;
            case 15: (new NppFileBrowser(this))->show(); break;
            case 16: {
                auto* fl = new NppFunctionList(this);
                if (activeEditor()) fl->parseDocument(activeEditor()->text());
                fl->show();
                break;
            }
            case 17: (new NppProjectPanel("Proyecto Main", this))->show(); break;
            case 18: (new NppClipboardHistory(this))->show(); break;
            case 19: (new NppPluginsAdmin(this))->show(); break;
            case 20: (new NppPreferenceDlg(this))->show(); break;
            case 21: (new NppAboutDlg(this))->exec(); break;
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

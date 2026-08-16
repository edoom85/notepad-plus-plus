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
#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QPrintDialog>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciprinter.h>

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
#include "NppMD5Dlg.h"
#include "NppSummaryDlg.h"
#include "NppDocumentMap.h"
#include "NppVerticalFileSwitcher.h"


/// Ventana principal de Notepad++ en Qt6.




/// Equivale a Notepad_plus_Window + Notepad_plus en la versión Win32.
class NotepadPlusWindowQt : public QMainWindow {
    Q_OBJECT

public:
    explicit NotepadPlusWindowQt(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Notepad++ [Linux Port]");
        QIcon appIcon = NppIconProvider::get(NppIconProvider::IconType::AppLogo);
        setWindowIcon(appIcon);
        qApp->setWindowIcon(appIcon);
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
        menuBar()->setNativeMenuBar(false);

        // ── 1. Archivo ──
        QMenu* fileMenu = menuBar()->addMenu("&Archivo");

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::New),  "&Nuevo",          QKeySequence::New,  this, &NotepadPlusWindowQt::newDocument);
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Open), "&Abrir...",       QKeySequence::Open, this, &NotepadPlusWindowQt::onOpen);
        
        QMenu* openFolderMenu = fileMenu->addMenu("Abrir carpeta contenedora");
        openFolderMenu->addAction("Explorador de archivos", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    QString dir = QFileInfo(QString::fromStdString(path)).absolutePath();
                    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
                }
            }
        });
        openFolderMenu->addAction("Terminal", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    QString dir = QFileInfo(QString::fromStdString(path)).absolutePath();
                    QProcess::startDetached("x-terminal-emulator", QStringList() << "--working-directory" << dir);
                }
            }
        });

        fileMenu->addAction("Abrir en nueva instancia", [this]() {
            QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
        });
        fileMenu->addSeparator();

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
        fileMenu->addAction("Guardar &como...", QKeySequence::SaveAs, [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0 && activeEditor()) {
                QString selected = QFileDialog::getSaveFileName(this, "Guardar como");
                if (!selected.isEmpty()) {
                    NppString path = selected.toStdString();
                    _mainTabs->setTabFilePath(idx, path);
                    _mainTabs->setTabText(idx, QFileInfo(selected).fileName());
                    QFile file(selected);
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&file);
                        out << activeEditor()->text();
                        file.close();
                    }
                }
            }
        });
        fileMenu->addAction("Guardar una copia como...", [this]() {
            if (activeEditor()) {
                QString selected = QFileDialog::getSaveFileName(this, "Guardar una copia como");
                if (!selected.isEmpty()) {
                    QFile file(selected);
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&file);
                        out << activeEditor()->text();
                        file.close();
                    }
                }
            }
        });
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::SaveAll), "Guardar &todo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), [this]() {});
        fileMenu->addSeparator();

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "&Cerrar pestaña", QKeySequence::Close, [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) onCloseTab(idx);
        });
        fileMenu->addAction("Cerrar to&do", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W), [this]() {
            while (_mainTabs->count() > 0) onCloseTab(0);
        });
        fileMenu->addAction("Cerrar todo excepto el actual", [this]() {
            int current = _mainTabs->currentIndex();
            for (int i = _mainTabs->count() - 1; i >= 0; --i) {
                if (i != current) onCloseTab(i);
            }
        });
        fileMenu->addSeparator();

        fileMenu->addAction("&Imprimir...", QKeySequence::Print, [this]() {
            if (auto* ed = activeEditor()) {
                QsciPrinter printer;
                QPrintDialog dlg(&printer, this);
                if (dlg.exec() == QDialog::Accepted) {
                    printer.printRange(ed);
                }
            }
        });
        fileMenu->addSeparator();
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "&Salir", QKeySequence::Quit, qApp, &QApplication::quit);

        // ── 2. Editar ──
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
        editMenu->addAction("E&liminar", QKeySequence::Delete, [this]() {
            if (auto* ed = activeEditor()) ed->removeSelectedText();
        });
        editMenu->addAction("Seleccionar &todo", QKeySequence::SelectAll, [this]() {
            if (auto* ed = activeEditor()) ed->selectAll();
        });
        editMenu->addSeparator();

        QMenu* copyPathMenu = editMenu->addMenu("Copiar al portapapeles");
        copyPathMenu->addAction("Copiar ruta del archivo actual", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) QApplication::clipboard()->setText(QString::fromStdString(_mainTabs->tabFilePath(idx)));
        });
        copyPathMenu->addAction("Copiar nombre del archivo", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) QApplication::clipboard()->setText(QFileInfo(QString::fromStdString(_mainTabs->tabFilePath(idx))).fileName());
        });
        copyPathMenu->addAction("Copiar ruta del directorio", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) QApplication::clipboard()->setText(QFileInfo(QString::fromStdString(_mainTabs->tabFilePath(idx))).absolutePath());
        });

        QMenu* indentMenu = editMenu->addMenu("Indentación");
        indentMenu->addAction("Aumentar sangría (Tab)", QKeySequence(Qt::Key_Tab), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                ed->indent(line);
            }
        });
        indentMenu->addAction("Reducir sangría (Shift+Tab)", QKeySequence(Qt::SHIFT | Qt::Key_Tab), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                ed->unindent(line);
            }
        });


        QMenu* lineOpsMenu = editMenu->addMenu("Operaciones con líneas");
        lineOpsMenu->addAction("Duplicar línea actual", QKeySequence(Qt::CTRL | Qt::Key_D), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                QString text = ed->text(line);
                ed->insertAt(text, line + 1, 0);
            }
        });
        lineOpsMenu->addAction("Eliminar línea actual", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                ed->setSelection(line, 0, line + 1, 0);
                ed->removeSelectedText();
            }
        });
        lineOpsMenu->addAction("Mover línea arriba", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up), [this]() {});
        lineOpsMenu->addAction("Mover línea abajo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down), [this]() {});
        lineOpsMenu->addAction("Unir líneas", QKeySequence(Qt::CTRL | Qt::Key_J), [this]() {});
        lineOpsMenu->addAction("Eliminar líneas vacías", [this]() {});

        QMenu* convertMenu = editMenu->addMenu("Convertir mayúsculas/minúsculas");
        convertMenu->addAction("A MAYÚSCULAS", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U), [this]() {
            if (auto* ed = activeEditor()) {
                QString sel = ed->selectedText();
                if (!sel.isEmpty()) ed->replaceSelectedText(sel.toUpper());
            }
        });
        convertMenu->addAction("A minúsculas", QKeySequence(Qt::CTRL | Qt::Key_U), [this]() {
            if (auto* ed = activeEditor()) {
                QString sel = ed->selectedText();
                if (!sel.isEmpty()) ed->replaceSelectedText(sel.toLower());
            }
        });

        QMenu* spaceOpsMenu = editMenu->addMenu("Operaciones de espacio");
        spaceOpsMenu->addAction("Trim espacios al final", [this]() {});
        spaceOpsMenu->addAction("Trim espacios al inicio", [this]() {});
        spaceOpsMenu->addAction("Convertir espacios a tabuladores", [this]() {});
        spaceOpsMenu->addAction("Convertir tabuladores a espacios", [this]() {});

        QMenu* commentMenu = editMenu->addMenu("Comentar/Descomentar");
        commentMenu->addAction("Alternar comentario de línea", QKeySequence(Qt::CTRL | Qt::Key_Q), [this]() {});
        commentMenu->addAction("Comentario de bloque", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q), [this]() {});

        // ── 3. Buscar ──
        QMenu* searchMenu = menuBar()->addMenu("&Buscar");
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Find),    "&Buscar...",     QKeySequence::Find, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction("Buscar siguiente", QKeySequence::FindNext, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction("Buscar anterior", QKeySequence::FindPrevious, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Replace), "&Reemplazar...", QKeySequence::Replace, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addAction("Buscar en &archivos...", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addSeparator();

        searchMenu->addAction("Ir a &línea...", QKeySequence(Qt::CTRL | Qt::Key_G), [this]() {
            auto* dlg = new NppGoToLineDlg(this);
            if (activeEditor()) dlg->setInfo(activeEditor()->firstVisibleLine() + 1, activeEditor()->lines());
            connect(dlg, &NppGoToLineDlg::goToLine, [this](int line) {
                if (auto* ed = activeEditor()) ed->setCursorPosition(line - 1, 0);
            });
            dlg->show();
        });
        searchMenu->addAction("Ir a coincidencia de corchete", QKeySequence(Qt::CTRL | Qt::Key_B), [this]() {});
        searchMenu->addAction("Marcar...", QKeySequence(Qt::CTRL | Qt::Key_M), [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            dlg->show();
        });
        searchMenu->addSeparator();

        QMenu* bookmarkMenu = searchMenu->addMenu("Marcar como favorito");
        bookmarkMenu->addAction("Alternar marcador", QKeySequence(Qt::CTRL | Qt::Key_F2), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                ed->markerAdd(line, 0);
            }
        });
        bookmarkMenu->addAction("Siguiente marcador", QKeySequence(Qt::Key_F2), [this]() {});
        bookmarkMenu->addAction("Marcador anterior", QKeySequence(Qt::SHIFT | Qt::Key_F2), [this]() {});
        bookmarkMenu->addAction("Borrar todos los marcadores", [this]() {
            if (auto* ed = activeEditor()) ed->markerDeleteAll();
        });

        // ── 4. Vista ──

        QMenu* viewMenu = menuBar()->addMenu("&Vista");
        
        viewMenu->addAction("Siempre visible", [this]() {
            bool isTop = (windowFlags() & Qt::WindowStaysOnTopHint);
            setWindowFlag(Qt::WindowStaysOnTopHint, !isTop);
            show();
        });
        viewMenu->addAction("Activar modo de pantalla completa", QKeySequence(Qt::Key_F11), [this]() {
            if (isFullScreen()) showNormal(); else showFullScreen();
        });
        viewMenu->addAction("Solo documento actual visible", QKeySequence(Qt::Key_F12), [this]() {});
        viewMenu->addAction("Modo \"Sin distracciones\"", [this]() {});
        viewMenu->addSeparator();

        QMenu* viewFileMenu = viewMenu->addMenu("Ver archivo actual en");
        viewFileMenu->addAction("Navegador predeterminado", [this]() {});
        viewFileMenu->addAction("Firefox", [this]() {});
        viewFileMenu->addAction("Chrome", [this]() {});

        QMenu* showLinesMenu = viewMenu->addMenu("Mostrar opciones de líneas");
        showLinesMenu->addAction("Mostrar todo", [this]() {
            if (auto* ed = activeEditor()) {
                ed->setEolVisibility(true);
                ed->setWhitespaceVisibility(QsciScintilla::WsVisible);
            }
        });
        showLinesMenu->addAction("Mostrar espacio y tabulación", [this]() {
            if (auto* ed = activeEditor()) ed->setWhitespaceVisibility(QsciScintilla::WsVisible);
        });
        showLinesMenu->addAction("Mostrar fin de línea (EOL)", [this]() {
            if (auto* ed = activeEditor()) ed->setEolVisibility(true);
        });

        QMenu* zoomSubMenu = viewMenu->addMenu("Zoom");
        zoomSubMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::ZoomIn),  "Acercar &Zoom", QKeySequence::ZoomIn, [this]() {
            if (auto* ed = activeEditor()) ed->zoomIn();
        });
        zoomSubMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::ZoomOut), "Alejar Z&oom",  QKeySequence::ZoomOut, [this]() {
            if (auto* ed = activeEditor()) ed->zoomOut();
        });
        zoomSubMenu->addAction("Restablecer Zoom", QKeySequence(Qt::CTRL | Qt::Key_0), [this]() {
            if (auto* ed = activeEditor()) ed->zoomTo(0);
        });

        QMenu* moveCloneMenu = viewMenu->addMenu("Mover/Clonar el documento actual");
        moveCloneMenu->addAction("Mover a otra vista", [this]() {
            _subTabs->setVisible(true);
            _splitter->setRatio(0.5);
        });
        moveCloneMenu->addAction("Clonar en otra vista", [this]() {
            _subTabs->setVisible(true);
            _splitter->setRatio(0.5);
        });

        QMenu* tabSubMenu = viewMenu->addMenu("Pestañas");
        tabSubMenu->addAction("Pestaña siguiente", QKeySequence(Qt::CTRL | Qt::Key_PageDown), [this]() {
            int current = _mainTabs->currentIndex();
            if (current < _mainTabs->count() - 1) _mainTabs->setCurrentIndex(current + 1);
        });
        tabSubMenu->addAction("Pestaña anterior", QKeySequence(Qt::CTRL | Qt::Key_PageUp), [this]() {
            int current = _mainTabs->currentIndex();
            if (current > 0) _mainTabs->setCurrentIndex(current - 1);
        });

        viewMenu->addAction("Ajuste del texto (Word Wrap)", [this]() {
            if (auto* ed = activeEditor()) {
                auto mode = ed->wrapMode();
                ed->setWrapMode(mode == QsciScintilla::WrapNone ? QsciScintilla::WrapWord : QsciScintilla::WrapNone);
            }
        });
        viewMenu->addAction("Enfocar en otra vista", QKeySequence(Qt::Key_F8), [this]() {});
        viewMenu->addAction("Ocultar líneas", QKeySequence(Qt::ALT | Qt::Key_H), [this]() {});
        viewMenu->addSeparator();

        viewMenu->addAction("Contraer todo", QKeySequence(Qt::ALT | Qt::Key_0), [this]() {
            if (auto* ed = activeEditor()) ed->foldAll(false);
        });
        viewMenu->addAction("Expandir todo", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_0), [this]() {
            if (auto* ed = activeEditor()) ed->foldAll(true);
        });
        viewMenu->addAction("Contraer nivel actual", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F), [this]() {});
        viewMenu->addAction("Expandir nivel actual", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_F), [this]() {});
        
        QMenu* foldLevelMenu = viewMenu->addMenu("Niveles para contraer pestaña actual");
        for (int i = 1; i <= 8; ++i) {
            foldLevelMenu->addAction(QString("Nivel %1").arg(i), [this, i]() {
                if (auto* ed = activeEditor()) ed->foldLine(i);
            });
        }
        
        QMenu* unfoldLevelMenu = viewMenu->addMenu("Niveles para expandir pestaña actual");
        for (int i = 1; i <= 8; ++i) {
            unfoldLevelMenu->addAction(QString("Nivel %1").arg(i), [this, i]() {
                if (auto* ed = activeEditor()) ed->foldLine(i);
            });
        }
        viewMenu->addSeparator();

        viewMenu->addAction("Resumen...", [this]() {
            auto* dlg = new NppSummaryDlg(this);
            dlg->show();
        });
        viewMenu->addSeparator();

        QMenu* projMenu = viewMenu->addMenu("Proyecto");
        projMenu->addAction("Panel de proyecto 1", [this]() {
            (new NppProjectPanel("Proyecto 1", this))->show();
        });
        projMenu->addAction("Panel de proyecto 2", [this]() {
            (new NppProjectPanel("Proyecto 2", this))->show();
        });
        projMenu->addAction("Panel de proyecto 3", [this]() {
            (new NppProjectPanel("Proyecto 3", this))->show();
        });

        viewMenu->addAction("Carpeta como área de trabajo", [this]() {
            (new NppFileBrowser(this))->show();
        });
        viewMenu->addAction("Mapa del documento", [this]() {
            (new NppDocumentMap(this))->show();
        });
        viewMenu->addAction("Lista de documentos", [this]() {
            (new NppVerticalFileSwitcher(this))->show();
        });
        viewMenu->addAction("Lista de funciones", [this]() {
            auto* fl = new NppFunctionList(this);
            if (activeEditor()) fl->parseDocument(activeEditor()->text());
            fl->show();
        });
        viewMenu->addSeparator();

        viewMenu->addAction("Sincronización vertical", [this]() {});
        viewMenu->addAction("Sincronización horizontal", [this]() {});
        viewMenu->addSeparator();

        viewMenu->addAction("Texto derecha-izquierda", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R), [this]() {});
        viewMenu->addAction("Texto izquierda-derecha", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L), [this]() {});
        viewMenu->addSeparator();

        viewMenu->addAction("Monitorizando (tail -f)", [this]() {});


        // ── 5. Codificación ──
        QMenu* encMenu = menuBar()->addMenu("C&odificación");
        encMenu->addAction("ANSI", [this]() {});
        encMenu->addAction("UTF-8", [this]() {});
        encMenu->addAction("UTF-8 sin BOM", [this]() {});
        encMenu->addAction("UTF-16 BE BOM", [this]() {});
        encMenu->addAction("UTF-16 LE BOM", [this]() {});
        encMenu->addSeparator();
        encMenu->addAction("Convertir a ANSI", [this]() {});
        encMenu->addAction("Convertir a UTF-8", [this]() {});
        encMenu->addAction("Convertir a UTF-8 sin BOM", [this]() {});

        // ── 6. Lenguaje ──
        QMenu* langMenu = menuBar()->addMenu("&Lenguaje");
        langMenu->addAction("Texto plano (Normal Text)", [this]() {
            if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText);
        });
        langMenu->addSeparator();

        QMenu* menuC = langMenu->addMenu("C");
        menuC->addAction("C",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C++", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("CSS", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CSS); });

        QMenu* menuH = langMenu->addMenu("H");
        menuH->addAction("HTML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::HTML); });

        QMenu* menuJ = langMenu->addMenu("J");
        menuJ->addAction("Java",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Java); });
        menuJ->addAction("JavaScript", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JavaScript); });
        menuJ->addAction("JSON",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JSON); });

        QMenu* menuM = langMenu->addMenu("M");
        menuM->addAction("Markdown", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });

        QMenu* menuP = langMenu->addMenu("P");
        menuP->addAction("Python", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Python); });

        QMenu* menuS = langMenu->addMenu("S");
        menuS->addAction("Shell (Bash)", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuS->addAction("SQL",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::SQL); });

        QMenu* menuX = langMenu->addMenu("X");
        menuX->addAction("XML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::XML); });

        QMenu* menuY = langMenu->addMenu("Y");
        menuY->addAction("YAML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::YAML); });

        langMenu->addSeparator();
        langMenu->addAction("Definir tu lenguaje (UDL)...", [this]() {
            auto* dlg = new NppUserDefineDlg(this);
            dlg->show();
        });

        // ── 7. Configuración ──
        QMenu* configMenu = menuBar()->addMenu("Con&figuración");
        configMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Settings), "&Preferencias...", [this]() {
            auto* dlg = new NppPreferenceDlg(this);
            dlg->show();
        });
        configMenu->addAction("Configurador de &estilos...", [this]() {
            auto* dlg = new NppPreferenceDlg(this);
            dlg->show();
        });
        configMenu->addAction("Configurador de &accesos directos...", [this]() {
            auto* dlg = new NppShortcutMapper(this);
            dlg->show();
        });
        
        QMenu* importMenu = configMenu->addMenu("Importar");
        importMenu->addAction("Importar plugin(s)...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });
        importMenu->addAction("Importar tema(s) de estilo...", [this]() {});
        configMenu->addSeparator();
        configMenu->addAction("Editar menú contextual emergente", [this]() {});

        // ── 8. Herramientas ──
        QMenu* toolsMenu = menuBar()->addMenu("Herramien&tas");
        toolsMenu->addAction("Generar &MD5...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        toolsMenu->addAction("Generar &SHA-256...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });

        // ── 9. Macro ──
        QMenu* macroMenu = menuBar()->addMenu("&Macro");
        macroMenu->addAction("Iniciar grabación", [this]() {});
        macroMenu->addAction("Detener grabación", [this]() {});
        macroMenu->addAction("Reproducción", [this]() {});
        macroMenu->addAction("Guardar macro grabada...", [this]() {});

        // ── 10. Ejecutar ──
        QMenu* runMenu = menuBar()->addMenu("&Ejecutar");
        runMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Run), "&Ejecutar comando...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R), [this]() {
            auto* dlg = new NppRunDlg(this);
            dlg->show();
        });

        // ── 11. Plugins ──
        QMenu* pluginsMenu = menuBar()->addMenu("&Plugins");
        pluginsMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Plugins), "Administrador de &Plugins...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });

        // ── 12. Ayuda ──
        QMenu* helpMenu = menuBar()->addMenu("A&yuda");
        helpMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::About), "&Acerca de Notepad++...", [this]() {
            auto* dlg = new NppAboutDlg(this);
            dlg->exec();
        });
        helpMenu->addAction("Ayuda en línea", [this]() {
            QDesktopServices::openUrl(QUrl("https://notepad-plus-plus.org/resources/"));
        });
        helpMenu->addAction("Documentación en línea", [this]() {
            QDesktopServices::openUrl(QUrl("https://npp-user-manual.org/"));
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

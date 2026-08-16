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
#include "../../Platform/PlatformMacroEngine.h"
#include "../../Platform/PlatformSessionManager.h"
#include "NppStyleConfigDlg.h"
#include "NppRunMacroDlg.h"
#include "NppFindCharsInRange.h"
#include "NppColumnEditor.h"
#include "NppAnsiCharPanel.h"
#include <QInputDialog>




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
        connect(_mainTabs, &NppTabWidget::contextMenuOnTab,
                this, &NotepadPlusWindowQt::showTabContextMenu);
        connect(_mainTabs, &QTabWidget::currentChanged,
                this, &NotepadPlusWindowQt::onTabChanged);


        // ── 4. Status Bar ───────────────────────────────────────────────────
        _statusBar = new NppStatusBar(this);
        setStatusBar(_statusBar->statusBar());
        updateStatusBar();

        // ── 5. Restaurar sesión XML o crear documento vacío ─────────────────
        QStringList sessionFiles;
        int activeIdx = 0;
        if (NppSessionManager::loadSession(sessionFiles, activeIdx)) {
            for (const QString& file : sessionFiles) {
                openFile(file.toStdString());
            }
            if (activeIdx >= 0 && activeIdx < _mainTabs->count()) {
                _mainTabs->setCurrentIndex(activeIdx);
            }
        } else {
            newDocument();
        }


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


private:
    QsciScintilla* activeEditor() const {
        QWidget* w = _mainTabs->currentWidget();
        return qobject_cast<QsciScintilla*>(w);
    }

    void setupFindReplaceConnections(NppFindReplaceDlg* dlg) {
        if (!dlg) return;
        connect(dlg, &NppFindReplaceDlg::findNext, [this, dlg](const QString& target) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            bool wrap = dlg->isWrapAround();
            bool found = ed->findFirst(target, regex, caseSens, wholeWord, wrap, true);
            if (!found) {
                statusBar()->showMessage(QString("Búsqueda finalizada: no se encontró '%1'").arg(target), 3000);
            }
        });

        connect(dlg, &NppFindReplaceDlg::findPrev, [this, dlg](const QString& target) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            bool wrap = dlg->isWrapAround();
            bool found = ed->findFirst(target, regex, caseSens, wholeWord, wrap, false);
            if (!found) {
                statusBar()->showMessage(QString("Búsqueda anterior finalizada: no se encontró '%1'").arg(target), 3000);
            }
        });

        connect(dlg, &NppFindReplaceDlg::replaceOne, [this, dlg](const QString& target, const QString& replacement) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            if (ed->hasSelectedText() && ed->selectedText() == target) {
                ed->replace(replacement);
            }
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            bool wrap = dlg->isWrapAround();
            ed->findFirst(target, regex, caseSens, wholeWord, wrap, true);
        });

        connect(dlg, &NppFindReplaceDlg::replaceAll, [this, dlg](const QString& target, const QString& replacement) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            ed->beginUndoAction();
            int count = 0;
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            if (ed->findFirst(target, regex, caseSens, wholeWord, true, true, 0, 0)) {
                ed->replace(replacement);
                count++;
                while (ed->findNext()) {
                    ed->replace(replacement);
                    count++;
                }
            }
            ed->endUndoAction();
            statusBar()->showMessage(QString("Reemplazar todo: %1 coincidencia(s) reemplazada(s)").arg(count), 4000);
        });
    }


public:
    void newDocument() {
        static int newCount = 1;
        auto* editor = createEditor();
        NppString name = "new " + std::to_string(newCount++);
        int idx = _mainTabs->addTab(editor, name);
        _mainTabs->setCurrentIndex(idx);
        updateStatusBar();
    }

private:
    void createMenus() {
        menuBar()->setNativeMenuBar(false);

        // ── 1. Archivo ──

        QMenu* fileMenu = menuBar()->addMenu("&Archivo");

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::New),  "&Nuevo",          QKeySequence::New,  this, &NotepadPlusWindowQt::newDocument);
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Open), "&Abrir...",       QKeySequence::Open, this, &NotepadPlusWindowQt::onOpen);
        
        QMenu* openFolderMenu = fileMenu->addMenu("Abrir carpeta contenedora en");
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

        fileMenu->addAction("Abrir en visor predeterminado", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path)));
            }
        });
        fileMenu->addAction("Abrir carpeta como espacio de trabajo", [this]() {
            (new NppFileBrowser(this))->show();
        });
        fileMenu->addAction("Recargar desde disco", QKeySequence(Qt::CTRL | Qt::Key_R), [this]() {});
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
        fileMenu->addAction("Guardar &como...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S), [this]() {
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
        fileMenu->addAction("Renombrar...", [this]() {});
        fileMenu->addSeparator();

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "Cerrar pestaña activa", QKeySequence::Close, [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) onCloseTab(idx);
        });
        fileMenu->addAction("Cerrar todas las pestañas", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W), [this]() {
            while (_mainTabs->count() > 0) onCloseTab(0);
        });
        
        QMenu* closeSpecialMenu = fileMenu->addMenu("Cerrado especial");
        closeSpecialMenu->addAction("Cerrar todo excepto el actual", [this]() {
            int current = _mainTabs->currentIndex();
            for (int i = _mainTabs->count() - 1; i >= 0; --i) {
                if (i != current) onCloseTab(i);
            }
        });
        closeSpecialMenu->addAction("Cerrar todo a la izquierda", [this]() {});
        closeSpecialMenu->addAction("Cerrar todo a la derecha", [this]() {});
        closeSpecialMenu->addAction("Cerrar no modificados", [this]() {});

        fileMenu->addAction("Mover a la Papelera de reciclaje", [this]() {});
        fileMenu->addSeparator();

        fileMenu->addAction("Cargar sesión...", [this]() {});
        fileMenu->addAction("Guardar sesión...", [this]() {});
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
        fileMenu->addAction("Imprimir ahora", [this]() {});
        fileMenu->addSeparator();

        fileMenu->addAction("Restaurar archivo cerrado recientemente", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T), [this]() {});
        fileMenu->addAction("Abrir todos los archivos recientes", [this]() {});
        fileMenu->addAction("Vaciar lista de archivos recientes", [this]() {});
        fileMenu->addSeparator();

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Close), "&Salir", QKeySequence(Qt::ALT | Qt::Key_F4), qApp, &QApplication::quit);

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
        editMenu->addAction("Borrar", QKeySequence::Delete, [this]() {
            if (auto* ed = activeEditor()) ed->removeSelectedText();
        });
        editMenu->addAction("Seleccionar todo", QKeySequence::SelectAll, [this]() {
            if (auto* ed = activeEditor()) ed->selectAll();
        });
        editMenu->addAction("Selección de inicio/fin", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B), [this]() {});
        editMenu->addAction("Selección de inicio/fin en modo columna", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_B), [this]() {});
        editMenu->addSeparator();

        QMenu* insertMenu = editMenu->addMenu("Insertar");
        insertMenu->addAction("Fecha y hora corta", [this]() {});
        insertMenu->addAction("Fecha y hora larga", [this]() {});

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

        QMenu* indentMenu = editMenu->addMenu("Tabulación");
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

        QMenu* convertMenu = editMenu->addMenu("Conversión de mayúsculas y minúsculas");
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
        convertMenu->addAction("Convertir Primera Letra En Mayúscula", [this]() {});

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

        QMenu* commentMenu = editMenu->addMenu("Comentar / Descomentar");
        commentMenu->addAction("Alternar comentario de línea", QKeySequence(Qt::CTRL | Qt::Key_Q), [this]() {});
        commentMenu->addAction("Comentario de bloque", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q), [this]() {});

        QMenu* acMenu = editMenu->addMenu("Autocompletar");
        acMenu->addAction("Completar palabra", QKeySequence(Qt::CTRL | Qt::Key_Space), [this]() {});
        acMenu->addAction("Completar función", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Space), [this]() {});

        QMenu* eolMenu = editMenu->addMenu("Conversión fin de línea");
        eolMenu->addAction("Formato Windows (CR LF)", [this]() {});
        eolMenu->addAction("Formato Unix (LF)", [this]() {});
        eolMenu->addAction("Formato Mac (CR)", [this]() {});

        QMenu* spaceOpsMenu = editMenu->addMenu("Operaciones de limpieza");
        spaceOpsMenu->addAction("Trim espacios al final", [this]() {});
        spaceOpsMenu->addAction("Trim espacios al inicio", [this]() {});
        spaceOpsMenu->addAction("Trim inicio y final", [this]() {});

        editMenu->addMenu("Pegado especial");
        editMenu->addMenu("Selección");
        editMenu->addSeparator();

        QMenu* multiSelMenu = editMenu->addMenu("Multiselección total");
        multiSelMenu->addAction("Siguiente multiselección", [this]() {});
        multiSelMenu->addAction("Deshacer la última selección múltiple añadida", [this]() {});
        multiSelMenu->addAction("Saltar actual & Ir a siguiente multiselección", [this]() {});
        editMenu->addSeparator();

        editMenu->addAction("Modo de columna...", [this]() {});
        editMenu->addAction("Editor de columna...", QKeySequence(Qt::ALT | Qt::Key_C), [this]() {
            auto* dlg = new NppColumnEditor(this);
            connect(dlg, &NppColumnEditor::columnEditRequested, [this](bool isText, const QString& text, int start, int inc, int fmt) {
                auto* ed = activeEditor();
                if (!ed) return;
                ed->beginUndoAction();
                int lines = ed->lines();
                int currentVal = start;
                for (int i = 0; i < lines; ++i) {
                    if (isText) {
                        ed->insertAt(text, i, 0);
                    } else {
                        QString numStr;
                        if (fmt == 1) numStr = QString::number(currentVal, 8);
                        else if (fmt == 2) numStr = QString::number(currentVal, 16).toUpper();
                        else if (fmt == 3) numStr = QString::number(currentVal, 2);
                        else numStr = QString::number(currentVal);
                        ed->insertAt(numStr, i, 0);
                        currentVal += inc;
                    }
                }
                ed->endUndoAction();
            });
            dlg->show();
        });
        editMenu->addAction("Panel de caracteres", [this]() {
            auto* panel = new NppAnsiCharPanel(this);
            connect(panel, &NppAnsiCharPanel::charSelected, [this](const QString& ch) {
                if (auto* ed = activeEditor()) ed->insert(ch);
            });
            panel->show();
        });
        editMenu->addAction("Historial de portapapeles", [this]() {
            auto* history = new NppClipboardHistory(this);
            connect(history, &NppClipboardHistory::pasteRequested, [this](const QString& text) {
                if (auto* ed = activeEditor()) ed->insert(text);
            });
            history->show();
        });
        editMenu->addSeparator();


        QMenu* readOnlyNpp = editMenu->addMenu("Atributo solo lectura en Notepad++");
        readOnlyNpp->addAction("Alternar solo lectura", [this]() {
            if (auto* ed = activeEditor()) ed->setReadOnly(!ed->isReadOnly());
        });
        editMenu->addAction("Atributo de solo lectura en Windows", [this]() {});

        // ── 3. Buscar ──
        QMenu* searchMenu = menuBar()->addMenu("&Buscar");
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Find),    "Buscar...",     QKeySequence::Find, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            setupFindReplaceConnections(dlg);
            dlg->selectTab(0, activeEditor() ? activeEditor()->selectedText() : "");
        });
        searchMenu->addAction("Buscar en archivos", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            setupFindReplaceConnections(dlg);
            dlg->selectTab(2);
        });
        searchMenu->addAction("Buscar siguiente", QKeySequence::FindNext, [this]() {
            auto* ed = activeEditor();
            if (ed) ed->findNext();
        });
        searchMenu->addAction("Buscar anterior", QKeySequence::FindPrevious, [this]() {
            auto* ed = activeEditor();
            if (ed) ed->findNext();
        });
        searchMenu->addAction("Seleccionar y buscar siguiente", QKeySequence(Qt::CTRL | Qt::Key_F3), [this]() {});
        searchMenu->addAction("Seleccionar y buscar anterior", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F3), [this]() {});
        searchMenu->addAction("Búsqueda (volátil) siguiente", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F3), [this]() {});
        searchMenu->addAction("Búsqueda (volátil) anterior", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_F3), [this]() {});
        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Replace), "Sustituir...", QKeySequence::Replace, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            setupFindReplaceConnections(dlg);
            dlg->selectTab(1, activeEditor() ? activeEditor()->selectedText() : "");
        });
        searchMenu->addAction("Búsqueda incremental", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I), [this]() {});
        searchMenu->addAction("Ventana de resultados de búsqueda", QKeySequence(Qt::Key_F7), [this]() {});
        searchMenu->addAction("Resultados de búsqueda siguiente", QKeySequence(Qt::Key_F4), [this]() {});
        searchMenu->addAction("Resultados de búsqueda anterior", QKeySequence(Qt::SHIFT | Qt::Key_F4), [this]() {});
        searchMenu->addAction("Ir a la línea...", QKeySequence(Qt::CTRL | Qt::Key_G), [this]() {
            auto* dlg = new NppGoToLineDlg(this);
            if (activeEditor()) dlg->setInfo(activeEditor()->firstVisibleLine() + 1, activeEditor()->lines());
            connect(dlg, &NppGoToLineDlg::goToLine, [this](int line) {
                if (auto* ed = activeEditor()) ed->setCursorPosition(line - 1, 0);
            });
            connect(dlg, &NppGoToLineDlg::goToOffset, [this](int offset) {
                if (auto* ed = activeEditor()) ed->SendScintilla(QsciScintilla::SCI_GOTOPOS, offset);
            });
            dlg->show();
        });

        searchMenu->addAction("Ir al corchete", QKeySequence(Qt::CTRL | Qt::Key_B), [this]() {});
        searchMenu->addAction("Seleccionar todo lo que haya entre {} [] o ()", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_B), [this]() {});
        searchMenu->addAction("Marcar...", QKeySequence(Qt::CTRL | Qt::Key_M), [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            setupFindReplaceConnections(dlg);
            dlg->selectTab(4, activeEditor() ? activeEditor()->selectedText() : "");
        });
        searchMenu->addSeparator();


        searchMenu->addMenu("Historial de cambios");
        searchMenu->addSeparator();

        searchMenu->addMenu("Resaltar todas las coincidencias buscadas");
        searchMenu->addMenu("Resaltar una coincidencia");
        searchMenu->addMenu("Limpiar estilo");
        searchMenu->addMenu("Subir");
        searchMenu->addMenu("Bajar");
        searchMenu->addMenu("Copiar texto con estilo");
        searchMenu->addSeparator();

        QMenu* bookmarkMenu = searchMenu->addMenu("Marcadores");
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
        searchMenu->addSeparator();

        searchMenu->addAction("Buscar caracteres por tipo...", [this]() {
            auto* dlg = new NppFindCharsInRangeDlg(this);
            connect(dlg, &NppFindCharsInRangeDlg::findInRangeRequested, [this](int minVal, int maxVal) {
                auto* ed = activeEditor();
                if (!ed) return;
                QString text = ed->text();
                int curLine = 0, curIndex = 0;
                ed->getCursorPosition(&curLine, &curIndex);
                int startPos = 0;
                for (int i = startPos; i < text.length(); ++i) {
                    char16_t code = text[i].unicode();
                    if (code >= minVal && code <= maxVal) {
                        ed->setSelection(0, i, 0, i + 1);
                        statusBar()->showMessage(QString("Carácter encontrado en el rango [%1, %2]: '%3' (Código: %4)")
                            .arg(minVal).arg(maxVal).arg(text[i]).arg(code), 4000);
                        return;
                    }
                }
                statusBar()->showMessage("No se encontraron más caracteres en el rango especificado", 3000);
            });
            dlg->show();
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

        QMenu* charSetsMenu = encMenu->addMenu("Juegos de caracteres");
        
        QMenu* csWest = charSetsMenu->addMenu("Occidental");
        csWest->addAction("OEM 850", [this]() {});
        csWest->addAction("ISO 8859-1", [this]() {});
        csWest->addAction("ISO 8859-15", [this]() {});
        csWest->addAction("Windows-1252", [this]() {});

        QMenu* csCentral = charSetsMenu->addMenu("Europa Central");
        csCentral->addAction("OEM 852", [this]() {});
        csCentral->addAction("ISO 8859-2", [this]() {});
        csCentral->addAction("Windows-1250", [this]() {});

        QMenu* csCyrillic = charSetsMenu->addMenu("Círilico");
        csCyrillic->addAction("OEM 866", [this]() {});
        csCyrillic->addAction("ISO 8859-5", [this]() {});
        csCyrillic->addAction("KOI8-R", [this]() {});
        csCyrillic->addAction("KOI8-U", [this]() {});
        csCyrillic->addAction("Windows-1251", [this]() {});

        QMenu* csSouth = charSetsMenu->addMenu("Europa del Sur");
        csSouth->addAction("ISO 8859-3", [this]() {});

        QMenu* csGreek = charSetsMenu->addMenu("Griego");
        csGreek->addAction("ISO 8859-7", [this]() {});
        csGreek->addAction("Windows-1253", [this]() {});

        QMenu* csTurkish = charSetsMenu->addMenu("Turco");
        csTurkish->addAction("ISO 8859-9", [this]() {});
        csTurkish->addAction("Windows-1254", [this]() {});

        QMenu* csHebrew = charSetsMenu->addMenu("Hebreo");
        csHebrew->addAction("ISO 8859-8", [this]() {});
        csHebrew->addAction("Windows-1255", [this]() {});

        QMenu* csArabic = charSetsMenu->addMenu("Árabe");
        csArabic->addAction("ISO 8859-6", [this]() {});
        csArabic->addAction("Windows-1256", [this]() {});

        QMenu* csBaltic = charSetsMenu->addMenu("Báltico");
        csBaltic->addAction("ISO 8859-4", [this]() {});
        csBaltic->addAction("ISO 8859-13", [this]() {});
        csBaltic->addAction("Windows-1257", [this]() {});

        QMenu* csViet = charSetsMenu->addMenu("Vietnamita");
        csViet->addAction("Windows-1258", [this]() {});

        QMenu* csEastAsian = charSetsMenu->addMenu("Asiático Oriental");
        csEastAsian->addAction("Chino Simplificado (GB2312)", [this]() {});
        csEastAsian->addAction("Chino Tradicional (Big5)", [this]() {});
        csEastAsian->addAction("Japonés (Shift-JIS)", [this]() {});
        csEastAsian->addAction("Coreano (EUC-KR)", [this]() {});

        encMenu->addSeparator();
        encMenu->addAction("Convertir a ANSI", [this]() {});
        encMenu->addAction("Convertir a UTF-8", [this]() {});
        encMenu->addAction("Convertir a UTF-8 sin BOM", [this]() {});
        encMenu->addAction("Convertir a UTF-16 BE BOM", [this]() {});
        encMenu->addAction("Convertir a UTF-16 LE BOM", [this]() {});


        // ── 6. Lenguaje ──
        QMenu* langMenu = menuBar()->addMenu("&Lenguaje");
        langMenu->addAction("Ninguno (texto normal)", [this]() {
            if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText);
        });
        langMenu->addSeparator();

        QMenu* menuA = langMenu->addMenu("A");
        menuA->addAction("ActionScript", [this]() {});
        menuA->addAction("Ada",          [this]() {});
        menuA->addAction("ASN.1",        [this]() {});
        menuA->addAction("ASP",          [this]() {});
        menuA->addAction("Assembly",     [this]() {});
        menuA->addAction("AutoIt",       [this]() {});

        QMenu* menuB = langMenu->addMenu("B");
        menuB->addAction("Batch",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuB->addAction("BASH",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });

        QMenu* menuC = langMenu->addMenu("C");
        menuC->addAction("C",            [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C++",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C#",           [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("CSS",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CSS); });
        menuC->addAction("CMake",        [this]() {});
        menuC->addAction("COBOL",        [this]() {});
        menuC->addAction("CoffeeScript", [this]() {});

        QMenu* menuD = langMenu->addMenu("D");
        menuD->addAction("D",            [this]() {});
        menuD->addAction("Diff",         [this]() {});

        QMenu* menuE = langMenu->addMenu("E");
        menuE->addAction("Erlang",       [this]() {});

        QMenu* menuF = langMenu->addMenu("F");
        menuF->addAction("Fortran",      [this]() {});
        menuF->addAction("F#",           [this]() {});

        QMenu* menuG = langMenu->addMenu("G");
        menuG->addAction("GUI4CLI",      [this]() {});

        QMenu* menuH = langMenu->addMenu("H");
        menuH->addAction("HTML",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::HTML); });
        menuH->addAction("Haskell",      [this]() {});

        QMenu* menuI = langMenu->addMenu("I");
        menuI->addAction("INI",          [this]() {});
        menuI->addAction("INNO Setup",   [this]() {});

        QMenu* menuJ = langMenu->addMenu("J");
        menuJ->addAction("Java",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Java); });
        menuJ->addAction("JavaScript",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JavaScript); });
        menuJ->addAction("JSON",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JSON); });
        menuJ->addAction("JSP",          [this]() {});

        langMenu->addAction("KIXtart",   [this]() {});

        QMenu* menuL = langMenu->addMenu("L");
        menuL->addAction("LISP",         [this]() {});
        menuL->addAction("Lua",          [this]() {});

        QMenu* menuM = langMenu->addMenu("M");
        menuM->addAction("Make",         [this]() {});
        menuM->addAction("Markdown",     [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });
        menuM->addAction("MATLAB",       [this]() {});
        menuM->addAction("MS-DOS",       [this]() {});

        QMenu* menuN = langMenu->addMenu("N");
        menuN->addAction("Nim",          [this]() {});
        menuN->addAction("NSI",          [this]() {});

        QMenu* menuO = langMenu->addMenu("O");
        menuO->addAction("Objective-C",  [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuP = langMenu->addMenu("P");
        menuP->addAction("Pascal",       [this]() {});
        menuP->addAction("Perl",         [this]() {});
        menuP->addAction("PHP",          [this]() {});
        menuP->addAction("PostScript",   [this]() {});
        menuP->addAction("PowerShell",   [this]() {});
        menuP->addAction("Properties",   [this]() {});
        menuP->addAction("Python",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Python); });

        QMenu* menuR = langMenu->addMenu("R");
        menuR->addAction("R",             [this]() {});
        menuR->addAction("Resource File", [this]() {});

        QMenu* menuS = langMenu->addMenu("S");
        menuS->addAction("Ruby",         [this]() {});
        menuS->addAction("Rust",         [this]() {});
        menuS->addAction("Shell",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuS->addAction("Scheme",       [this]() {});
        menuS->addAction("Smalltalk",    [this]() {});
        menuS->addAction("SQL",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::SQL); });
        menuS->addAction("Swift",        [this]() {});

        QMenu* menuT = langMenu->addMenu("T");
        menuT->addAction("TCL",          [this]() {});
        menuT->addAction("TOML",         [this]() {});

        QMenu* menuV = langMenu->addMenu("V");
        menuV->addAction("VHDL",         [this]() {});
        menuV->addAction("Verilog",      [this]() {});
        menuV->addAction("Visual Basic", [this]() {});
        menuV->addAction("Visual Prolog",[this]() {});

        langMenu->addAction("XML",  [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::XML); });
        langMenu->addAction("YAML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::YAML); });

        langMenu->addSeparator();
        QMenu* udlMenu = langMenu->addMenu("Definido por el usuario");
        udlMenu->addAction("Markdown (preinstalled)", [this]() {});
        udlMenu->addAction("Markdown (preinstalled dark mode)", [this]() {});
        
        langMenu->addAction("Definido por el usuario...", [this]() {
            auto* dlg = new NppUserDefineDlg(this);
            dlg->show();
        });


        // ── 7. Configuración ──
        QMenu* configMenu = menuBar()->addMenu("Con&figuración");
        configMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Settings), "&Preferencias...", [this]() {
            auto* dlg = new NppPreferenceDlg(this);
            connect(dlg, &NppPreferenceDlg::toolbarVisibilityChanged, [this](bool visible) {
                if (_toolbar) _toolbar->setVisible(visible);
            });

            connect(dlg, &NppPreferenceDlg::statusbarVisibilityChanged, [this](bool visible) {
                if (statusBar()) statusBar()->setVisible(visible);
            });
            connect(dlg, &NppPreferenceDlg::menuBarVisibilityChanged, [this](bool visible) {
                if (menuBar()) menuBar()->setVisible(visible);
            });
            connect(dlg, &NppPreferenceDlg::tabSizeChanged, [this](int size) {
                if (auto* ed = activeEditor()) ed->setTabWidth(size);
            });
            connect(dlg, &NppPreferenceDlg::tabUseSpacesChanged, [this](bool useSpaces) {
                if (auto* ed = activeEditor()) ed->setIndentationsUseTabs(!useSpaces);
            });
            connect(dlg, &NppPreferenceDlg::indentGuidesChanged, [this](bool show) {
                if (auto* ed = activeEditor()) ed->setIndentationGuides(show);
            });
            connect(dlg, &NppPreferenceDlg::caretLineHighlightChanged, [this](bool show) {
                if (auto* ed = activeEditor()) ed->setCaretLineVisible(show);
            });
            connect(dlg, &NppPreferenceDlg::wordWrapChanged, [this](bool wrap) {
                if (auto* ed = activeEditor()) ed->setWrapMode(wrap ? QsciScintilla::WrapWord : QsciScintilla::WrapNone);
            });
            connect(dlg, &NppPreferenceDlg::lineNumbersVisibilityChanged, [this](bool show) {
                if (auto* ed = activeEditor()) ed->setMarginLineNumbers(0, show);
            });
            connect(dlg, &NppPreferenceDlg::codeFoldingToggled, [this](bool fold) {
                if (auto* ed = activeEditor()) ed->setFolding(fold ? QsciScintilla::PlainFoldStyle : QsciScintilla::NoFoldStyle);
            });
            connect(dlg, &NppPreferenceDlg::eolModeChanged, [this](int mode) {
                if (auto* ed = activeEditor()) {
                    if (mode == 0) ed->setEolMode(QsciScintilla::EolUnix);
                    else if (mode == 1) ed->setEolMode(QsciScintilla::EolWindows);
                    else if (mode == 2) ed->setEolMode(QsciScintilla::EolMac);
                }
            });
            dlg->show();
        });

        configMenu->addAction("Configurador de &estilos...", [this]() {
            auto* dlg = new NppStyleConfigDlg(this);
            connect(dlg, &NppStyleConfigDlg::styleApplied, [this](const QString& theme, const QColor& /*fg*/, const QColor& /*bg*/) {
                if (auto* ed = activeEditor()) {
                    NppLexerManager::applyTheme(ed, theme);
                }
            });
            dlg->show();
        });


        configMenu->addAction("Configurador de &accesos directos...", [this]() {
            auto* dlg = new NppShortcutMapper(this);
            dlg->show();
        });
        configMenu->addSeparator();

        configMenu->addAction("Editar menú contextual emergente", [this]() {});
        configMenu->addSeparator();
        
        QMenu* importMenu = configMenu->addMenu("Importar");
        importMenu->addAction("Importar plugin(s)...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });
        importMenu->addAction("Importar tema(s) de estilo...", [this]() {});


        // ── 8. Herramientas ──
        QMenu* toolsMenu = menuBar()->addMenu("Herramien&tas");
        
        QMenu* md5Menu = toolsMenu->addMenu("MD5");
        md5Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        md5Menu->addAction("Generar desde archivos...", [this]() {});

        QMenu* sha1Menu = toolsMenu->addMenu("SHA-1");
        sha1Menu->addAction("Generar...", [this]() {});
        sha1Menu->addAction("Generar desde archivos...", [this]() {});

        QMenu* sha256Menu = toolsMenu->addMenu("SHA-256");
        sha256Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        sha256Menu->addAction("Generar desde archivos...", [this]() {});

        QMenu* sha512Menu = toolsMenu->addMenu("SHA-512");
        sha512Menu->addAction("Generar...", [this]() {});
        sha512Menu->addAction("Generar desde archivos...", [this]() {});

        // ── 9. Macro ──
        QMenu* macroMenu = menuBar()->addMenu("&Macro");
        macroMenu->addAction("Iniciar grabación", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), [this]() {});
        macroMenu->addAction("Detener grabación", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), [this]() {});
        macroMenu->addAction("Reproducción", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P), [this]() {});
        macroMenu->addAction("Guardar macro grabada actualmente...", [this]() {});
        macroMenu->addAction("Ejecutar la macro múltiples veces...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_R), [this]() {
            auto* dlg = new NppRunMacroDlg(this);
            dlg->show();
        });
        macroMenu->addSeparator();
        macroMenu->addAction("Trim espacios al final y guardar", [this]() {});

        // ── 10. Ejecutar ──
        QMenu* runMenu = menuBar()->addMenu("&Ejecutar");
        runMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Run), "&Ejecutar...", QKeySequence(Qt::Key_F5), [this]() {
            auto* dlg = new NppRunDlg(this);
            dlg->show();
        });
        runMenu->addSeparator();
        runMenu->addAction("Obtener ayuda en línea", [this]() {
            QDesktopServices::openUrl(QUrl("https://npp-user-manual.org/"));
        });
        runMenu->addAction("Foro de Notepad++", [this]() {
            QDesktopServices::openUrl(QUrl("https://community.notepad-plus-plus.org/"));
        });
        runMenu->addAction("Página del proyecto en GitHub", [this]() {
            QDesktopServices::openUrl(QUrl("https://github.com/notepad-plus-plus/notepad-plus-plus"));
        });

        // ── 11. Plugins ──
        QMenu* pluginsMenu = menuBar()->addMenu("&Plugins");
        pluginsMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Plugins), "Administrador de &Plugins...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });

        // ── 12. Ayuda ──
        QMenu* helpMenu = menuBar()->addMenu("A&yuda");
        helpMenu->addAction("Contenido de la ayuda", QKeySequence(Qt::Key_F1), [this]() {
            QDesktopServices::openUrl(QUrl("https://npp-user-manual.org/"));
        });
        helpMenu->addAction("Obtener ayuda en línea", [this]() {
            QDesktopServices::openUrl(QUrl("https://npp-user-manual.org/"));
        });
        helpMenu->addAction("Foro de Notepad++", [this]() {
            QDesktopServices::openUrl(QUrl("https://community.notepad-plus-plus.org/"));
        });
        helpMenu->addAction("Página del proyecto en GitHub", [this]() {
            QDesktopServices::openUrl(QUrl("https://github.com/notepad-plus-plus/notepad-plus-plus"));
        });
        helpMenu->addSeparator();
        helpMenu->addAction("Comprobar actualizaciones...", [this]() {});
        helpMenu->addSeparator();
        helpMenu->addAction("Información de depuración...", [this]() {});
        helpMenu->addAction("Argumentos de la línea de comandos...", [this]() {});
        helpMenu->addSeparator();
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

protected:
    void closeEvent(QCloseEvent* event) override {
        saveWindowState();

        // Auto-guardar sesión XML en ~/.config/notepadplusplus/session.xml
        QStringList sessionFiles;
        for (int i = 0; i < _mainTabs->count(); ++i) {
            NppString path = _mainTabs->tabFilePath(i);
            if (!path.empty()) {
                sessionFiles.append(QString::fromStdString(path));
            }
        }
        NppSessionManager::saveSession(sessionFiles, _mainTabs->currentIndex());

        event->accept();
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

    void showTabContextMenu(int index, const QPoint& globalPos) {
        if (index < 0) return;
        QMenu menu(this);

        menu.addAction("Cerrar", [this, index]() { onCloseTab(index); });
        menu.addAction("Cerrar todas EXCEPTO esta", [this, index]() {
            for (int i = _mainTabs->count() - 1; i >= 0; --i) {
                if (i != index) onCloseTab(i);
            }
        });
        menu.addAction("Cerrar pestañas a la izquierda", [this, index]() {
            for (int i = index - 1; i >= 0; --i) onCloseTab(i);
        });
        menu.addAction("Cerrar pestañas a la derecha", [this, index]() {
            for (int i = _mainTabs->count() - 1; i > index; --i) onCloseTab(i);
        });
        menu.addSeparator();

        menu.addAction("Guardar", [this, index]() {
            _mainTabs->setCurrentIndex(index);
        });
        menu.addAction("Guardar como...", [this, index]() {
            _mainTabs->setCurrentIndex(index);
        });
        menu.addAction("Renombrar...", [this, index]() {
            _mainTabs->setCurrentIndex(index);
        });
        menu.addSeparator();

        QMenu* copyMenu = menu.addMenu("Copiar al portapapeles");
        copyMenu->addAction("Copiar ruta del archivo actual", [this, index]() {
            QApplication::clipboard()->setText(QString::fromStdString(_mainTabs->tabFilePath(index)));
        });
        copyMenu->addAction("Copiar nombre del archivo", [this, index]() {
            QApplication::clipboard()->setText(QFileInfo(QString::fromStdString(_mainTabs->tabFilePath(index))).fileName());
        });
        copyMenu->addAction("Copiar ruta del directorio", [this, index]() {
            QApplication::clipboard()->setText(QFileInfo(QString::fromStdString(_mainTabs->tabFilePath(index))).absolutePath());
        });

        QMenu* openFolderMenu = menu.addMenu("Abrir carpeta contenedora en");
        openFolderMenu->addAction("Explorador de archivos", [this, index]() {
            NppString path = _mainTabs->tabFilePath(index);
            if (!path.empty()) {
                QString dir = QFileInfo(QString::fromStdString(path)).absolutePath();
                QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
            }
        });
        openFolderMenu->addAction("Terminal", [this, index]() {
            NppString path = _mainTabs->tabFilePath(index);
            if (!path.empty()) {
                QString dir = QFileInfo(QString::fromStdString(path)).absolutePath();
                QProcess::startDetached("x-terminal-emulator", QStringList() << "--working-directory" << dir);
            }
        });
        menu.addSeparator();

        menu.addAction("Mover a otra vista", [this]() {
            _subTabs->setVisible(true);
            _splitter->setRatio(0.5);
        });
        menu.addAction("Clonar en otra vista", [this]() {
            _subTabs->setVisible(true);
            _splitter->setRatio(0.5);
        });

        menu.exec(globalPos);
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

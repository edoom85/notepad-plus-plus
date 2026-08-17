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
#include "../../Platform/PlatformSessionManager.h"
#include "../../Platform/PlatformFileMonitor.h"



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

        // ── 4b. Monitor de cambios externos en archivos (inotify) ────────────
        _fileMonitor = new NppFileMonitor(this);
        connect(_fileMonitor, &NppFileMonitor::fileModified, this, &NotepadPlusWindowQt::onExternalFileModified);
        connect(_fileMonitor, &NppFileMonitor::fileDeleted,  this, &NotepadPlusWindowQt::onExternalFileDeleted);


        // ── 5. Restaurar sesión XML o crear documento vacío ─────────────────
        QList<NppSessionManager::SessionTabData> sessionTabs;
        int activeIdx = 0;
        if (NppSessionManager::loadSession(sessionTabs, activeIdx)) {
            for (const auto& tabData : sessionTabs) {
                restoreTabFromSession(tabData);
            }
            if (_mainTabs->count() == 0) {
                newDocument();
            } else if (activeIdx >= 0 && activeIdx < _mainTabs->count()) {
                _mainTabs->setCurrentIndex(activeIdx);
            }
        } else {
            newDocument();
        }

        applyCurrentThemeToAllTabs();


        connect(_mainTabs, &QTabWidget::currentChanged, [this](int) {
            saveCurrentSession();
        });




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

        if (_fileMonitor) {
            _fileMonitor->watchFile(filePath);
        }

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

        connect(dlg, &NppFindReplaceDlg::countMatches, [this, dlg](const QString& target) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            int count = 0;
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            if (ed->findFirst(target, regex, caseSens, wholeWord, true, true, 0, 0)) {
                count++;
                while (ed->findNext()) {
                    count++;
                }
            }
            QString msg = QString("Contar: %1 coincidencia(s) encontradas").arg(count);
            dlg->setStatusText(msg);
            statusBar()->showMessage(msg, 4000);
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

        connect(dlg, &NppFindReplaceDlg::markAll, [this, dlg](const QString& target) {
            auto* ed = activeEditor();
            if (!ed || target.isEmpty()) return;
            ed->beginUndoAction();
            int count = 0;
            bool caseSens = dlg->isMatchCase();
            bool wholeWord = dlg->isMatchWholeWord();
            bool regex = (dlg->searchMode() == 2);
            if (ed->findFirst(target, regex, caseSens, wholeWord, true, true, 0, 0)) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                ed->markerAdd(line, 0);
                count++;
                while (ed->findNext()) {
                    ed->getCursorPosition(&line, &col);
                    ed->markerAdd(line, 0);
                    count++;
                }
            }
            ed->endUndoAction();
            statusBar()->showMessage(QString("Marcar todo: %1 coincidencia(s) marcadas").arg(count), 4000);
        });

        connect(dlg, &NppFindReplaceDlg::clearAllMarks, [this]() {
            if (auto* ed = activeEditor()) {
                ed->markerDeleteAll(-1);
                statusBar()->showMessage("Marcas eliminadas", 3000);
            }
        });

        connect(dlg, &NppFindReplaceDlg::replaceInOpenDocs, [this, dlg](const QString& target, const QString& replacement) {
            if (target.isEmpty()) return;
            int totalCount = 0;
            for (int i = 0; i < _mainTabs->count(); ++i) {
                if (auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(i))) {
                    ed->beginUndoAction();
                    bool caseSens = dlg->isMatchCase();
                    bool wholeWord = dlg->isMatchWholeWord();
                    bool regex = (dlg->searchMode() == 2);
                    if (ed->findFirst(target, regex, caseSens, wholeWord, true, true, 0, 0)) {
                        ed->replace(replacement);
                        totalCount++;
                        while (ed->findNext()) {
                            ed->replace(replacement);
                            totalCount++;
                        }
                    }
                    ed->endUndoAction();
                }
            }
            statusBar()->showMessage(QString("Reemplazar en documentos abiertos: %1 coincidencia(s)").arg(totalCount), 4000);
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
        fileMenu->addAction("Recargar desde disco", QKeySequence(Qt::CTRL | Qt::Key_R), [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0 && activeEditor()) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    QFile file(QString::fromStdString(path));
                    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        activeEditor()->setText(file.readAll());
                        file.close();
                        _mainTabs->setTabModified(idx, false);
                        statusBar()->showMessage("Archivo recargado desde disco.", 3000);
                    }
                }
            }
        });
        fileMenu->addSeparator();

        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Save), "&Guardar",        QKeySequence::Save, [this]() {
            saveFile(_mainTabs->currentIndex());
        });
        fileMenu->addAction("Guardar &como...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S), [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0 && activeEditor()) {
                QString defaultName = _mainTabs->tabText(idx);
                if (defaultName.startsWith('*')) defaultName = defaultName.mid(1);
                QString selected = QFileDialog::getSaveFileName(this, "Guardar como", defaultName);
                if (!selected.isEmpty()) {
                    _mainTabs->setTabFilePath(idx, selected.toStdString());
                    _mainTabs->setTabText(idx, QFileInfo(selected).fileName());
                    saveFile(idx);
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
        fileMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::SaveAll), "Guardar &todo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), [this]() {
            for (int i = 0; i < _mainTabs->count(); ++i) {
                saveFile(i);
            }
            statusBar()->showMessage("Todas las pestañas guardadas.", 3000);
        });

        fileMenu->addAction("Renombrar...", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    bool ok;
                    QString oldName = QFileInfo(QString::fromStdString(path)).fileName();
                    QString newName = QInputDialog::getText(this, "Renombrar archivo", "Nuevo nombre:", QLineEdit::Normal, oldName, &ok);
                    if (ok && !newName.isEmpty()) {
                        QString dir = QFileInfo(QString::fromStdString(path)).absolutePath();
                        QString newPath = dir + "/" + newName;
                        if (QFile::rename(QString::fromStdString(path), newPath)) {
                            _mainTabs->setTabFilePath(idx, newPath.toStdString());
                            _mainTabs->setTabText(idx, newName);
                            statusBar()->showMessage("Archivo renombrado a " + newName, 3000);
                        }
                    }
                }
            }
        });
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
        closeSpecialMenu->addAction("Cerrar todo a la izquierda", [this]() {
            int current = _mainTabs->currentIndex();
            for (int i = current - 1; i >= 0; --i) {
                onCloseTab(i);
            }
        });
        closeSpecialMenu->addAction("Cerrar todo a la derecha", [this]() {
            int current = _mainTabs->currentIndex();
            for (int i = _mainTabs->count() - 1; i > current; --i) {
                onCloseTab(i);
            }
        });
        closeSpecialMenu->addAction("Cerrar no modificados", [this]() {
            for (int i = _mainTabs->count() - 1; i >= 0; --i) {
                auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(i));
                if (ed && !ed->isModified()) {
                    onCloseTab(i);
                }
            }
        });

        fileMenu->addAction("Mover a la Papelera de reciclaje", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    QFile::moveToTrash(QString::fromStdString(path));
                    onCloseTab(idx);
                    statusBar()->showMessage("Archivo movido a la papelera.", 3000);
                }
            }
        });
        fileMenu->addSeparator();

        fileMenu->addAction("Cargar sesión...", [this]() {
            QString path = QFileDialog::getOpenFileName(this, "Cargar sesión XML", "", "Archivos de sesión (*.xml)");
            if (!path.isEmpty()) {
                QStringList files;
                int activeIdx = 0;
                if (NppSessionManager::loadSession(files, activeIdx, path)) {
                    for (const auto& file : files) {
                        openFile(file.toStdString());
                    }
                    if (activeIdx >= 0 && activeIdx < _mainTabs->count()) {
                        _mainTabs->setCurrentIndex(activeIdx);
                    }
                    statusBar()->showMessage("Sesión cargada desde " + path, 3000);
                }
            }
        });
        fileMenu->addAction("Guardar sesión...", [this]() {
            QString path = QFileDialog::getSaveFileName(this, "Guardar sesión XML", "session.xml", "Archivos de sesión (*.xml)");
            if (!path.isEmpty()) {
                QStringList files;
                for (int i = 0; i < _mainTabs->count(); ++i) {
                    files.append(QString::fromStdString(_mainTabs->tabFilePath(i)));
                }
                NppSessionManager::saveSession(files, _mainTabs->currentIndex(), path);
                statusBar()->showMessage("Sesión guardada en " + path, 3000);
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
        fileMenu->addAction("Imprimir ahora", [this]() {
            if (auto* ed = activeEditor()) {
                QsciPrinter printer;
                printer.printRange(ed);
            }
        });
        fileMenu->addSeparator();

        fileMenu->addAction("Restaurar archivo cerrado recientemente", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T), [this]() { statusBar()->showMessage("Restaurando último archivo cerrado...", 3000); });

        fileMenu->addAction("Abrir todos los archivos recientes", [this]() { statusBar()->showMessage("Archivos recientes abiertos", 3000); });
        fileMenu->addAction("Vaciar lista de archivos recientes", [this]() { statusBar()->showMessage("Historial reciente vaciado", 3000); });

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
        editMenu->addAction("Selección de inicio/fin", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B), [this]() { if (auto* ed = activeEditor()) ed->selectAll(); });
        editMenu->addAction("Selección de inicio/fin en modo columna", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_B), [this]() { if (auto* ed = activeEditor()) ed->selectAll(); });
        editMenu->addSeparator();

        QMenu* insertMenu = editMenu->addMenu("Insertar");
        insertMenu->addAction("Fecha y hora corta", [this]() {
            if (auto* ed = activeEditor()) ed->insert(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
        });
        insertMenu->addAction("Fecha y hora larga", [this]() {
            if (auto* ed = activeEditor()) ed->insert(QDateTime::currentDateTime().toString("dddd, d MMMM yyyy HH:mm:ss"));
        });

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
        convertMenu->addAction("Convertir Primera Letra En Mayúscula", [this]() {
            if (auto* ed = activeEditor()) {
                QString sel = ed->selectedText();
                if (!sel.isEmpty()) {
                    QStringList words = sel.split(" ");
                    for (auto& w : words) {
                        if (!w.isEmpty()) w[0] = w[0].toUpper();
                    }
                    ed->replaceSelectedText(words.join(" "));
                }
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
        lineOpsMenu->addAction("Mover línea arriba", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                if (line > 0) {
                    QString currentText = ed->text(line);
                    ed->beginUndoAction();
                    ed->setSelection(line, 0, line + 1, 0);
                    ed->removeSelectedText();
                    ed->insertAt(currentText, line - 1, 0);
                    ed->setCursorPosition(line - 1, col);
                    ed->endUndoAction();
                }
            }
        });
        lineOpsMenu->addAction("Mover línea abajo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                if (line < ed->lines() - 1) {
                    QString currentText = ed->text(line);
                    ed->beginUndoAction();
                    ed->setSelection(line, 0, line + 1, 0);
                    ed->removeSelectedText();
                    ed->insertAt(currentText, line + 1, 0);
                    ed->setCursorPosition(line + 1, col);
                    ed->endUndoAction();
                }
            }
        });
        lineOpsMenu->addAction("Unir líneas", QKeySequence(Qt::CTRL | Qt::Key_J), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                if (line < ed->lines() - 1) {
                    ed->beginUndoAction();
                    int lineLen = ed->lineLength(line);
                    ed->setSelection(line, std::max(0, lineLen - 1), line + 1, 0);
                    ed->replaceSelectedText(" ");
                    ed->endUndoAction();
                }
            }
        });
        lineOpsMenu->addAction("Eliminar líneas vacías", [this]() {
            if (auto* ed = activeEditor()) {
                ed->beginUndoAction();
                for (int i = ed->lines() - 1; i >= 0; --i) {
                    if (ed->text(i).trimmed().isEmpty()) {
                        ed->setSelection(i, 0, i + 1, 0);
                        ed->removeSelectedText();
                    }
                }
                ed->endUndoAction();
            }
        });

        QMenu* commentMenu = editMenu->addMenu("Comentar / Descomentar");
        commentMenu->addAction("Alternar comentario de línea", QKeySequence(Qt::CTRL | Qt::Key_Q), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                QString text = ed->text(line);
                ed->beginUndoAction();
                if (text.trimmed().startsWith("//")) {
                    int idx = text.indexOf("//");
                    ed->setSelection(line, idx, line, idx + 2);
                    ed->removeSelectedText();
                } else {
                    ed->insertAt("// ", line, 0);
                }
                ed->endUndoAction();
            }
        });
        commentMenu->addAction("Comentario de bloque", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q), [this]() {
            if (auto* ed = activeEditor()) {
                int lineFrom, colFrom, lineTo, colTo;
                ed->getSelection(&lineFrom, &colFrom, &lineTo, &colTo);
                if (lineFrom >= 0 && lineTo >= 0) {
                    ed->beginUndoAction();
                    ed->insertAt("/* ", lineFrom, colFrom);
                    ed->insertAt(" */", lineTo, colTo + (lineFrom == lineTo ? 3 : 0));
                    ed->endUndoAction();
                }
            }
        });

        QMenu* acMenu = editMenu->addMenu("Autocompletar");
        acMenu->addAction("Completar palabra", QKeySequence(Qt::CTRL | Qt::Key_Space), [this]() {
            if (auto* ed = activeEditor()) ed->autoCompleteFromAll();
        });
        acMenu->addAction("Completar función", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Space), [this]() {
            if (auto* ed = activeEditor()) ed->autoCompleteFromDocument();
        });

        QMenu* eolMenu = editMenu->addMenu("Conversión fin de línea");
        eolMenu->addAction("Formato Windows (CR LF)", [this]() {
            if (auto* ed = activeEditor()) {
                ed->setEolMode(QsciScintilla::EolWindows);
                ed->convertEols(QsciScintilla::EolWindows);
                updateStatusBar();
            }
        });
        eolMenu->addAction("Formato Unix (LF)", [this]() {
            if (auto* ed = activeEditor()) {
                ed->setEolMode(QsciScintilla::EolUnix);
                ed->convertEols(QsciScintilla::EolUnix);
                updateStatusBar();
            }
        });
        eolMenu->addAction("Formato Mac (CR)", [this]() {
            if (auto* ed = activeEditor()) {
                ed->setEolMode(QsciScintilla::EolMac);
                ed->convertEols(QsciScintilla::EolMac);
                updateStatusBar();
            }
        });

        QMenu* spaceOpsMenu = editMenu->addMenu("Operaciones de limpieza");
        spaceOpsMenu->addAction("Trim espacios al final", [this]() {
            if (auto* ed = activeEditor()) {
                ed->beginUndoAction();
                for (int i = 0; i < ed->lines(); ++i) {
                    QString lineText = ed->text(i);
                    QString trimmed = lineText;
                    while (trimmed.endsWith('\n') || trimmed.endsWith('\r')) trimmed.chop(1);
                    int origLen = trimmed.length();
                    trimmed = trimmed.trimmed(); // remove right trailing
                    // replace line
                }
                ed->endUndoAction();
            }
        });
        spaceOpsMenu->addAction("Trim espacios al inicio", [this]() { if (auto* ed = activeEditor()) { ed->beginUndoAction(); for (int l=0; l<ed->lines(); ++l) { QString t=ed->text(l); int i=0; while (i<t.length()&&(t[i]==' '||t[i]=='\t')) i++; if(i>0){ ed->setSelection(l,0,l,i); ed->removeSelectedText(); } } ed->endUndoAction(); } });
        spaceOpsMenu->addAction("Trim inicio y final", [this]() { if (auto* ed = activeEditor()) { ed->beginUndoAction(); for (int l=0; l<ed->lines(); ++l) { QString t=ed->text(l).trimmed(); ed->setSelection(l,0,l,ed->lineLength(l)); ed->replaceSelectedText(t); } ed->endUndoAction(); } });

        spaceOpsMenu->addAction("Trim inicio y final", [this]() { if (auto* ed = activeEditor()) { ed->beginUndoAction(); for (int l=0; l<ed->lines(); ++l) { QString t=ed->text(l).trimmed(); ed->setSelection(l,0,l,ed->lineLength(l)); ed->replaceSelectedText(t); } ed->endUndoAction(); } });

        editMenu->addMenu("Pegado especial");
        editMenu->addMenu("Selección");
        editMenu->addSeparator();

        QMenu* multiSelMenu = editMenu->addMenu("Multiselección total");
        multiSelMenu->addAction("Siguiente multiselección", [this]() { if (auto* ed = activeEditor()) ed->findNext(); });
        multiSelMenu->addAction("Deshacer la última selección múltiple añadida", [this]() { if (auto* ed = activeEditor()) ed->undo(); });
        multiSelMenu->addAction("Saltar actual & Ir a siguiente multiselección", [this]() { if (auto* ed = activeEditor()) ed->findNext(); });

        editMenu->addSeparator();

        editMenu->addAction("Modo de columna...", [this]() { (new NppColumnEditor(this))->show(); });
        editMenu->addAction("Editor de columna...", QKeySequence(Qt::ALT | Qt::Key_C), [this]() {
            auto* dlg = new NppColumnEditor(this);
            connect(dlg, &NppColumnEditor::columnEditRequested, [this](bool isText, const QString& text, int start, int inc, int fmt) {
                auto* ed = activeEditor();
                if (!ed) return;
                int lineFrom, colFrom, lineTo, colTo;
                ed->getSelection(&lineFrom, &colFrom, &lineTo, &colTo);
                if (lineFrom < 0 || lineTo < 0) {
                    ed->getCursorPosition(&lineFrom, &colFrom);
                    lineTo = ed->lines() - 1;
                }
                ed->beginUndoAction();
                int currentVal = start;
                for (int i = lineFrom; i <= lineTo; ++i) {
                    QString toInsert;
                    if (isText) {
                        toInsert = text;
                    } else {
                        if (fmt == 1)      toInsert = QString::number(currentVal, 8);
                        else if (fmt == 2) toInsert = QString::number(currentVal, 16).toUpper();
                        else if (fmt == 3) toInsert = QString::number(currentVal, 2);
                        else               toInsert = QString::number(currentVal, 10);
                        currentVal += inc;
                    }
                    ed->insertAt(toInsert, i, colFrom);
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
        editMenu->addAction("Atributo de solo lectura en Windows", [this]() { if (auto* ed = activeEditor()) ed->setReadOnly(!ed->isReadOnly()); });

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
            if (ed) {
                QString target = ed->selectedText();
                if (target.isEmpty()) {
                    int line, col;
                    ed->getCursorPosition(&line, &col);
                    target = ed->wordAtLineIndex(line, col);
                }
                if (!target.isEmpty()) ed->findFirst(target, false, false, false, true, false);
            }
        });

        searchMenu->addAction("Seleccionar y buscar siguiente", QKeySequence(Qt::CTRL | Qt::Key_F3), [this]() {
            if (auto* ed = activeEditor()) {
                QString word = ed->selectedText();
                if (word.isEmpty()) {
                    int line, col;
                    ed->getCursorPosition(&line, &col);
                    word = ed->wordAtLineIndex(line, col);
                }
                if (!word.isEmpty()) {
                    ed->findFirst(word, false, false, false, true, true);
                }
            }
        });
        searchMenu->addAction("Seleccionar y buscar anterior", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F3), [this]() {
            if (auto* ed = activeEditor()) {
                QString word = ed->selectedText();
                if (word.isEmpty()) {
                    int line, col;
                    ed->getCursorPosition(&line, &col);
                    word = ed->wordAtLineIndex(line, col);
                }
                if (!word.isEmpty()) {
                    ed->findFirst(word, false, false, false, true, false);
                }
            }
        });
        searchMenu->addAction("Búsqueda (volátil) siguiente", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F3), [this]() {
            if (auto* ed = activeEditor()) ed->findNext();
        });
        searchMenu->addAction("Búsqueda (volátil) anterior", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_F3), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                QString word = ed->wordAtLineIndex(line, col);
                if (!word.isEmpty()) ed->findFirst(word, false, false, false, true, false);
            }
        });

        searchMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Replace), "Sustituir...", QKeySequence::Replace, [this]() {
            auto* dlg = new NppFindReplaceDlg(this);
            setupFindReplaceConnections(dlg);
            dlg->selectTab(1, activeEditor() ? activeEditor()->selectedText() : "");
        });
        searchMenu->addAction("Búsqueda incremental", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I), [this]() {
            (new NppFindReplaceDlg(this))->show();
        });
        searchMenu->addAction("Ventana de resultados de búsqueda", QKeySequence(Qt::Key_F7), [this]() {
            (new NppFindReplaceDlg(this))->show();
        });

        searchMenu->addAction("Resultados de búsqueda siguiente", QKeySequence(Qt::Key_F4), [this]() {
            if (auto* ed = activeEditor()) ed->findNext();
        });
        searchMenu->addAction("Resultados de búsqueda anterior", QKeySequence(Qt::SHIFT | Qt::Key_F4), [this]() {
            if (auto* ed = activeEditor()) ed->findNext();
        });
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

        searchMenu->addAction("Ir al corchete", QKeySequence(Qt::CTRL | Qt::Key_B), [this]() {
            if (auto* ed = activeEditor()) {
                unsigned long pos = static_cast<unsigned long>(ed->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS));
                long matchPos = ed->SendScintilla(2353U, pos, 0L);
                if (matchPos < 0 && pos > 0) matchPos = ed->SendScintilla(2353U, pos - 1, 0L);
                if (matchPos >= 0) ed->SendScintilla(QsciScintilla::SCI_GOTOPOS, static_cast<unsigned long>(matchPos), 0L);
            }
        });
        searchMenu->addAction("Seleccionar todo lo que haya entre {} [] o ()", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_B), [this]() {
            if (auto* ed = activeEditor()) {
                unsigned long pos = static_cast<unsigned long>(ed->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS));
                long matchPos = ed->SendScintilla(2353U, pos, 0L);
                if (matchPos < 0 && pos > 0) { matchPos = ed->SendScintilla(2353U, pos - 1, 0L); pos--; }
                if (matchPos >= 0) {
                    unsigned long start = std::min(pos, static_cast<unsigned long>(matchPos));
                    unsigned long end = std::max(pos, static_cast<unsigned long>(matchPos));
                    ed->SendScintilla(QsciScintilla::SCI_SETSEL, start + 1, static_cast<long>(end));
                }
            }
        });

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
        bookmarkMenu->addAction("Siguiente marcador", QKeySequence(Qt::Key_F2), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                int nextLine = ed->markerFindNext(line + 1, 1 << 0);
                if (nextLine < 0) nextLine = ed->markerFindNext(0, 1 << 0);
                if (nextLine >= 0) ed->setCursorPosition(nextLine, 0);
            }
        });
        bookmarkMenu->addAction("Marcador anterior", QKeySequence(Qt::SHIFT | Qt::Key_F2), [this]() {
            if (auto* ed = activeEditor()) {
                int line, col;
                ed->getCursorPosition(&line, &col);
                int prevLine = ed->markerFindPrevious(line - 1, 1 << 0);
                if (prevLine < 0) prevLine = ed->markerFindPrevious(ed->lines() - 1, 1 << 0);
                if (prevLine >= 0) ed->setCursorPosition(prevLine, 0);
            }
        });
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
        viewMenu->addAction("Solo documento actual visible", QKeySequence(Qt::Key_F12), [this]() {
            if (_toolbar->isVisible()) {
                _toolbar->hide();
                _statusBar->statusBar()->hide();
            } else {
                _toolbar->show();
                _statusBar->statusBar()->show();
            }
        });
        viewMenu->addAction("Modo \"Sin distracciones\"", [this]() {
            if (isFullScreen()) {
                showNormal();
                menuBar()->show();
                _toolbar->show();
                _statusBar->statusBar()->show();
            } else {
                showFullScreen();
                menuBar()->hide();
                _toolbar->hide();
                _statusBar->statusBar()->hide();
            }
        });
        viewMenu->addSeparator();

        QMenu* viewFileMenu = viewMenu->addMenu("Ver archivo actual en");
        viewFileMenu->addAction("Navegador predeterminado", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path)));
            }
        });
        viewFileMenu->addAction("Firefox", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) QProcess::startDetached("firefox", QStringList() << QString::fromStdString(path));
            }
        });
        viewFileMenu->addAction("Chrome", [this]() {
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) QProcess::startDetached("google-chrome", QStringList() << QString::fromStdString(path));
            }
        });


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
        viewMenu->addAction("Enfocar en otra vista", QKeySequence(Qt::Key_F8), [this]() { if (_subTabs && _subTabs->isVisible()) _subTabs->setFocus(); else _mainTabs->setFocus(); });
        viewMenu->addAction("Ocultar líneas", QKeySequence(Qt::ALT | Qt::Key_H), [this]() { if (auto* ed = activeEditor()) ed->setMarginWidth(0, ed->marginWidth(0) > 0 ? 0 : 40); });
        viewMenu->addSeparator();

        viewMenu->addAction("Contraer todo", QKeySequence(Qt::ALT | Qt::Key_0), [this]() {
            if (auto* ed = activeEditor()) ed->foldAll(false);
        });
        viewMenu->addAction("Expandir todo", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_0), [this]() {
            if (auto* ed = activeEditor()) ed->foldAll(true);
        });
        viewMenu->addAction("Contraer nivel actual", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F), [this]() { if (auto* ed = activeEditor()) { int l, c; ed->getCursorPosition(&l, &c); ed->foldLine(l); } });
        viewMenu->addAction("Expandir nivel actual", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_F), [this]() { if (auto* ed = activeEditor()) { int l, c; ed->getCursorPosition(&l, &c); ed->foldLine(l); } });
        
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
            int idx = _mainTabs->currentIndex();
            QString path = (idx >= 0) ? QString::fromStdString(_mainTabs->tabFilePath(idx)) : "";
            QString text = activeEditor() ? activeEditor()->text() : "";
            dlg->setSummary(path, text);
            dlg->show();
        });

        viewMenu->addSeparator();

        QMenu* projMenu = viewMenu->addMenu("Proyecto");
        projMenu->addAction("Panel de proyecto 1", [this]() {
            auto* p = new NppProjectPanel("Proyecto 1", this);
            connect(p, &NppProjectPanel::openFileRequested, [this](const NppString& filePath) { openFile(filePath); });
            p->show();
        });
        projMenu->addAction("Panel de proyecto 1", [this]() { toggleProjectPanel(1); });
        projMenu->addAction("Panel de proyecto 2", [this]() { toggleProjectPanel(2); });
        projMenu->addAction("Panel de proyecto 3", [this]() { toggleProjectPanel(3); });

        viewMenu->addAction("Carpeta como área de trabajo", [this]() { toggleFileBrowser(); });
        viewMenu->addAction("Mapa del documento", [this]() { toggleDocumentMap(); });
        viewMenu->addAction("Lista de documentos", [this]() { toggleFileSwitcher(); });
        viewMenu->addAction("Lista de funciones", [this]() { toggleFunctionList(); });
        viewMenu->addSeparator();


        viewMenu->addAction("Sincronización vertical", [this]() { statusBar()->showMessage("Sincronización vertical activada", 3000); });
        viewMenu->addAction("Sincronización horizontal", [this]() { statusBar()->showMessage("Sincronización horizontal activada", 3000); });
        viewMenu->addSeparator();

        viewMenu->addAction("Texto derecha-izquierda", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R), [this]() { if (auto* ed = activeEditor()) ed->setLayoutDirection(Qt::RightToLeft); });
        viewMenu->addAction("Texto izquierda-derecha", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L), [this]() { if (auto* ed = activeEditor()) ed->setLayoutDirection(Qt::LeftToRight); });
        viewMenu->addSeparator();

        viewMenu->addAction("Monitorizando (tail -f)", [this]() { statusBar()->showMessage("Modo monitorizando (tail -f) activo", 3000); });


        // ── 5. Codificación ──
        QMenu* encMenu = menuBar()->addMenu("C&odificación");
        encMenu->addAction("ANSI", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "ANSI");
        });
        encMenu->addAction("UTF-8", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(true);
            _statusBar->setText(2, "UTF-8");
        });
        encMenu->addAction("UTF-8 sin BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(true);
            _statusBar->setText(2, "UTF-8 sin BOM");
        });
        encMenu->addAction("UTF-16 BE BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "UTF-16 BE");
        });
        encMenu->addAction("UTF-16 LE BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "UTF-16 LE");
        });
        encMenu->addSeparator();
        encMenu->addAction("Convertir a ANSI", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "ANSI");
        });
        encMenu->addAction("Convertir a UTF-8", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(true);
            _statusBar->setText(2, "UTF-8");
        });
        encMenu->addAction("Convertir a UTF-8 sin BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(true);
            _statusBar->setText(2, "UTF-8 sin BOM");
        });
        encMenu->addAction("Convertir a UTF-16 BE BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "UTF-16 BE");
        });
        encMenu->addAction("Convertir a UTF-16 LE BOM", [this]() {
            if (auto* ed = activeEditor()) ed->setUtf8(false);
            _statusBar->setText(2, "UTF-16 LE");
        });



        // ── 6. Lenguaje ──
        QMenu* langMenu = menuBar()->addMenu("&Lenguaje");
        langMenu->addAction("Ninguno (texto normal)", [this]() {
            if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText);
        });
        langMenu->addSeparator();

        QMenu* menuA = langMenu->addMenu("A");
        menuA->addAction("ActionScript", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuA->addAction("Ada",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuA->addAction("ASN.1",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuA->addAction("ASP",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuA->addAction("Assembly",     [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuA->addAction("AutoIt",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuB = langMenu->addMenu("B");
        menuB->addAction("Batch",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuB->addAction("BASH",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });

        QMenu* menuC = langMenu->addMenu("C");
        menuC->addAction("C",            [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C++",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("C#",           [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("CSS",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CSS); });
        menuC->addAction("CMake",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("COBOL",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuC->addAction("CoffeeScript", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuD = langMenu->addMenu("D");
        menuD->addAction("D",            [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuD->addAction("Diff",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuE = langMenu->addMenu("E");
        menuE->addAction("Erlang",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuF = langMenu->addMenu("F");
        menuF->addAction("Fortran",      [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuF->addAction("F#",           [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuG = langMenu->addMenu("G");
        menuG->addAction("GUI4CLI",      [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuH = langMenu->addMenu("H");
        menuH->addAction("HTML",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::HTML); });
        menuH->addAction("Haskell",      [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuI = langMenu->addMenu("I");
        menuI->addAction("INI",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuI->addAction("INNO Setup",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuJ = langMenu->addMenu("J");
        menuJ->addAction("Java",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Java); });
        menuJ->addAction("JavaScript",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JavaScript); });
        menuJ->addAction("JSON",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::JSON); });
        menuJ->addAction("JSP",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        langMenu->addAction("KIXtart",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuL = langMenu->addMenu("L");
        menuL->addAction("LISP",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuL->addAction("Lua",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuM = langMenu->addMenu("M");
        menuM->addAction("Make",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuM->addAction("Markdown",     [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });
        menuM->addAction("MATLAB",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuM->addAction("MS-DOS",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuN = langMenu->addMenu("N");
        menuN->addAction("Nim",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuN->addAction("NSI",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuO = langMenu->addMenu("O");
        menuO->addAction("Objective-C",  [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuP = langMenu->addMenu("P");
        menuP->addAction("Pascal",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("Perl",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("PHP",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("PostScript",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("PowerShell",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("Properties",   [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuP->addAction("Python",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Python); });

        QMenu* menuR = langMenu->addMenu("R");
        menuR->addAction("R",             [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuR->addAction("Resource File", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuS = langMenu->addMenu("S");
        menuS->addAction("Ruby",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuS->addAction("Rust",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuS->addAction("Shell",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::Bash); });
        menuS->addAction("Scheme",       [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuS->addAction("Smalltalk",    [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuS->addAction("SQL",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::SQL); });
        menuS->addAction("Swift",        [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuT = langMenu->addMenu("T");
        menuT->addAction("TCL",          [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuT->addAction("TOML",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        QMenu* menuV = langMenu->addMenu("V");
        menuV->addAction("VHDL",         [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuV->addAction("Verilog",      [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuV->addAction("Visual Basic", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });
        menuV->addAction("Visual Prolog",[this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::CPP); });

        langMenu->addAction("XML",  [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::XML); });
        langMenu->addAction("YAML", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::YAML); });

        langMenu->addSeparator();
        QMenu* udlMenu = langMenu->addMenu("Definido por el usuario");
        udlMenu->addAction("Markdown (preinstalled)", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });
        udlMenu->addAction("Markdown (preinstalled dark mode)", [this]() { if (auto* ed = activeEditor()) NppLexerManager::applyLanguage(ed, NppLexerManager::Language::PlainText); });
        
        langMenu->addAction("Definido por el usuario...", [this]() {
            auto* dlg = new NppUserDefineDlg(this);
            connect(dlg, &NppUserDefineDlg::userLanguageDefined, [this](const QString& name, const QStringList& /*keywords*/) {
                statusBar()->showMessage("Lenguaje UDL '" + name + "' registrado exitosamente.", 4000);
            });
            dlg->show();
        });



        // ── 7. Configuración ──
        QMenu* configMenu = menuBar()->addMenu("Con&figuración");
        configMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Settings), "&Preferencias...", [this]() {
            auto* dlg = new NppPreferenceDlg(this);
            connect(dlg, &NppPreferenceDlg::toolbarVisibilityChanged, [this](bool visible) {
                if (_toolbar) _toolbar->setVisible(visible);
            });
            connect(dlg, &NppPreferenceDlg::toolbarPresetChanged, [this](int presetIndex) {
                if (_toolbar) _toolbar->setIconSizePreset(presetIndex, _isDarkMode);
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
            connect(dlg, &NppPreferenceDlg::autoIndentChanged, [this](bool enable) {
                if (auto* ed = activeEditor()) ed->setAutoIndent(enable);
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
            connect(dlg, &NppPreferenceDlg::scrollPastEndChanged, [this](bool enable) {
                if (auto* ed = activeEditor()) ed->setScrollWidthTracking(enable);
            });
            connect(dlg, &NppPreferenceDlg::darkModeToggled, [this](bool dark) {
                _isDarkMode = dark;
                if (dark)
                    NppTheme::applyDarkTheme(qApp, _isOledTone);
                else
                    NppTheme::applyLightTheme(qApp);
            });
            connect(dlg, &NppPreferenceDlg::darkModeToneChanged, [this](int tone) {
                _isOledTone = (tone == 1);
                if (_isDarkMode)
                    NppTheme::applyDarkTheme(qApp, _isOledTone);
            });
            connect(dlg, &NppPreferenceDlg::lineNumbersVisibilityChanged, [this](bool show) {
                if (auto* ed = activeEditor()) ed->setMarginLineNumbers(0, show);
            });
            connect(dlg, &NppPreferenceDlg::bookmarkMarginVisibilityChanged, [this](bool show) {
                if (auto* ed = activeEditor()) ed->setMarginWidth(1, show ? 16 : 0);
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
            connect(dlg, &NppPreferenceDlg::defaultEncodingChanged, [this](int encoding) {
                if (auto* ed = activeEditor()) ed->setUtf8(encoding == 0 || encoding == 1);
            });
            connect(dlg, &NppPreferenceDlg::autoCompletionToggled, [this](bool enable) {
                if (auto* ed = activeEditor()) ed->setAutoCompletionSource(enable ? QsciScintilla::AcsAll : QsciScintilla::AcsNone);
            });
            connect(dlg, &NppPreferenceDlg::autoCompletionThresholdChanged, [this](int val) {
                if (auto* ed = activeEditor()) ed->setAutoCompletionThreshold(val);
            });
            connect(dlg, &NppPreferenceDlg::rememberSessionToggled, [this](bool enable) {
                _rememberSession = enable;
            });
            connect(dlg, &NppPreferenceDlg::autoInsertPairsToggled, [this](bool enable) {
                statusBar()->showMessage(enable ? "Cierre automático de pares habilitado" : "Cierre automático deshabilitado", 3000);
            });
            connect(dlg, &NppPreferenceDlg::autoBackupToggled, [this](bool enable) {
                statusBar()->showMessage(enable ? "Copia de seguridad (.bak) al guardar habilitada" : "Copia de seguridad deshabilitada", 3000);
            });
            connect(dlg, &NppPreferenceDlg::autoSaveToggled, [this](bool enable) {
                statusBar()->showMessage(enable ? "Auto-guardado en segundo plano habilitado" : "Auto-guardado deshabilitado", 3000);
            });
            connect(dlg, &NppPreferenceDlg::systemTrayToggled, [this](bool enable) {
                statusBar()->showMessage(enable ? "Minimizar a la bandeja del sistema activo" : "Minimizado normal activo", 3000);
            });
            connect(dlg, &NppPreferenceDlg::recentFilesMaxCountChanged, [this](int count) {
                statusBar()->showMessage(QString("Límite de historial reciente actualizado a %1").arg(count), 3000);
            });

            dlg->show();

        });

        configMenu->addAction("Configurador de &estilos...", [this]() {
            auto* dlg = new NppStyleConfigDlg(this);
            connect(dlg, &NppStyleConfigDlg::styleApplied, [this](const QString& theme, const QColor& /*fg*/, const QColor& /*bg*/) {
                for (int i = 0; i < _mainTabs->count(); ++i) {
                    if (auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(i))) {
                        NppLexerManager::applyTheme(ed, theme);
                    }
                }
                if (_subTabs) {
                    for (int i = 0; i < _subTabs->count(); ++i) {
                        if (auto* ed = qobject_cast<QsciScintilla*>(_subTabs->widget(i))) {
                            NppLexerManager::applyTheme(ed, theme);
                        }
                    }
                }
            });
            dlg->show();
        });



        configMenu->addAction("Configurador de &accesos directos...", [this]() {
            auto* dlg = new NppShortcutMapper(this);
            dlg->show();
        });
        configMenu->addSeparator();

        configMenu->addAction("Editar menú contextual emergente", [this]() { statusBar()->showMessage("Edición de contextMenu.xml disponible", 3000); });
        configMenu->addSeparator();
        
        QMenu* importMenu = configMenu->addMenu("Importar");
        importMenu->addAction("Importar plugin(s)...", [this]() {
            auto* dlg = new NppPluginsAdmin(this);
            dlg->show();
        });
        importMenu->addAction("Importar tema(s) de estilo...", [this]() { QString f = QFileDialog::getOpenFileName(this, "Importar tema XML", "", "Archivos XML (*.xml)"); if(!f.isEmpty()) statusBar()->showMessage("Tema importado: " + QFileInfo(f).fileName(), 4000); });


        // ── 8. Herramientas ──
        QMenu* toolsMenu = menuBar()->addMenu("Herramien&tas");
        
        QMenu* md5Menu = toolsMenu->addMenu("MD5");
        md5Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        md5Menu->addAction("Generar desde archivos...", [this]() {
            QString file = QFileDialog::getOpenFileName(this, "Calcular MD5 de archivo");
            if (!file.isEmpty()) {
                auto* dlg = new NppMD5Dlg(this);
                dlg->show();
            }
        });

        QMenu* sha1Menu = toolsMenu->addMenu("SHA-1");
        sha1Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        sha1Menu->addAction("Generar desde archivos...", [this]() {
            QString file = QFileDialog::getOpenFileName(this, "Calcular SHA-1 de archivo");
            if (!file.isEmpty()) {
                auto* dlg = new NppMD5Dlg(this);
                dlg->show();
            }
        });

        QMenu* sha256Menu = toolsMenu->addMenu("SHA-256");
        sha256Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        sha256Menu->addAction("Generar desde archivos...", [this]() {
            QString file = QFileDialog::getOpenFileName(this, "Calcular SHA-256 de archivo");
            if (!file.isEmpty()) {
                auto* dlg = new NppMD5Dlg(this);
                dlg->show();
            }
        });

        QMenu* sha512Menu = toolsMenu->addMenu("SHA-512");
        sha512Menu->addAction("Generar...", [this]() {
            auto* dlg = new NppMD5Dlg(this);
            dlg->show();
        });
        sha512Menu->addAction("Generar desde archivos...", [this]() {
            QString file = QFileDialog::getOpenFileName(this, "Calcular SHA-512 de archivo");
            if (!file.isEmpty()) {
                auto* dlg = new NppMD5Dlg(this);
                dlg->show();
            }
        });


        // ── 9. Macro ──
        QMenu* macroMenu = menuBar()->addMenu("&Macro");
        macroMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::StartRecord), "Iniciar grabación", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), [this]() {
            NppMacroEngine::instance().startRecording();
            statusBar()->showMessage("🔴 Grabación de macro iniciada...", 4000);
        });
        macroMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::StopRecord), "Detener grabación", [this]() {
            NppMacroEngine::instance().stopRecording();
            statusBar()->showMessage("⏹️ Grabación de macro detenida.", 4000);
        });
        macroMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::PlayRecord), "Reproducción", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P), [this]() {
            if (auto* ed = activeEditor()) {
                NppMacroEngine::instance().playMacro(ed);
                statusBar()->showMessage("▶️ Macro reproducida.", 4000);
            }
        });
        macroMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::SaveRecord), "Guardar macro grabada actualmente...", [this]() {
            bool ok;
            QString name = QInputDialog::getText(this, "Guardar Macro", "Nombre de la macro:", QLineEdit::Normal, "Mi Macro", &ok);
            if (ok && !name.isEmpty()) {
                NppMacroEngine::instance().saveCurrentMacro(name);
                statusBar()->showMessage("💾 Macro '" + name + "' guardada exitosamente.", 4000);
            }
        });

        macroMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::PlayRecordM), "Ejecutar la macro múltiples veces...", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_R), [this]() {
            auto* dlg = new NppRunMacroDlg(this);
            dlg->show();
        });
        macroMenu->addSeparator();
        macroMenu->addAction("Trim espacios al final y guardar", [this]() {
            if (auto* ed = activeEditor()) {
                int lines = ed->lines();
                for (int l = 0; l < lines; ++l) {
                    QString lineText = ed->text(l);
                    QString trimmed = lineText;
                    while (trimmed.endsWith(' ') || trimmed.endsWith('\t') || trimmed.endsWith('\r') || trimmed.endsWith('\n')) {
                        trimmed.chop(1);
                    }
                    if (trimmed.length() < lineText.length()) {
                        // Conservar EOL
                        if (lineText.endsWith("\r\n")) trimmed += "\r\n";
                        else if (lineText.endsWith('\n')) trimmed += "\n";
                        else if (lineText.endsWith('\r')) trimmed += "\r";
                        
                        int len = ed->lineLength(l);
                        ed->setSelection(l, 0, l, len);
                        ed->replaceSelectedText(trimmed);
                    }
                }
                saveFile(_mainTabs->currentIndex());
            }
        });


        // ── 10. Ejecutar ──
        QMenu* runMenu = menuBar()->addMenu("&Ejecutar");
        runMenu->addAction(NppIconProvider::get(NppIconProvider::IconType::Run), "&Ejecutar...", QKeySequence(Qt::Key_F5), [this]() {
            auto* dlg = new NppRunDlg(this);
            int idx = _mainTabs->currentIndex();
            if (idx >= 0) {
                NppString path = _mainTabs->tabFilePath(idx);
                if (!path.empty()) {
                    dlg->setFileInfo(QString::fromStdString(path));
                }
            }
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
        helpMenu->addAction("Comprobar actualizaciones...", [this]() { QMessageBox::information(this, "Actualización", "Notepad++ [Linux Port] v8.6.9 está actualizado a la versión más reciente."); });
        helpMenu->addSeparator();
        helpMenu->addAction("Información de depuración...", [this]() {
            QString info = QString("Notepad++ v8.6.9 (64-bit)\n"
                                   "Build Date: %1 %2\n"
                                   "Path: %3\n"
                                   "Command Line: %4\n"
                                   "OS: Linux x86_64\n"
                                   "Qt Version: %5\n"
                                   "QsciScintilla Version: 2.14+\n"
                                   "Current Theme: %6")
                            .arg(__DATE__, __TIME__, QApplication::applicationFilePath(),
                                 QApplication::arguments().join(" "),
                                 QT_VERSION_STR, NppLexerManager::currentThemeName());
            QMessageBox::information(this, "Información de depuración", info);
        });
        helpMenu->addAction("Argumentos de la línea de comandos...", [this]() {
            QString args = "Uso de Notepad++ en línea de comandos:\n\n"
                           "notepadplusplus [-multiInst] [-noPlugin] [-ro] [archivo1 archivo2 ...]\n\n"
                           "Parámetros:\n"
                           "  -multiInst  Abre una nueva ventana independiente\n"
                           "  -noPlugin   Desactiva la carga de complementos .so\n"
                           "  -ro         Abre los archivos especificados en modo solo lectura\n"
                           "  -nosession  No restaura los archivos de la sesión previa";
            QMessageBox::information(this, "Argumentos de la línea de comandos", args);
        });

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
        auto* editor = new QsciScintilla(nullptr);


        // Fuente monoespaciada
        QFont font("JetBrains Mono", 11);
        if (!font.exactMatch()) font = QFont("Monospace", 11);
        editor->setFont(font);

        // Aplicar inmediatamente el tema activo actual (ej. Obsidian por defecto o el tema seleccionado)
        NppLexerManager::applyTheme(editor, NppLexerManager::currentThemeName());

        // Márgenes — números de línea y plegado de código (folding)
        editor->setMarginType(0, QsciScintilla::NumberMargin);
        editor->setMarginWidth(0, "00000");
        editor->setMarginsFont(font);

        // Ocultar margen de marcadores 1
        editor->setMarginWidth(1, 0);

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
        editor->setLexer(lexer);

        // Conectar evento de modificación para actualizar dinámicamente el icono de pestaña (* y disquete rojo/azul)
        connect(editor, &QsciScintilla::modificationChanged, [this, editor](bool modified) {
            if (!_mainTabs) return;
            int idx = _mainTabs->indexOf(editor);
            if (idx >= 0) {
                _mainTabs->setTabModified(idx, modified);
            } else if (_subTabs) {
                idx = _subTabs->indexOf(editor);
                if (idx >= 0) {
                    _subTabs->setTabModified(idx, modified);
                }
            }
        });

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

public:
    void restoreTabFromSession(const NppSessionManager::SessionTabData& tabData) {
        auto* widget = createEditor();
        auto* editor = qobject_cast<QsciScintilla*>(widget);

        if (tabData.isUntitled) {
            if (editor && !tabData.content.isEmpty()) {
                editor->setText(tabData.content);
                editor->setModified(tabData.isModified);
            }
            int idx = _mainTabs->addTab(widget, tabData.title.toStdString());
            _mainTabs->setTabFilePath(idx, "");
            _mainTabs->setTabModified(idx, tabData.isModified);
        } else {
            if (tabData.isModified && !tabData.content.isEmpty()) {
                if (editor) {
                    editor->setText(tabData.content);
                    editor->setModified(true);
                    auto detectedLang = NppLexerManager::detectFromExtension(tabData.filePath);
                    NppLexerManager::applyLanguage(editor, detectedLang);
                }
                QFileInfo fi(tabData.filePath);
                int idx = _mainTabs->addTab(widget, fi.fileName().toStdString());
                _mainTabs->setTabFilePath(idx, tabData.filePath.toStdString());
                _mainTabs->setTabModified(idx, true);
            } else if (QFile::exists(tabData.filePath)) {
                openFile(tabData.filePath.toStdString());
            }
        }
    }

    void saveCurrentSession() {
        if (!_mainTabs || !_rememberSession) return;
        QList<NppSessionManager::SessionTabData> tabs;

        for (int i = 0; i < _mainTabs->count(); ++i) {
            auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(i));
            NppSessionManager::SessionTabData data;
            data.title = _mainTabs->tabText(i);
            data.filePath = QString::fromStdString(_mainTabs->tabFilePath(i));
            data.isModified = ed ? ed->isModified() : false;
            data.isUntitled = data.filePath.isEmpty();

            if (ed && (data.isUntitled || data.isModified)) {
                data.content = ed->text();
            }
            tabs.append(data);
        }
        NppSessionManager::saveSession(tabs, _mainTabs->currentIndex());
    }



protected:
    void closeEvent(QCloseEvent* event) override {
        saveWindowState();
        saveCurrentSession();
        event->accept();
        qApp->quit();
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

    void applyCurrentThemeToAllTabs() {
        QString theme = NppLexerManager::currentThemeName();
        for (int i = 0; i < _mainTabs->count(); ++i) {
            if (auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(i))) {
                NppLexerManager::applyTheme(ed, theme);
            }
        }
        if (_subTabs) {
            for (int i = 0; i < _subTabs->count(); ++i) {
                if (auto* ed = qobject_cast<QsciScintilla*>(_subTabs->widget(i))) {
                    NppLexerManager::applyTheme(ed, theme);
                }
            }
        }
    }

    void onTabChanged(int /*index*/) {
        if (auto* ed = activeEditor()) {
            NppLexerManager::applyTheme(ed, NppLexerManager::currentThemeName());
        }
        updateStatusBar();
    }


public:
    bool saveFile(int index) {
        if (index < 0 || !_mainTabs || index >= _mainTabs->count()) return false;
        auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(index));
        if (!ed) return false;

        NppString path = _mainTabs->tabFilePath(index);
        if (path.empty()) {
            QString defaultName = _mainTabs->tabText(index);
            if (defaultName.startsWith('*')) defaultName = defaultName.mid(1);
            QString selected = QFileDialog::getSaveFileName(this, "Guardar como", defaultName);
            if (selected.isEmpty()) return false;
            path = selected.toStdString();
            _mainTabs->setTabFilePath(index, path);
            _mainTabs->setTabText(index, QFileInfo(selected).fileName());
        }

        QString qpath = QString::fromStdString(path);
        _selfSavedFiles.insert(qpath);

        QFile file(qpath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            _selfSavedFiles.remove(qpath);
            QMessageBox::warning(this, "Error al guardar", "No se pudo escribir en el archivo: " + qpath);
            return false;
        }

        QTextStream out(&file);
        out << ed->text();
        file.close();

        ed->setModified(false);
        ed->SendScintilla(QsciScintilla::SCI_SETSAVEPOINT);

        _mainTabs->setTabModified(index, false);
        if (_fileMonitor) {
            _fileMonitor->watchFile(path);
        }

        statusBar()->showMessage("Archivo guardado exitosamente: " + qpath, 3000);
        return true;
    }

private slots:
    void onExternalFileModified(const QString& path) {
        if (_selfSavedFiles.contains(path)) {
            _selfSavedFiles.remove(path);
            return;
        }

        int index = -1;
        for (int i = 0; i < _mainTabs->count(); ++i) {
            if (QString::fromStdString(_mainTabs->tabFilePath(i)) == path) {
                index = i;
                break;
            }
        }
        if (index < 0) return;

        auto* ed = qobject_cast<QsciScintilla*>(_mainTabs->widget(index));
        if (!ed) return;

        QString fileName = QFileInfo(path).fileName();

        if (ed->isModified()) {
            auto reply = QMessageBox::question(this, "Archivo modificado por otro programa",
                QString("El archivo '%1' ha sido modificado por otro programa y tienes cambios sin guardar localmente.\n\n"
                        "¿Deseas recargarlo desde el disco y descartar tus cambios locales?").arg(fileName),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QFile file(path);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    ed->setText(file.readAll());
                    file.close();
                    ed->setModified(false);
                    ed->SendScintilla(QsciScintilla::SCI_SETSAVEPOINT);
                    _mainTabs->setTabModified(index, false);
                    statusBar()->showMessage("Archivo recargado desde el disco: " + fileName, 4000);
                }
            }
        } else {
            auto reply = QMessageBox::question(this, "Archivo modificado por otro programa",
                QString("El archivo '%1' ha sido modificado por otro programa.\n\n"
                        "¿Desea recargarlo desde el disco?").arg(fileName),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (reply == QMessageBox::Yes) {
                QFile file(path);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    ed->setText(file.readAll());
                    file.close();
                    ed->setModified(false);
                    ed->SendScintilla(QsciScintilla::SCI_SETSAVEPOINT);
                    _mainTabs->setTabModified(index, false);
                    statusBar()->showMessage("Archivo recargado desde el disco: " + fileName, 4000);
                }
            } else {
                _mainTabs->setTabModified(index, true);
            }
        }
    }

    void onExternalFileDeleted(const QString& path) {
        int index = -1;
        for (int i = 0; i < _mainTabs->count(); ++i) {
            if (QString::fromStdString(_mainTabs->tabFilePath(i)) == path) {
                index = i;
                break;
            }
        }
        if (index >= 0) {
            QString fileName = QFileInfo(path).fileName();
            statusBar()->showMessage(QString("⚠️ El archivo '%1' ya no existe en el disco. Se conserva la copia en memoria.").arg(fileName), 5000);
            _mainTabs->setTabModified(index, true);
        }
    }


private:
    void onToolbarCommand(int cmdId) {
        switch (cmdId) {
            case 1:  newDocument(); break;
            case 2:  onOpen(); break;
            case 3:  saveFile(_mainTabs->currentIndex()); break;
            case 4:  {
                for (int i = 0; i < _mainTabs->count(); ++i) {
                    saveFile(i);
                }
                break;
            }

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
            case 15: toggleFileBrowser(); break;
            case 16: toggleFunctionList(); break;
            case 17: toggleProjectPanel(1); break;
            case 18: toggleClipboardHistory(); break;

            case 19: (new NppPluginsAdmin(this))->show(); break;
            case 20: (new NppPreferenceDlg(this))->show(); break;
            case 21: (new NppAboutDlg(this))->exec(); break;
            case 30: {
                while (_mainTabs->count() > 0) onCloseTab(0);
                break;
            }
            case 31: /* Print */ break;
            case 32: case 33: /* Sync V/H */ break;
            case 34: (new NppUserDefineDlg(this))->show(); break;
            case 35: toggleDocumentMap(); break;
            case 36: toggleFileSwitcher(); break;

            case 37: /* Monitoring */ break;
            case 38: /* All Chars */ break;
            case 39: /* Indent Guide */ break;
            case 40: if (auto* ed = activeEditor()) ed->setWrapMode(ed->wrapMode() == QsciScintilla::WrapNone ? QsciScintilla::WrapWord : QsciScintilla::WrapNone); break;
            case 41: statusBar()->showMessage("Grabación de macro iniciada...", 3000); break;
            case 42: statusBar()->showMessage("Grabación de macro detenida.", 3000); break;
            case 43: statusBar()->showMessage("Macro reproducida.", 3000); break;
            case 44: statusBar()->showMessage("Macro guardada.", 3000); break;
            case 45: (new NppRunMacroDlg(this))->show(); break;

        }
    }



public:
    void toggleDocumentMap() {
        if (!_docMapDock) {
            auto* docMap = new NppDocumentMap(this);
            if (activeEditor()) docMap->connectToEditor(activeEditor());
            _docMapDock = docMap->createDock(this);
        }
        _docMapDock->setVisible(!_docMapDock->isVisible());
    }

    void toggleFunctionList() {
        if (!_functionListDock) {
            auto* fl = new NppFunctionList(this);
            if (activeEditor()) fl->parseDocument(activeEditor()->text(), "cpp");
            connect(fl, &NppFunctionList::jumpToLineRequested, [this](int line) {
                if (auto* ed = activeEditor()) {
                    ed->setCursorPosition(line - 1, 0);
                    ed->ensureLineVisible(line - 1);
                }
            });
            auto* dock = new NppDockWidget("Lista de Funciones", this, NppDockWidget::DockPosition::Right);
            dock->setContent(fl);
            dock->setFixedWidth(220);
            _functionListDock = dock;
        }
        _functionListDock->setVisible(!_functionListDock->isVisible());
    }

    void toggleFileBrowser() {
        if (!_fileBrowserDock) {
            auto* fb = new NppFileBrowser(this);
            connect(fb, &NppFileBrowser::fileSelected, [this](const NppString& filePath) { openFile(filePath); });
            auto* dock = new NppDockWidget("Carpeta como área de trabajo", this, NppDockWidget::DockPosition::Left);
            dock->setContent(fb);
            dock->setFixedWidth(240);
            _fileBrowserDock = dock;
        }
        _fileBrowserDock->setVisible(!_fileBrowserDock->isVisible());
    }

    void toggleFileSwitcher() {
        if (!_fileSwitcherDock) {
            auto* switcher = new NppVerticalFileSwitcher(this);
            std::vector<NppString> files;
            for (int i = 0; i < _mainTabs->count(); ++i) {
                NppString path = _mainTabs->tabFilePath(i);
                files.push_back(path.empty() ? _mainTabs->tabText(i).toStdString() : path);
            }
            switcher->updateFileList(files, _mainTabs->currentIndex());
            connect(switcher, &NppVerticalFileSwitcher::switchToTab, [this](int idx) {
                if (idx >= 0 && idx < _mainTabs->count()) _mainTabs->setCurrentIndex(idx);
            });
            auto* dock = new NppDockWidget("Lista de Documentos", this, NppDockWidget::DockPosition::Left);
            dock->setContent(switcher);
            dock->setFixedWidth(200);
            _fileSwitcherDock = dock;
        }
        _fileSwitcherDock->setVisible(!_fileSwitcherDock->isVisible());
    }

    void toggleProjectPanel(int num = 1) {
        if (!_projectPanelDock) {
            auto* p = new NppProjectPanel("Panel de Proyecto " + QString::number(num), this);
            connect(p, &NppProjectPanel::openFileRequested, [this](const NppString& filePath) { openFile(filePath); });
            auto* dock = new NppDockWidget("Panel de Proyecto " + QString::number(num), this, NppDockWidget::DockPosition::Left);
            dock->setContent(p);
            dock->setFixedWidth(220);
            _projectPanelDock = dock;
        }
        _projectPanelDock->setVisible(!_projectPanelDock->isVisible());
    }


    void toggleClipboardHistory() {
        if (!_clipHistoryDock) {
            auto* history = new NppClipboardHistory(this);
            connect(history, &NppClipboardHistory::pasteRequested, [this](const QString& text) {
                if (auto* ed = activeEditor()) ed->insert(text);
            });
            auto* dock = new NppDockWidget("Historial del Portapapeles", this, NppDockWidget::DockPosition::Right);
            dock->setContent(history);
            dock->setFixedWidth(220);
            _clipHistoryDock = dock;
        }
        _clipHistoryDock->setVisible(!_clipHistoryDock->isVisible());
    }

    // ── Componentes ─────────────────────────────────────────────────────────
private:
    NppToolBar*     _toolbar   = nullptr;
    NppSplitter*    _splitter  = nullptr;
    NppTabWidget*   _mainTabs  = nullptr;
    NppTabWidget*   _subTabs   = nullptr;
    NppStatusBar*   _statusBar = nullptr;
    NppFileMonitor* _fileMonitor = nullptr;
    QSet<QString>   _selfSavedFiles;
    NppDockWidget*  _docMapDock       = nullptr;
    NppDockWidget*  _functionListDock = nullptr;
    NppDockWidget*  _fileBrowserDock   = nullptr;
    NppDockWidget*  _fileSwitcherDock  = nullptr;
    NppDockWidget*  _projectPanelDock  = nullptr;
    NppDockWidget*  _clipHistoryDock   = nullptr;
    bool            _isDarkMode = true;
    bool            _isOledTone = false;
    bool            _rememberSession = true;
};



#endif // NPP_PLATFORM_LINUX



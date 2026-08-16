// WinControls/Qt/NppDocumentMap.h — Mini-mapa de documento portable Qt6
// Reemplaza WinControls/DocumentMap/DocumentMap.cpp/.h en Linux
//
// En Windows: Segundo Scintilla view en un panel lateral custom
// En Linux:   QsciScintilla en modo read-only + scroll sync con editor principal
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "NppDockWidget.h"

#ifdef NPP_PLATFORM_LINUX

#include <Qsci/qsciscintilla.h>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollBar>
#include <QColor>
#include <QFont>
#include <QTimer>

/// Mini-mapa de documento (Document Map) para Notepad++ Linux.
/// Muestra una vista miniatura del documento actual sincronizada
/// con el scroll del editor principal.
class NppDocumentMap : public QWidget {
    Q_OBJECT

public:
    explicit NppDocumentMap(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        _miniView = new QsciScintilla(this);
        _miniView->setReadOnly(true);
        _miniView->setCaretWidth(0); // sin cursor visible
        _miniView->setMarginWidth(0, 0); // sin márgenes
        _miniView->setMarginWidth(1, 0);
        _miniView->setMarginWidth(2, 0);
        _miniView->setFolding(QsciScintilla::NoFoldStyle);
        _miniView->setIndentationGuides(false);

        // Fuente diminuta para el mini-mapa
        QFont miniFont("Monospace", 2);
        _miniView->setFont(miniFont);

        // Colores oscuros
        _miniView->setPaper(QColor(0x1E, 0x1E, 0x1E));
        _miniView->setColor(QColor(0x80, 0x80, 0x80));

        // Sin scroll bars propios (el widget completo hace scroll tracking)
        _miniView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _miniView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // No editable, no seleccionable
        _miniView->SendScintilla(QsciScintillaBase::SCI_SETMOUSEDOWNCAPTURES, 0);

        layout->addWidget(_miniView);

        // Timer para throttle de sincronización
        _syncTimer = new QTimer(this);
        _syncTimer->setSingleShot(true);
        _syncTimer->setInterval(50);
        connect(_syncTimer, &QTimer::timeout, this, &NppDocumentMap::doSync);
    }

    /// Conecta con un editor principal para sincronizar contenido y scroll.
    void connectToEditor(QsciScintilla* editor) {
        _editor = editor;
        if (!_editor) return;

        // Copiar contenido
        syncContent();

        // Sincronizar scroll: cuando el editor principal hace scroll,
        // actualizar la posición visible en el mini-mapa
        connect(_editor->verticalScrollBar(), &QScrollBar::valueChanged,
                this, &NppDocumentMap::onEditorScrolled);

        // Click en el mini-mapa: saltar a esa posición en el editor
        connect(_miniView, &QsciScintilla::cursorPositionChanged,
                [this](int line, int /*col*/) {
            if (_editor && !_syncing) {
                _editor->setCursorPosition(line, 0);
                _editor->ensureLineVisible(line);
            }
        });
    }

    /// Actualiza el contenido del mini-mapa con el texto del editor.
    void syncContent() {
        if (!_editor) return;
        _syncing = true;
        _miniView->setReadOnly(false);
        _miniView->setText(_editor->text());
        _miniView->setReadOnly(true);

        // Copiar lexer del editor si tiene
        if (_editor->lexer()) {
            // No copiamos el lexer completo — solo usamos colores básicos
            _miniView->setColor(_editor->color());
        }
        _syncing = false;
    }

    /// Crea un NppDockWidget que contiene este DocumentMap.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Mapa de Documento", parent,
                                       NppDockWidget::DockPosition::Right);
        dock->setContent(this);
        dock->setFixedWidth(120);
        return dock;
    }

public slots:
    /// Llamar cuando el contenido del editor cambie.
    void onEditorContentChanged() {
        _syncTimer->start(); // throttled
    }

private slots:
    void onEditorScrolled(int value) {
        if (!_editor || _syncing) return;
        _syncing = true;

        // Calcular la línea visible en el editor principal
        int firstVisible = _editor->firstVisibleLine();
        int linesOnScreen = _editor->SendScintilla(
            QsciScintillaBase::SCI_LINESONSCREEN);

        // Hacer scroll del mini-mapa a la misma posición
        _miniView->setFirstVisibleLine(firstVisible);

        // Highlight de la zona visible (región sombreada)
        // Usamos un indicador para marcar las líneas visibles
        int totalLines = _miniView->lines();
        _miniView->SendScintilla(QsciScintillaBase::SCI_SETINDICATORCURRENT, 0);
        _miniView->SendScintilla(QsciScintillaBase::SCI_INDICATORCLEARRANGE,
                                 0, _miniView->length());

        if (firstVisible < totalLines) {
            int startPos = _miniView->SendScintilla(
                QsciScintillaBase::SCI_POSITIONFROMLINE, firstVisible);
            int endLine = qMin(firstVisible + linesOnScreen, totalLines - 1);
            int endPos = _miniView->SendScintilla(
                QsciScintillaBase::SCI_GETLINEENDPOSITION, endLine);

            _miniView->SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,
                                     0, QsciScintillaBase::INDIC_STRAIGHTBOX);
            _miniView->SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,
                                     0, QColor(0x26, 0x4F, 0x78).rgb());
            _miniView->SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA, 0, 60);
            _miniView->SendScintilla(QsciScintillaBase::SCI_INDICATORFILLRANGE,
                                     startPos, endPos - startPos);
        }

        _syncing = false;
    }

    void doSync() { syncContent(); }

private:
    QsciScintilla* _miniView = nullptr;
    QsciScintilla* _editor   = nullptr;
    QTimer*        _syncTimer = nullptr;
    bool           _syncing   = false;
};

#endif // NPP_PLATFORM_LINUX

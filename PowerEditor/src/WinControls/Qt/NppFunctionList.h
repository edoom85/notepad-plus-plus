// WinControls/Qt/NppFunctionList.h — Lista de Funciones y Símbolos Qt6
// Reemplaza WinControls/FunctionList/FunctionList.cpp/.h en Linux
//
// En Windows: Custom DockingWnd + TreeView Win32 + regex rules
// En Linux:   NppDockWidget + NppTreeView + filtro rápido
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "NppDockWidget.h"
#include "NppTreeView.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QIcon>
#include <QString>
#include <vector>
#include <regex>

/// Estructura de un símbolo parseado del código.
struct SymbolItem {
    QString name;
    QString type; // "function", "class", "method", "variable"
    int line;
};

/// Componente Function List para Notepad++ Linux.
/// Extrae funciones, clases y métodos del documento actual y los muestra en una vista de árbol.
class NppFunctionList : public QWidget {
    Q_OBJECT

public:
    explicit NppFunctionList(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // Barra superior con filtro y botón refrescar
        auto* topBar = new QHBoxLayout();
        _filterEdit = new QLineEdit(this);
        _filterEdit->setPlaceholderText("Filtrar funciones/símbolos...");
        _filterEdit->setClearButtonEnabled(true);
        _filterEdit->setStyleSheet(
            "QLineEdit { background: #3C3C3C; color: #D4D4D4; "
            "border: 1px solid #555; padding: 4px; margin: 4px; }"
        );

        auto* btnRefresh = new QPushButton("🔄", this);
        btnRefresh->setToolTip("Recargar lista de funciones");
        btnRefresh->setFixedWidth(32);
        btnRefresh->setStyleSheet(
            "QPushButton { background: #3C3C3C; color: #FFF; border: 1px solid #555; border-radius: 2px; padding: 4px; }"
            "QPushButton:hover { background: #505050; }"
        );

        topBar->addWidget(_filterEdit);
        topBar->addWidget(btnRefresh);
        layout->addLayout(topBar);

        // Árbol de símbolos
        _treeView = new NppTreeView(this);
        layout->addWidget(_treeView);

        // Conexiones
        connect(btnRefresh, &QPushButton::clicked, this, &NppFunctionList::onRefreshRequested);
        connect(_filterEdit, &QLineEdit::textChanged, this, &NppFunctionList::onFilterChanged);
        connect(_treeView, &NppTreeView::itemSelected, this, &NppFunctionList::onSymbolSelected);
    }

    /// Parsea el texto del documento para extraer funciones/símbolos.
    void parseDocument(const QString& content, const QString& language = "cpp") {
        _symbols.clear();
        _treeView->clear();

        if (content.isEmpty()) return;

        // Parser regex básico para C++/C/JS/Python
        std::string text = content.toStdString();

        if (language == "cpp" || language == "c" || language == "js") {
            // Regex para funciones C/C++/JS: tipo nombre(params)
            std::regex funcRegex(R"((?:[a-zA-Z_]\w*\s+)+([a-zA-Z_]\w*)\s*\([^)]*\)\s*\{)");
            auto words_begin = std::sregex_iterator(text.begin(), text.end(), funcRegex);
            auto words_end = std::sregex_iterator();

            int lineNum = 1;
            size_t lastPos = 0;

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                size_t matchPos = match.position();

                // Contar líneas transcurridas
                for (size_t p = lastPos; p < matchPos; ++p) {
                    if (text[p] == '\n') lineNum++;
                }
                lastPos = matchPos;

                QString funcName = QString::fromStdString(match[1].str());
                _symbols.push_back({funcName, "function", lineNum});
            }
        } else if (language == "python") {
            // Regex Python: def func_name(
            std::regex pyRegex(R"(def\s+([a-zA-Z_]\w*)\s*\()");
            auto words_begin = std::sregex_iterator(text.begin(), text.end(), pyRegex);
            auto words_end = std::sregex_iterator();

            int lineNum = 1;
            size_t lastPos = 0;

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                size_t matchPos = match.position();
                for (size_t p = lastPos; p < matchPos; ++p) {
                    if (text[p] == '\n') lineNum++;
                }
                lastPos = matchPos;

                QString funcName = QString::fromStdString(match[1].str());
                _symbols.push_back({funcName, "function", lineNum});
            }
        }

        populateTree();
    }

    /// Crea un NppDockWidget que contiene este FunctionList.
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Lista de Funciones", parent,
                                       NppDockWidget::DockPosition::Right);
        dock->setContent(this);
        return dock;
    }

signals:
    /// Emitida cuando se solicita ir a una línea en el editor.
    void jumpToLineRequested(int lineNumber);

    /// Emitida para refrescar.
    void refreshRequested();

private:
    void populateTree() {
        _treeView->clear();
        QString filter = _filterEdit->text().trimmed();

        auto* rootGroup = _treeView->addRootItem("Funciones (" + QString::number(_symbols.size()) + ")");

        for (const auto& sym : _symbols) {
            if (!filter.isEmpty() && !sym.name.contains(filter, Qt::CaseInsensitive))
                continue;

            auto* item = _treeView->addChildItem(rootGroup, sym.name.toStdString() + " [L" + std::to_string(sym.line) + "]");
            item->setData(sym.line, Qt::UserRole + 1);
        }

        _treeView->expandAllNodes();
    }

private slots:
    void onRefreshRequested() { emit refreshRequested(); }

    void onFilterChanged() { populateTree(); }

    void onSymbolSelected(const QString& /*text*/, const QModelIndex& index) {
        QVariant data = _treeView->model()->data(index, Qt::UserRole + 1);
        if (data.isValid()) {
            int line = data.toInt();
            if (line > 0) emit jumpToLineRequested(line);
        }
    }

private:
    QLineEdit*               _filterEdit = nullptr;
    NppTreeView*             _treeView   = nullptr;
    std::vector<SymbolItem>  _symbols;
};

#endif // NPP_PLATFORM_LINUX

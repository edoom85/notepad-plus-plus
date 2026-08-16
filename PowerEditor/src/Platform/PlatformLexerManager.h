// Platform/PlatformLexerManager.h — Gestor de Lexers y Resaltado de Sintaxis (Qt6)
// Soporta auto-detección por extensión (.json, .py, .cpp, .html, .xml, .js, .css, etc.)
// y menú de selección de lenguaje de Notepad++.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformTheme.h"

#ifdef NPP_PLATFORM_LINUX


#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexeryaml.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerjson.h>

#include <QMenu>
#include <QFileInfo>
#include <QString>
#include <QFont>
#include <QColor>

/// Gestor de resaltado de sintaxis (Lexers QsciScintilla) para Notepad++ Linux.
class NppLexerManager {
public:
    enum class Language {
        PlainText,
        CPP,
        JSON,
        Python,
        HTML,
        XML,
        CSS,
        JavaScript,
        Java,
        Bash,
        SQL,
        YAML
    };

    /// Aplica un lexer específico a un editor QsciScintilla.
    static void applyLanguage(QsciScintilla* editor, Language lang) {
        if (!editor) return;

        QFont font("JetBrains Mono", 11);
        if (!font.exactMatch()) font = QFont("Monospace", 11);

        QsciLexer* lexer = nullptr;

        switch (lang) {
            case Language::JSON:       lexer = new QsciLexerJSON(editor); break;
            case Language::Python:     lexer = new QsciLexerPython(editor); break;
            case Language::HTML:       lexer = new QsciLexerHTML(editor); break;
            case Language::XML:        lexer = new QsciLexerXML(editor); break;
            case Language::CSS:        lexer = new QsciLexerCSS(editor); break;
            case Language::JavaScript: lexer = new QsciLexerJavaScript(editor); break;
            case Language::Java:       lexer = new QsciLexerJava(editor); break;
            case Language::Bash:       lexer = new QsciLexerBash(editor); break;
            case Language::SQL:        lexer = new QsciLexerSQL(editor); break;
            case Language::YAML:       lexer = new QsciLexerYAML(editor); break;
            case Language::CPP:        lexer = new QsciLexerCPP(editor); break;
            case Language::PlainText:
            default:                   lexer = nullptr; break;
        }

        if (lexer) {
            lexer->setFont(font);
            QColor paperBg = NppTheme::paperColor();
            QColor textFg  = NppTheme::textColor();
            // Aplicar fondo de papel y texto adaptativos (Modo Oscuro / Modo Claro)
            for (int i = 0; i <= 128; ++i) {
                lexer->setPaper(paperBg, i);
                lexer->setColor(textFg, i);
            }
        }


        editor->setLexer(lexer);
    }


    /// Auto-detecta el lenguaje basado en la extensión del archivo.
    static Language detectFromExtension(const QString& filePath) {
        QString ext = QFileInfo(filePath).suffix().toLower();

        if (ext == "json")                             return Language::JSON;
        if (ext == "py" || ext == "pyw")               return Language::Python;
        if (ext == "cpp" || ext == "c" || ext == "h"
         || ext == "hpp" || ext == "cc" || ext == "cxx") return Language::CPP;
        if (ext == "html" || ext == "htm")             return Language::HTML;
        if (ext == "xml" || ext == "svg" || ext == "plist") return Language::XML;
        if (ext == "css" || ext == "scss")             return Language::CSS;
        if (ext == "js" || ext == "ts" || ext == "jsx" || ext == "mjs") return Language::JavaScript;
        if (ext == "java")                             return Language::Java;
        if (ext == "sh" || ext == "bash" || ext == "zsh") return Language::Bash;
        if (ext == "sql")                              return Language::SQL;
        if (ext == "yaml" || ext == "yml")             return Language::YAML;

        return Language::PlainText;
    }
};

#endif // NPP_PLATFORM_LINUX

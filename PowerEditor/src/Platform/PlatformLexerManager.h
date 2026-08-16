// Platform/PlatformLexerManager.h — Gestor de Lexers y Temas de Colores (Qt6)
// Soporta auto-detección por extensión (.json, .py, .cpp, .html, .xml, .js, .css, etc.),
// cambio dinámico de temas (Obsidian, zenburn, Monokai, VS Code Dark, Solarized, Light Mode)
// y aplicación de colores exactos por token en Scintilla.
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

/// Estructura de paleta de colores para un tema de Notepad++.
struct ThemePalette {
    QString name;
    QColor paperBg;
    QColor textFg;
    QColor keyFg;        // Claves / Propiedades / Identificadores
    QColor stringFg;     // Cadenas de texto
    QColor numberFg;     // Números
    QColor keywordFg;    // Palabras clave / Booleans
    QColor commentFg;    // Comentarios
    QColor operatorFg;   // Operadores (:, ,, {, })
    QColor marginBg;     // Fondo de margen de números de línea
    QColor marginFg;     // Texto de números de línea
};

/// Gestor de resaltado de sintaxis y temas para Notepad++ Linux.
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

    /// Obtiene la paleta de colores de un tema por su nombre.
    static ThemePalette getPalette(const QString& themeName) {
        QString name = themeName.toLower();

        if (name.contains("obsidian")) {
            return {
                "Obsidian",
                QColor("#293134"), // paperBg
                QColor("#E0E2E4"), // textFg
                QColor("#93C763"), // keyFg (Soft Mint Green)
                QColor("#EC7600"), // stringFg (Warm Orange)
                QColor("#FFCD22"), // numberFg (Gold)
                QColor("#93C763"), // keywordFg
                QColor("#66747D"), // commentFg
                QColor("#E8E8E8"), // operatorFg
                QColor("#242A2C"), // marginBg
                QColor("#819299")  // marginFg
            };
        } else if (name.contains("zenburn")) {
            return {
                "zenburn",
                QColor("#3F3F3F"),
                QColor("#DCDCCC"),
                QColor("#8CD0D3"),
                QColor("#CC9393"),
                QColor("#8CD0D3"),
                QColor("#F0DFA0"),
                QColor("#7F9F7F"),
                QColor("#DCDCCC"),
                QColor("#353535"),
                QColor("#7F9F7F")
            };
        } else if (name.contains("monokai")) {
            return {
                "Monokai",
                QColor("#272822"),
                QColor("#F8F8F2"),
                QColor("#A6E22E"),
                QColor("#E6DB74"),
                QColor("#AE81FF"),
                QColor("#F92672"),
                QColor("#75715E"),
                QColor("#F8F8F2"),
                QColor("#20211C"),
                QColor("#75715E")
            };
        } else if (name.contains("solarized dark")) {
            return {
                "Solarized Dark",
                QColor("#002B36"),
                QColor("#839496"),
                QColor("#268BD2"),
                QColor("#2AA198"),
                QColor("#D33682"),
                QColor("#859900"),
                QColor("#586E75"),
                QColor("#839496"),
                QColor("#073642"),
                QColor("#586E75")
            };
        } else if (name.contains("solarized light")) {
            return {
                "Solarized Light",
                QColor("#FDF6E3"),
                QColor("#657B83"),
                QColor("#268BD2"),
                QColor("#2AA198"),
                QColor("#D33682"),
                QColor("#859900"),
                QColor("#93A1A1"),
                QColor("#657B83"),
                QColor("#EEE8D5"),
                QColor("#93A1A1")
            };
        } else if (name.contains("light")) {
            return {
                "Default Light",
                QColor("#FFFFFF"),
                QColor("#000000"),
                QColor("#0000FF"),
                QColor("#800080"),
                QColor("#FF0000"),
                QColor("#0000FF"),
                QColor("#008000"),
                QColor("#000000"),
                QColor("#F0F0F0"),
                QColor("#6E6E6E")
            };
        }

        // Predeterminado: VS Code Dark / Default Dark Mode
        return {
            "VS Code Dark",
            QColor("#1E1E1E"),
            QColor("#D4D4D4"),
            QColor("#61AFEF"),
            QColor("#98C379"),
            QColor("#D19A66"),
            QColor("#C678DD"),
            QColor("#7F848E"),
            QColor("#56B6C2"),
            QColor("#252526"),
            QColor("#858585")
        };
    }

    /// Aplica un tema completo a un editor Scintilla.
    static void applyTheme(QsciScintilla* editor, const QString& themeName) {
        if (!editor) return;

        _currentThemeName = themeName;
        ThemePalette pal = getPalette(themeName);

        editor->setPaper(pal.paperBg);
        editor->setColor(pal.textFg);
        editor->setMarginsBackgroundColor(pal.marginBg);
        editor->setMarginsForegroundColor(pal.marginFg);
        editor->setFoldMarginColors(pal.marginBg, pal.marginBg);

        if (auto* lexer = editor->lexer()) {
            lexer->setDefaultPaper(pal.paperBg);
            lexer->setDefaultColor(pal.textFg);
            for (int i = 0; i <= 128; ++i) {
                lexer->setPaper(pal.paperBg, i);
            }

            if (auto* json = qobject_cast<QsciLexerJSON*>(lexer)) {
                json->setColor(pal.textFg, QsciLexerJSON::Default);
                json->setColor(pal.numberFg, QsciLexerJSON::Number);
                json->setColor(pal.stringFg, QsciLexerJSON::String);
                json->setColor(pal.keyFg, QsciLexerJSON::Property);
                json->setColor(pal.commentFg, QsciLexerJSON::CommentLine);
                json->setColor(pal.commentFg, QsciLexerJSON::CommentBlock);
                json->setColor(pal.operatorFg, QsciLexerJSON::Operator);
                json->setColor(pal.keywordFg, QsciLexerJSON::Keyword);
            } else if (auto* cpp = qobject_cast<QsciLexerCPP*>(lexer)) {
                cpp->setColor(pal.commentFg, QsciLexerCPP::Comment);
                cpp->setColor(pal.commentFg, QsciLexerCPP::CommentLine);
                cpp->setColor(pal.commentFg, QsciLexerCPP::CommentDoc);
                cpp->setColor(pal.numberFg, QsciLexerCPP::Number);
                cpp->setColor(pal.keywordFg, QsciLexerCPP::Keyword);
                cpp->setColor(pal.stringFg, QsciLexerCPP::DoubleQuotedString);
                cpp->setColor(pal.stringFg, QsciLexerCPP::SingleQuotedString);
                cpp->setColor(pal.operatorFg, QsciLexerCPP::Operator);
                cpp->setColor(pal.keyFg, QsciLexerCPP::Identifier);
            } else if (auto* py = qobject_cast<QsciLexerPython*>(lexer)) {
                py->setColor(pal.commentFg, QsciLexerPython::Comment);
                py->setColor(pal.numberFg, QsciLexerPython::Number);
                py->setColor(pal.stringFg, QsciLexerPython::DoubleQuotedString);
                py->setColor(pal.stringFg, QsciLexerPython::SingleQuotedString);
                py->setColor(pal.keywordFg, QsciLexerPython::Keyword);
                py->setColor(pal.keyFg, QsciLexerPython::ClassName);
                py->setColor(pal.keyFg, QsciLexerPython::FunctionMethodName);
                py->setColor(pal.operatorFg, QsciLexerPython::Operator);
            }
        }
    }

    /// Aplica un lexer específico a un editor QsciScintilla.
    static void applyLanguage(QsciScintilla* editor, Language lang) {
        if (!editor) return;

        QFont font("JetBrains Mono", 11);
        if (!font.exactMatch()) font = QFont("Monospace", 11);

        QsciLexer* lexer = nullptr;

        switch (lang) {
            case Language::JSON:       lexer = new QsciLexerJSON(editor); break;
            case Language::Python:     lexer = new QsciLexerPython(editor); break;
            case Language::CPP:        lexer = new QsciLexerCPP(editor); break;
            case Language::HTML:       lexer = new QsciLexerHTML(editor); break;
            case Language::XML:        lexer = new QsciLexerXML(editor); break;
            case Language::CSS:        lexer = new QsciLexerCSS(editor); break;
            case Language::JavaScript: lexer = new QsciLexerJavaScript(editor); break;
            case Language::Java:       lexer = new QsciLexerJava(editor); break;
            case Language::Bash:       lexer = new QsciLexerBash(editor); break;
            case Language::SQL:        lexer = new QsciLexerSQL(editor); break;
            case Language::YAML:       lexer = new QsciLexerYAML(editor); break;
            default:                   lexer = nullptr; break;
        }

        if (lexer) {
            lexer->setFont(font);
        }

        editor->setLexer(lexer);
        
        // Aplica el tema actual sobre el nuevo lexer
        applyTheme(editor, _currentThemeName);
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

private:
    static inline QString _currentThemeName = "VS Code Dark";
};

#endif // NPP_PLATFORM_LINUX

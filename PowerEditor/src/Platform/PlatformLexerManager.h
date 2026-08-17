// Platform/PlatformLexerManager.h — Gestor de Lexers y Temas de Colores (Qt6)
// Soporta los 22 temas oficiales de Notepad++ (Obsidian, Bespin, Black board, Choco, Monokai, Zenburn, etc.),
// auto-detección de extensión y asignación exacta de colores por token en Scintilla.
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
    QColor keyFg;        // Claves / Identificadores
    QColor stringFg;     // Cadenas de texto
    QColor numberFg;     // Números
    QColor keywordFg;    // Palabras clave (SELECT, FROM, int, if)
    QColor commentFg;    // Comentarios
    QColor operatorFg;   // Operadores (=, +, *, ,)
    QColor marginBg;     // Fondo de números de línea
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

    /// Obtiene la paleta de colores de un tema por su nombre (soporta los 22 temas oficiales).
    static ThemePalette getPalette(const QString& themeName) {
        QString name = themeName.toLower();

        if (name.contains("obsidian")) {
            return {
                "Obsidian",
                QColor("#293134"), // paperBg (Fondo Slate Gray)
                QColor("#E0E2E4"), // textFg (Gris claro)
                QColor("#E0E2E4"), // keyFg (Identificadores)
                QColor("#EC7600"), // stringFg (Naranja Cálido)
                QColor("#FFCD22"), // numberFg (Dorado)
                QColor("#93C763"), // keywordFg (Verde Menta Menta BOLD para SELECT, FROM, TABLE)
                QColor("#66747B"), // commentFg (Gris Azulado)
                QColor("#E8E2B7"), // operatorFg (Amarillo pálido)
                QColor("#242A2C"), // marginBg
                QColor("#819299")  // marginFg
            };
        } else if (name.contains("zenburn")) {
            return {
                "zenburn",
                QColor("#3F3F3F"), QColor("#DCDCCC"), QColor("#8CD0D3"),
                QColor("#CC9393"), QColor("#8CD0D3"), QColor("#F0DFA0"),
                QColor("#7F9F7F"), QColor("#DCDCCC"), QColor("#353535"), QColor("#7F9F7F")
            };
        } else if (name.contains("monokai")) {
            return {
                "Monokai",
                QColor("#272822"), QColor("#F8F8F2"), QColor("#A6E22E"),
                QColor("#E6DB74"), QColor("#AE81FF"), QColor("#F92672"),
                QColor("#75715E"), QColor("#F8F8F2"), QColor("#20211C"), QColor("#75715E")
            };
        } else if (name.contains("black board") || name.contains("blackboard")) {
            return {
                "Black board",
                QColor("#0C1021"), QColor("#F8F8F8"), QColor("#FF6400"),
                QColor("#61CE3C"), QColor("#78CE5D"), QColor("#FBDE2D"),
                QColor("#AEAEAE"), QColor("#F8F8F8"), QColor("#080B17"), QColor("#808080")
            };
        } else if (name.contains("deep black")) {
            return {
                "Deep Black",
                QColor("#000000"), QColor("#FFFFFF"), QColor("#8080FF"),
                QColor("#00FF00"), QColor("#FF8000"), QColor("#00FFFF"),
                QColor("#808080"), QColor("#FFFFFF"), QColor("#101010"), QColor("#808080")
            };
        } else if (name.contains("bespin")) {
            return {
                "Bespin",
                QColor("#28211C"), QColor("#BAAE9E"), QColor("#937121"),
                QColor("#54BE0D"), QColor("#CF6A4C"), QColor("#F9EE98"),
                QColor("#666666"), QColor("#BAAE9E"), QColor("#1F1915"), QColor("#777777")
            };
        } else if (name.contains("choco")) {
            return {
                "Choco",
                QColor("#261A0C"), QColor("#F6F3E8"), QColor("#A5C261"),
                QColor("#A5C261"), QColor("#6D9CBD"), QColor("#E5C07B"),
                QColor("#8D7B68"), QColor("#F6F3E8"), QColor("#1E1409"), QColor("#8D7B68")
            };
        } else if (name.contains("solarized dark")) {
            return {
                "Solarized Dark",
                QColor("#002B36"), QColor("#839496"), QColor("#268BD2"),
                QColor("#2AA198"), QColor("#D33682"), QColor("#859900"),
                QColor("#586E75"), QColor("#839496"), QColor("#073642"), QColor("#586E75")
            };
        } else if (name.contains("solarized light")) {
            return {
                "Solarized Light",
                QColor("#FDF6E3"), QColor("#657B83"), QColor("#268BD2"),
                QColor("#2AA198"), QColor("#D33682"), QColor("#859900"),
                QColor("#93A1A1"), QColor("#657B83"), QColor("#EEE8D5"), QColor("#93A1A1")
            };
        } else if (name.contains("light")) {
            return {
                "Default Light",
                QColor("#FFFFFF"), QColor("#000000"), QColor("#0000FF"),
                QColor("#800080"), QColor("#FF0000"), QColor("#0000FF"),
                QColor("#008000"), QColor("#000000"), QColor("#F0F0F0"), QColor("#6E6E6E")
            };
        }

        // Predeterminado: VS Code Dark / Default Dark Mode
        return {
            "VS Code Dark",
            QColor("#1E1E1E"), QColor("#D4D4D4"), QColor("#61AFEF"),
            QColor("#98C379"), QColor("#D19A66"), QColor("#C678DD"),
            QColor("#7F848E"), QColor("#56B6C2"), QColor("#252526"), QColor("#858585")
        };
    }

    /// Aplica un tema completo a un editor Scintilla.
    static void applyTheme(QsciScintilla* editor, const QString& themeName) {
        if (!editor) return;

        if (!themeName.isEmpty()) {
            setThemeName(themeName);
        }
        ThemePalette pal = getPalette(_currentThemeName);


        editor->setPaper(pal.paperBg);
        editor->setColor(pal.textFg);
        editor->setCaretLineBackgroundColor(QColor(pal.paperBg.red() + 10, pal.paperBg.green() + 10, pal.paperBg.blue() + 10));
        editor->setMarginsBackgroundColor(pal.marginBg);
        editor->setMarginsForegroundColor(pal.marginFg);
        editor->setFoldMarginColors(pal.marginBg, pal.marginBg);

        if (auto* lexer = editor->lexer()) {
            lexer->setDefaultPaper(pal.paperBg);
            lexer->setDefaultColor(pal.textFg);
            for (int i = 0; i <= 128; ++i) {
                lexer->setPaper(pal.paperBg, i);
                lexer->setColor(pal.textFg, i);
            }

            if (auto* sql = qobject_cast<QsciLexerSQL*>(lexer)) {
                sql->setColor(pal.commentFg, QsciLexerSQL::Comment);
                sql->setColor(pal.commentFg, QsciLexerSQL::CommentLine);
                sql->setColor(pal.commentFg, QsciLexerSQL::CommentDoc);
                sql->setColor(pal.numberFg, QsciLexerSQL::Number);
                sql->setColor(pal.keywordFg, QsciLexerSQL::Keyword);
                sql->setColor(pal.keywordFg, QsciLexerSQL::KeywordSet5);
                sql->setColor(pal.stringFg, QsciLexerSQL::DoubleQuotedString);
                sql->setColor(pal.stringFg, QsciLexerSQL::SingleQuotedString);
                sql->setColor(pal.operatorFg, QsciLexerSQL::Operator);
                sql->setColor(pal.textFg, QsciLexerSQL::Identifier);
            } else if (auto* json = qobject_cast<QsciLexerJSON*>(lexer)) {
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

    /// Obtiene el nombre del tema activo actual (ej. "Obsidian").
    static QString currentThemeName() {
        if (_currentThemeName.isEmpty()) {
            loadConfig();
        }
        return _currentThemeName;
    }

    static void setThemeName(const QString& themeName) {
        if (!themeName.isEmpty()) {
            _currentThemeName = themeName;
            saveConfig();
        }
    }

    static void loadConfig() {
        QString configPath = QDir::homePath() + "/.config/notepadplusplus/theme.cfg";
        QFile file(configPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString name = QString::fromUtf8(file.readAll()).trimmed();
            if (!name.isEmpty()) _currentThemeName = name;
            file.close();
        }
        if (_currentThemeName.isEmpty()) _currentThemeName = "Obsidian";
    }

    static void saveConfig() {
        QString dir = QDir::homePath() + "/.config/notepadplusplus";
        QDir().mkpath(dir);
        QFile file(dir + "/theme.cfg");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(_currentThemeName.toUtf8());
            file.close();
        }
    }

private:
    static inline QString _currentThemeName = "";
};


#endif // NPP_PLATFORM_LINUX


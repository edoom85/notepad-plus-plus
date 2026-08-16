// Platform/PlatformLexerManager.h — Gestor de Lexers y Resaltado de Sintaxis (Qt6)
// Soporta auto-detección por extensión (.json, .py, .cpp, .html, .xml, .js, .css, etc.)
// y menús de selección de lenguaje de Notepad++ con temas de color vibrantes.
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
            configureThemeColors(lexer);
        }

        editor->setLexer(lexer);
        
        // Re-estilizar números de línea y márgenes tras cambiar el lexer
        QColor marginBg = NppTheme::marginBackgroundColor();
        QColor marginFg = NppTheme::marginForegroundColor();
        editor->setMarginsBackgroundColor(marginBg);
        editor->setMarginsForegroundColor(marginFg);
        editor->setFoldMarginColors(marginBg, marginBg);
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
    /// Configura los colores de resaltado sintáctico según el tema (Modo Oscuro / Modo Claro).
    static void configureThemeColors(QsciLexer* lexer) {
        if (!lexer) return;

        bool isDark = NppTheme::isDarkMode();
        QColor paperBg = NppTheme::paperColor();

        // 1. Configurar el fondo de papel para todos los estilos sin alterar el color del texto por defecto
        for (int i = 0; i <= 128; ++i) {
            lexer->setPaper(paperBg, i);
        }

        // 2. Si estamos en modo oscuro, aplicar paleta vibrante estilo One Dark / Notepad++ Dark Theme
        if (isDark) {
            lexer->setDefaultPaper(paperBg);
            lexer->setDefaultColor(QColor(0xAB, 0xB2, 0xBF));

            if (auto* json = qobject_cast<QsciLexerJSON*>(lexer)) {
                json->setColor(QColor(0xAB, 0xB2, 0xBF), QsciLexerJSON::Default);
                json->setColor(QColor(0xD1, 0x9A, 0x66), QsciLexerJSON::Number);          // Números (Naranja/Marrón)
                json->setColor(QColor(0x98, 0xC3, 0x79), QsciLexerJSON::String);          // Cadenas (Verde)
                json->setColor(QColor(0xE0, 0x6C, 0x75), QsciLexerJSON::UnclosedString);  // Cadena sin cerrar (Rojo)
                json->setColor(QColor(0x61, 0xAF, 0xEF), QsciLexerJSON::Property);        // Claves / Propiedades "id", "name" (Azul/Cyan)
                json->setColor(QColor(0x56, 0xB6, 0xC2), QsciLexerJSON::EscapeSequence);  // Secuencias de escape
                json->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerJSON::CommentLine);     // Comentarios (Gris)
                json->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerJSON::CommentBlock);    // Comentarios bloque
                json->setColor(QColor(0x56, 0xB6, 0xC2), QsciLexerJSON::Operator);        // Operadores : y ,
                json->setColor(QColor(0xC6, 0x78, 0xDD), QsciLexerJSON::Keyword);         // true, false, null (Púrpura)
            }
            else if (auto* cpp = qobject_cast<QsciLexerCPP*>(lexer)) {
                cpp->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerCPP::Comment);
                cpp->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerCPP::CommentLine);
                cpp->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerCPP::CommentDoc);
                cpp->setColor(QColor(0xD1, 0x9A, 0x66), QsciLexerCPP::Number);
                cpp->setColor(QColor(0xC6, 0x78, 0xDD), QsciLexerCPP::Keyword);
                cpp->setColor(QColor(0x98, 0xC3, 0x79), QsciLexerCPP::DoubleQuotedString);
                cpp->setColor(QColor(0x98, 0xC3, 0x79), QsciLexerCPP::SingleQuotedString);
                cpp->setColor(QColor(0x56, 0xB6, 0xC2), QsciLexerCPP::Operator);
                cpp->setColor(QColor(0x61, 0xAF, 0xEF), QsciLexerCPP::Identifier);
                cpp->setColor(QColor(0xE5, 0xC0, 0x7B), QsciLexerCPP::PreProcessor);
            }
            else if (auto* py = qobject_cast<QsciLexerPython*>(lexer)) {
                py->setColor(QColor(0x7F, 0x84, 0x8E), QsciLexerPython::Comment);
                py->setColor(QColor(0xD1, 0x9A, 0x66), QsciLexerPython::Number);
                py->setColor(QColor(0x98, 0xC3, 0x79), QsciLexerPython::DoubleQuotedString);
                py->setColor(QColor(0x98, 0xC3, 0x79), QsciLexerPython::SingleQuotedString);
                py->setColor(QColor(0xC6, 0x78, 0xDD), QsciLexerPython::Keyword);
                py->setColor(QColor(0x61, 0xAF, 0xEF), QsciLexerPython::ClassName);
                py->setColor(QColor(0x61, 0xAF, 0xEF), QsciLexerPython::FunctionMethodName);
                py->setColor(QColor(0x56, 0xB6, 0xC2), QsciLexerPython::Operator);
            }
        }
    }
};

#endif // NPP_PLATFORM_LINUX

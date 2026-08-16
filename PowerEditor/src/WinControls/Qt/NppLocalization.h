// WinControls/Qt/NppLocalization.h — Gestor XML de Localización e Idiomas Qt6
// Reemplaza WinControls/Babble / NativeLang.xml parser en Linux
//
// En Windows: Parser XML TinyXML / Custom XML parser para traducir diálogos y menús Win32
// En Linux:   QDomDocument / QXmlStreamReader para cargar nativeLang.xml y aplicar traducciones
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QXmlStreamReader>
#include <QFile>
#include <QMap>
#include <QString>
#include <QDir>
#include <QCoreApplication>

/// Parser y gestor de localización de lenguajes XML para Notepad++ Linux.
/// Carga los archivos `nativeLang.xml` / `spanish.xml` / `english.xml` oficiales de Notepad++.
class NppLocalization {
public:
    static NppLocalization& instance() {
        static NppLocalization s_instance;
        return s_instance;
    }

    /// Carga un archivo XML de idioma de Notepad++.
    bool loadLanguageFile(const QString& xmlFilePath) {
        QFile file(xmlFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        _translations.clear();
        QXmlStreamReader xml(&file);

        QString currentContext;

        while (!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();

            if (token == QXmlStreamReader::StartElement) {
                QString name = xml.name().toString();
                auto attrs = xml.attributes();

                if (attrs.hasAttribute("name") && attrs.hasAttribute("value")) {
                    QString key = name + "." + attrs.value("name").toString();
                    _translations[key] = attrs.value("value").toString();
                } else if (attrs.hasAttribute("id") && attrs.hasAttribute("name")) {
                    QString key = "id." + attrs.value("id").toString();
                    _translations[key] = attrs.value("name").toString();
                }
            }
        }

        file.close();
        return !xml.hasError();
    }

    /// Obtiene la traducción de una clave dada (o retorna la clave por defecto si no se encuentra).
    QString translate(const QString& key, const QString& defaultValue = QString()) const {
        if (_translations.contains(key))
            return _translations[key];
        return defaultValue.isEmpty() ? key : defaultValue;
    }

    /// Comprueba si hay un archivo de localización cargado.
    bool isLoaded() const { return !_translations.isEmpty(); }

    /// Cantidad de cadenas traducidas cargadas.
    int count() const { return _translations.size(); }

private:
    NppLocalization() = default;
    QMap<QString, QString> _translations;
};

#endif // NPP_PLATFORM_LINUX

// Platform/PlatformSessionManager.h — Gestor de Sesiones XML (session.xml) Qt6
// Reemplaza Session/ (SessionSaveDlg.cpp/h) en Linux.
//
// Permite guardar y restaurar automáticamente la lista de archivos abiertos y pestañas en XML.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

/// Gestor de persistencia de sesión XML (session.xml) para Notepad++ Linux.
class NppSessionManager {
public:
    /// Obtiene la ruta por defecto del archivo de sesión (~/.config/notepadplusplus/session.xml).
    static QString defaultSessionPath() {
        QString configDir = QDir::homePath() + "/.config/notepadplusplus";
        QDir().mkpath(configDir);
        return configDir + "/session.xml";
    }

    /// Guarda la sesión actual a un archivo XML.
    static bool saveSession(const QStringList& filePaths, int activeIndex, const QString& xmlPath = defaultSessionPath()) {
        QFile file(xmlPath.isEmpty() ? defaultSessionPath() : xmlPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("NotepadPlus");
        writer.writeStartElement("Session");
        writer.writeAttribute("activeView", "0");

        writer.writeStartElement("mainTab");
        writer.writeAttribute("activeIndex", QString::number(activeIndex));

        for (const QString& path : filePaths) {
            if (!path.isEmpty()) {
                writer.writeStartElement("File");
                writer.writeAttribute("filename", path);
                writer.writeEndElement(); // File
            }
        }

        writer.writeEndElement(); // mainTab
        writer.writeEndElement(); // Session
        writer.writeEndElement(); // NotepadPlus
        writer.writeEndDocument();

        file.close();
        return true;
    }

    /// Carga la sesión guardada desde un archivo XML.
    static bool loadSession(QStringList& outPaths, int& outActiveIndex, const QString& xmlPath = defaultSessionPath()) {
        outPaths.clear();
        outActiveIndex = 0;

        QString targetPath = xmlPath.isEmpty() ? defaultSessionPath() : xmlPath;
        QFile file(targetPath);
        if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        QXmlStreamReader reader(&file);
        while (!reader.atEnd() && !reader.hasError()) {
            QXmlStreamReader::TokenType token = reader.readNext();
            if (token == QXmlStreamReader::StartElement) {
                QString name = reader.name().toString();
                if (name == "mainTab") {
                    outActiveIndex = reader.attributes().value("activeIndex").toInt();
                } else if (name == "File") {
                    QString filename = reader.attributes().value("filename").toString();
                    if (!filename.isEmpty()) {
                        outPaths.append(filename);
                    }
                }
            }
        }

        file.close();
        return !outPaths.isEmpty();
    }
};

#endif // NPP_PLATFORM_LINUX

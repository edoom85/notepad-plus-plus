// Platform/PlatformSessionManager.h — Gestor de Sesiones XML y Backup Snapshots (session.xml) Qt6
// Reemplaza Session/ (SessionSaveDlg.cpp/h) y Buffer/DocSnapshot en Linux.
//
// Guarda y restaura la lista de pestañas abiertas, documentos no guardados (new 1, new 2),
// cambios pendientes en archivos y estado de modificación.
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
#include <QList>

/// Gestor de persistencia de sesión XML y snapshots de respaldo para Notepad++ Linux.
class NppSessionManager {
public:
    struct SessionTabData {
        QString title;          // ej. "new 1" o "main.cpp"
        QString filePath;       // Ruta absoluta si está guardado en disco
        QString backupPath;     // Ruta a la copia de respaldo en ~/.config/notepadplusplus/backup
        QString content;        // Contenido en memoria para guardar a respaldo
        bool isModified = false;
        bool isUntitled = false;
        int activeIndex = 0;
    };

    /// Directorio de respaldos temporales (~/.config/notepadplusplus/backup).
    static QString defaultBackupDir() {
        QString dir = QDir::homePath() + "/.config/notepadplusplus/backup";
        QDir().mkpath(dir);
        return dir;
    }

    /// Ruta por defecto de la sesión XML (~/.config/notepadplusplus/session.xml).
    static QString defaultSessionPath() {
        QString configDir = QDir::homePath() + "/.config/notepadplusplus";
        QDir().mkpath(configDir);
        return configDir + "/session.xml";
    }

    /// Guarda la sesión actual y realiza snapshots de pestañas no guardadas o modificadas.
    static bool saveSession(const QList<SessionTabData>& tabs, int activeIndex, const QString& xmlPath = defaultSessionPath()) {
        QFile file(xmlPath.isEmpty() ? defaultSessionPath() : xmlPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QString backupDir = defaultBackupDir();

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("NotepadPlus");
        writer.writeStartElement("Session");
        writer.writeAttribute("activeView", "0");

        writer.writeStartElement("mainTab");
        writer.writeAttribute("activeIndex", QString::number(activeIndex));

        for (int i = 0; i < tabs.size(); ++i) {
            auto t = tabs[i];
            QString backupFile = "";

            // Si es un documento sin título (new 1) o está modificado, crear snapshot de respaldo
            if (t.isUntitled || t.isModified) {
                QString safeTitle = t.title;
                safeTitle.replace('/', '_').replace('\\', '_').replace(' ', '_');
                backupFile = QString("%1/backup_%2_%3.txt").arg(backupDir).arg(i).arg(safeTitle);
                
                QFile bFile(backupFile);
                if (bFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    bFile.write(t.content.toUtf8());
                    bFile.close();
                }
            }

            writer.writeStartElement("File");
            writer.writeAttribute("title", t.title);
            writer.writeAttribute("filename", t.filePath);
            writer.writeAttribute("backupFilePath", backupFile);
            writer.writeAttribute("isModified", t.isModified ? "1" : "0");
            writer.writeAttribute("isUntitled", t.isUntitled ? "1" : "0");
            writer.writeEndElement(); // File
        }

        writer.writeEndElement(); // mainTab
        writer.writeEndElement(); // Session
        writer.writeEndElement(); // NotepadPlus
        writer.writeEndDocument();

        file.close();
        return true;
    }

    /// Carga la sesión guardada incluyendo respaldos de pestañas no guardadas.
    static bool loadSession(QList<SessionTabData>& outTabs, int& outActiveIndex, const QString& xmlPath = defaultSessionPath()) {
        outTabs.clear();
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
                    SessionTabData t;
                    t.title = reader.attributes().value("title").toString();
                    t.filePath = reader.attributes().value("filename").toString();
                    t.backupPath = reader.attributes().value("backupFilePath").toString();
                    t.isModified = (reader.attributes().value("isModified").toString() == "1");
                    t.isUntitled = (reader.attributes().value("isUntitled").toString() == "1");

                    // Si existe un archivo de respaldo, leer su contenido
                    if (!t.backupPath.isEmpty() && QFile::exists(t.backupPath)) {
                        QFile bFile(t.backupPath);
                        if (bFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            t.content = QString::fromUtf8(bFile.readAll());
                            bFile.close();
                        }
                    }

                    if (!t.title.isEmpty() || !t.filePath.isEmpty()) {
                        outTabs.append(t);
                    }
                }
            }
        }

        file.close();
        return !outTabs.isEmpty();
    }

    /// Sobrecarga compatible para guardar lista de rutas simples.
    static bool saveSession(const QStringList& filePaths, int activeIndex, const QString& xmlPath = defaultSessionPath()) {
        QList<SessionTabData> tabs;
        for (const QString& path : filePaths) {
            SessionTabData t;
            t.filePath = path;
            t.title = QFileInfo(path).fileName();
            t.isModified = false;
            t.isUntitled = false;
            tabs.append(t);
        }
        return saveSession(tabs, activeIndex, xmlPath);
    }

    /// Sobrecarga compatible para cargar lista simple de rutas.
    static bool loadSession(QStringList& outPaths, int& outActiveIndex, const QString& xmlPath = defaultSessionPath()) {
        QList<SessionTabData> tabs;
        if (loadSession(tabs, outActiveIndex, xmlPath)) {
            outPaths.clear();
            for (const auto& t : tabs) {
                outPaths.append(t.filePath.isEmpty() ? t.title : t.filePath);
            }
            return true;
        }
        return false;
    }
};

#endif // NPP_PLATFORM_LINUX



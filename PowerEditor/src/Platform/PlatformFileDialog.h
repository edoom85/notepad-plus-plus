// Platform/PlatformFileDialog.h — Diálogos de archivo nativos (Linux/Qt6)
// Reemplaza CustomFileDialog / Win32 GetOpenFileName / GetSaveFileName en Linux
//
// En Windows: CustomFileDialog usa IFileDialog (Vista+) con extensiones custom
// En Linux:   QFileDialog (Qt6) — se integra con el portal XDG en Wayland/X11
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QFileDialog>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <vector>

/// Diálogos de archivo nativos para Notepad++ Linux.
/// Equivalente portable a CustomFileDialog de Win32.
class NppFileDialog {
public:
    /// Abre diálogo para seleccionar uno o más archivos.
    /// Retorna lista de rutas seleccionadas (vacía si se cancela).
    static std::vector<NppString> openFiles(
        QWidget* parent = nullptr,
        const NppString& title = "Abrir archivo",
        const NppString& startDir = {},
        const NppString& filters = "Todos los archivos (*)")
    {
        QStringList result = QFileDialog::getOpenFileNames(
            parent,
            QString::fromStdString(title),
            QString::fromStdString(startDir),
            QString::fromStdString(filters));

        std::vector<NppString> paths;
        paths.reserve(result.size());
        for (const auto& r : result)
            paths.push_back(r.toStdString());
        return paths;
    }

    /// Abre diálogo para seleccionar un solo archivo.
    static NppString openFile(
        QWidget* parent = nullptr,
        const NppString& title = "Abrir archivo",
        const NppString& startDir = {},
        const NppString& filters = "Todos los archivos (*)")
    {
        QString result = QFileDialog::getOpenFileName(
            parent,
            QString::fromStdString(title),
            QString::fromStdString(startDir),
            QString::fromStdString(filters));
        return result.toStdString();
    }

    /// Abre diálogo para guardar un archivo.
    static NppString saveFile(
        QWidget* parent = nullptr,
        const NppString& title = "Guardar como",
        const NppString& startDir = {},
        const NppString& filters = "Todos los archivos (*)",
        const NppString& defaultSuffix = "")
    {
        QFileDialog dlg(parent,
            QString::fromStdString(title),
            QString::fromStdString(startDir),
            QString::fromStdString(filters));
        dlg.setAcceptMode(QFileDialog::AcceptSave);
        if (!defaultSuffix.empty())
            dlg.setDefaultSuffix(QString::fromStdString(defaultSuffix));

        if (dlg.exec() == QDialog::Accepted && !dlg.selectedFiles().isEmpty())
            return dlg.selectedFiles().first().toStdString();
        return {};
    }

    /// Abre diálogo para seleccionar un directorio.
    static NppString selectDirectory(
        QWidget* parent = nullptr,
        const NppString& title = "Seleccionar carpeta",
        const NppString& startDir = {})
    {
        QString result = QFileDialog::getExistingDirectory(
            parent,
            QString::fromStdString(title),
            QString::fromStdString(startDir),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        return result.toStdString();
    }

    /// Filtros predefinidos de Notepad++ (portables).
    static NppString defaultFilters() {
        return
            "Todos los archivos (*);;"
            "C/C++ (*.c *.cpp *.cxx *.cc *.h *.hpp *.hxx);;"
            "Python (*.py *.pyw);;"
            "JavaScript (*.js *.jsx *.mjs);;"
            "TypeScript (*.ts *.tsx);;"
            "HTML (*.html *.htm *.xhtml);;"
            "CSS (*.css *.scss *.sass *.less);;"
            "XML (*.xml *.xsl *.xslt *.svg);;"
            "JSON (*.json);;"
            "YAML (*.yaml *.yml);;"
            "Markdown (*.md *.markdown);;"
            "Shell (*.sh *.bash *.zsh);;"
            "SQL (*.sql);;"
            "Java (*.java);;"
            "Rust (*.rs);;"
            "Go (*.go);;"
            "Plain Text (*.txt *.log *.ini *.cfg *.conf)";
    }
};

#endif // NPP_PLATFORM_LINUX

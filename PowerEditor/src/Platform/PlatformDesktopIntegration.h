// Platform/PlatformDesktopIntegration.h — Integración con escritorio Linux
// Reemplaza regExtDlg, ShellExecute, asociación de archivos Win32, y system tray
//
// En Windows: Registry (HKEY_CLASSES_ROOT), ShellExecute, Shell_NotifyIcon
// En Linux:   XDG MIME, .desktop files, xdg-open, QSystemTrayIcon
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformString.h"

#ifdef NPP_PLATFORM_LINUX

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QWidget>

/// Integración con el escritorio Linux para Notepad++.
/// Gestiona: System tray, asociación de archivos, apertura de URLs,
/// y archivos .desktop.
class NppDesktopIntegration : public QObject {
    Q_OBJECT

public:
    explicit NppDesktopIntegration(QWidget* mainWindow, QObject* parent = nullptr)
        : QObject(parent)
        , _mainWindow(mainWindow)
    {}

    // ── System Tray Icon ────────────────────────────────────────────────────

    /// Crea e instala el icono de bandeja del sistema.
    bool createTrayIcon(const QIcon& icon) {
        if (!QSystemTrayIcon::isSystemTrayAvailable())
            return false;

        _trayIcon = new QSystemTrayIcon(icon, this);

        auto* menu = new QMenu();
        menu->setStyleSheet(
            "QMenu { background: #252526; color: #D4D4D4; }"
            "QMenu::item:selected { background: #094771; }");

        auto* showAction = menu->addAction("Mostrar Notepad++");
        auto* hideAction = menu->addAction("Minimizar a bandeja");
        menu->addSeparator();
        auto* quitAction = menu->addAction("Salir");

        connect(showAction, &QAction::triggered, [this]() {
            if (_mainWindow) {
                _mainWindow->show();
                _mainWindow->raise();
                _mainWindow->activateWindow();
            }
        });
        connect(hideAction, &QAction::triggered, [this]() {
            if (_mainWindow) _mainWindow->hide();
        });
        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

        connect(_trayIcon, &QSystemTrayIcon::activated,
                [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger && _mainWindow) {
                if (_mainWindow->isVisible()) {
                    _mainWindow->hide();
                } else {
                    _mainWindow->show();
                    _mainWindow->raise();
                }
            }
        });

        _trayIcon->setContextMenu(menu);
        _trayIcon->setToolTip("Notepad++ [Linux Port]");
        _trayIcon->show();
        return true;
    }

    /// Muestra un mensaje en el globo de la bandeja.
    void showTrayMessage(const QString& title, const QString& message,
                         QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                         int durationMs = 3000) {
        if (_trayIcon)
            _trayIcon->showMessage(title, message, icon, durationMs);
    }

    // ── Abrir con programa externo ──────────────────────────────────────────

    /// Equivalente a ShellExecute("open", url/file).
    static bool openExternal(const NppString& pathOrUrl) {
        return QProcess::startDetached("xdg-open",
            {QString::fromStdString(pathOrUrl)});
    }

    /// Abre una URL en el navegador predeterminado.
    static bool openUrl(const NppString& url) {
        return openExternal(url);
    }

    /// Abre un archivo con su aplicación predeterminada.
    static bool openFileExternal(const NppString& filePath) {
        return openExternal(filePath);
    }

    // ── Archivos .desktop e integración XDG ─────────────────────────────────

    /// Genera e instala el archivo .desktop para Notepad++.
    static bool installDesktopFile() {
        QString desktopDir = QStandardPaths::writableLocation(
            QStandardPaths::ApplicationsLocation);
        QDir().mkpath(desktopDir);

        QString desktopFile = desktopDir + "/notepad-plus-plus.desktop";
        QFile file(desktopFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream out(&file);
        out << "[Desktop Entry]\n"
            << "Type=Application\n"
            << "Name=Notepad++\n"
            << "GenericName=Text Editor\n"
            << "Comment=Editor de texto y código fuente avanzado\n"
            << "Exec=notepadplusplus %F\n"
            << "Icon=notepad-plus-plus\n"
            << "Terminal=false\n"
            << "Categories=Development;TextEditor;Utility;\n"
            << "MimeType=text/plain;text/x-c;text/x-c++;text/x-python;"
            << "text/html;text/xml;text/css;application/json;"
            << "text/x-java;text/x-script.python;text/x-shellscript;"
            << "text/markdown;application/javascript;\n"
            << "Keywords=text;editor;code;notepad;programming;\n"
            << "StartupWMClass=notepadplusplus\n";

        file.close();

        // Actualizar la base de datos de archivos .desktop
        QProcess::startDetached("update-desktop-database",
            {desktopDir});

        return true;
    }

    /// Registra Notepad++ como editor predeterminado para tipos MIME comunes.
    static void registerAsDefaultEditor() {
        QStringList mimeTypes = {
            "text/plain", "text/x-c", "text/x-c++src",
            "text/x-python", "text/html", "text/xml",
            "text/css", "application/json", "text/x-java",
            "text/x-shellscript", "text/markdown",
            "application/javascript"
        };

        for (const auto& mime : mimeTypes) {
            QProcess::execute("xdg-mime",
                {"default", "notepad-plus-plus.desktop", mime});
        }
    }

private:
    QWidget*          _mainWindow = nullptr;
    QSystemTrayIcon*  _trayIcon   = nullptr;
};

#endif // NPP_PLATFORM_LINUX

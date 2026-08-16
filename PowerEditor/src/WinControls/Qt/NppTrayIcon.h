// WinControls/Qt/NppTrayIcon.h — Control de TrayIcon WinControls Qt6
// Reemplaza WinControls/TrayIcon/TrayIcon.cpp/.h en Linux
//
// En Windows: NOTIFYICONDATA + Shell_NotifyIcon
// En Linux:   Wrapper sobre PlatformDesktopIntegration / QSystemTrayIcon
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../../Platform/PlatformDesktopIntegration.h"

#ifdef NPP_PLATFORM_LINUX

#include <QWidget>
#include <QIcon>
#include <QString>

/// TrayIcon control portable para Notepad++ Linux.
class NppTrayIcon {
public:
    explicit NppTrayIcon(QWidget* parent = nullptr)
        : _integration(parent, parent)
    {}

    bool init(const QIcon& icon) {
        return _integration.createTrayIcon(icon);
    }

    void showMessage(const QString& title, const QString& msg) {
        _integration.showTrayMessage(title, msg);
    }

private:
    NppDesktopIntegration _integration;
};

#endif // NPP_PLATFORM_LINUX

// WinControls/Qt/NppAboutDlg.h — Diálogo About portable Qt6
// Reemplaza WinControls/AboutDlg/AboutDlg.cpp/.h en Linux
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QSysInfo>
#include <Qsci/qsciglobal.h>

/// Diálogo "Acerca de" para Notepad++ Linux.
class NppAboutDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppAboutDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Acerca de Notepad++");
        setFixedSize(480, 380);
        buildUI();
    }

private:
    void buildUI() {
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(12);
        mainLayout->setContentsMargins(24, 24, 24, 24);

        // Título
        auto* lblTitle = new QLabel("Notepad++", this);
        QFont titleFont("Inter", 24, QFont::Bold);
        lblTitle->setFont(titleFont);
        lblTitle->setStyleSheet("color: #569CD6;");
        lblTitle->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(lblTitle);

        // Subtítulo
        auto* lblSubtitle = new QLabel("Linux Native Port", this);
        QFont subFont("Inter", 12);
        lblSubtitle->setFont(subFont);
        lblSubtitle->setStyleSheet("color: #4EC9B0;");
        lblSubtitle->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(lblSubtitle);

        mainLayout->addSpacing(8);

        // Información de versión
        QString versionInfo = QString(
            "<table style='color: #D4D4D4; font-size: 10pt;'>"
            "<tr><td style='padding-right: 12px;'><b>Versión:</b></td>"
            "    <td>9.0-linux (Port nativo)</td></tr>"
            "<tr><td><b>Qt:</b></td>"
            "    <td>%1</td></tr>"
            "<tr><td><b>QScintilla:</b></td>"
            "    <td>%2</td></tr>"
            "<tr><td><b>Compilador:</b></td>"
            "    <td>GCC %3</td></tr>"
            "<tr><td><b>OS:</b></td>"
            "    <td>%4 %5</td></tr>"
            "<tr><td><b>Arquitectura:</b></td>"
            "    <td>%6</td></tr>"
            "</table>")
            .arg(qVersion())
            .arg(QSCINTILLA_VERSION_STR)
            .arg(__VERSION__)
            .arg(QSysInfo::prettyProductName())
            .arg(QSysInfo::kernelVersion())
            .arg(QSysInfo::currentCpuArchitecture());

        auto* lblInfo = new QLabel(versionInfo, this);
        lblInfo->setTextFormat(Qt::RichText);
        lblInfo->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(lblInfo);

        mainLayout->addSpacing(8);

        // Créditos
        auto* lblCredits = new QLabel(
            "<span style='color: #969696; font-size: 9pt;'>"
            "Notepad++ es creado por Don Ho.<br>"
            "Port a Linux por el proyecto notepad-plus-plus/linux-native-port.<br>"
            "Motor de edición: Scintilla por Neil Hodgson.<br>"
            "Licencia: GNU General Public License v3."
            "</span>", this);
        lblCredits->setTextFormat(Qt::RichText);
        lblCredits->setWordWrap(true);
        lblCredits->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(lblCredits);

        mainLayout->addStretch();

        // Botones
        auto* btnLayout = new QHBoxLayout();
        auto* btnHomepage = new QPushButton("🌐 Sitio Web", this);
        auto* btnGitHub   = new QPushButton("🐙 GitHub", this);
        auto* btnClose    = new QPushButton("Cerrar", this);

        QString btnStyle =
            "QPushButton { background: #0E639C; color: #FFF; border: none; "
            "padding: 8px 16px; border-radius: 3px; }"
            "QPushButton:hover { background: #1177BB; }";
        btnHomepage->setStyleSheet(btnStyle);
        btnGitHub->setStyleSheet(btnStyle);
        btnClose->setStyleSheet(btnStyle);

        btnLayout->addWidget(btnHomepage);
        btnLayout->addWidget(btnGitHub);
        btnLayout->addStretch();
        btnLayout->addWidget(btnClose);
        mainLayout->addLayout(btnLayout);

        connect(btnHomepage, &QPushButton::clicked, []() {
            QDesktopServices::openUrl(QUrl("https://notepad-plus-plus.org"));
        });
        connect(btnGitHub, &QPushButton::clicked, []() {
            QDesktopServices::openUrl(QUrl("https://github.com/edoom85/notepad-plus-plus"));
        });
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    }
};

#endif // NPP_PLATFORM_LINUX

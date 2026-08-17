// Platform/PlatformIconProvider.h — Proveedor de iconos oficiales .ico (Linux/Qt6)
// Carga los iconos oficiales de Notepad++ (.ico/.bmp) según el estilo seleccionado en Preferencias:
// Iconos pequeños/grandes, con/sin relleno (regular/filled) y modo claro/oscuro.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QColor>
#include <QSize>

/// Generador y proveedor de iconos nativos para Notepad++ Linux.
class NppIconProvider {
public:
    enum class IconType {
        New,
        Open,
        Save,
        SaveAll,
        Close,
        CloseAll,
        Print,
        Cut,
        Copy,
        Paste,
        Undo,
        Redo,
        Find,
        Replace,
        ZoomIn,
        ZoomOut,
        SyncV,
        SyncH,
        UDL,
        DocMap,
        DocList,
        FunctionList,
        FileBrowser,
        Monitoring,
        AllChars,
        IndentGuide,
        Wrap,
        StartRecord,
        StopRecord,
        PlayRecord,
        SaveRecord,
        PlayRecordM,
        ProjectPanel,
        ClipboardHistory,
        Plugins,
        Settings,
        About,
        Run,
        AppLogo
    };


    static inline int  s_presetIndex = 4; // 0: SmallReg, 1: LargeReg, 2: SmallFilled, 3: LargeFilled, 4: Default
    static inline bool s_isDarkMode  = true;

    static void setToolbarStyle(int presetIndex, bool isDarkMode = true) {
        s_presetIndex = presetIndex;
        s_isDarkMode  = isDarkMode;
    }

    /// Obtiene un QIcon garantizado para el tipo de acción solicitado.
    static QIcon get(IconType type) {
        QStyle* style = QApplication::style();

        // 1. Intentar cargar el archivo .ico oficial de Notepad++ según el preset
        QString filename;
        switch (type) {
            case IconType::New:              filename = "new_off.ico"; break;
            case IconType::Open:             filename = "open_off.ico"; break;
            case IconType::Save:             filename = "save_off.ico"; break;
            case IconType::SaveAll:          filename = "saveall_off.ico"; break;
            case IconType::Close:            filename = "close_off.ico"; break;
            case IconType::CloseAll:         filename = "closeall_off.ico"; break;
            case IconType::Print:            filename = "print_off.ico"; break;
            case IconType::Cut:              filename = "cut_off.ico"; break;
            case IconType::Copy:             filename = "copy_off.ico"; break;
            case IconType::Paste:            filename = "paste_off.ico"; break;
            case IconType::Undo:             filename = "undo_off.ico"; break;
            case IconType::Redo:             filename = "redo_off.ico"; break;
            case IconType::Find:             filename = "find_off.ico"; break;
            case IconType::Replace:          filename = "findrep_off.ico"; break;
            case IconType::ZoomIn:           filename = "zoomIn_off.ico"; break;
            case IconType::ZoomOut:          filename = "zoomOut_off.ico"; break;
            case IconType::SyncV:            filename = "syncV_off.ico"; break;
            case IconType::SyncH:            filename = "syncH_off.ico"; break;
            case IconType::UDL:              filename = "udl_off.ico"; break;
            case IconType::DocMap:           filename = "docMap_off.ico"; break;
            case IconType::DocList:          filename = "docList_off.ico"; break;
            case IconType::FunctionList:    filename = "funcList_off.ico"; break;
            case IconType::FileBrowser:      filename = "fileBrowser_off.ico"; break;
            case IconType::Monitoring:       filename = "monitoring_off.ico"; break;
            case IconType::AllChars:         filename = "allChars_off.ico"; break;
            case IconType::IndentGuide:      filename = "indentGuide_off.ico"; break;
            case IconType::Wrap:             filename = "wrap_off.ico"; break;
            case IconType::StartRecord:      filename = "startrecord_off.ico"; break;
            case IconType::StopRecord:       filename = "stoprecord_off.ico"; break;
            case IconType::PlayRecord:       filename = "playrecord_off.ico"; break;
            case IconType::SaveRecord:       filename = "saverecord_off.ico"; break;
            case IconType::PlayRecordM:      filename = "playrecord_m_off.ico"; break;
            case IconType::ProjectPanel:    filename = "docList_off.ico"; break;
            case IconType::ClipboardHistory: filename = "docMap_off.ico"; break;
            default: break;
        }

        if (!filename.isEmpty()) {
            QString qrcPath;
            switch (s_presetIndex) {
                case 0: // SmallReg (sin relleno)
                    qrcPath = QString(":/icons/dark/toolbar/regular/%1").arg(filename);
                    break;
                case 1: // LargeReg (sin relleno)
                    qrcPath = QString(":/icons/dark/toolbar/regular/%1").arg(filename);
                    break;
                case 2: // SmallFilled (con relleno)
                    qrcPath = QString(":/icons/light/toolbar/filled/%1").arg(filename);
                    break;
                case 3: // LargeFilled (con relleno)
                    qrcPath = QString(":/icons/light/toolbar/filled/%1").arg(filename);
                    break;
                case 4: // Default ("Iconos pequeños predeterminados" - Colores oficiales Notepad++)
                default:
                    qrcPath = QString(":/icons/light/toolbar/regular/%1").arg(filename);
                    break;
            }

            QIcon icon = loadIco(qrcPath);
            if (!icon.isNull()) return icon;

            // Intentar fallback a light regular
            qrcPath = QString(":/icons/light/toolbar/regular/%1").arg(filename);
            icon = loadIco(qrcPath);
            if (!icon.isNull()) return icon;
        }


        // 2. Fallback según tipo de acción
        switch (type) {
            case IconType::New:
                return fetchThemeOrStandard("document-new", QStyle::SP_FileIcon, QColor(0x56, 0x9C, 0xD6), "📄");
            case IconType::Open:
                return fetchThemeOrStandard("document-open", QStyle::SP_DialogOpenButton, QColor(0xD6, 0x9D, 0x85), "📁");
            case IconType::Save:
                return fetchThemeOrStandard("document-save", QStyle::SP_DialogSaveButton, QColor(0x4E, 0xC9, 0xB0), "💾");
            case IconType::SaveAll:
                return fetchThemeOrStandard("document-save-all", QStyle::SP_DriveFDIcon, QColor(0x4E, 0xC9, 0xB0), "💾💾");
            case IconType::Close:
                return fetchThemeOrStandard("window-close", QStyle::SP_DialogCloseButton, QColor(0xF4, 0x47, 0x47), "❌");
            case IconType::CloseAll:
                return fetchThemeOrStandard("window-close", QStyle::SP_DialogCloseButton, QColor(0xF4, 0x47, 0x47), "❌❌");
            case IconType::Print:
                return fetchThemeOrStandard("document-print", QStyle::SP_FileDialogListView, QColor(0x9B, 0x9B, 0x9B), "🖨");
            case IconType::Undo:
                return fetchThemeOrStandard("edit-undo", QStyle::SP_ArrowLeft, QColor(0xDC, 0xDC, 0xAA), "↩");
            case IconType::Redo:
                return fetchThemeOrStandard("edit-redo", QStyle::SP_ArrowRight, QColor(0xDC, 0xDC, 0xAA), "↪");
            case IconType::Cut:
                return fetchThemeOrStandard("edit-cut", QStyle::SP_CustomBase, QColor(0xC5, 0x86, 0xC0), "✂");
            case IconType::Copy:
                return fetchThemeOrStandard("edit-copy", QStyle::SP_FileDialogListView, QColor(0xC5, 0x86, 0xC0), "📋");
            case IconType::Paste:
                return fetchThemeOrStandard("edit-paste", QStyle::SP_FileDialogDetailedView, QColor(0xC5, 0x86, 0xC0), "📌");
            case IconType::Find:
                return fetchThemeOrStandard("edit-find", QStyle::SP_FileDialogContentsView, QColor(0xDC, 0xDC, 0xAA), "🔍");
            case IconType::Replace:
                return fetchThemeOrStandard("edit-find-replace", QStyle::SP_BrowserReload, QColor(0x56, 0x9C, 0xD6), "🔄");
            case IconType::ZoomIn:
                return fetchThemeOrStandard("zoom-in", QStyle::SP_ArrowUp, QColor(0x4E, 0xC9, 0xB0), "🔍+");
            case IconType::ZoomOut:
                return fetchThemeOrStandard("zoom-out", QStyle::SP_ArrowDown, QColor(0x4E, 0xC9, 0xB0), "🔍-");
            case IconType::Settings:
                return fetchThemeOrStandard("preferences-system", QStyle::SP_FileDialogInfoView, QColor(0x9B, 0x9B, 0x9B), "⚙");
            case IconType::About:
                return fetchThemeOrStandard("help-about", QStyle::SP_MessageBoxInformation, QColor(0x56, 0x9C, 0xD6), "ℹ");
            case IconType::AppLogo: {
                QIcon icon = loadIco(":/Platform/npp_icon.png");
                if (icon.isNull()) icon = loadIco(":/icons/npp_256.ico");
                if (icon.isNull()) icon = fetchThemeOrStandard("notepad++", QStyle::SP_TitleBarMenuButton, QColor(0x90, 0xEE, 0x90), "🦎");
                return icon;
            }
            case IconType::FileBrowser:
                return fetchThemeOrStandard("folder", QStyle::SP_DirIcon, QColor(0xD6, 0x9D, 0x85), "📂");
            case IconType::FunctionList:
                return fetchThemeOrStandard("text-x-generic", QStyle::SP_FileIcon, QColor(0x4E, 0xC9, 0xB0), "ƒ");
            case IconType::ProjectPanel:
                return fetchThemeOrStandard("folder-development", QStyle::SP_DirLinkIcon, QColor(0x56, 0x9C, 0xD6), "📦");
            case IconType::ClipboardHistory:
                return fetchThemeOrStandard("edit-paste", QStyle::SP_FileDialogDetailedView, QColor(0xD4, 0xD4, 0xD4), "📋");
            case IconType::Run:
                return fetchThemeOrStandard("system-run", QStyle::SP_MediaPlay, QColor(0x6A, 0x99, 0x55), "▶");
            case IconType::Plugins:
                return fetchThemeOrStandard("application-x-executable", QStyle::SP_TitleBarMenuButton, QColor(0xC5, 0x86, 0xC0), "🧩");
            default:
                return style->standardIcon(QStyle::SP_FileIcon);
        }
    }

private:
    static QIcon loadIco(const QString& path) {
        QIcon icon(path);
        if (!icon.availableSizes().isEmpty()) {
            return icon;
        }
        return QIcon();
    }

    static QIcon fetchThemeOrStandard(const QString& themeName, QStyle::StandardPixmap sp, const QColor& accentColor, const QString& symbol) {
        if (QIcon::hasThemeIcon(themeName)) {
            QIcon icon = QIcon::fromTheme(themeName);
            if (!icon.availableSizes().isEmpty()) return icon;
        }

        QIcon stdIcon = QApplication::style()->standardIcon(sp);
        if (!stdIcon.availableSizes().isEmpty()) return stdIcon;

        return createFallbackIcon(accentColor, symbol);
    }

    static QIcon createFallbackIcon(const QColor& color, const QString& symbol) {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);

        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);

        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(1, 1, 22, 22, 4, 4);

        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPixelSize(12);
        f.setBold(true);
        p.setFont(f);
        p.drawText(pix.rect(), Qt::AlignCenter, symbol);

        return QIcon(pix);
    }
};

#endif // NPP_PLATFORM_LINUX

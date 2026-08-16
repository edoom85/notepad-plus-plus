// Platform/PlatformIconProvider.h — Proveedor de iconos vectoriales y fallbacks (Linux/Qt6)
// Garantiza que todos los botones y menús de Notepad++ tengan iconos limpios y visibles
// incluso si el tema de iconos del sistema (KDE/GNOME) no está instalado.
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
        Undo,
        Redo,
        Cut,
        Copy,
        Paste,
        Find,
        Replace,
        ZoomIn,
        ZoomOut,
        Settings,
        About,
        FileBrowser,
        FunctionList,
        ProjectPanel,
        ClipboardHistory,
        Run,
        Plugins
    };

    /// Obtiene un QIcon garantizado para el tipo de acción solicitado.
    static QIcon get(IconType type) {
        QStyle* style = QApplication::style();

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
    static QIcon fetchThemeOrStandard(const QString& themeName, QStyle::StandardPixmap sp, const QColor& accentColor, const QString& symbol) {
        // 1. Intentar cargar del sistema si el icono del tema existe
        if (QIcon::hasThemeIcon(themeName)) {
            QIcon icon = QIcon::fromTheme(themeName);
            if (!icon.isNull()) return icon;
        }

        // 2. Usar el icono estándar de QStyle si no es nulo
        QIcon stdIcon = QApplication::style()->standardIcon(sp);
        if (!stdIcon.isNull()) return stdIcon;

        // 3. Fallback dibujado vectorialmente en QPixmap (nunca falla)
        return createFallbackIcon(accentColor, symbol);
    }

    static QIcon createFallbackIcon(const QColor& color, const QString& symbol) {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);

        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);

        // Fondo circular redondeado
        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(1, 1, 22, 22, 4, 4);

        // Símbolo de texto centrado en blanco
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

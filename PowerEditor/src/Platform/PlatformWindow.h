// Platform/PlatformWindow.h — Clase base Window portable
// Parte del plan de migración Notepad++ → Linux nativo
//
// Reemplaza WinControls/Window.h con una implementación dual:
//   - Windows: delega a Win32 (HWND, ShowWindow, MoveWindow, etc.)
//   - Linux:   delega a GTK4 (GtkWidget*, gtk_widget_show/hide, etc.)
//
// La clase original Window.h tiene implementación inline completa en Win32.
// Esta versión la convierte en una interfaz portable con backends separados.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

// ─── Clase base Window portable ──────────────────────────────────────────────

class PlatformWindow {
public:
    PlatformWindow()                              = default;
    PlatformWindow(const PlatformWindow&)         = delete;
    PlatformWindow& operator=(const PlatformWindow&) = delete;
    virtual ~PlatformWindow()                     = default;

    /// Inicializa la ventana con su instancia de aplicación y ventana padre.
    virtual void init(NppHinst hInst, NppHwnd parent) {
        _hInst   = hInst;
        _hParent = parent;
    }

    /// Libera recursos de la ventana. Obligatorio en subclases.
    virtual void destroy() = 0;

    /// Muestra u oculta la ventana.
    virtual void display(bool toShow = true) const;

    /// Redibuja la ventana.
    virtual void redraw(bool forceUpdate = false) const;

    /// Mueve y redimensiona la ventana al rectángulo dado (posición absoluta).
    virtual void reSizeTo(RECT& rc);

    /// Mueve y redimensiona usando width/height del rectángulo.
    virtual void reSizeToWH(RECT& rc);

    /// Obtiene el rectángulo cliente (área interior, sin decoraciones).
    virtual void getClientRect(RECT& rc) const;

    /// Obtiene el rectángulo de ventana en coordenadas de pantalla.
    virtual void getWindowRect(RECT& rc) const;

    /// Ancho del área cliente.
    virtual int getWidth() const;

    /// Altura del área cliente (0 si la ventana no es visible).
    virtual int getHeight() const;

    /// Comprueba si la ventana es visible.
    virtual bool isVisible() const;

    /// Asigna el foco de teclado a esta ventana.
    void grabFocus() const;

    // ── Accessors ────────────────────────────────────────────────────────────
    NppHwnd   getHSelf()   const { return _hSelf;   }
    NppHwnd   getHParent() const { return _hParent; }
    NppHinst  getHinst()   const { return _hInst;   }

protected:
    NppHinst _hInst   = NPP_NULL_HANDLE;
    NppHwnd  _hParent = NPP_NULL_HWND;
    NppHwnd  _hSelf   = NPP_NULL_HWND;
};

// ─── Implementaciones por plataforma ─────────────────────────────────────────

#ifdef NPP_PLATFORM_WINDOWS

// Implementación Windows — delega directamente a Win32
inline void PlatformWindow::display(bool toShow) const {
    ::ShowWindow(_hSelf, toShow ? SW_SHOW : SW_HIDE);
}
inline void PlatformWindow::redraw(bool forceUpdate) const {
    ::InvalidateRect(_hSelf, nullptr, TRUE);
    if (forceUpdate) ::UpdateWindow(_hSelf);
}
inline void PlatformWindow::reSizeTo(RECT& rc) {
    ::MoveWindow(_hSelf, rc.left, rc.top, rc.right, rc.bottom, TRUE);
    redraw();
}
inline void PlatformWindow::reSizeToWH(RECT& rc) {
    ::MoveWindow(_hSelf, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    redraw();
}
inline void PlatformWindow::getClientRect(RECT& rc) const {
    ::GetClientRect(_hSelf, &rc);
}
inline void PlatformWindow::getWindowRect(RECT& rc) const {
    ::GetWindowRect(_hSelf, &rc);
}
inline int PlatformWindow::getWidth() const {
    RECT rc; ::GetClientRect(_hSelf, &rc); return rc.right - rc.left;
}
inline int PlatformWindow::getHeight() const {
    RECT rc; ::GetClientRect(_hSelf, &rc);
    return ::IsWindowVisible(_hSelf) ? (rc.bottom - rc.top) : 0;
}
inline bool PlatformWindow::isVisible() const {
    return ::IsWindowVisible(_hSelf) != FALSE;
}
inline void PlatformWindow::grabFocus() const {
    ::SetFocus(_hSelf);
}

#elif defined(NPP_PLATFORM_LINUX)

// Implementación Linux — delega a Qt6 QWidget
// NOTA: Los archivos .cpp que incluyan este header deben incluir <QWidget> primero.

#include <QWidget>

inline void PlatformWindow::display(bool toShow) const {
    if (!_hSelf) return;
    _hSelf->setVisible(toShow);
}
inline void PlatformWindow::redraw(bool forceUpdate) const {
    if (!_hSelf) return;
    if (forceUpdate)
        _hSelf->repaint();
    else
        _hSelf->update();
}
inline void PlatformWindow::reSizeTo(RECT& rc) {
    if (!_hSelf) return;
    _hSelf->setGeometry(rc.left, rc.top, rc.right, rc.bottom);
}
inline void PlatformWindow::reSizeToWH(RECT& rc) {
    if (!_hSelf) return;
    _hSelf->setGeometry(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}
inline void PlatformWindow::getClientRect(RECT& rc) const {
    if (!_hSelf) { rc = {}; return; }
    rc.left   = 0;
    rc.top    = 0;
    rc.right  = _hSelf->width();
    rc.bottom = _hSelf->height();
}
inline void PlatformWindow::getWindowRect(RECT& rc) const {
    if (!_hSelf) { rc = {}; return; }
    // mapToGlobal convierte coordenadas locales a de pantalla
    QPoint topLeft = _hSelf->mapToGlobal(QPoint(0, 0));
    rc.left   = topLeft.x();
    rc.top    = topLeft.y();
    rc.right  = topLeft.x() + _hSelf->width();
    rc.bottom = topLeft.y() + _hSelf->height();
}
inline int PlatformWindow::getWidth() const {
    return _hSelf ? _hSelf->width() : 0;
}
inline int PlatformWindow::getHeight() const {
    if (!_hSelf || !_hSelf->isVisible()) return 0;
    return _hSelf->height();
}
inline bool PlatformWindow::isVisible() const {
    return _hSelf && _hSelf->isVisible();
}
inline void PlatformWindow::grabFocus() const {
    if (_hSelf) _hSelf->setFocus();
}

#endif // NPP_PLATFORM_LINUX


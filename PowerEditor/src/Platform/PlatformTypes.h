// Platform/PlatformTypes.h — Tipos portables Win32 → Linux
// Parte del plan de migración Notepad++ → Linux nativo
//
// En Windows: tipos nativos Win32 sin cambios (transparente).
// En Linux:   tipos equivalentes usando GTK4/GLib o stubs.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

// ─── Detección de plataforma ─────────────────────────────────────────────────

#if defined(_WIN32) || defined(_WIN64)
  #define NPP_PLATFORM_WINDOWS 1
#elif defined(__linux__)
  #define NPP_PLATFORM_LINUX 1
#elif defined(__APPLE__)
  #define NPP_PLATFORM_MACOS 1
#endif

// ─── Windows: usar tipos nativos, sin cambios ────────────────────────────────

#ifdef NPP_PLATFORM_WINDOWS

  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <commctrl.h>

  // Tipos re-exportados por conveniencia (sin cambios)
  using NppHwnd     = HWND;
  using NppHinst    = HINSTANCE;
  using NppHmenu    = HMENU;
  using NppHdc      = HDC;
  using NppHfont    = HFONT;
  using NppHbrush   = HBRUSH;
  using NppHbitmap  = HBITMAP;
  using NppHicon    = HICON;
  using NppHandle   = HANDLE;
  using NppColorRef = COLORREF;
  using NppUint     = UINT;
  using NppWparam   = WPARAM;
  using NppLparam   = LPARAM;
  using NppLresult  = LRESULT;
  using NppDword    = DWORD;
  using NppWord     = WORD;
  using NppBool     = BOOL;

  // Constantes nulas
  #define NPP_NULL_HWND   nullptr
  #define NPP_NULL_HANDLE nullptr
  #define NPP_NULL_MENU   nullptr

  // Macro de visibilidad de símbolo (para plugins)
  #define NPP_EXPORT __declspec(dllexport)
  #define NPP_IMPORT __declspec(dllimport)

// ─── Linux: equivalentes Qt6 ─────────────────────────────────────────────────

#elif defined(NPP_PLATFORM_LINUX)

  #include <cstdint>

  // Forward declarations de Qt — evita incluir headers pesados aquí.
  // Los archivos .cpp que necesiten la API completa incluyen los headers Qt directamente.
  class QWidget;
  class QApplication;
  class QMenu;
  class QPainter;
  class QFont;
  class QBrush;
  class QPixmap;
  class QIcon;

  // Handles de ventana: en Qt el equivalente es QWidget*
  using NppHwnd     = QWidget*;
  using NppHinst    = QApplication*;
  using NppHmenu    = QMenu*;
  using NppHdc      = QPainter*;
  using NppHfont    = QFont*;
  using NppHbrush   = QBrush*;
  using NppHbitmap  = QPixmap*;
  using NppHicon    = QIcon*;
  using NppHandle   = void*;
  using NppColorRef = uint32_t;   // 0x00BBGGRR (igual que Win32 COLORREF)
  using NppUint     = unsigned int;
  using NppWparam   = uintptr_t;
  using NppLparam   = intptr_t;
  using NppLresult  = intptr_t;
  using NppDword    = uint32_t;
  using NppWord     = uint16_t;
  using NppBool     = int;        // como en Win32, 0=false, !0=true

  // Constantes nulas
  #define NPP_NULL_HWND   nullptr
  #define NPP_NULL_HANDLE nullptr
  #define NPP_NULL_MENU   nullptr

  // Macro de visibilidad de símbolo (para plugins Linux .so)
  #define NPP_EXPORT __attribute__((visibility("default")))
  #define NPP_IMPORT


  // ── Compatibilidad: tipos Win32 como aliases ────────────────────────────────
  // Estos aliases permiten que código existente que usa HWND/HINSTANCE/etc.
  // compile en Linux sin cambiar cada aparición individualmente todavía.
  // A medida que se porta cada módulo, se migran a NppHwnd/NppHinst/etc.
  //
  // NOTA: Solo activos cuando NO se incluye <windows.h>
  #ifndef _WINDOWS_H
    using HWND      = NppHwnd;
    using HINSTANCE = NppHinst;
    using HMENU     = NppHmenu;
    using HDC       = NppHdc;
    using HFONT     = NppHfont;
    using HBRUSH    = NppHbrush;
    using HBITMAP   = NppHbitmap;
    using HICON     = NppHicon;
    using HANDLE    = NppHandle;
    using COLORREF  = NppColorRef;
    using UINT      = NppUint;
    using WPARAM    = NppWparam;
    using LPARAM    = NppLparam;
    using LRESULT   = NppLresult;
    using DWORD     = NppDword;
    using WORD      = NppWord;
    using BOOL      = NppBool;

    #ifndef TRUE
      #define TRUE  1
    #endif
    #ifndef FALSE
      #define FALSE 0
    #endif
    #ifndef NULL
      #define NULL nullptr
    #endif
    #ifndef MAX_PATH
      #define MAX_PATH 4096
    #endif
    #ifndef INVALID_HANDLE_VALUE
      #define INVALID_HANDLE_VALUE ((NppHandle)(-1))
    #endif
  #endif // _WINDOWS_H

  // ── Helpers de color ───────────────────────────────────────────────────────
  // Compatibilidad con macros Win32 de manejo de COLORREF
  #define RGB(r,g,b)    ((NppColorRef)(((uint8_t)(r)) | (((uint8_t)(g)) << 8) | (((uint8_t)(b)) << 16)))
  #define GetRValue(c)  ((uint8_t)((c) & 0xFF))
  #define GetGValue(c)  ((uint8_t)(((c) >> 8) & 0xFF))
  #define GetBValue(c)  ((uint8_t)(((c) >> 16) & 0xFF))

  // ── Macro RECT ─────────────────────────────────────────────────────────────
  struct RECT {
    int left   = 0;
    int top    = 0;
    int right  = 0;
    int bottom = 0;
  };

  inline int RECT_Width(const RECT& r)  { return r.right - r.left; }
  inline int RECT_Height(const RECT& r) { return r.bottom - r.top; }

  // ── POINT ──────────────────────────────────────────────────────────────────
  struct POINT {
    int x = 0;
    int y = 0;
  };

  // ── SIZE ───────────────────────────────────────────────────────────────────
  struct SIZE {
    int cx = 0;
    int cy = 0;
  };

#endif // NPP_PLATFORM_LINUX

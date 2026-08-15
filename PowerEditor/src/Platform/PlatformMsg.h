// Platform/PlatformMsg.h — Sistema de mensajes portable (Win32 WM_* → Linux)
// Parte del plan de migración Notepad++ → Linux nativo
//
// En Windows los componentes se comunican mediante SendMessage/PostMessage con
// códigos WM_*. En Linux no existe este mecanismo; lo reemplazamos con:
//   - Señales/callbacks GTK (para widgets)
//   - Un bus de eventos interno ligero (NppMsgBus) para mensajes Notepad++
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include <functional>
#include <cstdint>

// ─── Windows: usar Win32 puro ────────────────────────────────────────────────

#ifdef NPP_PLATFORM_WINDOWS

  // En Windows todo esto ya existe, solo re-exportamos helpers.
  inline NppLresult NppSendMsg(NppHwnd hwnd, NppUint msg, NppWparam wp, NppLparam lp) {
      return ::SendMessage(hwnd, msg, wp, lp);
  }
  inline bool NppPostMsg(NppHwnd hwnd, NppUint msg, NppWparam wp, NppLparam lp) {
      return ::PostMessage(hwnd, msg, wp, lp) != FALSE;
  }

// ─── Linux: bus de mensajes interno ──────────────────────────────────────────

#elif defined(NPP_PLATFORM_LINUX)

  #include <unordered_map>
  #include <vector>
  #include <mutex>

  // Códigos de mensaje internos Notepad++ — espejo portable de los NPPM_* y WM_*
  // más usados. No cubre todos los WM_* de Win32 (los de sistema no aplican en Linux).
  enum class NppMsg : uint32_t {

    // ── Ciclo de vida ───────────────────────────────────────────────────────
    SHUTDOWN            = 0x0001,
    INIT_COMPLETE       = 0x0002,

    // ── Archivos ────────────────────────────────────────────────────────────
    FILE_OPEN           = 0x0100,
    FILE_CLOSE          = 0x0101,
    FILE_SAVE           = 0x0102,
    FILE_SAVE_AS        = 0x0103,
    FILE_RELOAD         = 0x0104,
    FILE_RENAMED        = 0x0105,
    FILE_DELETED        = 0x0106,

    // ── Buffer / Editor ─────────────────────────────────────────────────────
    BUFFER_SWITCH       = 0x0200,
    BUFFER_MODIFIED     = 0x0201,
    BUFFER_ENCODED      = 0x0202,
    LANG_CHANGED        = 0x0203,
    READONLY_CHANGED    = 0x0204,

    // ── UI ──────────────────────────────────────────────────────────────────
    TOOLBAR_REDRAW      = 0x0300,
    STATUSBAR_UPDATE    = 0x0301,
    TAB_CHANGED         = 0x0302,
    THEME_CHANGED       = 0x0303,
    DPI_CHANGED         = 0x0304,
    FOCUS_EDITOR        = 0x0305,

    // ── Plugins ─────────────────────────────────────────────────────────────
    PLUGIN_LOADED       = 0x0400,
    PLUGIN_UNLOADED     = 0x0401,

    // ── Búsqueda ────────────────────────────────────────────────────────────
    FIND_RESULT         = 0x0500,
    FIND_MARK_ALL       = 0x0501,

    // ── Scintilla relay ─────────────────────────────────────────────────────
    SCI_NOTIFY          = 0x0600,  // SCNotification relay desde Scintilla

    // Extensible — agregar nuevos mensajes aquí
    USER_BASE           = 0x1000,
  };

  // Tipo de callback de manejador de mensajes
  using NppMsgHandler = std::function<NppLresult(NppWparam, NppLparam)>;

  /// Bus de mensajes interno ligero.
  /// Reemplaza SendMessage/PostMessage para comunicación entre componentes Notepad++.
  class NppMsgBus {
  public:
    static NppMsgBus& instance();

    /// Registra un handler para un mensaje. Retorna ID del handler (para deregistrar).
    int subscribe(NppMsg msg, NppMsgHandler handler);

    /// Elimina un handler por ID.
    void unsubscribe(NppMsg msg, int handlerId);

    /// Envía un mensaje de forma síncrona. Retorna resultado del último handler.
    NppLresult send(NppMsg msg, NppWparam wp = 0, NppLparam lp = 0);

    /// Encola un mensaje para procesamiento asíncrono en el main loop GTK.
    void post(NppMsg msg, NppWparam wp = 0, NppLparam lp = 0);

  private:
    NppMsgBus() = default;
    struct HandlerEntry {
      int id;
      NppMsgHandler fn;
    };
    std::unordered_map<uint32_t, std::vector<HandlerEntry>> _handlers;
    std::mutex _mutex;
    int _nextId = 1;
  };

  // Helpers globales — interfaz compatible con el código existente (post-port)
  inline NppLresult NppSendMsg(NppHwnd /*hwnd*/, NppMsg msg, NppWparam wp = 0, NppLparam lp = 0) {
      return NppMsgBus::instance().send(msg, wp, lp);
  }
  inline void NppPostMsg(NppMsg msg, NppWparam wp = 0, NppLparam lp = 0) {
      NppMsgBus::instance().post(msg, wp, lp);
  }

#endif // NPP_PLATFORM_LINUX

// Platform/PlatformPlugin.h — Interfaz de plugins portable Linux/Windows
// Parte del plan de migración Notepad++ → Linux nativo
//
// En Windows los plugins son .dll cargados con LoadLibraryEx, y se comunican
// mediante HWND + WM_* mensajes. En Linux son .so cargados con dlopen,
// y se comunican mediante NppMsgBus + GtkWidget*.
//
// Este header redefine PluginInterface.h para ser portable:
//   - Windows: comportamiento idéntico al original
//   - Linux:   usa dlopen/dlsym, GtkWidget*, NPP_EXPORT, NppString
//
// Los plugins que quieran soportar Linux deben incluir este header
// en lugar de PluginInterface.h directamente.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"
#include "PlatformString.h"
#include "PlatformMsg.h"

// ─── Estructura de datos que el host pasa a cada plugin ───────────────────────
// En Windows: HWND de la ventana principal y los dos Scintilla.
// En Linux:   GtkWidget* de la ventana principal y los dos Scintilla.

struct NppData {
    NppHwnd _nppHandle            = NPP_NULL_HWND; // Ventana principal
    NppHwnd _scintillaMainHandle  = NPP_NULL_HWND; // Editor primario
    NppHwnd _scintillaSecondHandle= NPP_NULL_HWND; // Editor secundario (split)
};

// ─── Tecla de atajo para comandos de plugin ───────────────────────────────────
struct ShortcutKey {
    bool  _isCtrl  = false;
    bool  _isAlt   = false;
    bool  _isShift = false;
    NppWord _key   = 0;        // código de tecla virtual (mismo que Win32 VK_*)
};

// ─── Ítem de menú que expone un plugin ───────────────────────────────────────
static constexpr int menuItemSize = 64;

// Tipo de función de comando de plugin (sin parámetros, sin retorno)
using PFUNCPLUGINCMD = void(*)();

struct FuncItem {
    NppChar       _itemName[menuItemSize] = { NppT('\0') };
    PFUNCPLUGINCMD _pFunc      = nullptr;
    int            _cmdID      = 0;
    bool           _init2Check = false;
    ShortcutKey*   _pShKey     = nullptr;
};

// ─── Tipos de puntero a función para las entradas del plugin ─────────────────
using PFUNCGETNAME      = const NppChar*(*)();
using PFUNCSETINFO      = void(*)(NppData);
using PFUNCGETFUNCSARRAY= FuncItem*(*)(int*);
using PBENOTIFIED       = void(*)(struct SCNotification*);

#ifdef NPP_PLATFORM_WINDOWS
  using PMESSAGEPROC = NppLresult(*)(NppUint, NppWparam, NppLparam);
  using PFUNCISUNICODE = NppBool(*)();
#else
  // En Linux el "messageProc" recibe NppMsg en lugar de UINT Win32
  using PMESSAGEPROC   = NppLresult(*)(NppMsg, NppWparam, NppLparam);
  using PFUNCISUNICODE = NppBool(*)(); // siempre true en Linux (UTF-8)
#endif

// ─── Macros de exportación de símbolo ────────────────────────────────────────
// Los plugins deben usar NPP_PLUGIN_EXPORT para declarar sus funciones públicas.
// En Windows: __declspec(dllexport)
// En Linux:   __attribute__((visibility("default")))

#ifdef NPP_PLATFORM_WINDOWS
  #define NPP_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
  #define NPP_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// ─── Declaraciones que DEBE implementar cada plugin ─────────────────────────
//
// Estas son las funciones que el PluginsManager busca con GetProcAddress/dlsym.
// Los autores de plugins deben implementarlas todas.
//
// NPP_PLUGIN_EXPORT void        setInfo(NppData nppData);
// NPP_PLUGIN_EXPORT const NppChar* getName();
// NPP_PLUGIN_EXPORT FuncItem*   getFuncsArray(int* nbF);
// NPP_PLUGIN_EXPORT void        beNotified(SCNotification* notifyCode);
// NPP_PLUGIN_EXPORT NppLresult  messageProc(NppMsg msg, NppWparam wParam, NppLparam lParam);
// NPP_PLUGIN_EXPORT NppBool     isUnicode();  // siempre true

// ─── Nombres de símbolo esperados (para dlsym/GetProcAddress) ───────────────
namespace NppPluginSymbols {
    static constexpr const char* SET_INFO       = "setInfo";
    static constexpr const char* GET_NAME       = "getName";
    static constexpr const char* GET_FUNCS      = "getFuncsArray";
    static constexpr const char* BE_NOTIFIED    = "beNotified";
    static constexpr const char* MSG_PROC       = "messageProc";
    static constexpr const char* IS_UNICODE     = "isUnicode";
    static constexpr const char* LEXER_COUNT    = "GetLexerCount";   // plugins lexer
    static constexpr const char* LEXER_NAME     = "GetLexerName";
    static constexpr const char* CREATE_LEXER   = "CreateLexer";
}

// ─── Cargador de plugins (capa de abstracción dlopen/LoadLibraryEx) ──────────

class NppPluginLoader {
public:
    /// Carga un plugin desde la ruta dada. Retorna true si tiene éxito.
    bool load(const NppString& path);

    /// Libera el plugin.
    void unload();

    /// ¿Está cargado?
    bool isLoaded() const { return _handle != nullptr; }

    /// Resuelve un símbolo por nombre. Retorna nullptr si no existe.
    void* resolve(const char* symbol) const;

    /// Ruta del archivo cargado.
    const NppString& path() const { return _path; }

    /// Mensaje de error del último fallo.
    NppString lastError() const;

    ~NppPluginLoader() { unload(); }

private:
    void*     _handle = nullptr;
    NppString _path;
};

// ─── Implementación inline por plataforma ────────────────────────────────────

#ifdef NPP_PLATFORM_WINDOWS

  inline bool NppPluginLoader::load(const NppString& path) {
      _path = path;
      const DWORD dwFlags = 
          GetProcAddress(GetModuleHandle("kernel32.dll"), "AddDllDirectory")
          ? (0x00000100 | 0x00001000)   // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | DEFAULT_DIRS
          : 0;
      _handle = reinterpret_cast<void*>(
          ::LoadLibraryExW(path.c_str(), NULL, dwFlags));
      return _handle != nullptr;
  }
  inline void NppPluginLoader::unload() {
      if (_handle) { ::FreeLibrary(reinterpret_cast<HMODULE>(_handle)); _handle = nullptr; }
  }
  inline void* NppPluginLoader::resolve(const char* symbol) const {
      return reinterpret_cast<void*>(
          ::GetProcAddress(reinterpret_cast<HMODULE>(_handle), symbol));
  }
  inline NppString NppPluginLoader::lastError() const {
      DWORD err = ::GetLastError();
      if (!err) return "";
      LPWSTR buf = nullptr;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          nullptr, err, 0, reinterpret_cast<LPWSTR>(&buf), 0, nullptr);
      NppString msg(buf ? buf : "Unknown error");
      if (buf) LocalFree(buf);
      return msg;
  }

#elif defined(NPP_PLATFORM_LINUX)

  #include <dlfcn.h>

  inline bool NppPluginLoader::load(const NppString& path) {
      _path = path;
      // RTLD_NOW: resuelve todos los símbolos inmediatamente (falla rápido si hay símbolos faltantes)
      // RTLD_LOCAL: los símbolos del plugin no son visibles globalmente (evita colisiones)
      _handle = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
      return _handle != nullptr;
  }
  inline void NppPluginLoader::unload() {
      if (_handle) { ::dlclose(_handle); _handle = nullptr; }
  }
  inline void* NppPluginLoader::resolve(const char* symbol) const {
      if (!_handle) return nullptr;
      return ::dlsym(_handle, symbol);
  }
  inline NppString NppPluginLoader::lastError() const {
      const char* err = ::dlerror();
      return err ? NppString(err) : NppString("Unknown error");
  }

#endif // platform

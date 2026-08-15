// Platform/PlatformString.h — Abstracción de strings Unicode portables
// Parte del plan de migración Notepad++ → Linux nativo
//
// Notepad++ usa std::wstring (UTF-16) pervasivamente en Windows.
// En Linux usamos std::string (UTF-8) de forma nativa.
//
// Este header define:
//   NppString  — string nativa de la plataforma
//   NppChar    — caracter nativo
//   NppT(x)   — literal de string (L"..." en Win, "..." en Linux)
//   NppSV      — string_view nativa
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include <string>
#include <string_view>

// ─── Windows: UTF-16 (wchar_t), igual que antes ──────────────────────────────

#ifdef _WIN32

  using NppChar   = wchar_t;
  using NppString = std::wstring;
  using NppSV     = std::wstring_view;
  #define NppT(x) L##x

  // Helpers de conversión (ya existen en Common.h, aquí solo declarados)
  #include <string>
  namespace NppPlatform {
    std::string  toUtf8(const NppString& ws);
    NppString    fromUtf8(const std::string& s);
  }

// ─── Linux: UTF-8 (char), string de C++ estándar ────────────────────────────

#else // Linux / macOS

  using NppChar   = char;
  using NppString = std::string;
  using NppSV     = std::string_view;
  #define NppT(x) x

  // Conversión no-op en Linux (ya es UTF-8)
  namespace NppPlatform {
    inline std::string  toUtf8(const NppString& s)    { return s; }
    inline NppString    fromUtf8(const std::string& s) { return s; }
  }

#endif

// ─── Helpers cross-platform ──────────────────────────────────────────────────

namespace NppPlatform {

  /// Compara dos NppString ignorando mayúsculas/minúsculas.
  /// Equivalente a _wcsicmp en Windows o strcasecmp en Linux.
  int icompare(const NppString& a, const NppString& b);

  /// Convierte un número entero a NppString (equivalente a std::to_wstring / std::to_string).
  NppString toString(int value);
  NppString toString(long value);
  NppString toString(long long value);
  NppString toString(unsigned int value);
  NppString toString(size_t value);
  NppString toString(double value);

  /// Trim de espacios en blancos.
  NppString trim(const NppString& s);

  /// Convierte a minúsculas.
  NppString toLower(const NppString& s);

  /// Convierte a mayúsculas.
  NppString toUpper(const NppString& s);

} // namespace NppPlatform

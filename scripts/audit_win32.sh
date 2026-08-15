#!/usr/bin/env bash
# audit_win32.sh — Genera un reporte de uso de APIs Win32 en el código fuente
# Parte del plan de migración Notepad++ → Linux nativo
# Uso: bash scripts/audit_win32.sh [directorio_src]

SRC="${1:-PowerEditor/src}"
OUTPUT="scripts/win32_audit_report.txt"

echo "=== Auditoría Win32 — Notepad++ Linux Port ===" > "$OUTPUT"
echo "Fecha: $(date)" >> "$OUTPUT"
echo "Directorio: $SRC" >> "$OUTPUT"
echo "" >> "$OUTPUT"

# ─── Categorías a auditar ────────────────────────────────────────────────────

declare -A CATEGORIES
CATEGORIES["HWND/HINSTANCE/handles"]="HWND\|HINSTANCE\|HMODULE\|HANDLE\|HKEY\|HFONT\|HICON\|HBRUSH\|HMENU\|HDC\|HBITMAP\|HGDIOBJ"
CATEGORIES["Win32 Window API"]="RegisterClassEx\|CreateWindow\|CreateWindowEx\|DefWindowProc\|ShowWindow\|MoveWindow\|InvalidateRect\|UpdateWindow\|DestroyWindow\|GetClientRect\|GetWindowRect"
CATEGORIES["Win32 Mensajes"]="SendMessage\|PostMessage\|WM_\|WPARAM\|LPARAM\|LRESULT\|DispatchMessage\|TranslateMessage\|PeekMessage\|GetMessage"
CATEGORIES["Win32 Dialogs"]="DialogBox\|CreateDialogParam\|EndDialog\|DialogProc\|IsDialogMessage\|DefDlgProc\|IDD_"
CATEGORIES["Win32 Menus"]="CreateMenu\|InsertMenu\|AppendMenu\|CheckMenuItem\|GetSubMenu\|HMENU\|MENUITEMINFO"
CATEGORIES["LoadLibrary/DLL"]="LoadLibrary\|LoadLibraryEx\|FreeLibrary\|GetProcAddress\|__declspec(dllexport)\|__declspec(dllimport)"
CATEGORIES["Win32 Filesystem"]="FindFirstFile\|FindNextFile\|FindClose\|WIN32_FIND_DATA\|CreateFile\|ReadFile\|WriteFile\|DeleteFile\|CopyFile\|MoveFile\|GetModuleFileName\|PathAppend\|PathFindFileName"
CATEGORIES["Win32 Registry"]="RegOpenKey\|RegCloseKey\|RegQueryValue\|RegSetValue\|HKEY_CURRENT_USER\|HKEY_LOCAL_MACHINE"
CATEGORIES["GDI / Painting"]="CreateFont\|SelectObject\|TextOut\|BitBlt\|CreateDC\|LOGFONT\|COLORREF\|GetSysColor\|SetTextColor\|SetBkColor\|DrawText\|GetTextExtent"
CATEGORIES["ReadDirectoryChanges"]="ReadDirectoryChanges\|FILE_NOTIFY_INFORMATION\|FILE_ACTION_"
CATEGORIES["wstring/wchar_t (UTF-16)"]="std::wstring\|wchar_t\|wcslen\|wcscmp\|wcscat\|wsprintf\|swprintf\|wprintf\|_wcsicmp\|L\""
CATEGORIES["Shell API"]="SHGetFolderPath\|SHBrowseForFolder\|IFileDialog\|IShellFolder\|ShellExecute\|CoInitialize"
CATEGORIES["DPI Windows"]="GetDeviceCaps\|LOGPIXELSX\|LOGPIXELSY\|GetDpiForWindow\|SetProcessDpiAwareness"
CATEGORIES["DarkMode Win32"]="uxtheme\|DwmSetWindowAttribute\|OpenThemeData\|UAHMenuBar\|IatHook"
CATEGORIES["Authenticode/Crypto"]="WinVerifyTrust\|CryptQueryObject\|WINTRUST_DATA\|crypt32\|wintrust"
CATEGORIES["Tray Icon"]="Shell_NotifyIcon\|NOTIFYICONDATA\|NIM_ADD\|NIM_DELETE\|NIM_MODIFY"
CATEGORIES["Process API"]="CreateProcess\|PROCESS_INFORMATION\|STARTUPINFO\|TerminateProcess\|WaitForSingleObject\|CreatePipe"

# ─── Generar reporte ──────────────────────────────────────────────────────────

TOTAL_FILES=0
TOTAL_HITS=0

for CATEGORY in "${!CATEGORIES[@]}"; do
    PATTERN="${CATEGORIES[$CATEGORY]}"
    echo "────────────────────────────────────────────" >> "$OUTPUT"
    echo "📌 $CATEGORY" >> "$OUTPUT"
    
    HITS=$(grep -rl "$PATTERN" "$SRC" --include="*.cpp" --include="*.h" 2>/dev/null | wc -l)
    echo "   Archivos afectados: $HITS" >> "$OUTPUT"
    
    grep -rl "$PATTERN" "$SRC" --include="*.cpp" --include="*.h" 2>/dev/null \
        | sed 's|^|   • |' >> "$OUTPUT"
    
    TOTAL_HITS=$((TOTAL_HITS + HITS))
    echo "" >> "$OUTPUT"
done

# ─── Resumen ──────────────────────────────────────────────────────────────────
TOTAL_FILES=$(find "$SRC" \( -name "*.cpp" -o -name "*.h" \) | wc -l)

echo "════════════════════════════════════════════" >> "$OUTPUT"
echo "RESUMEN" >> "$OUTPUT"
echo "  Total archivos .cpp/.h en $SRC: $TOTAL_FILES" >> "$OUTPUT"
echo "  Total referencias Win32 (sum de categorías): $TOTAL_HITS" >> "$OUTPUT"
echo "════════════════════════════════════════════" >> "$OUTPUT"

echo "✅ Reporte generado: $OUTPUT"
cat "$OUTPUT"

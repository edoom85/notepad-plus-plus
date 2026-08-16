#!/usr/bin/env bash
# scripts/migrate_strings.sh — Migración UTF-16 → UTF-8 para el port Linux
# Fase 5 del plan de migración Notepad++ → Linux nativo
#
# Este script reemplaza patrones Win32 de strings con sus equivalentes portables:
#   std::wstring     → NppString    (std::string en Linux)
#   wchar_t          → NppChar      (char en Linux)
#   TCHAR            → NppChar
#   L"..."           → "..."        (elimina el prefijo L de literales)
#   TEXT("...")       → NppT("...")  (macro portable)
#   _T("...")        → NppT("...")
#   _tcscmp          → strcmp equivalents
#   lstrlen/wcslen   → strlen/NppString::size()
#
# USO:
#   ./scripts/migrate_strings.sh [--dry-run] [directorio]
#
# OPCIONES:
#   --dry-run   Muestra los cambios sin aplicarlos
#   directorio  Directorio a procesar (default: PowerEditor/src)
#
# Copyright (C) Notepad++ contributors. GPL v3+

set -euo pipefail

# ─── Colores ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# ─── Argumentos ──────────────────────────────────────────────────────────────
DRY_RUN=false
TARGET_DIR="PowerEditor/src"

for arg in "$@"; do
    case "$arg" in
        --dry-run) DRY_RUN=true ;;
        *)         TARGET_DIR="$arg" ;;
    esac
done

if [[ ! -d "$TARGET_DIR" ]]; then
    echo -e "${RED}Error: Directorio '$TARGET_DIR' no encontrado.${NC}"
    exit 1
fi

echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  Notepad++ Linux Port — Migración UTF-16 → UTF-8          ║${NC}"
echo -e "${CYAN}║  Directorio: ${TARGET_DIR}${NC}"
echo -e "${CYAN}║  Modo: $(if $DRY_RUN; then echo 'DRY RUN (sin cambios)'; else echo 'APLICAR CAMBIOS'; fi)${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# ─── Contadores ───────────────────────────────────────────────────────────────
TOTAL_FILES=0
TOTAL_CHANGES=0

# ─── Función de reemplazo ────────────────────────────────────────────────────
do_replace() {
    local pattern="$1"
    local replacement="$2"
    local description="$3"
    local count

    count=$(grep -rlE "$pattern" "$TARGET_DIR" \
        --include='*.cpp' --include='*.h' --include='*.cxx' 2>/dev/null | wc -l || true)

    if [[ "$count" -gt 0 ]]; then
        echo -e "  ${GREEN}✓${NC} ${description}: ${YELLOW}${count}${NC} archivos"
        TOTAL_CHANGES=$((TOTAL_CHANGES + count))

        if ! $DRY_RUN; then
            find "$TARGET_DIR" \( -name '*.cpp' -o -name '*.h' -o -name '*.cxx' \) \
                -exec sed -i -E "s/${pattern}/${replacement}/g" {} +
        fi
    else
        echo -e "  ${CYAN}–${NC} ${description}: 0 archivos (ya migrado)"
    fi
}


# ─── Fase 1: Tipos de string ─────────────────────────────────────────────────
echo -e "${YELLOW}═══ Fase 1: Tipos de string ═══${NC}"

do_replace \
    '\bstd::wstring\b' \
    'NppString' \
    'std::wstring → NppString'

do_replace \
    '\bgeneric_string\b' \
    'NppString' \
    'generic_string → NppString'

do_replace \
    '\bwchar_t\b' \
    'NppChar' \
    'wchar_t → NppChar'

do_replace \
    '\bTCHAR\b' \
    'NppChar' \
    'TCHAR → NppChar'

do_replace \
    '\bLPCTSTR\b' \
    'const NppChar*' \
    'LPCTSTR → const NppChar*'

do_replace \
    '\bLPTSTR\b' \
    'NppChar*' \
    'LPTSTR → NppChar*'

echo ""

# ─── Fase 2: Literales de string ─────────────────────────────────────────────
echo -e "${YELLOW}═══ Fase 2: Literales de string ═══${NC}"

# L"..." → "..." (quitar prefijo L de literales wide string)
do_replace \
    'L"([^"]*)"' \
    '"\1"' \
    'L"..." → "..." (eliminar prefijo L)'

# TEXT("...") → NppT("...")
do_replace \
    '\bTEXT\(' \
    'NppT(' \
    'TEXT() → NppT()'

# _T("...") → NppT("...")
do_replace \
    '\b_T\(' \
    'NppT(' \
    '_T() → NppT()'

echo ""

# ─── Fase 3: Funciones de string Win32 ───────────────────────────────────────
echo -e "${YELLOW}═══ Fase 3: Funciones de string Win32 ═══${NC}"

do_replace \
    '\blstrlen\b' \
    'strlen' \
    'lstrlen → strlen'

do_replace \
    '\b_tcscmp\b' \
    'strcmp' \
    '_tcscmp → strcmp'

do_replace \
    '\b_tcsicmp\b' \
    'strcasecmp' \
    '_tcsicmp → strcasecmp'

do_replace \
    '\b_tcscpy\b' \
    'strcpy' \
    '_tcscpy → strcpy'

do_replace \
    '\b_tcsncpy\b' \
    'strncpy' \
    '_tcsncpy → strncpy'

do_replace \
    '\b_tcscat\b' \
    'strcat' \
    '_tcscat → strcat'

do_replace \
    '\b_tcschr\b' \
    'strchr' \
    '_tcschr → strchr'

do_replace \
    '\b_tcsrchr\b' \
    'strrchr' \
    '_tcsrchr → strrchr'

do_replace \
    '\b_tcsstr\b' \
    'strstr' \
    '_tcsstr → strstr'

do_replace \
    '\b_stprintf\b' \
    'snprintf' \
    '_stprintf → snprintf'

do_replace \
    '\b_sntprintf\b' \
    'snprintf' \
    '_sntprintf → snprintf'

do_replace \
    '\bwsprintf\b' \
    'sprintf' \
    'wsprintf → sprintf'

do_replace \
    '\b_ttoi\b' \
    'atoi' \
    '_ttoi → atoi'

do_replace \
    '\b_ttol\b' \
    'atol' \
    '_ttol → atol'

echo ""

# ─── Fase 4: Conversiones de codificación ────────────────────────────────────
echo -e "${YELLOW}═══ Fase 4: Conversiones de codificación ═══${NC}"

do_replace \
    '\bMultiByteToWideChar\b' \
    'nppMBtoWC' \
    'MultiByteToWideChar → nppMBtoWC (stub portable)'

do_replace \
    '\bWideCharToMultiByte\b' \
    'nppWCtoMB' \
    'WideCharToMultiByte → nppWCtoMB (stub portable)'

echo ""

# ─── Resumen ─────────────────────────────────────────────────────────────────
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"
echo -e "  Total de archivos afectados: ${YELLOW}${TOTAL_CHANGES}${NC}"
if $DRY_RUN; then
    echo -e "  ${RED}Modo DRY RUN — no se aplicaron cambios${NC}"
    echo -e "  Ejecuta sin --dry-run para aplicar."
else
    echo -e "  ${GREEN}✅ Migración completada exitosamente${NC}"
fi
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"

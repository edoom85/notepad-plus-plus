#!/usr/bin/env bash
# scripts/run_tests.sh — Suite de pruebas y validación para Notepad++ Linux Nativo (Qt6)
# Fase 10 del plan de migración
#
# Copyright (C) Notepad++ contributors. GPL v3+

set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  Notepad++ Linux Port — Suite de Pruebas de Integración    ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""

PASSED=0
FAILED=0

run_test() {
    local name="$1"
    local cmd="$2"

    echo -n "Testing $name... "
    if eval "$cmd" >/dev/null 2>&1; then
        echo -e "${GREEN}[PASSED]${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}[FAILED]${NC}"
        FAILED=$((FAILED + 1))
    fi
}

# 1. Verificar binarios generados
run_test "Ejecutable notepadplusplus existe" "[ -f build/notepadplusplus ]"
run_test "Ejecutable es de 64 bits ELF" "file build/notepadplusplus | grep -q 'ELF 64-bit'"
run_test "Enlace dinámico a libQt6Widgets" "ldd build/notepadplusplus | grep -q 'libQt6Widgets'"
run_test "Enlace dinámico a libqscintilla2_qt6" "ldd build/notepadplusplus | grep -q 'libqscintilla2_qt6'"

# 2. Verificar opciones CLI
run_test "Opción --version" "./build/notepadplusplus --version | grep -q 'Notepad++'"
run_test "Opción --help" "./build/notepadplusplus --help | grep -q 'Uso:'"

# 3. Verificar script de versión
run_test "Generador de versión NppLibsVersion.h" "bash scripts/generate_version_header.sh"
run_test "Header NppLibsVersion.h contiene commit" "grep -q 'NPP_GIT_COMMIT' PowerEditor/src/NppLibsVersion.h"

echo ""
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"
echo -e "  Pruebas superadas: ${GREEN}${PASSED}${NC}"
echo -e "  Pruebas fallidas:  ${RED}${FAILED}${NC}"
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"

if [ "$FAILED" -eq 0 ]; then
    echo -e "${GREEN}🎉 TODAS LAS PRUEBAS SE EJECUTARON EXITOSAMENTE${NC}"
    exit 0
else
    echo -e "${RED}❌ ALGUNAS PRUEBAS FALLARON${NC}"
    exit 1
fi

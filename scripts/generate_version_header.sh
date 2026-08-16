#!/usr/bin/env bash
# scripts/generate_version_header.sh — Generador del header de versión de librerías
# Reemplaza NppLibsVersionH-generator.bat en Linux
#
# Genera PowerEditor/src/NppLibsVersion.h con información de Git y versión del sistema
#
# Copyright (C) Notepad++ contributors. GPL v3+

set -euo pipefail

OUTPUT_FILE="PowerEditor/src/NppLibsVersion.h"
GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "linux-native-port")
BUILD_DATE=$(date -u +"%Y-%m-%d %H:%M:%S UTC")

cat <<EOF > "$OUTPUT_FILE"
// NppLibsVersion.h — Autogenerado por generate_version_header.sh
// NO EDITAR DIRECTAMENTE

#pragma once

#define NPP_VERSION_STRING "9.0-linux"
#define NPP_GIT_COMMIT "$GIT_COMMIT"
#define NPP_GIT_BRANCH "$GIT_BRANCH"
#define NPP_BUILD_DATE "$BUILD_DATE"
EOF

echo "✅ $OUTPUT_FILE generado exitosamente (Commit: $GIT_COMMIT, Branch: $GIT_BRANCH)"

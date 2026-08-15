#!/usr/bin/env bash
# scripts/install_deps.sh — Instala dependencias para el port Linux de Notepad++
# Detecta el gestor de paquetes y procede automáticamente.
# Copyright (C) Notepad++ contributors. GPL v3+

set -euo pipefail

echo "=== Notepad++ Linux Port — Instalación de dependencias ==="
echo ""

# ─── Detectar distribución ────────────────────────────────────────────────────
detect_distro() {
    if command -v pacman &>/dev/null; then echo "arch"
    elif command -v apt-get &>/dev/null; then echo "debian"
    elif command -v dnf &>/dev/null; then echo "fedora"
    elif command -v zypper &>/dev/null; then echo "suse"
    else echo "unknown"
    fi
}

DISTRO=$(detect_distro)
echo "Distribución detectada: $DISTRO"
echo ""

# ─── Listas de paquetes por distro ───────────────────────────────────────────

install_arch() {
    echo "[Arch/Manjaro] Instalando con pacman..."
    sudo pacman -S --needed --noconfirm \
        base-devel \
        cmake \
        ninja \
        gtk4 \
        glib2 \
        pango \
        cairo \
        gdk-pixbuf2 \
        libxml2 \
        pcre2 \
        pkg-config \
        git
}

install_debian() {
    echo "[Debian/Ubuntu/Mint] Instalando con apt-get..."
    sudo apt-get update -q
    sudo apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        libgtk-4-dev \
        libglib2.0-dev \
        libpango1.0-dev \
        libcairo2-dev \
        libgdk-pixbuf-2.0-dev \
        libxml2-dev \
        libpcre2-dev \
        pkg-config \
        git
}

install_fedora() {
    echo "[Fedora/RHEL] Instalando con dnf..."
    sudo dnf install -y \
        gcc \
        gcc-c++ \
        cmake \
        ninja-build \
        gtk4-devel \
        glib2-devel \
        pango-devel \
        cairo-devel \
        gdk-pixbuf2-devel \
        libxml2-devel \
        pcre2-devel \
        pkgconfig \
        git
}

install_suse() {
    echo "[openSUSE] Instalando con zypper..."
    sudo zypper install -y \
        gcc \
        gcc-c++ \
        cmake \
        ninja \
        gtk4-devel \
        glib2-devel \
        pango-devel \
        cairo-devel \
        gdk-pixbuf-devel \
        libxml2-devel \
        pcre2-devel \
        pkg-config \
        git
}

# ─── Instalar según distro ────────────────────────────────────────────────────
case "$DISTRO" in
    arch)    install_arch ;;
    debian)  install_debian ;;
    fedora)  install_fedora ;;
    suse)    install_suse ;;
    unknown)
        echo "⚠️  Distribución no reconocida. Instalar manualmente:"
        echo "   - gcc/g++ >= 11   - cmake >= 3.15   - ninja"
        echo "   - gtk4-dev        - glib2-dev        - pkg-config"
        exit 1
        ;;
esac

# ─── Verificar instalación ────────────────────────────────────────────────────
echo ""
echo "=== Verificación ==="

check_cmd() {
    if command -v "$1" &>/dev/null; then
        echo "  ✅ $1: $(${1} --version 2>&1 | head -1)"
    else
        echo "  ❌ $1: NO ENCONTRADO"
    fi
}

check_pkg() {
    if pkg-config --exists "$1" 2>/dev/null; then
        echo "  ✅ $1: $(pkg-config --modversion "$1")"
    else
        echo "  ❌ $1: NO ENCONTRADO"
    fi
}

check_cmd gcc
check_cmd g++
check_cmd cmake
check_cmd ninja
check_cmd pkg-config
check_cmd git
check_pkg gtk4
check_pkg glib-2.0
check_pkg cairo
check_pkg pango

echo ""
echo "✅ Dependencias listas. Puedes continuar con el build."

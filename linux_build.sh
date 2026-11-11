#!/usr/bin/env bash
# ============================================================================
# MapGyver Linux x64 build script
#
# Builds MapGyver natively on Linux. The generated Makefile already links
# against libmpv, the Ultralight libs bundled in External/ultralight/lib/linux
# and the system OpenSSL, and bakes in an $ORIGIN rpath, so this script only
# needs to:
#   1. init git submodules (juce_organicui, juce_sharedtexture, juce_simpleweb,
#      juce_timeline)
#   2. clone benkuper's JUCE fork (branch juce8_local) into ./JUCE
#   3. run the Release build
#   4. stage the non-system runtime libs (Servus, Ultralight) into build/lib
#      and the Ultralight resources into build/resources
#
# System requirements (Arch names; Debian/Ubuntu: see install_linux_deps.sh):
#   base-devel git freetype2 libx11 libxinerama libxrandr libxcursor
#   libxcomposite mesa alsa-lib gtk3 webkit2gtk-4.1 curl openssl
#   mpv (libmpv) ndi-sdk (AUR)
#
# Optional, for network discovery (silences the "avahi client" warnings):
#   sudo systemctl enable --now avahi-daemon
# ============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JOBS="$(nproc)"
BUILD="$ROOT/Builds/LinuxMakefile/build"

echo "==> [1/4] Initializing git submodules"
git -C "$ROOT" submodule update --init --recursive

echo "==> [2/4] Checking out benkuper JUCE fork (juce8_local)"
if [ ! -d "$ROOT/JUCE/modules" ]; then
    git clone --depth 1 --branch juce8_local https://github.com/benkuper/JUCE "$ROOT/JUCE"
fi

echo "==> [3/4] Building (Release, ${JOBS} jobs)"
make -C "$ROOT/Builds/LinuxMakefile" -j"$JOBS" CONFIG=Release

echo "==> [4/4] Staging runtime libs and Ultralight resources"
mkdir -p "$BUILD/lib"
cp "$ROOT/External/servus/lib/linux/libServus.so.6" "$BUILD/lib/"
cp "$ROOT/External/ultralight/lib/linux/"*.so "$BUILD/lib/"
mkdir -p "$BUILD/resources"
cp -r "$ROOT/Resources/ultralight/resources/." "$BUILD/resources/"

echo
echo "Build complete: $BUILD/MapGyver"
echo "Run it with: $BUILD/MapGyver"

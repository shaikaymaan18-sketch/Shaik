#!/usr/bin/sh
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

ARCH_RAW="$PROCESSOR_ARCHITECTURE"

case "$ARCH_RAW" in
    AMD64) ARCH="x64" ;;
    ARM64) ARCH="arm64" ;;
    *) echo "load-msvc-env.sh: Unsupported architecture: $ARCH_RAW"; exit 1 ;;
esac

VS_BASE=""
for p in \
    "/c/Program Files/Microsoft Visual Studio/18/Community" \
	"/c/Program Files/Microsoft Visual Studio/17/Community" \
    "/c/Program Files/Microsoft Visual Studio/2022/Community"
do
    echo "load-msvc-env.sh: $p"
    if [ -d "$p/VC/Auxiliary/Build" ]; then
        VS_BASE="$p"
        break
    fi
done

if [ -z "$VS_BASE" ]; then
    echo "load-msvc-env.sh: Could not locate Visual Studio installation."
    exit 1
fi

MSVC_ROOT="$VS_BASE/VC/Tools/MSVC"
if [ ! -d "$MSVC_ROOT" ]; then
    echo "load-msvc-env.sh: Could not locate MSVC tools: $MSVC_ROOT"
    exit 1
fi

MSVC_VER=$(ls "$MSVC_ROOT" | sort -V | tail -n1)
BIN_PATH="$MSVC_ROOT/$MSVC_VER/bin/Host$ARCH/$ARCH"

export PATH="$BIN_PATH:$PATH"

echo "MSVC environment loaded:"
echo "  VS path:   $VS_BASE"
echo "  MSVC ver:  $MSVC_VER"
echo "  Arch:      $ARCH"
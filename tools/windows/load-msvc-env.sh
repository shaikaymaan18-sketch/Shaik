#!/usr/bin/sh
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

VCVARS_BASH_URL="https://github.com/Vee99BR/vcvars-bash/raw/refs/heads/main/vcvarsall.sh"
ARCH_RAW="$PROCESSOR_ARCHITECTURE"

case "$ARCH_RAW" in
    AMD64) ARCH="x64" ;;
    ARM64) ARCH="arm64" ;;
    *) echo "load-msvc-env.sh: Unsupported architecture: $ARCH_RAW"; exit 1 ;;
esac

TMP_DIR="$(mktemp -d)"
VCVARS_BASH="$TMP_DIR/vcvarsall.sh"

curl -sL "$VCVARS_BASH_URL" -o "$VCVARS_BASH"
chmod +x "$VCVARS_BASH"

eval "$("$VCVARS_BASH" "$ARCH")"

echo "MSVC environment loaded for $ARCH via vcvars-bash"
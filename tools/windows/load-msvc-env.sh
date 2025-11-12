#!/usr/bin/sh
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# shellcheck disable=SC2154
CLPATH=$(cygpath -u "$VCToolsInstallDir\\bin\\Host${VSCMD_ARG_HOST_ARCH}\\${VSCMD_ARG_TGT_ARCH}")
export PATH="$CLPATH:$PATH"

#!/bin/sh -e

# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# Update local Workflow to most recent

REPO="https://github.com/Eden-CI/Workflow.git"
DEST="$PWD"
TMPCLONE=$(mktemp -d)

git clone --depth 1 "$REPO" "$TMPCLONE"

cp "$DEST/.ci/license-header.sh" "$TMPCLONE/.ci/"
cp "$DEST/.github/workflows/license-header.yml" "$TMPCLONE/.github/workflows/"

find "$TMPCLONE" -mindepth 1 -maxdepth 1 ! -name ".ci" ! -name ".github" -exec rm -rf {} +

rm -rf "$DEST/.ci" "$DEST/.github"

cp -r "$TMPCLONE/.ci" "$TMPCLONE/.github" "$DEST/"

rm -rf "$TMPCLONE"


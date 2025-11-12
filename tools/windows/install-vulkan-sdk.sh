#!/usr/bin/sh
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

: "${VULKAN_SDK_VER:=1.4.328.1}"
: "${VULKAN_SDK:=C:/VulkanSDK/$VULKAN_SDK_VER}"
EXE_FILE="vulkansdk-windows-X64-$VULKAN_SDK_VER.exe"
URI="https://sdk.lunarg.com/sdk/download/$VULKAN_SDK_VER/windows/$EXE_FILE"
DESTINATION="./$EXE_FILE"

if command -v cygpath >/dev/null 2>&1; then
    VULKAN_ROOT_UNIX=$(cygpath -u "$VULKAN_SDK")
else
    VULKAN_ROOT_UNIX="$VULKAN_SDK"
fi

# Check if Vulkan SDK is already installed
if [ -d "$VULKAN_ROOT_UNIX" ]; then
    echo "-- Vulkan SDK already installed at $VULKAN_ROOT_UNIX"
    exit 0
fi

echo "Downloading Vulkan SDK $VULKAN_SDK_VER from $URI"
curl -L -o "$DESTINATION" "$URI"
chmod +x "$DESTINATION"
echo "Finished downloading $EXE_FILE"

echo "Installing Vulkan SDK $VULKAN_SDK_VER..."
"$DESTINATION" --root "$VULKAN_ROOT_UNIX" --accept-licenses --default-answer --confirm-command install

echo "Finished installing Vulkan SDK $VULKAN_SDK_VER"

# GitHub Actions integration
if [ "${GITHUB_ACTIONS:-false}" = "true" ]; then
    echo "VULKAN_SDK=$VULKAN_SDK" >> "$GITHUB_ENV"
    echo "$VULKAN_SDK/bin" >> "$GITHUB_PATH"
fi


// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#version 450

layout(binding = 0) uniform sampler2D color_texture;

void main() {
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 color = texelFetch(color_texture, coord, 0);
    bool invalid = any(isnan(color)) || any(isinf(color));
    float depth = color.r;
    if (invalid) {
        depth = 1.0;
    }
    gl_FragDepth = clamp(depth, 0.0, 1.0);
}

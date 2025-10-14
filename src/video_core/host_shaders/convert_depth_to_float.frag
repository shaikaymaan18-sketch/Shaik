// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#version 450

layout(binding = 0) uniform sampler2D depth_texture;
layout(location = 0) out vec4 output_color;

void main() {
    ivec2 coord = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(depth_texture, coord, 0).r;
    bool invalid = isnan(depth) || isinf(depth);
    float sanitized_depth = clamp(depth, 0.0, 1.0);
    if (invalid) {
        sanitized_depth = 0.0;
    }
    output_color = vec4(sanitized_depth, sanitized_depth, sanitized_depth, 1.0);
}

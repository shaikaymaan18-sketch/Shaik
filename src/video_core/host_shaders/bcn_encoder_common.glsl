// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#extension GL_ARB_shading_language_420pack : require
#extension GL_KHR_shader_subgroup_arithmetic : require
#extension GL_KHR_shader_subgroup_clustered : require

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, rgba8) uniform readonly image2DArray src_image;

layout(binding = 1, std430) writeonly buffer OutputBuffer {
    uint out_blocks[];
};

layout(push_constant) uniform PushConstants {
    uvec2 blocks_dim;
} pc;

const float ALPHA_THRESHOLD = 128.0 / 255.0;

uint PackRGB565(vec3 c) {
    uvec3 q = uvec3(round(clamp(c, 0.0, 1.0) * vec3(31.0, 63.0, 31.0)));
    return (q.r << 11) | (q.g << 5) | q.b;
}

vec3 UnpackRGB565(uint v) {
    float r = float((v >> 11) & 0x1Fu) / 31.0;
    float g = float((v >> 5)  & 0x3Fu) / 63.0;
    float b = float(v & 0x1Fu) / 31.0;
    return vec3(r, g, b);
}

float DistSq(vec3 a, vec3 b) {
    vec3 d = a - b;
    return dot(d, d);
}

void main() {
    const uint local_idx = gl_LocalInvocationIndex;
    const uint subgroup_block_idx = local_idx / 16u;
    const uint texel_idx = local_idx % 16u;

    const uvec2 wg_block_base = gl_WorkGroupID.xy * uvec2(2u, 2u);
    const uvec2 block_coord = wg_block_base + uvec2(subgroup_block_idx % 2u, subgroup_block_idx / 2u);

    if (any(greaterThanEqual(block_coord.xy, pc.blocks_dim))) {
        return;
    }

    const uvec2 texel_coord = (block_coord.xy * 4u) + uvec2(texel_idx % 4u, texel_idx / 4u);
    const vec4 texel = imageLoad(src_image, ivec3(texel_coord, int(gl_GlobalInvocationID.z)));

    const bool opaque = IS_BC3 || (texel.a >= ALPHA_THRESHOLD);
    const uint active_count = opaque ? 1u : 0u;
    const uint opaque_count = subgroupClusteredAdd(active_count, 16u);
    const bool any_transparent = (!IS_BC3) && (opaque_count < 16u);

    if (!IS_BC3 && opaque_count == 0u) {
        if (texel_idx == 0u) {
                               uint out_index = (gl_GlobalInvocationID.z * pc.blocks_dim.y * pc.blocks_dim.x) + (block_coord.y * pc.blocks_dim.x) + block_coord.x;
                               out_blocks[out_index * 2u + 0u] = 0u;
                               out_blocks[out_index * 2u + 1u] = 0xFFFFFFFFu;
        }
        return;
    }

    const vec3 active_texel = opaque ? texel.rgb : vec3(0.0);
    const vec3 block_sum = subgroupClusteredAdd(active_texel, 16u);
    const vec3 mean = IS_BC3 ? (block_sum / 16.0) : (block_sum / max(float(opaque_count), 1.0));

    float c00 = 0.0, c01 = 0.0, c02 = 0.0;
    float c11 = 0.0, c12 = 0.0, c22 = 0.0;

    if (opaque) {
        vec3 d = texel.rgb - mean;
        c00 = d.x * d.x; c01 = d.x * d.y; c02 = d.x * d.z;
        c11 = d.y * d.y; c12 = d.y * d.z; c22 = d.z * d.z;
    }

    c00 = subgroupClusteredAdd(c00, 16u);
    c01 = subgroupClusteredAdd(c01, 16u);
    c02 = subgroupClusteredAdd(c02, 16u);
    c11 = subgroupClusteredAdd(c11, 16u);
    c12 = subgroupClusteredAdd(c12, 16u);
    c22 = subgroupClusteredAdd(c22, 16u);

    vec3 axis = vec3(1.0, 1.0, 1.0);
    for (int iter = 0; iter < 4; ++iter) {
        vec3 next_axis;
        next_axis.x = (c00 * axis.x) + (c01 * axis.y) + (c02 * axis.z);
        next_axis.y = (c01 * axis.x) + (c11 * axis.y) + (c12 * axis.z);
        next_axis.z = (c02 * axis.x) + (c12 * axis.y) + (c22 * axis.z);

        axis = next_axis;
        float len_sq = dot(axis, axis);
        axis = len_sq > 1e-12 ? axis * inversesqrt(len_sq) : vec3(1.0, 0.0, 0.0);
    }

    const float proj = opaque ? dot(texel.rgb - mean, axis) : 0.0;
    const float safe_proj_min = opaque ? proj : 1e9;
    const float safe_proj_max = opaque ? proj : -1e9;

    const float min_proj = subgroupClusteredMin(safe_proj_min, 16u);
    const float max_proj = subgroupClusteredMax(safe_proj_max, 16u);

    vec3 ep_max = mean + axis * max_proj;
    vec3 ep_min = mean + axis * min_proj;

    uint p0 = PackRGB565(ep_max);
    uint p1 = PackRGB565(ep_min);

    if (IS_BC3 || !any_transparent) {
        if (p0 < p1) {
            uint tmp = p0; p0 = p1; p1 = tmp;
            vec3 tmpv = ep_max; ep_max = ep_min; ep_min = tmpv;
        } else if (p0 == p1) {
            p1 = p0 > 0u ? p0 - 1u : p0;
        }
    } else if (p0 > p1) {
        uint tmp = p0; p0 = p1; p1 = tmp;
        vec3 tmpv = ep_max; ep_max = ep_min; ep_min = tmpv;
    }

    const vec3 c0 = UnpackRGB565(p0);
    const vec3 c1 = UnpackRGB565(p1);

    vec3 c2, c3;
    if (IS_BC3) {
        c2 = (2.0 * c0 + c1) / 3.0;
        c3 = (c0 + 2.0 * c1) / 3.0;
    } else {
        c2 = any_transparent ? (c0 + c1) * 0.5 : (2.0 * c0 + c1) / 3.0;
        c3 = (c0 + 2.0 * c1) / 3.0;
    }

    uint best = 0u;
    if (!IS_BC3 && any_transparent && !opaque) {
        best = 3u;
    } else {
        float best_d = DistSq(texel.rgb, c0);
        float d1 = DistSq(texel.rgb, c1);
        if (d1 < best_d) { best = 1u; best_d = d1; }

        float d2 = DistSq(texel.rgb, c2);
        if (d2 < best_d) { best = 2u; best_d = d2; }

        if (IS_BC3 || !any_transparent) {
            float d3 = DistSq(texel.rgb, c3);
            if (d3 < best_d) { best = 3u; best_d = d3; }
        }
    }

    const uint color_indices = subgroupClusteredOr(best << (texel_idx * 2u), 16u);

    uint alpha_low = 0u;
    uint alpha_high = 0u;

    if (IS_BC3) {
        const float a_min = subgroupClusteredMin(texel.a, 16u);
        const float a_max = subgroupClusteredMax(texel.a, 16u);

        const uint a0 = uint(round(a_max * 255.0));
        const uint a1 = uint(round(a_min * 255.0));
        const float fa0 = float(a0) / 255.0;
        const float fa1 = float(a1) / 255.0;

        float levels[8];
        levels[0] = fa0;
        levels[1] = fa1;
        for (int k = 1; k <= 6; ++k) {
            levels[1 + k] = ((7.0 - float(k)) * fa0 + float(k) * fa1) / 7.0;
        }

        float best_d_alpha = abs(texel.a - levels[0]);
        uint best_a = 0u;
        for (uint k = 1u; k < 8u; ++k) {
            float d = abs(texel.a - levels[k]);
            if (d < best_d_alpha) { best_d_alpha = d; best_a = k; }
        }

        const uint shifted_a = best_a << ((texel_idx % 8u) * 3u);
        const uint g0_contrib = (texel_idx < 8u) ? shifted_a : 0u;
        const uint g1_contrib = (texel_idx >= 8u) ? shifted_a : 0u;

        const uint idx_group0 = subgroupClusteredOr(g0_contrib, 16u);
        const uint idx_group1 = subgroupClusteredOr(g1_contrib, 16u);

        alpha_low  = a0 | (a1 << 8u) | ((idx_group0 & 0xFFu) << 16u) | (((idx_group0 >> 8u) & 0xFFu) << 24u);
        alpha_high = ((idx_group0 >> 16u) & 0xFFu) | ((idx_group1 & 0xFFu) << 8u) |
        (((idx_group1 >> 8u) & 0xFFu) << 16u) | (((idx_group1 >> 16u) & 0xFFu) << 24u);
    }

    if (texel_idx == 0u) {
        uint out_index = (gl_GlobalInvocationID.z * pc.blocks_dim.y * pc.blocks_dim.x) + (block_coord.y * pc.blocks_dim.x) + block_coord.x;

        if (IS_BC3) {
            out_blocks[out_index * 4u] = alpha_low;
            out_blocks[out_index * 4u + 1u] = alpha_high;
            out_blocks[out_index * 4u + 2u] = p0 | (p1 << 16u);
            out_blocks[out_index * 4u + 3u] = color_indices;
        } else {
            out_blocks[out_index * 2u] = p0 | (p1 << 16u);
            out_blocks[out_index * 2u + 1u] = color_indices;
        }
    }
}
// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <unordered_map>

#include <boost/container/static_vector.hpp>

#include "video_core/renderer_vulkan/maxwell_to_vk.h"
#include "video_core/renderer_vulkan/vk_render_pass_cache.h"
#include "video_core/surface.h"
#include "video_core/vulkan_common/vulkan_device.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"


namespace Vulkan {
namespace {
using VideoCore::Surface::PixelFormat;
using VideoCore::Surface::SurfaceType;

        constexpr SurfaceType GetSurfaceType(PixelFormat format) {
            switch (format) {
                // Depth formats
                case PixelFormat::D16_UNORM:
                case PixelFormat::D32_FLOAT:
                case PixelFormat::X8_D24_UNORM:
                    return SurfaceType::Depth;

                    // Stencil formats
                case PixelFormat::S8_UINT:
                    return SurfaceType::Stencil;

                    // Depth+Stencil formats
                case PixelFormat::D24_UNORM_S8_UINT:
                case PixelFormat::S8_UINT_D24_UNORM:
                case PixelFormat::D32_FLOAT_S8_UINT:
                    return SurfaceType::DepthStencil;

                    // Everything else is a color texture
                default:
                    return SurfaceType::ColorTexture;
            }
        }

        VkImageLayout AttachmentLayout(const Device& device, SurfaceType surface_type, bool want_feedback_loop) {
    if (want_feedback_loop && device.SupportsAttachmentFeedbackLoopLayout()) {
        // Single layout works for color and depth/stencil images.
        return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    }

    // Normal (non-feedback) attachment layouts
    switch (surface_type) {
    case SurfaceType::ColorTexture:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case SurfaceType::Depth:
        return device.SupportsSeparateDepthStencilLayouts()
            ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case SurfaceType::Stencil:
        return device.SupportsSeparateDepthStencilLayouts()
            ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case SurfaceType::DepthStencil:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    default:
        return VK_IMAGE_LAYOUT_GENERAL; // last-resort fallback
        }
    }   

        VkAttachmentDescription AttachmentDescription(const Device& device, PixelFormat format,
                                                      VkSampleCountFlagBits samples,
                                                      VkAttachmentLoadOp load_op,
                                                      VkAttachmentStoreOp store_op,
                                                      VkAttachmentLoadOp stencil_load_op,
                                                      VkAttachmentStoreOp stencil_store_op,
                                                      bool want_feedback_loop) {
            using MaxwellToVK::SurfaceFormat;

            const SurfaceType surface_type = GetSurfaceType(format);
            const bool has_stencil = surface_type == SurfaceType::DepthStencil ||
                                     surface_type == SurfaceType::Stencil;
            const VkImageLayout layout = AttachmentLayout(device, surface_type, want_feedback_loop);
            const VkAttachmentLoadOp resolved_stencil_load =
                has_stencil ? stencil_load_op : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            const VkAttachmentStoreOp resolved_stencil_store =
                has_stencil ? stencil_store_op : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            return {
                .flags = {},
                .format = SurfaceFormat(device, FormatType::Optimal, true, format).format,
                .samples = samples,
                .loadOp = load_op,
                .storeOp = store_op,
                .stencilLoadOp = resolved_stencil_load,
                .stencilStoreOp = resolved_stencil_store,
                .initialLayout = layout,
                .finalLayout = layout,
            };
        }
    } // Anonymous namespace

RenderPassCache::RenderPassCache(const Device& device_) : device{&device_} {}

VkRenderPass RenderPassCache::Get(const RenderPassKey& key) {
    std::scoped_lock lock{mutex};
    const auto [pair, is_new] = cache.try_emplace(key);
    if (!is_new) {
        return *pair->second;
    }
    boost::container::static_vector<VkAttachmentDescription, 9> descriptions;
    std::array<VkAttachmentReference, 8> references{};
    u32 num_attachments{};
    u32 num_colors{};
    const bool supports_feedback_loop = device->SupportsAttachmentFeedbackLoopLayout();
    for (size_t index = 0; index < key.color_formats.size(); ++index) {
        const PixelFormat format{key.color_formats[index]};
        if (format == PixelFormat::Invalid) {
            references[index] = VkAttachmentReference{
                .attachment = VK_ATTACHMENT_UNUSED,
                .layout = VK_IMAGE_LAYOUT_GENERAL,
            };
            continue;
        }

        const SurfaceType surface_type = GetSurfaceType(format);
        const VkImageLayout layout = AttachmentLayout(*device, surface_type, supports_feedback_loop);
        references[index] = VkAttachmentReference{
            .attachment = num_colors,
            .layout = layout,
        };
        const VkAttachmentLoadOp load_op = key.color_load_ops[index];
        const VkAttachmentStoreOp store_op = key.color_store_ops[index];
        descriptions.push_back(AttachmentDescription(*device, format, key.samples, load_op, store_op,
                                                     VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                     VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                     supports_feedback_loop));
        num_attachments = static_cast<u32>(index + 1);
        ++num_colors;
    }

    const bool has_depth{key.depth_format != PixelFormat::Invalid};
    VkAttachmentReference depth_reference{};
    if (has_depth) {
        const SurfaceType depth_type = GetSurfaceType(key.depth_format);
        const VkImageLayout depth_layout = AttachmentLayout(*device, depth_type, supports_feedback_loop);
        depth_reference = VkAttachmentReference{
            .attachment = num_colors,
            .layout = depth_layout,
        };
        descriptions.push_back(AttachmentDescription(*device, key.depth_format, key.samples,
                                                     key.depth_load_op, key.depth_store_op,
                                                     key.stencil_load_op, key.stencil_store_op,
                                                     supports_feedback_loop));
    }
    const VkSubpassDescription subpass{
        .flags = 0u,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = num_attachments,
        .pColorAttachments = references.data(),
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = has_depth ? &depth_reference : nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkDependencyFlags dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;
    if (supports_feedback_loop) {
        dependency_flags |= VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT;
    }
    const VkSubpassDependency dependency{
            .srcSubpass = 0,  // Current subpass
            .dstSubpass = 0,  // Same subpass (self-dependency)
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            .dependencyFlags = dependency_flags
    };
    const VkSubpassDependency* dependency_ptr = supports_feedback_loop ? &dependency : nullptr;
    const u32 dependency_count = supports_feedback_loop ? 1u : 0u;
    pair->second = device->GetLogical().CreateRenderPass({
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = static_cast<u32>(descriptions.size()),
        .pAttachments = descriptions.empty() ? nullptr : descriptions.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = dependency_count,
        .pDependencies = dependency_ptr,
    });
    return *pair->second;
}

} // namespace Vulkan


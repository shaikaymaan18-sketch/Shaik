// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>

#include "common/settings.h"
#include "video_core/renderer_vulkan/present/frame_gen.h"
#include "video_core/renderer_vulkan/vk_present_manager.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

namespace {

constexpr u64 LSFG_REQUIRED_FRAMES = 2;
constexpr u32 LSFG_RECURRENCE_FRAMES = 2;

[[nodiscard]] f32 ManualFlowScale(const VideoCore::FrameGenConfig& config) {
    return static_cast<f32>(config.flow_scale) / 100.0f;
}

[[nodiscard]] f32 ConfiguredFlowScale(const VideoCore::FrameGenConfig& config,
                                      VkExtent2D guest_extent, VkExtent2D presented_extent) {
    if (!config.flow_scale_auto) {
        return ManualFlowScale(config);
    }
    if (guest_extent.width == 0 || presented_extent.width == 0) {
        return 1.0f;
    }

    const f32 rendered_width = static_cast<f32>(guest_extent.width) *
                               Settings::values.resolution_info.up_factor;
    const f32 ratio = rendered_width / static_cast<f32>(presented_extent.width);

    constexpr f32 FLOW_SCALE_STEPS = 20.0f;
    const f32 stepped = std::ceil(ratio * FLOW_SCALE_STEPS) / FLOW_SCALE_STEPS;
    return std::clamp(stepped, 0.25f, 1.0f);
}

VkImageMemoryBarrier MakeTransitionBarrier(VkImage image, VkAccessFlags src_access,
                                           VkAccessFlags dst_access, VkImageLayout old_layout,
                                           VkImageLayout new_layout) {
    return VkImageMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
}

VkImageCopy MakeCopyRegion(VkExtent2D extent) {
    return VkImageCopy{
        .srcSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .srcOffset = {},
        .dstSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .dstOffset = {},
        .extent = {.width = extent.width, .height = extent.height, .depth = 1},
    };
}

void CopyPresentedFrame(vk::CommandBuffer cmdbuf, VkImage source, LsfgImage& destination,
                        VkExtent2D extent) {
    const auto make_barrier = MakeTransitionBarrier;

    const std::array before{
        make_barrier(source, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                     VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
        make_barrier(destination.Handle(), VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                     destination.Layout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
    };
    cmdbuf.PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, {}, {}, before);

    cmdbuf.CopyImage(source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination.Handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MakeCopyRegion(extent));

    const std::array after{
        make_barrier(source, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL),
        make_barrier(destination.Handle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL),
    };
    cmdbuf.PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           0, {}, {}, after);

    destination.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
}

} // Anonymous namespace

FrameGen::FrameGen(MemoryAllocator& memory_allocator_, Scheduler& scheduler_)
    : memory_allocator{memory_allocator_}, scheduler{scheduler_} {}

FrameGen::~FrameGen() = default;

void FrameGen::UpdateConfig(const VideoCore::FrameGenConfig& new_config) {
    if (!has_config) {
        config = new_config;
        has_config = true;
        return;
    }
    if (config == new_config) {
        return;
    }

    const bool has_toggled_lsfg = config.enabled != new_config.enabled;
    const bool enabling = !config.enabled && new_config.enabled;
    const bool must_reset_pipeline = has_toggled_lsfg;
    const bool must_reload_shaders = enabling;
    const bool must_reset_pacer =
        has_toggled_lsfg || config.multiplier != new_config.multiplier ||
        config.target_rate != new_config.target_rate;

    if (must_reset_pipeline) {
        if (chain) {
            scheduler.Finish();
            chain.reset();
        }
        if (must_reload_shaders) {
            shaders.reset();
        }

        plan = {};
        peak_guest_extent = {};
        built_extent = {};
        built_format = VK_FORMAT_UNDEFINED;
        built_flow_scale = 0.0f;
        frame_count = 0;
        warm_streak = 0;
        generated = false;

        // this will pickup the dll again once toggled
        if (must_reload_shaders) {
            unavailable = false;
        }
    }

    if (must_reset_pacer) {
        pacer.Reset();
    }
    config = new_config;
}

void FrameGen::Process(const Device& device, Frame* frame, VkFormat format,
                       VkExtent2D guest_extent) {
    generated = false;

    if (unavailable || !config.enabled) {
        warm_streak = 0;
        return;
    }

    if (!frame->storage_view) {
        unavailable = true;
        return;
    }

    if (!shaders) {
        shaders.emplace(device);
        if (!shaders->IsValid()) {
            unavailable = true;
            return;
        }
    }

    peak_guest_extent.width = std::max(peak_guest_extent.width, guest_extent.width);
    peak_guest_extent.height = std::max(peak_guest_extent.height, guest_extent.height);

    const VkExtent2D extent{.width = frame->width, .height = frame->height};
    const f32 flow_scale = ConfiguredFlowScale(config, peak_guest_extent, extent);
    if (!chain || built_extent.width != extent.width || built_extent.height != extent.height ||
        built_format != format || built_flow_scale != flow_scale) {
        Rebuild(device, extent, format, flow_scale);
    }

    const u64 count = frame_count++;
    last_count = count;
    last_generations = plan.generations;

    const bool warm = plan.warm && count + 1 >= LSFG_REQUIRED_FRAMES;
    warm_streak = warm ? warm_streak + 1 : 0;
    generated = warm && warm_streak >= LSFG_RECURRENCE_FRAMES && plan.generations > 0;

    scheduler.RequestOutsideRenderPassOperationContext();
    scheduler.Record([this, source = *frame->image, extent, count,
                      dispatch = warm](vk::CommandBuffer cmdbuf) {
        CopyPresentedFrame(cmdbuf, source, chain->Input(count), extent);
        if (dispatch) {
            chain->DispatchShared(cmdbuf, count);
        }
    });
}

size_t FrameGen::WantedGenerations(size_t capacity) {
    if (unavailable) {
        plan = {};
        return 0;
    }
    plan = pacer.Plan(capacity, config);
    return plan.generations;
}

size_t FrameGen::GeneratedFrameCount() const {
    return generated ? last_generations : 0;
}

void FrameGen::GenerateInto(const Device& device, Frame* destination, size_t generation) {
    chain->SetTarget(device, last_generations, generation, destination->index,
                     *destination->storage_view);

    const VkExtent2D extent{.width = destination->width, .height = destination->height};

    scheduler.RequestOutsideRenderPassOperationContext();
    scheduler.Record([this, count = last_count, generation_count = last_generations, generation,
                      target = destination->index, image = *destination->image,
                      extent](vk::CommandBuffer cmdbuf) {
        chain->DispatchGeneration(cmdbuf, count, generation_count, generation, target, image,
                                  extent);
    });
}

void FrameGen::Rebuild(const Device& device, VkExtent2D extent, VkFormat format, f32 flow_scale) {
    scheduler.Finish();
    chain.reset();

    built_flow_scale = flow_scale;

    chain.emplace(device, memory_allocator, *shaders, extent, format, built_flow_scale);
    built_extent = extent;
    built_format = format;
    frame_count = 0;
    warm_streak = 0;
    generated = false;
}

} // namespace Vulkan

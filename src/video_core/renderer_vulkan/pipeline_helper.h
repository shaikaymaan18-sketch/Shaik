// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include <limits>

#include <boost/container/small_vector.hpp>

#include "common/common_types.h"
#include "shader_recompiler/backend/spirv/emit_spirv.h"
#include "shader_recompiler/shader_info.h"
#include "video_core/renderer_vulkan/vk_texture_cache.h"
#include "video_core/renderer_vulkan/vk_update_descriptor.h"
#include "video_core/texture_cache/types.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

using Shader::Backend::SPIRV::NUM_TEXTURE_AND_IMAGE_SCALING_WORDS;

namespace detail {

inline VkImageAspectFlags AttachmentAspectMask(VideoCore::Surface::PixelFormat format) {
    switch (VideoCore::Surface::GetFormatType(format)) {
    case VideoCore::Surface::SurfaceType::ColorTexture:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case VideoCore::Surface::SurfaceType::Depth:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VideoCore::Surface::SurfaceType::Stencil:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case VideoCore::Surface::SurfaceType::DepthStencil:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return 0;
    }
}

inline VkImageSubresourceRange NormalizeImageViewRange(const ImageView& image_view) {
    const VkImageAspectFlags aspect_mask = AttachmentAspectMask(image_view.format);
    VkImageSubresourceRange range{
        .aspectMask = aspect_mask,
        .baseMipLevel = static_cast<u32>(image_view.range.base.level),
        .levelCount = static_cast<u32>(image_view.range.extent.levels),
        .baseArrayLayer = static_cast<u32>(image_view.range.base.layer),
        .layerCount = static_cast<u32>(image_view.range.extent.layers),
    };
    if ((image_view.flags & VideoCommon::ImageViewFlagBits::Slice) != VideoCommon::ImageViewFlagBits{}) {
        range.baseArrayLayer = 0;
        range.layerCount = 1;
    }
    return range;
}

inline bool SubresourceRangeIntersects(const VkImageSubresourceRange& lhs,
                                       const VkImageSubresourceRange& rhs) {
    if ((lhs.aspectMask & rhs.aspectMask) == 0) {
        return false;
    }
    const auto range_end = [](u32 base, u32 count, u32 remaining_value) -> u32 {
        return count == remaining_value ? std::numeric_limits<u32>::max() : base + count;
    };
    const u32 lhs_level_end = range_end(lhs.baseMipLevel, lhs.levelCount, VK_REMAINING_MIP_LEVELS);
    const u32 rhs_level_end = range_end(rhs.baseMipLevel, rhs.levelCount, VK_REMAINING_MIP_LEVELS);
    if (lhs_level_end <= rhs.baseMipLevel || rhs_level_end <= lhs.baseMipLevel) {
        return false;
    }
    const u32 lhs_layer_end = range_end(lhs.baseArrayLayer, lhs.layerCount, VK_REMAINING_ARRAY_LAYERS);
    const u32 rhs_layer_end = range_end(rhs.baseArrayLayer, rhs.layerCount, VK_REMAINING_ARRAY_LAYERS);
    if (lhs_layer_end <= rhs.baseArrayLayer || rhs_layer_end <= lhs.baseArrayLayer) {
        return false;
    }
    return true;
}

} // namespace detail

class DescriptorLayoutBuilder {
public:
    DescriptorLayoutBuilder(const Device& device_) : device{&device_} {}

    bool CanUsePushDescriptor() const noexcept {
        return device->IsKhrPushDescriptorSupported() &&
               num_descriptors <= device->MaxPushDescriptors();
    }

    // TODO(crueter): utilize layout binding flags
    vk::DescriptorSetLayout CreateDescriptorSetLayout(bool use_push_descriptor) const {
        if (bindings.empty()) {
            return nullptr;
        }
        const VkDescriptorSetLayoutCreateFlags flags =
            use_push_descriptor ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
        return device->GetLogical().CreateDescriptorSetLayout({
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindings = bindings.data(),
        });
    }

    vk::DescriptorUpdateTemplate CreateTemplate(VkDescriptorSetLayout descriptor_set_layout,
                                                VkPipelineLayout pipeline_layout,
                                                bool use_push_descriptor) const {
        if (entries.empty()) {
            return nullptr;
        }
        const VkDescriptorUpdateTemplateType type =
            use_push_descriptor ? VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR
                                : VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
        return device->GetLogical().CreateDescriptorUpdateTemplate({
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .descriptorUpdateEntryCount = static_cast<u32>(entries.size()),
            .pDescriptorUpdateEntries = entries.data(),
            .templateType = type,
            .descriptorSetLayout = descriptor_set_layout,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .pipelineLayout = pipeline_layout,
            .set = 0,
        });
    }

    vk::PipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout) const {
        using Shader::Backend::SPIRV::RenderAreaLayout;
        using Shader::Backend::SPIRV::RescalingLayout;
        const u32 size_offset = is_compute ? sizeof(RescalingLayout::down_factor) : 0u;
        const VkPushConstantRange range{
            .stageFlags = static_cast<VkShaderStageFlags>(
                is_compute ? VK_SHADER_STAGE_COMPUTE_BIT : VK_SHADER_STAGE_ALL_GRAPHICS),
            .offset = 0,
            .size = static_cast<u32>(sizeof(RescalingLayout)) - size_offset +
                    static_cast<u32>(sizeof(RenderAreaLayout)),
        };
        return device->GetLogical().CreatePipelineLayout({
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = descriptor_set_layout ? 1U : 0U,
            .pSetLayouts = bindings.empty() ? nullptr : &descriptor_set_layout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &range,
        });
    }

    void Add(const Shader::Info& info, VkShaderStageFlags stage) {
        is_compute |= (stage & VK_SHADER_STAGE_COMPUTE_BIT) != 0;

        Add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, info.constant_buffer_descriptors);
        Add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stage, info.storage_buffers_descriptors);
        Add(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, stage, info.texture_buffer_descriptors);
        Add(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, stage, info.image_buffer_descriptors);
        Add(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage, info.texture_descriptors);
        Add(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stage, info.image_descriptors);
    }

private:
    template <typename Descriptors>
    void Add(VkDescriptorType type, VkShaderStageFlags stage, const Descriptors& descriptors) {
        const size_t num{descriptors.size()};
        for (size_t i = 0; i < num; ++i) {
            bindings.push_back({
                .binding = binding,
                .descriptorType = type,
                .descriptorCount = descriptors[i].count,
                .stageFlags = stage,
                .pImmutableSamplers = nullptr,
            });
            entries.push_back({
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = descriptors[i].count,
                .descriptorType = type,
                .offset = offset,
                .stride = sizeof(DescriptorUpdateEntry),
            });
            ++binding;
            num_descriptors += descriptors[i].count;
            offset += sizeof(DescriptorUpdateEntry);
        }
    }

    const Device* device{};
    bool is_compute{};
    boost::container::small_vector<VkDescriptorSetLayoutBinding, 32> bindings;
    boost::container::small_vector<VkDescriptorUpdateTemplateEntry, 32> entries;
    u32 binding{};
    u32 num_descriptors{};
    size_t offset{};
};

class RescalingPushConstant {
public:
    explicit RescalingPushConstant() noexcept {}

    void PushTexture(bool is_rescaled) noexcept {
        *texture_ptr |= is_rescaled ? texture_bit : 0u;
        texture_bit <<= 1u;
        if (texture_bit == 0u) {
            texture_bit = 1u;
            ++texture_ptr;
        }
    }

    void PushImage(bool is_rescaled) noexcept {
        *image_ptr |= is_rescaled ? image_bit : 0u;
        image_bit <<= 1u;
        if (image_bit == 0u) {
            image_bit = 1u;
            ++image_ptr;
        }
    }

    const std::array<u32, NUM_TEXTURE_AND_IMAGE_SCALING_WORDS>& Data() const noexcept {
        return words;
    }

private:
    std::array<u32, NUM_TEXTURE_AND_IMAGE_SCALING_WORDS> words{};
    u32* texture_ptr{words.data()};
    u32* image_ptr{words.data() + Shader::Backend::SPIRV::NUM_TEXTURE_SCALING_WORDS};
    u32 texture_bit{1u};
    u32 image_bit{1u};
};

class RenderAreaPushConstant {
public:
    bool uses_render_area{};
    std::array<f32, 4> words{};
};

inline void PushImageDescriptors(TextureCache& texture_cache,
                                 GuestDescriptorQueue& guest_descriptor_queue,
                                 const Shader::Info& info, RescalingPushConstant& rescaling,
                                 const VideoCommon::SamplerId*& samplers,
                                 const VideoCommon::ImageViewInOut*& views) {
    const Framebuffer* framebuffer = texture_cache.GetFramebuffer();
    const auto& feedback_req = texture_cache.PeekFeedbackLoopRequest();

    const auto choose_layout = [&](const ImageView& image_view) -> VkImageLayout {
        if (!feedback_req.active || !feedback_req.supported || framebuffer == nullptr) {
            return VK_IMAGE_LAYOUT_GENERAL;
        }
        const VkImage descriptor_image = image_view.ImageHandle();
        if (descriptor_image == VK_NULL_HANDLE) {
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        const VkImageSubresourceRange view_range = detail::NormalizeImageViewRange(image_view);

        for (u32 slot = 0; slot < VideoCommon::NUM_RT; ++slot) {
            if (((feedback_req.color_mask >> slot) & 1u) == 0) {
                continue;
            }
            if (framebuffer->ColorImage(slot) != descriptor_image) {
                continue;
            }
            const VkImageSubresourceRange* attachment_range = framebuffer->ColorSubresourceRange(slot);
            if (attachment_range == nullptr) {
                continue;
            }
            if (detail::SubresourceRangeIntersects(view_range, *attachment_range)) {
                return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            }
        }
        if (feedback_req.depth) {
            const VkImage depth_image = framebuffer->DepthStencilImage();
            if (depth_image == descriptor_image) {
                const VkImageSubresourceRange* attachment_range =
                    framebuffer->DepthStencilSubresourceRange();
                if (attachment_range != nullptr &&
                    detail::SubresourceRangeIntersects(view_range, *attachment_range)) {
                    return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
                }
            }
        }
        return VK_IMAGE_LAYOUT_GENERAL;
    };
    const u32 num_texture_buffers = Shader::NumDescriptors(info.texture_buffer_descriptors);
    const u32 num_image_buffers = Shader::NumDescriptors(info.image_buffer_descriptors);
    views += num_texture_buffers;
    views += num_image_buffers;
    for (const auto& desc : info.texture_descriptors) {
        for (u32 index = 0; index < desc.count; ++index) {
            const VideoCommon::ImageViewId image_view_id{(views++)->id};
            const VideoCommon::SamplerId sampler_id{*(samplers++)};
            ImageView& image_view{texture_cache.GetImageView(image_view_id)};
            const VkImageView vk_image_view{image_view.Handle(desc.type)};
            const Sampler& sampler{texture_cache.GetSampler(sampler_id)};
            const bool use_fallback_sampler{sampler.HasAddedAnisotropy() &&
                                            !image_view.SupportsAnisotropy()};
            const VkSampler vk_sampler{use_fallback_sampler ? sampler.HandleWithDefaultAnisotropy()
                                                            : sampler.Handle()};
            const VkImageLayout layout = choose_layout(image_view);
            guest_descriptor_queue.AddSampledImage(vk_image_view, vk_sampler, layout);
            rescaling.PushTexture(texture_cache.IsRescaling(image_view));
        }
    }
    for (const auto& desc : info.image_descriptors) {
        for (u32 index = 0; index < desc.count; ++index) {
            ImageView& image_view{texture_cache.GetImageView((views++)->id)};
            if (desc.is_written) {
                texture_cache.MarkModification(image_view.image_id);
            }
            const VkImageView vk_image_view{image_view.StorageView(desc.type, desc.format)};
            const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL; // Storage images must remain in GENERAL layout
            guest_descriptor_queue.AddImage(vk_image_view, layout);
            rescaling.PushImage(texture_cache.IsRescaling(image_view));
        }
    }
}

} // namespace Vulkan

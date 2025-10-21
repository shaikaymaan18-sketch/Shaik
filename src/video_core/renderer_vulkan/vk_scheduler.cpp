// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "video_core/renderer_vulkan/vk_query_cache.h"

#include "common/thread.h"
#include "video_core/renderer_vulkan/vk_command_pool.h"
#include "video_core/renderer_vulkan/vk_master_semaphore.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/renderer_vulkan/vk_state_tracker.h"
#include "video_core/renderer_vulkan/vk_texture_cache.h"
#include "video_core/vulkan_common/vulkan_device.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

namespace {
struct StageAccessInfo {
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags access = 0;
};

[[nodiscard]] StageAccessInfo StageAccessForLayout(VkImageLayout layout,
                                                   [[maybe_unused]] VkImageAspectFlags aspect_mask) noexcept {
    switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        return {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0};
    case VK_IMAGE_LAYOUT_GENERAL:
        return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
        constexpr VkPipelineStageFlags depth_stages =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        return {depth_stages,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
    }
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT};
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT};
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_SHADER_READ_BIT};
    default:
        // Fallback to a conservative barrier when encountering uncommon layouts.
        return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
    }
}

[[nodiscard]] VkImageLayout OptimalLayoutForRange(const VkImageSubresourceRange& range) noexcept {
    if ((range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
} // Anonymous namespace


void Scheduler::CommandChunk::ExecuteAll(vk::CommandBuffer cmdbuf,
                                         vk::CommandBuffer upload_cmdbuf) {
    auto command = first;
    while (command != nullptr) {
        auto next = command->GetNext();
        command->Execute(cmdbuf, upload_cmdbuf);
        command->~Command();
        command = next;
    }
    submit = false;
    command_offset = 0;
    first = nullptr;
    last = nullptr;
}

Scheduler::Scheduler(const Device& device_, StateTracker& state_tracker_)
    : device{device_}, state_tracker{state_tracker_},
      master_semaphore{std::make_unique<MasterSemaphore>(device)},
      command_pool{std::make_unique<CommandPool>(*master_semaphore, device)} {
    AcquireNewChunk();
    AllocateWorkerCommandBuffer();
    worker_thread = std::jthread([this](std::stop_token token) { WorkerThread(token); });
}

Scheduler::~Scheduler() = default;

u64 Scheduler::Flush(VkSemaphore signal_semaphore, VkSemaphore wait_semaphore) {
    // When flushing, we only send data to the worker thread; no waiting is necessary.
    const u64 signal_value = SubmitExecution(signal_semaphore, wait_semaphore);
    AllocateNewContext();
    return signal_value;
}

void Scheduler::Finish(VkSemaphore signal_semaphore, VkSemaphore wait_semaphore) {
    // When finishing, we need to wait for the submission to have executed on the device.
    const u64 presubmit_tick = CurrentTick();
    SubmitExecution(signal_semaphore, wait_semaphore);
    Wait(presubmit_tick);
    AllocateNewContext();
}

void Scheduler::WaitWorker() {
    DispatchWork();

    // Ensure the queue is drained.
    {
        std::unique_lock ql{queue_mutex};
        event_cv.wait(ql, [this] { return work_queue.empty(); });
    }

    // Now wait for execution to finish.
    std::scoped_lock el{execution_mutex};
}

void Scheduler::DispatchWork() {
    if (chunk->Empty()) {
        return;
    }
    {
        std::scoped_lock ql{queue_mutex};
        work_queue.push(std::move(chunk));
    }
    event_cv.notify_all();
    AcquireNewChunk();
}

void Scheduler::RequestRenderpass(const Framebuffer* framebuffer) {
    const VkRenderPass renderpass = framebuffer->RenderPass();
    const VkFramebuffer framebuffer_handle = framebuffer->Handle();
    const VkExtent2D render_area = framebuffer->RenderArea();
    if (renderpass == state.renderpass && framebuffer_handle == state.framebuffer &&
        render_area.width == state.render_area.width &&
        render_area.height == state.render_area.height) {
        return;
    }
    EndRenderPass(false);
    state.renderpass = renderpass;
    state.framebuffer = framebuffer_handle;
    state.render_area = render_area;

    const u32 framebuffer_image_count = framebuffer->NumImages();
    const auto framebuffer_images = framebuffer->Images();
    const auto framebuffer_ranges = framebuffer->ImageRanges();
    const auto framebuffer_layouts = framebuffer->ImageLayouts();
    std::array<VkImageLayout, 9> previous_layouts{};
    previous_layouts.fill(VK_IMAGE_LAYOUT_GENERAL);
    for (size_t i = 0; i < framebuffer_image_count; ++i) {
        const VkImage image = framebuffer_images[i];
        const u64 key = ImageKey(image);
        if (const auto it = image_layout_cache.find(key); it != image_layout_cache.end()) {
            previous_layouts[i] = it->second;
        } else {
            previous_layouts[i] = VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    Record([renderpass, framebuffer_handle, render_area, framebuffer_image_count,
            framebuffer_images, framebuffer_ranges, framebuffer_layouts,
            previous_layouts](vk::CommandBuffer cmdbuf) {
        std::array<VkImageMemoryBarrier, 9> barriers{};
        VkPipelineStageFlags src_stage_mask = 0;
        VkPipelineStageFlags dst_stage_mask = 0;
        size_t barrier_count = 0;
        for (size_t i = 0; i < framebuffer_image_count; ++i) {
            const VkImageSubresourceRange& range = framebuffer_ranges[i];
            VkImageLayout target_layout = framebuffer_layouts[i];
            if (target_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                target_layout = OptimalLayoutForRange(range);
            }
            const VkImageLayout old_layout = previous_layouts[i];
            if (old_layout == target_layout) {
                continue;
            }

            const StageAccessInfo src_info = StageAccessForLayout(old_layout, range.aspectMask);
            const StageAccessInfo dst_info = StageAccessForLayout(target_layout, range.aspectMask);

            barriers[barrier_count++] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = src_info.access,
                .dstAccessMask = dst_info.access,
                .oldLayout = old_layout,
                .newLayout = target_layout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = framebuffer_images[i],
                .subresourceRange = range,
            };
            src_stage_mask |= src_info.stage;
            dst_stage_mask |= dst_info.stage;
        }

        if (barrier_count > 0) {
            cmdbuf.PipelineBarrier(
                src_stage_mask != 0 ? src_stage_mask
                                    : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                dst_stage_mask != 0 ? dst_stage_mask
                                    : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
                0, {}, {}, {barriers.data(), barrier_count});
        }

        const VkRenderPassBeginInfo renderpass_bi{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = renderpass,
            .framebuffer = framebuffer_handle,
            .renderArea =
                {
                    .offset = {.x = 0, .y = 0},
                    .extent = render_area,
                },
            .clearValueCount = 0,
            .pClearValues = nullptr,
        };
        cmdbuf.BeginRenderPass(renderpass_bi, VK_SUBPASS_CONTENTS_INLINE);
    });
    num_renderpass_images = framebuffer_image_count;
    renderpass_images = framebuffer_images;
    renderpass_image_ranges = framebuffer_ranges;
    renderpass_image_layouts = framebuffer_layouts;

    for (size_t i = 0; i < framebuffer_image_count; ++i) {
        VkImageLayout target_layout = framebuffer_layouts[i];
        if (target_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
            target_layout = OptimalLayoutForRange(framebuffer_ranges[i]);
        }
        image_layout_cache[ImageKey(framebuffer_images[i])] = target_layout;
        renderpass_image_layouts[i] = target_layout;
    }
}

void Scheduler::RequestOutsideRenderPassOperationContext() {
    EndRenderPass(true);
}

bool Scheduler::UpdateGraphicsPipeline(GraphicsPipeline* pipeline) {
    if (state.graphics_pipeline == pipeline) {
        return false;
    }
    state.graphics_pipeline = pipeline;
    return true;
}

bool Scheduler::UpdateRescaling(bool is_rescaling) {
    if (state.rescaling_defined && is_rescaling == state.is_rescaling) {
        return false;
    }
    state.rescaling_defined = true;
    state.is_rescaling = is_rescaling;
    return true;
}

void Scheduler::WorkerThread(std::stop_token stop_token) {
    Common::SetCurrentThreadName("VulkanWorker");

    const auto TryPopQueue{[this](auto& work) -> bool {
        if (work_queue.empty()) {
            return false;
        }

        work = std::move(work_queue.front());
        work_queue.pop();
        event_cv.notify_all();
        return true;
    }};

    while (!stop_token.stop_requested()) {
        std::unique_ptr<CommandChunk> work;

        {
            std::unique_lock lk{queue_mutex};

            // Wait for work.
            event_cv.wait(lk, stop_token, [&] { return TryPopQueue(work); });

            // If we've been asked to stop, we're done.
            if (stop_token.stop_requested()) {
                return;
            }

            // Exchange lock ownership so that we take the execution lock before
            // the queue lock goes out of scope. This allows us to force execution
            // to complete in the next step.
            std::exchange(lk, std::unique_lock{execution_mutex});

            // Perform the work, tracking whether the chunk was a submission
            // before executing.
            const bool has_submit = work->HasSubmit();
            work->ExecuteAll(current_cmdbuf, current_upload_cmdbuf);

            // If the chunk was a submission, reallocate the command buffer.
            if (has_submit) {
                AllocateWorkerCommandBuffer();
            }
        }

        {
            std::scoped_lock rl{reserve_mutex};

            // Recycle the chunk back to the reserve.
            chunk_reserve.emplace_back(std::move(work));
        }
    }
}

void Scheduler::AllocateWorkerCommandBuffer() {
    current_cmdbuf = vk::CommandBuffer(command_pool->Commit(), device.GetDispatchLoader());
    current_cmdbuf.Begin({
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    });
    current_upload_cmdbuf = vk::CommandBuffer(command_pool->Commit(), device.GetDispatchLoader());
    current_upload_cmdbuf.Begin({
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    });
}

u64 Scheduler::SubmitExecution(VkSemaphore signal_semaphore, VkSemaphore wait_semaphore) {
    EndPendingOperations();
    InvalidateState();

    const u64 signal_value = master_semaphore->NextTick();
    RecordWithUploadBuffer([signal_semaphore, wait_semaphore, signal_value,
                            this](vk::CommandBuffer cmdbuf, vk::CommandBuffer upload_cmdbuf) {
        static constexpr VkMemoryBarrier WRITE_BARRIER{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        };
        upload_cmdbuf.PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, WRITE_BARRIER);
        upload_cmdbuf.End();
        cmdbuf.End();

        if (on_submit) {
            on_submit();
        }

        std::scoped_lock lock{submit_mutex};
        switch (const VkResult result = master_semaphore->SubmitQueue(
                    cmdbuf, upload_cmdbuf, signal_semaphore, wait_semaphore, signal_value)) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_DEVICE_LOST:
            device.ReportLoss();
            [[fallthrough]];
        default:
            vk::Check(result);
            break;
        }
    });
    chunk->MarkSubmit();
    DispatchWork();
    return signal_value;
}

void Scheduler::AllocateNewContext() {
    // Enable counters once again. These are disabled when a command buffer is finished.
}

void Scheduler::InvalidateState() {
    state.graphics_pipeline = nullptr;
    state.rescaling_defined = false;
    state_tracker.InvalidateCommandBufferState();
}

void Scheduler::EndPendingOperations() {
    query_cache->CounterReset(VideoCommon::QueryType::ZPassPixelCount64);
    EndRenderPass(true);
}

void Scheduler::EndRenderPass(bool force_general)
{
    if (!state.renderpass) {
        return;
    }

    query_cache->CounterEnable(VideoCommon::QueryType::ZPassPixelCount64, false);
    query_cache->NotifySegment(false);

    if (force_general) {
        Record([num_images = num_renderpass_images,
                       images = renderpass_images,
                       ranges = renderpass_image_ranges,
                       layouts = renderpass_image_layouts](vk::CommandBuffer cmdbuf) {
            cmdbuf.EndRenderPass();

            if (num_images == 0) {
                return;
            }

            std::array<VkImageMemoryBarrier, 9> barriers{};
            VkPipelineStageFlags src_stages = 0;
            VkPipelineStageFlags dst_stages = 0;

            for (size_t i = 0; i < num_images; ++i) {
                const VkImageSubresourceRange& range = ranges[i];
                VkImageLayout old_layout = layouts[i];
                if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    old_layout = OptimalLayoutForRange(range);
                }
                constexpr VkImageLayout new_layout = VK_IMAGE_LAYOUT_GENERAL;

                const StageAccessInfo src_info = StageAccessForLayout(old_layout, range.aspectMask);
                const StageAccessInfo dst_info = StageAccessForLayout(new_layout, range.aspectMask);

                barriers[i] = VkImageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = src_info.access,
                    .dstAccessMask = dst_info.access,
                    .oldLayout = old_layout,
                    .newLayout = new_layout,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = images[i],
                    .subresourceRange = range,
                };
                src_stages |= src_info.stage;
                dst_stages |= dst_info.stage;
            }

            cmdbuf.PipelineBarrier(src_stages != 0 ? src_stages
                                                   : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT),
                                   dst_stages != 0 ? dst_stages
                                                   : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT),
                                   0,
                                   {},
                                   {},
                                   {barriers.data(), num_images});
        });

        for (size_t i = 0; i < num_renderpass_images; ++i) {
            image_layout_cache[ImageKey(renderpass_images[i])] = VK_IMAGE_LAYOUT_GENERAL;
        }
    } else {
        Record([](vk::CommandBuffer cmdbuf) {
            cmdbuf.EndRenderPass();
        });
        for (size_t i = 0; i < num_renderpass_images; ++i) {
            VkImageLayout layout = renderpass_image_layouts[i];
            if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                layout = OptimalLayoutForRange(renderpass_image_ranges[i]);
            }
            image_layout_cache[ImageKey(renderpass_images[i])] = layout;
        }
    }

    state.renderpass = nullptr;
    num_renderpass_images = 0;
}


void Scheduler::AcquireNewChunk() {
    std::scoped_lock rl{reserve_mutex};

    if (chunk_reserve.empty()) {
        // If we don't have anything reserved, we need to make a new chunk.
        chunk = std::make_unique<CommandChunk>();
    } else {
        // Otherwise, we can just take from the reserve.
        chunk = std::move(chunk_reserve.back());
        chunk_reserve.pop_back();
    }
}

} // namespace Vulkan

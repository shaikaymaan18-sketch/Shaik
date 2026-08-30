// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <mutex>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include "video_core/dma_pusher.h"

namespace Tegra {

class GPU;

namespace Control {

struct ChannelState;

class Scheduler {
public:
    void Push(GPU& gpu, s32 channel, CommandList&& entries);

    void DeclareChannel(std::shared_ptr<ChannelState> new_channel);

private:
    boost::container::unordered_flat_map<s32, std::shared_ptr<ChannelState>> channels;
    std::mutex scheduling_guard;
};

} // namespace Control

} // namespace Tegra

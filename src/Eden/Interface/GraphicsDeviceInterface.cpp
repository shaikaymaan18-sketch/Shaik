// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GraphicsDeviceInterface.h"
#include <QWindow>
#include "common/settings.h"
#include "common/settings_enums.h"
#include "qt_common/qt_common.h"

static const QString TranslateVSyncMode(VkPresentModeKHR mode,
                                                    Settings::RendererBackend backend) {
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return backend == Settings::RendererBackend::OpenGL
                   ? QtCommon::tr("Off")
                   : QStringLiteral("Immediate (%1)").arg(QtCommon::tr("VSync Off"));
    case VK_PRESENT_MODE_MAILBOX_KHR:
        return QStringLiteral("Mailbox (%1)").arg(QtCommon::tr("Recommended"));
    case VK_PRESENT_MODE_FIFO_KHR:
        return backend == Settings::RendererBackend::OpenGL
                   ? QtCommon::tr("On")
                   : QStringLiteral("FIFO (%1)").arg(QtCommon::tr("VSync On"));
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return QStringLiteral("FIFO Relaxed");
    default:
        return {};
        break;
    }
}

GraphicsDeviceInterface::GraphicsDeviceInterface(QQuickItem *parent)
    : QQuickItem(parent)
{
    // NB: QML does NOT guarantee ordering!
    setApi(Settings::values.renderer_backend.GetValue());
    setVsyncMode(int(Settings::values.vsync_mode.GetValue()));
    setDevice(Settings::values.vulkan_device.GetValue());
}

QStringList GraphicsDeviceInterface::devices()
{
    return vulkan_devices;
}

void GraphicsDeviceInterface::populateDevices()
{
    vulkan_devices.clear();
    vulkan_devices.reserve(records.size());
    device_present_modes.clear();
    device_present_modes.reserve(records.size());

    for (const auto &record : records) {
        vulkan_devices.push_back(QString::fromStdString(record.name));
        device_present_modes.push_back(record.vsync_support);

        // if (record.has_broken_compute) {
        //     expose_compute_option();
        // }
    }

    emit devicesChanged();
}

void GraphicsDeviceInterface::populateVsync()
{
    if (m_api == Settings::RendererBackend::Null) {
        return;
    }

    const auto &present_modes = //< relevant vector of present modes for the selected device or API
        m_isVulkan && m_device > -1 ? device_present_modes[m_device] : default_present_modes;

    m_vsyncModes.clear();
    m_vsyncModes.reserve(present_modes.size());

    for (const auto present_mode : present_modes) {
        const auto mode_name = TranslateVSyncMode(present_mode, m_api);
        if (mode_name.isEmpty()) {
            continue;
        }

        m_vsyncModes.append(mode_name);
    }

    emit vsyncModesChanged(m_vsyncModes);
}

Settings::RendererBackend GraphicsDeviceInterface::api() const
{
    return m_api;
}

void GraphicsDeviceInterface::setApi(const Settings::RendererBackend &newApi)
{
    if (m_api == newApi)
        return;

    m_api = newApi;
    emit apiChanged(m_api);

    m_isOpenGL = newApi == Settings::RendererBackend::OpenGL;
    m_isVulkan = newApi == Settings::RendererBackend::Vulkan;

    emit isOpenGLChanged(m_isOpenGL);
    emit isVulkanChanged(m_isVulkan);
}

void GraphicsDeviceInterface::componentComplete()
{
    VkDeviceInfo::PopulateRecords(records, (QWindow *) window());
    populateDevices();
    populateVsync();
}

bool GraphicsDeviceInterface::isOpenGL() const
{
    return m_isOpenGL;
}

bool GraphicsDeviceInterface::isVulkan() const
{
    return m_isVulkan;
}

int GraphicsDeviceInterface::device() const
{
    return m_device;
}

void GraphicsDeviceInterface::setDevice(int newDevice)
{
    if (m_device == newDevice)
        return;
    m_device = newDevice;
    emit deviceChanged(m_device);
}

QStringList GraphicsDeviceInterface::vsyncModes() const
{
    return m_vsyncModes;
}

int GraphicsDeviceInterface::vsyncMode() const
{
    return m_vsyncMode;
}

void GraphicsDeviceInterface::setVsyncMode(int newVsyncMode)
{
    if (m_vsyncMode == newVsyncMode)
        return;
    m_vsyncMode = newVsyncMode;
    emit vsyncModeChanged(m_vsyncMode);
}

// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QQuickItem>

#include "qt_common/util/vk_device_info.h"

class GraphicsDeviceInterface : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList vsyncModes READ vsyncModes NOTIFY vsyncModesChanged)

    Q_PROPERTY(Settings::RendererBackend api READ api WRITE setApi NOTIFY apiChanged)
    Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(int vsyncMode READ vsyncMode WRITE setVsyncMode NOTIFY vsyncModeChanged)

    Q_PROPERTY(bool isOpenGL READ isOpenGL NOTIFY isOpenGLChanged)
    Q_PROPERTY(bool isVulkan READ isVulkan NOTIFY isVulkanChanged)
public:
    explicit GraphicsDeviceInterface(QQuickItem *parent = nullptr);

    QStringList devices();

    Settings::RendererBackend api() const;
    void setApi(const Settings::RendererBackend &newApi);

    bool isOpenGL() const;
    bool isVulkan() const;

    int device() const;
    void setDevice(int newDevice);

    QStringList vsyncModes() const;

    int vsyncMode() const;
    void setVsyncMode(int newVsyncMode);

protected:
    void componentComplete();
signals:
    void apiChanged(Settings::RendererBackend api);
    void devicesChanged();

    void isOpenGLChanged(bool isOpenGL);
    void isVulkanChanged(bool isVulkan);

    void deviceChanged(int device);

    void vsyncModesChanged(QStringList vsyncModes);

    void vsyncModeChanged(int vsyncMode);

private:
    std::vector<VkDeviceInfo::Record> records{};

    QStringList vulkan_devices;
    QStringList m_vsyncModes;

    std::vector<std::vector<VkPresentModeKHR>> device_present_modes;

    void populateDevices();
    void populateVsync();

    Settings::RendererBackend m_api;
    bool m_isOpenGL;
    bool m_isVulkan;

    int m_device;
    int m_vsyncMode;
};

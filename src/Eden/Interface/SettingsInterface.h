// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSINTERFACE_H
#define SETTINGSINTERFACE_H

#include <QObject>
#include <QQmlEngine>

#include "QMLSetting.h"
#include "qt_common/config/shared_translation.h"
#include "Eden/Models/SettingsModel.h"

namespace SettingsCategories {
Q_NAMESPACE

enum class Category {
    Android = u32(Settings::Category::Android),
    Audio = u32(Settings::Category::Audio),
    Core = u32(Settings::Category::Core),
    Cpu = u32(Settings::Category::Cpu),
    CpuDebug = u32(Settings::Category::CpuDebug),
    CpuUnsafe = u32(Settings::Category::CpuUnsafe),
    Overlay = u32(Settings::Category::Overlay),
    Renderer = u32(Settings::Category::Renderer),
    RendererAdvanced = u32(Settings::Category::RendererAdvanced),
    RendererExtensions = u32(Settings::Category::RendererExtensions),
    RendererDebug = u32(Settings::Category::RendererDebug),
    System = u32(Settings::Category::System),
    SystemAudio = u32(Settings::Category::SystemAudio),
    DataStorage = u32(Settings::Category::DataStorage),
    Debugging = u32(Settings::Category::Debugging),
    DebuggingGraphics = u32(Settings::Category::DebuggingGraphics),
    GpuDriver = u32(Settings::Category::GpuDriver),
    Miscellaneous = u32(Settings::Category::Miscellaneous),
    Network = u32(Settings::Category::Network),
    WebService = u32(Settings::Category::WebService),
    AddOns = u32(Settings::Category::AddOns),
    Controls = u32(Settings::Category::Controls),
    Ui = u32(Settings::Category::Ui),
    UiAudio = u32(Settings::Category::UiAudio),
    UiGeneral = u32(Settings::Category::UiGeneral),
    UiLayout = u32(Settings::Category::UiLayout),
    UiGameList = u32(Settings::Category::UiGameList),
    Screenshots = u32(Settings::Category::Screenshots),
    Shortcuts = u32(Settings::Category::Shortcuts),
    Multiplayer = u32(Settings::Category::Multiplayer),
    Services = u32(Settings::Category::Services),
    Paths = u32(Settings::Category::Paths),
    LibraryApplet = u32(Settings::Category::LibraryApplet),
    MaxEnum = u32(Settings::Category::MaxEnum),
};
Q_ENUM_NS(Category)
}

class SettingsInterface : public QObject {
    Q_OBJECT

public:
    explicit SettingsInterface(QObject* parent = nullptr);

    QMLSetting *getSetting(Settings::BasicSetting *setting);
    Q_INVOKABLE QMLSetting *setting(const QString &key);
    Q_INVOKABLE SettingsModel *category(SettingsCategories::Category category,
                                        QList<QString> idInclude = {},
                                        QList<QString> idExclude = {});

    Q_INVOKABLE int id(const QString &key);

    bool global() const;
    void setGlobal(bool newGlobal);

signals:
    void globalChanged();

private:
    QMap<std::string, QMLSetting *> m_settings;

    std::unique_ptr<ConfigurationShared::TranslationMap> translations;
    std::unique_ptr<ConfigurationShared::ComboboxTranslationMap> combobox_translations;

    Q_PROPERTY(bool global READ global WRITE setGlobal NOTIFY globalChanged FINAL)
};

#endif // SETTINGSINTERFACE_H

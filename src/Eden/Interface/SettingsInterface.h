// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
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

#define FWD_CAT(category) category = u32(Settings::Category::category),
enum class Category {
    FWD_CAT(Android)
    FWD_CAT(Audio)
    FWD_CAT(Core)
    FWD_CAT(Cpu)
    FWD_CAT(CpuDebug)
    FWD_CAT(CpuUnsafe)
    FWD_CAT(Overlay)
    FWD_CAT(Renderer)
    FWD_CAT(RendererAdvanced)
    FWD_CAT(RendererHacks)
    FWD_CAT(RendererExtensions)
    FWD_CAT(RendererDebug)
    FWD_CAT(System)
    FWD_CAT(SystemAudio)
    FWD_CAT(DataStorage)
    FWD_CAT(Debugging)
    FWD_CAT(DebuggingGraphics)
    FWD_CAT(GpuDriver)
    FWD_CAT(Miscellaneous)
    FWD_CAT(Network)
    FWD_CAT(WebService)
    FWD_CAT(AddOns)
    FWD_CAT(Controls)
    FWD_CAT(Ui)
    FWD_CAT(UiAudio)
    FWD_CAT(UiGeneral)
    FWD_CAT(UiLayout)
    FWD_CAT(UiGameList)
    FWD_CAT(Screenshots)
    FWD_CAT(Shortcuts)
    FWD_CAT(Multiplayer)
    FWD_CAT(Services)
    FWD_CAT(Paths)
    FWD_CAT(LibraryApplet)
};
Q_ENUM_NS(Category)

#undef FWD_CAT
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

#ifndef SETTINGSINTERFACE_H
#define SETTINGSINTERFACE_H

#include <QObject>
#include <QtQmlIntegration/QtQmlIntegration>

#include "QMLSetting.h"
#include "shared_translation.h"
#include "yuzu/models/SettingsModel.h"

namespace SettingsCategories {
Q_NAMESPACE

enum class Category {
    Android = static_cast<u32>(Settings::Category::Android),
    Audio = static_cast<u32>(Settings::Category::Audio),
    Core = static_cast<u32>(Settings::Category::Core),
    Cpu = static_cast<u32>(Settings::Category::Cpu),
    CpuDebug = static_cast<u32>(Settings::Category::CpuDebug),
    CpuUnsafe = static_cast<u32>(Settings::Category::CpuUnsafe),
    Overlay = static_cast<u32>(Settings::Category::Overlay),
    Renderer = static_cast<u32>(Settings::Category::Renderer),
    RendererAdvanced = static_cast<u32>(Settings::Category::RendererAdvanced),
    RendererExtensions = static_cast<u32>(Settings::Category::RendererExtensions),
    RendererDebug = static_cast<u32>(Settings::Category::RendererDebug),
    System = static_cast<u32>(Settings::Category::System),
    SystemAudio = static_cast<u32>(Settings::Category::SystemAudio),
    DataStorage = static_cast<u32>(Settings::Category::DataStorage),
    Debugging = static_cast<u32>(Settings::Category::Debugging),
    DebuggingGraphics = static_cast<u32>(Settings::Category::DebuggingGraphics),
    GpuDriver = static_cast<u32>(Settings::Category::GpuDriver),
    Miscellaneous = static_cast<u32>(Settings::Category::Miscellaneous),
    Network = static_cast<u32>(Settings::Category::Network),
    WebService = static_cast<u32>(Settings::Category::WebService),
    AddOns = static_cast<u32>(Settings::Category::AddOns),
    Controls = static_cast<u32>(Settings::Category::Controls),
    Ui = static_cast<u32>(Settings::Category::Ui),
    UiAudio = static_cast<u32>(Settings::Category::UiAudio),
    UiGeneral = static_cast<u32>(Settings::Category::UiGeneral),
    UiLayout = static_cast<u32>(Settings::Category::UiLayout),
    UiGameList = static_cast<u32>(Settings::Category::UiGameList),
    Screenshots = static_cast<u32>(Settings::Category::Screenshots),
    Shortcuts = static_cast<u32>(Settings::Category::Shortcuts),
    Multiplayer = static_cast<u32>(Settings::Category::Multiplayer),
    Services = static_cast<u32>(Settings::Category::Services),
    Paths = static_cast<u32>(Settings::Category::Paths),
    Linux = static_cast<u32>(Settings::Category::Linux),
    LibraryApplet = static_cast<u32>(Settings::Category::LibraryApplet),
    MaxEnum = static_cast<u32>(Settings::Category::MaxEnum),
};
Q_ENUM_NS(Category)
}

class SettingsInterface : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
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

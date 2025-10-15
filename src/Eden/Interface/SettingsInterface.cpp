#include "SettingsInterface.h"
#include "common/settings.h"
#include "common/logging/log.h"
#include "qt_common/config/uisettings.h"

SettingsInterface::SettingsInterface(QObject* parent)
    : QObject{parent}
      ,  translations{ConfigurationShared::InitializeTranslations(parent)}
      ,  combobox_translations{ConfigurationShared::ComboboxEnumeration(parent)}
{
}

SettingsModel *SettingsInterface::category(SettingsCategories::Category category,
                                           QList<QString> idInclude,
                                           QList<QString> idExclude)
{
    std::vector<Settings::BasicSetting *> settings = Settings::values.linkage.by_category[static_cast<Settings::Category>(category)];
    std::vector<Settings::BasicSetting *> uisettings = UISettings::values.linkage.by_category[static_cast<Settings::Category>(category)];

    settings.insert(settings.end(), uisettings.begin(), uisettings.end());

    QList<QMLSetting *> settingsList;
    for (Settings::BasicSetting *setting : settings) {
        // paired settings get ignored
        if (setting->Specialization() == Settings::Specialization::Paired) {
            LOG_DEBUG(Frontend, "\"{}\" has specialization Paired: ignoring", setting->GetLabel());
            continue;
        }

        if ((idInclude.empty() || idInclude.contains(setting->GetLabel()))
            && (idExclude.empty() || !idExclude.contains(setting->GetLabel()))) {
            settingsList.append(this->getSetting(setting));
        }
    }

    SettingsModel *model = new SettingsModel(this);
    model->append(settingsList);

    return model;
}

int SettingsInterface::id(const QString &key)
{
    return setting(key)->id();
}

bool SettingsInterface::global() const
{
    return Settings::IsConfiguringGlobal();
}

void SettingsInterface::setGlobal(bool newGlobal)
{
    Settings::SetConfiguringGlobal(newGlobal);
}

QMLSetting *SettingsInterface::getSetting(Settings::BasicSetting *setting)
{
    if (setting == nullptr) {
        return nullptr;
    }

    if (m_settings.contains(setting->GetLabel())) {
        return m_settings.value(setting->GetLabel());
    }

    const int id = setting->Id();

    const auto [label, tooltip] = [&]() {
        const auto& setting_label = setting->GetLabel();
        if (translations->contains(id)) {
            return std::pair{translations->at(id).first, translations->at(id).second};
        }

        LOG_WARNING(Frontend, "Translation table lacks entry for \"{}\"", setting_label);
        return std::pair{QString::fromStdString(setting_label), QString()};
    }();

    const auto type = setting->EnumIndex();
    QStringList items;

    const ConfigurationShared::ComboboxTranslations* enumeration{nullptr};
    if (combobox_translations->contains(type)) {
        enumeration = &combobox_translations->at(type);
        for (const auto& [_, name] : *enumeration) {
            items << name;
        }
    }

    // TODO: Suffix (fr)
    QString suffix = "";

    if ((setting->Specialization() & Settings::SpecializationAttributeMask) ==
        Settings::Specialization::Percentage) {
        suffix = "%";
    }

    // paired setting (I/A)
    QMLSetting *other = this->getSetting(setting->PairedSetting());

    QMLSetting *qsetting = new QMLSetting(setting, this);
    qsetting->setLabel(label);
    qsetting->setTooltip(tooltip);
    qsetting->setCombo(items);
    qsetting->setSuffix(suffix);
    qsetting->setOther(other);

    m_settings.insert(setting->GetLabel(), qsetting);

    return qsetting;
}

QMLSetting *SettingsInterface::setting(const QString &key)
{
    std::string skey = key.toStdString();
    if (!m_settings.contains(skey)) {
        Settings::BasicSetting *basicSetting = Settings::values.linkage.by_key.contains(skey) ?
                                                   Settings::values.linkage.by_key[skey] :
                                                   UISettings::values.linkage.by_key[skey];
        return getSetting(basicSetting);
    } else {
        return m_settings.value(skey);
    }
}

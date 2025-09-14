#include "QMLSetting.h"
#include "common/settings.h"

#include <QVariant>

QMLSetting::QMLSetting(Settings::BasicSetting *setting, QObject *parent, RequestType request)
    : QObject(parent)
      , m_setting(setting)
{

    // TODO: restore/touch
    /*
    if (!Settings::IsConfiguringGlobal() && managed) {
        restore_button = CreateRestoreGlobalButton(setting.UsingGlobal(), this);

         touch = [this]() {
             LOG_DEBUG(Frontend, "Enabling custom setting for \"{}\"", setting.GetLabel());
             restore_button->setEnabled(true);
             restore_button->setVisible(true);
         };
     }
     */

    const auto type = setting->TypeId();

    request = [&]() {
        if (request != RequestType::Default) {
            return request;
        }
        switch (setting->Specialization() & Settings::SpecializationTypeMask) {
        case Settings::Specialization::Default:
            return RequestType::Default;
        case Settings::Specialization::Time:
            return RequestType::DateTimeEdit;
        case Settings::Specialization::Hex:
            return RequestType::HexEdit;
        case Settings::Specialization::RuntimeList:
            // managed = false;
            [[fallthrough]];
        case Settings::Specialization::List:
            return RequestType::ComboBox;
        case Settings::Specialization::Scalar:
            return RequestType::Slider;
        case Settings::Specialization::Countable:
            return RequestType::SpinBox;
        case Settings::Specialization::Radio:
            return RequestType::RadioGroup;
        default:
            break;
        }
        return request;
    }();

    if (type == typeid(bool)) {
        m_type = "bool";
        m_metaType = QMetaType::Bool;
    } else if (setting->IsEnum()) {
        m_metaType = QMetaType::UInt;

        if (request == RequestType::RadioGroup) {
            m_type = "radio";
            // TODO: Add the options and whatnot
            // see CreateRadioGroup
        } else {
            m_type = "enumCombo";
        }
    } else if (setting->IsIntegral()) {
        m_metaType = QMetaType::UInt;

        switch (request) {
        case RequestType::Slider:
        case RequestType::ReverseSlider:
            // TODO: Reversal and multiplier
            m_type = "intSlider";
            break;
        case RequestType::Default:
        case RequestType::LineEdit:
            m_type = "intSpin";
            break;
        case RequestType::DateTimeEdit:
            // TODO: disabled/restrict
            m_type = "time";
            break;
        case RequestType::SpinBox:
            // TODO: suffix
            m_type = "intSpin";
            break;
        case RequestType::HexEdit:
            m_type = "hex";
            break;
        case RequestType::ComboBox:
            // TODO: Add the options and whatnot
            // see CreateComboBox
            m_type = "intCombo";
            break;
        default:
            // UNIMPLEMENTED();
            break;
        }
    } else if (setting->IsFloatingPoint()) {
        m_metaType = QMetaType::Double;

        switch (request) {
        case RequestType::Default:
        case RequestType::SpinBox:
            // TODO: suffix
            m_type = "doubleSpin";
            break;
        case RequestType::Slider:
        case RequestType::ReverseSlider:
            // TODO: multiplier, suffix, reversal
            m_type = "doubleSlider";
            break;
        default:
            // UNIMPLEMENTED assert
            // UNIMPLEMENTED();
            break;
        }
    } else if (type == typeid(std::string)) {
        m_metaType = QMetaType::QString;

        switch (request) {
        case RequestType::Default:
        case RequestType::LineEdit:
            m_type = "stringLine";
            break;
        case RequestType::ComboBox:
            m_type = "stringCombo";
            break;
        default:
            // UNIMPLEMENTED();
            break;
        }
    }
}

QVariant QMLSetting::value() const
{
    QVariant var = QVariant::fromValue(QString::fromStdString(m_setting->ToString()));
    var.convert(QMetaType(m_metaType));
    return var;
}

void QMLSetting::setValue(const QVariant &newValue)
{
    QVariant var = newValue;
    var.convert(QMetaType(m_metaType));

    m_setting->LoadString(var.toString().toStdString());
    emit valueChanged();
}

bool QMLSetting::global() const
{
    return m_setting->UsingGlobal();
}

void QMLSetting::setGlobal(bool newGlobal)
{
    m_setting->SetGlobal(newGlobal);
    emit globalChanged();
    emit valueChanged();
}

void QMLSetting::restore()
{
    std::string toSet = Settings::IsConfiguringGlobal() ? m_setting->DefaultToString() : m_setting->ToStringGlobal();
    setValue(QString::fromStdString(toSet));
    setGlobal(false);
}

QMLSetting *QMLSetting::other() const
{
    return m_other;
}

void QMLSetting::setOther(QMLSetting *newOther)
{
    if (m_other == newOther)
        return;
    m_other = newOther;
    emit otherChanged();
}

u32 QMLSetting::max() const
{
    return std::strtol(m_setting->MaxVal().c_str(), nullptr, 0);
}

u32 QMLSetting::min() const
{
    return std::strtol(m_setting->MinVal().c_str(), nullptr, 0);
}

QString QMLSetting::suffix() const
{
    return m_suffix;
}

void QMLSetting::setSuffix(const QString &newSuffix)
{
    if (m_suffix == newSuffix)
        return;
    m_suffix = newSuffix;
    emit suffixChanged();
}

QStringList QMLSetting::combo() const
{
    return m_combo;
}

void QMLSetting::setCombo(const QStringList &newCombo)
{
    if (m_combo == newCombo)
        return;
    m_combo = newCombo;
    emit comboChanged();
}

QString QMLSetting::type() const
{
    return m_type;
}

void QMLSetting::setType(const QString &newType)
{
    if (m_type == newType)
        return;
    m_type = newType;
    emit typeChanged();
}

u32 QMLSetting::id() const
{
    return m_setting->Id();
}

QString QMLSetting::tooltip() const
{
    return m_tooltip;
}

void QMLSetting::setTooltip(const QString &newTooltip)
{
    if (m_tooltip == newTooltip)
        return;
    m_tooltip = newTooltip;
    emit tooltipChanged();
}

QString QMLSetting::label() const
{
    return m_label.isEmpty() ? QString::fromStdString(m_setting->GetLabel()) : m_label;
}

void QMLSetting::setLabel(const QString &newLabel)
{
    if (m_label == newLabel)
        return;
    m_label = newLabel;
    emit labelChanged();
}

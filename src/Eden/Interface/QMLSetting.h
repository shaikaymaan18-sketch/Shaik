#ifndef QMLSETTING_H
#define QMLSETTING_H

#include <QObject>
#include "common/settings_common.h"

enum class RequestType {
    Default,
    ComboBox,
    SpinBox,
    Slider,
    ReverseSlider,
    LineEdit,
    HexEdit,
    DateTimeEdit,
    RadioGroup,
    MaxEnum,
};

class QMLSetting : public QObject {
    Q_OBJECT

public:
    explicit QMLSetting(Settings::BasicSetting *setting, QObject *parent = nullptr, RequestType request = RequestType::Default);

    QVariant value() const;
    void setValue(const QVariant &newValue);

    bool global() const;
    void setGlobal(bool newGlobal);

    QString label() const;
    void setLabel(const QString &newLabel);

    QString tooltip() const;
    void setTooltip(const QString &newTooltip);

    u32 id() const;

    QString type() const;
    void setType(const QString &newType);

    QStringList combo() const;
    void setCombo(const QStringList &newCombo);

    QString suffix() const;
    void setSuffix(const QString &newSuffix);

    // TODO: float versions
    u32 min() const;
    u32 max() const;

    QMLSetting *other() const;
    void setOther(QMLSetting *newOther);

public slots:
    void restore();

signals:
    void valueChanged();
    void globalChanged();

    void labelChanged();

    void tooltipChanged();

    void typeChanged();

    void comboChanged();

    void suffixChanged();

    void otherChanged();

private:
    Settings::BasicSetting *m_setting;

    QMLSetting *m_other;

    QString m_label;
    QString m_tooltip;
    QString m_type;
    QStringList m_combo;
    QString m_suffix;

    QMetaType::Type m_metaType;

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(bool global READ global WRITE setGlobal NOTIFY globalChanged FINAL)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged FINAL)
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged FINAL)
    Q_PROPERTY(u32 id READ id FINAL CONSTANT)
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QStringList combo READ combo WRITE setCombo NOTIFY comboChanged FINAL)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged FINAL)

    Q_PROPERTY(u32 min READ min FINAL CONSTANT)
    Q_PROPERTY(u32 max READ max FINAL CONSTANT)
    Q_PROPERTY(QMLSetting *other READ other WRITE setOther NOTIFY otherChanged FINAL)
};

Q_DECLARE_METATYPE(QMLSetting *)

#endif // QMLSETTING_H

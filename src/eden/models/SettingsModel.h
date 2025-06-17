#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QAbstractListModel>
#include "eden/interface/QMLSetting.h"

class SettingsModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum SMTypes {
        LABEL = Qt::UserRole + 1,
        TOOLTIP,
        ID,
        VALUE,
        TYPE,
        COMBO, // combobox translations
        SUFFIX,
        MIN,
        MAX,
        OTHER
    };

    explicit SettingsModel(QObject* parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void append(QMLSetting *setting);
    void append(QList<QMLSetting *> settings);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<QMLSetting *> m_data;
};

#endif // SETTINGSMODEL_H

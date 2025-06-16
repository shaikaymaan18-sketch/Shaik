#include "SettingsModel.h"

SettingsModel::SettingsModel(QObject* parent) : QAbstractListModel(parent) {}

int SettingsModel::rowCount(const QModelIndex& parent) const {
    return m_data.count();
}

QVariant SettingsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    QMLSetting *s = m_data[index.row()];

    switch (role) {
    case LABEL:
        return s->label();
    case TOOLTIP:
        return s->tooltip();
    case VALUE:
        return s->value();
    case ID:
        return s->id();
    case TYPE:
        return s->type();
    case COMBO:
        return s->combo();
    case SUFFIX:
        return s->suffix();
    case MIN:
        return s->min();
    case MAX:
        return s->max();
    case OTHER:
        return QVariant::fromValue(s->other());
    default:
        break;
    }

    return QVariant();
}

bool SettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        QMLSetting *s = m_data[index.row()];

        switch (role) {
        case VALUE:
            s->setValue(value);

            break;
        }
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;

}

void SettingsModel::append(QMLSetting *setting)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_data << setting;
    endInsertRows();
}

void SettingsModel::append(QList<QMLSetting *> settings)
{
    for (QMLSetting *setting : settings) {
        append(setting);
    }
}

QHash<int, QByteArray> SettingsModel::roleNames() const
{
    QHash<int,QByteArray> rez;
    rez[LABEL] = "label";
    rez[TOOLTIP] = "tooltip";
    rez[VALUE] = "value";
    rez[ID] = "id";
    rez[TYPE] = "type";
    rez[COMBO] = "combo";
    rez[SUFFIX] = "suffix";
    rez[MIN] = "min";
    rez[MAX] = "max";
    rez[OTHER] = "other";

    return rez;
}

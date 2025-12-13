#pragma once

#include <QAbstractTableModel>

struct Dependency {
    QString name;
    QString version;
    QString url;
};

class DependencyModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit DependencyModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QList<Dependency> m_data;
};

// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <qnamespace.h>
#include "DependencyModel.h"

#include "dep_hashes.h"

DependencyModel::DependencyModel(QObject* parent) : QAbstractTableModel(parent) {
    for (size_t i = 0; i < Common::dep_hashes.size(); ++i) {
        QString name = QString::fromLocal8Bit(Common::dep_names.at(i));
        QString version = QString::fromLocal8Bit(Common::dep_hashes.at(i));
        QString url = QString::fromLocal8Bit(Common::dep_urls.at(i));

        m_data.append(Dependency{name, version, url});
    }
}

QVariant DependencyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical)
        return QString();

    switch (section) {
    case 0:
        return tr("Dependency");
    case 1:
    default:
        return tr("Version");
    }
}

int DependencyModel::rowCount(const QModelIndex& parent) const {
    return m_data.size();
}

int DependencyModel::columnCount(const QModelIndex& parent) const {
    return 2;
}

QVariant DependencyModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    Dependency d = m_data[index.row()];

    switch (index.column()) {
    case 0:
        return QStringLiteral("<a href=%1>%2</a>").arg(d.url, d.name);
    case 1:
    default:
        return d.version;
    }
}

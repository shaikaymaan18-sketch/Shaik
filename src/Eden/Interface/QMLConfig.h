#ifndef QMLCONFIG_H
#define QMLCONFIG_H

#include "qt_common/config/qt_config.h"

#include <QObject>
#include <qdebug.h>

class QMLConfig : public QObject {
    Q_OBJECT

    QtConfig *m_config;

public:
    QMLConfig()
        : m_config{new QtConfig}
    {}

    Q_INVOKABLE inline void reload() {
        qDebug() << "Reloading";
        m_config->ReloadAllValues();
    }
    Q_INVOKABLE inline void save() {
        qDebug() << "Saving";
        m_config->SaveAllValues();
    }
};

#endif // QMLCONFIG_H

#ifndef QMLCONFIG_H
#define QMLCONFIG_H

#include "qt_common/qt_config.h"

#include <QObject>

class QMLConfig : public QObject {
    Q_OBJECT

    QtConfig *m_config;

public:
    QMLConfig()
        : m_config{new QtConfig}
    {}

    Q_INVOKABLE inline void reload() {
        m_config->ReloadAllValues();
    }
    Q_INVOKABLE inline void save() {
        m_config->SaveAllValues();
    }
};

#endif // QMLCONFIG_H

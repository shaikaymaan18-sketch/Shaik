#pragma once

#include <QObject>

class MainWindowInterface : public QObject {
    Q_OBJECT
public:
    explicit MainWindowInterface(QObject* parent = nullptr);

    Q_INVOKABLE void installFirmware();
    Q_INVOKABLE void installFirmwareZip();
    Q_INVOKABLE void verifyIntegrity();
};

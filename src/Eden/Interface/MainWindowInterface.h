// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class GameListModel;
class MainWindowInterface : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool firmwareGood READ firmwareGood WRITE setFirmwareGood NOTIFY firmwareGoodChanged FINAL)
    Q_PROPERTY(QString firmwareTooltip READ firmwareTooltip WRITE setFirmwareTooltip NOTIFY
                   firmwareTooltipChanged FINAL)
    Q_PROPERTY(QString firmwareDisplay READ firmwareDisplay WRITE setFirmwareDisplay NOTIFY
                   firmwareDisplayChanged FINAL)

public:
    explicit MainWindowInterface(GameListModel* model, QObject* parent = nullptr);

    Q_INVOKABLE void installFirmware();
    Q_INVOKABLE void installFirmwareZip();
    Q_INVOKABLE void verifyIntegrity();
    Q_INVOKABLE void checkFirmwareDecryption();

    Q_INVOKABLE void installDecryptionKeys();

    Q_INVOKABLE void createHomeMenuApplicationMenuShortcut();
    Q_INVOKABLE void createHomeMenuDesktopShortcut();

    Q_INVOKABLE void openURL(const QUrl& url);
    Q_INVOKABLE void openModsPage();
    Q_INVOKABLE void openQuickstartGuide();
    Q_INVOKABLE void openFAQ();

    bool firmwareGood() const;
    void setFirmwareGood(bool newFirmwareGood);

    QString firmwareTooltip() const;
    void setFirmwareTooltip(const QString& newFirmwareTooltip);

    QString firmwareDisplay() const;
    void setFirmwareDisplay(const QString& newFirmwareDisplay);

public slots:
    void openRootDataFolder();
    void openNANDFolder();
    void openSDMCFolder();
    void openModFolder();
    void openLogFolder();

signals:
    void firmwareGoodChanged(bool firmwareGood);
    void firmwareTooltipChanged(QString firmwareTooltip);
    void firmwareDisplayChanged(QString firmwareDisplay);

private:
    GameListModel* m_gameList;
    void setFirmwareVersion();

    bool m_firmwareGood = false;

    QString m_firmwareDisplay{};
    QString m_firmwareTooltip{};
};

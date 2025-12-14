// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQuickWindow>
#include "core/hle/service/am/applet_manager.h"
#include "qt_common/qt_common.h"

namespace InputCommon {
class InputSubsystem;
}
class RenderWindow;
class GameListModel;
class QMLConfig;

class MainWindowInterface : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool firmwareGood READ firmwareGood WRITE setFirmwareGood NOTIFY firmwareGoodChanged FINAL)
    Q_PROPERTY(QString firmwareTooltip READ firmwareTooltip WRITE setFirmwareTooltip NOTIFY
                   firmwareTooltipChanged FINAL)
    Q_PROPERTY(QString firmwareDisplay READ firmwareDisplay WRITE setFirmwareDisplay NOTIFY
                   firmwareDisplayChanged FINAL)

public:
    explicit MainWindowInterface(GameListModel* model, QMLConfig* config, QQuickWindow* rootWindow,
                                 QObject* parent = nullptr);

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

    Q_INVOKABLE void openHomeMenu();

    Q_INVOKABLE void bootGame(const QString& filename, Service::AM::FrontendAppletParameters params,
                              StartGameType type = StartGameType::Normal);
    Q_INVOKABLE bool loadROM(const QString& filename, Service::AM::FrontendAppletParameters params);

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
    QMLConfig* m_config;
    RenderWindow* m_renderWindow;
    std::shared_ptr<InputCommon::InputSubsystem> input_subsystem;

    // Whether emulation is currently running in yuzu.
    bool emulation_running = false;
    // The path to the game currently running
    QString current_game_path;
    // Whether a user was set on the command line (skips UserSelector if it's forced to show up)
    bool user_flag_cmd_line = false;

    // Last game booted, used for multi-process apps
    QString last_filename_booted;

    bool m_firmwareGood = false;

    QString m_firmwareDisplay{};
    QString m_firmwareTooltip{};

    void shutdownGame();
    void setFirmwareVersion();
    void executeProgram(std::size_t program_index);
    Service::AM::FrontendAppletParameters applicationAppletParameters();
    Service::AM::FrontendAppletParameters libraryAppletParameters(u64 program_id,
                                                                  Service::AM::AppletId applet_id);
};

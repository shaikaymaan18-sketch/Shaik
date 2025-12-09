// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Eden/Models/GameListModel.h"
#include "MainWindowInterface.h"
#include "frontend_common/content_manager.h"

#include "qt_common/util/content.h"
#include "qt_common/util/game.h"

#include "qt_common/abstract/frontend.h"

MainWindowInterface::MainWindowInterface(GameListModel* model, QObject* parent)
    : QObject{parent}, m_gameList(model) {
    checkFirmwareDecryption();
}

void MainWindowInterface::installFirmware() {
    QtCommon::Content::InstallFirmware();
    checkFirmwareDecryption();
}

void MainWindowInterface::installFirmwareZip() {
    QtCommon::Content::InstallFirmwareZip();
    checkFirmwareDecryption();
}

void MainWindowInterface::verifyIntegrity() {
    QtCommon::Content::VerifyInstalledContents();
}

void MainWindowInterface::checkFirmwareDecryption() {
    if (!ContentManager::AreKeysPresent()) {
        QtCommon::Frontend::Warning(tr("Derivation Components Missing"),
                                    tr("Encryption keys are missing."));
    }

    setFirmwareVersion();
}

void MainWindowInterface::installDecryptionKeys() {
    QtCommon::Content::InstallKeys();

    m_gameList->populateAsync(UISettings::values.game_dirs);
    checkFirmwareDecryption();
}

void MainWindowInterface::setFirmwareVersion() {
    const auto pair = FirmwareManager::GetFirmwareVersion(*QtCommon::system.get());
    const auto firmware_data = pair.first;
    const auto result = pair.second;

    if (result.IsError() || !FirmwareManager::CheckFirmwarePresence(*QtCommon::system.get())) {
        LOG_INFO(Frontend, "Installed firmware: No firmware available");
        setFirmwareGood(false);
        return;
    }

    setFirmwareGood(true);

    const std::string display_version(firmware_data.display_version.data());
    const std::string display_title(firmware_data.display_title.data());

    LOG_INFO(Frontend, "Installed firmware: {}", display_version);

    setFirmwareDisplay(QString::fromStdString(display_version));
    setFirmwareTooltip(QString::fromStdString(display_title));
}

void MainWindowInterface::openRootDataFolder() {
    QtCommon::Game::OpenRootDataFolder();
}

void MainWindowInterface::openNANDFolder()
{
    QtCommon::Game::OpenNANDFolder();
}

void MainWindowInterface::openSDMCFolder()
{
    QtCommon::Game::OpenSDMCFolder();
}

void MainWindowInterface::openModFolder()
{
    QtCommon::Game::OpenModFolder();
}

void MainWindowInterface::openLogFolder()
{
    QtCommon::Game::OpenLogFolder();
}


QString MainWindowInterface::firmwareDisplay() const {
    return m_firmwareDisplay;
}

void MainWindowInterface::setFirmwareDisplay(const QString& newFirmwareDisplay) {
    if (m_firmwareDisplay == newFirmwareDisplay)
        return;
    m_firmwareDisplay = newFirmwareDisplay;
    emit firmwareDisplayChanged(m_firmwareDisplay);
}

QString MainWindowInterface::firmwareTooltip() const {
    return m_firmwareTooltip;
}

void MainWindowInterface::setFirmwareTooltip(const QString& newFirmwareTooltip) {
    if (m_firmwareTooltip == newFirmwareTooltip)
        return;
    m_firmwareTooltip = newFirmwareTooltip;
    emit firmwareTooltipChanged(m_firmwareTooltip);
}

bool MainWindowInterface::firmwareGood() const {
    return m_firmwareGood;
}

void MainWindowInterface::setFirmwareGood(bool newFirmwareGood) {
    if (m_firmwareGood == newFirmwareGood)
        return;
    m_firmwareGood = newFirmwareGood;
    emit firmwareGoodChanged(m_firmwareGood);
}

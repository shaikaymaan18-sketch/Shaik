// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Eden/Models/GameListModel.h"
#include "MainWindowInterface.h"
#include "QMLConfig.h"
#include "RenderWindow.h"
#include "common/string_util.h"
#include "core/hle/kernel/k_process.h"
#include "frontend_common/content_manager.h"

#include "hid_core/hid_core.h"
#include "qt_common/qt_string_lookup.h"
#include "qt_common/util/content.h"
#include "qt_common/util/game.h"

#include "qt_common/abstract/frontend.h"
#include "qt_common/qt_constants.h"

// Applets //
#include "core/frontend/applets/cabinet.h"
#include "core/frontend/applets/controller.h"
#include "core/frontend/applets/error.h"
#include "core/frontend/applets/general.h"
#include "core/frontend/applets/mii_edit.h"
#include "core/frontend/applets/profile_select.h"
#include "core/frontend/applets/software_keyboard.h"
#include "core/frontend/applets/web_browser.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"

#include <QDesktopServices>

MainWindowInterface::MainWindowInterface(GameListModel* model, QMLConfig* config,
                                         QQuickWindow* rootWindow, QObject* parent)
    : QObject{parent}, m_gameList(model), m_config(config),
      input_subsystem{std::make_shared<InputCommon::InputSubsystem>()} {
    m_renderWindow = new RenderWindow(rootWindow, input_subsystem);
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

// TODO(qml): The following are basically all identical to main_window.cpp
// Is there any way we can combine these?
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

void MainWindowInterface::createHomeMenuDesktopShortcut() {
    QtCommon::Game::CreateHomeMenuShortcut(QtCommon::Game::ShortcutTarget::Desktop);
}

void MainWindowInterface::createHomeMenuApplicationMenuShortcut() {
    QtCommon::Game::CreateHomeMenuShortcut(QtCommon::Game::ShortcutTarget::Applications);
}

void MainWindowInterface::openURL(const QUrl& url) {
    const bool open = QDesktopServices::openUrl(url);
    if (!open) {
        QtCommon::Frontend::Warning(tr("Error opening URL"),
                             tr("Unable to open the URL \"%1\".").arg(url.toString()));
    }
}

void MainWindowInterface::openModsPage() {
    openURL(QUrl(QString::fromLocal8Bit(QtCommon::Constants::modPage)));
}

void MainWindowInterface::openQuickstartGuide() {
    openURL(QUrl(QString::fromLocal8Bit(QtCommon::Constants::quickstartPage)));
}

void MainWindowInterface::openFAQ() {
    openURL(QUrl(QString::fromLocal8Bit(QtCommon::Constants::helpPage)));
}

void MainWindowInterface::openHomeMenu() {
    auto result = FirmwareManager::VerifyFirmware(*QtCommon::system.get());

    using namespace QtCommon::StringLookup;

    switch (result) {
    case FirmwareManager::ErrorFirmwareMissing:
        QtCommon::Frontend::Warning(this, tr("No firmware available"),
                             Lookup(FwCheckErrorFirmwareMissing));
        return;
    case FirmwareManager::ErrorFirmwareCorrupted:
        QtCommon::Frontend::Warning(this, tr("Firmware Corrupted"),
                             Lookup(FwCheckErrorFirmwareCorrupted));
        return;
    default:
        break;
    }

    constexpr u64 QLaunchId = static_cast<u64>(Service::AM::AppletProgramId::QLaunch);
    auto bis_system = QtCommon::system->GetFileSystemController().GetSystemNANDContents();

    auto qlaunch_applet_nca = bis_system->GetEntry(QLaunchId, FileSys::ContentRecordType::Program);
    if (!qlaunch_applet_nca) {
        QtCommon::Frontend::Warning(this, tr("Home Menu Applet"),
                             tr("Home Menu is not available. Please reinstall firmware."));
        return;
    }

    QtCommon::system->GetFrontendAppletHolder().SetCurrentAppletId(Service::AM::AppletId::QLaunch);

    const auto filename = QString::fromStdString((qlaunch_applet_nca->GetFullPath()));
    UISettings::values.roms_path = QFileInfo(filename).path().toStdString();
    bootGame(filename, libraryAppletParameters(QLaunchId, Service::AM::AppletId::QLaunch));
}

Service::AM::FrontendAppletParameters MainWindowInterface::applicationAppletParameters() {
    return Service::AM::FrontendAppletParameters{
        .applet_id = Service::AM::AppletId::Application,
        .applet_type = Service::AM::AppletType::Application,
    };
}

Service::AM::FrontendAppletParameters MainWindowInterface::libraryAppletParameters(
    u64 program_id, Service::AM::AppletId applet_id) {
    return Service::AM::FrontendAppletParameters{
        .program_id = program_id,
        .applet_id = applet_id,
        .applet_type = Service::AM::AppletType::LibraryApplet,
    };
}

void MainWindowInterface::executeProgram(std::size_t program_index) {
    shutdownGame();

    auto params = applicationAppletParameters();
    params.program_index = static_cast<s32>(program_index);
    params.launch_type = Service::AM::LaunchType::ApplicationInitiated;
    bootGame(last_filename_booted, params);
}

void MainWindowInterface::bootGame(const QString& filename,
                                   Service::AM::FrontendAppletParameters params,
                                   StartGameType type) {
    LOG_INFO(Frontend, "Eden starting...");

    // if (params.program_id == 0 ||
    //     params.program_id > static_cast<u64>(Service::AM::AppletProgramId::MaxProgramId)) {
    //     StoreRecentFile(filename); // Put the filename on top of the list
    // }

    // Save configurations
    // UpdateUISettings();
    m_config->save();

    u64 title_id{0};

    // last_filename_booted = filename;

    QtCommon::Content::configureFilesystemProvider(filename.toStdString());
    const auto v_file = Core::GetGameFileFromPath(QtCommon::vfs, filename.toUtf8().constData());
    const auto loader =
        Loader::GetLoader(*QtCommon::system, v_file, params.program_id, params.program_index);

    if (loader != nullptr && loader->ReadProgramId(title_id) == Loader::ResultStatus::Success &&
        type == StartGameType::Normal) {
        // Load per game settings
        const auto file_path =
            std::filesystem::path{Common::U16StringFromBuffer(filename.utf16(), filename.size())};
        const auto config_file_name = title_id == 0
                                          ? Common::FS::PathToUTF8String(file_path.filename())
                                          : fmt::format("{:016X}", title_id);
        QtConfig per_game_config(config_file_name, Config::ConfigType::PerGameConfig);
        QtCommon::system->HIDCore().ReloadInputDevices();
        QtCommon::system->ApplySettings();
    }

    Settings::LogSettings();

    // if (UISettings::values.select_user_on_boot && !user_flag_cmd_line) {
    //     const Core::Frontend::ProfileSelectParameters parameters{
    //                                                              .mode =
    //                                                              Service::AM::Frontend::UiMode::UserSelector,
    //                                                              .invalid_uid_list = {},
    //                                                              .display_options = {},
    //                                                              .purpose =
    //                                                              Service::AM::Frontend::UserSelectionPurpose::General,
    //                                                              };
    //     if (SelectAndSetCurrentUser(parameters) == false) {
    //         return;
    //     }
    // }

    if (!loadROM(filename, params)) {
        return;
    }

    qDebug() << "Successfully loaded ROM from" << filename;

    QtCommon::system->SetShuttingDown(false);

    // Create and start the emulation thread
    QtCommon::emu_thread = std::make_unique<EmuThread>();
    // emit EmulationStarting();
    QtCommon::emu_thread->start();

    // Register an ExecuteProgram callback such that Core can execute a sub-program
    QtCommon::system->RegisterExecuteProgramCallback(
        [this](std::size_t program_index_) { executeProgram(program_index_); });

    QtCommon::system->RegisterExitCallback([this] {
        QtCommon::emu_thread->ForceStop();
        shutdownGame();
    });

    // connect(render_window, &GRenderWindow::Closed, this, &MainWindow::OnStopGame);
    // connect(render_window, &GRenderWindow::MouseActivity, this, &MainWindow::OnMouseActivity);

    connect(QtCommon::emu_thread.get(), &EmuThread::LoadProgress, this,
            [](VideoCore::LoadCallbackStage stage, std::size_t value, std::size_t total) {
                qDebug() << (int) stage << value << total;
            });

    //     connect(QtCommon::emu_thread.get(), &EmuThread::LoadProgress, loading_screen,
            // &LoadingScreen::OnLoadProgress, Qt::QueuedConnection);

    //        // Update the GUI
    // UpdateStatusButtons();
    // if (ui->action_Single_Window_Mode->isChecked()) {
    //     game_list->hide();
    //     game_list_placeholder->hide();
    // }
    // status_bar_update_timer.start(500);
    // renderer_status_button->setDisabled(true);
    // refresh_button->setDisabled(true);

    // if (UISettings::values.hide_mouse || Settings::values.mouse_panning) {
    //     render_window->installEventFilter(render_window);
    //     render_window->setAttribute(Qt::WA_Hover, true);
    // }

    // if (UISettings::values.hide_mouse) {
    //     mouse_hide_timer.start();
    // }

    // render_window->InitializeCamera();

    std::string title_name;
    std::string title_version;
    const auto res = QtCommon::system->GetGameName(title_name);

    const auto metadata = [title_id] {
        const FileSys::PatchManager pm(title_id, QtCommon::system->GetFileSystemController(),
                                       QtCommon::system->GetContentProvider());
        return pm.GetControlMetadata();
    }();
    if (metadata.first != nullptr) {
        title_version = metadata.first->GetVersionString();
        title_name = metadata.first->GetApplicationName();
    }
    if (res != Loader::ResultStatus::Success || title_name.empty()) {
        title_name = Common::FS::PathToUTF8String(
            std::filesystem::path{Common::U16StringFromBuffer(filename.utf16(), filename.size())}
                .filename());
    }
    const bool is_64bit = QtCommon::system->Kernel().ApplicationProcess()->Is64Bit();
    const auto instruction_set_suffix = is_64bit ? tr("(64-bit)") : tr("(32-bit)");
    title_name = tr("%1 %2", "%1 is the title name. %2 indicates if the title is 64-bit or 32-bit")
                     .arg(QString::fromStdString(title_name), instruction_set_suffix)
                     .toStdString();
    LOG_INFO(Frontend, "Booting game: {:016X} | {} | {}", title_id, title_name, title_version);
    const auto gpu_vendor = QtCommon::system->GPU().Renderer().GetDeviceVendor();

    // TODO
    // UpdateWindowTitle(title_name, title_version, gpu_vendor);

    // loading_screen->Prepare(QtCommon::system->GetAppLoader());
    // loading_screen->show();

    emulation_running = true;
    // if (ui->action_Fullscreen->isChecked()) {
    //     ShowFullscreen();
    // }
    // OnStartGame();
    QtCommon::emu_thread->SetRunning(true);
}

bool MainWindowInterface::loadROM(const QString& filename, Service::AM::FrontendAppletParameters params) {
    // Shutdown previous session if the emu thread is still active...
    if (QtCommon::emu_thread != nullptr) {
        shutdownGame();
    }

    if (!m_renderWindow->initRenderTarget()) {
        return false;
    }

    QtCommon::system->SetFilesystem(QtCommon::vfs);

    if (params.launch_type == Service::AM::LaunchType::FrontendInitiated) {
        QtCommon::system->GetUserChannel().clear();
    }

    QtCommon::system->SetFrontendAppletSet({
        nullptr, // Amiibo Settings
        nullptr, // Controller Selector
        nullptr, // Error Display
        nullptr, // Mii Editor
        nullptr, // Parental Controls
        nullptr, // Photo Viewer
        nullptr, // Profile Selector
        nullptr, // Software Keyboard
        nullptr, // Web Browser
        nullptr, // Net Connect
    });

    /** firmware check */
    if (!QtCommon::Content::CheckGameFirmware(params.program_id, this)) {
        return false;
    }

    /** Exec */
    const Core::SystemResultStatus result{
                                          QtCommon::system->Load(*m_renderWindow, filename.toStdString(), params)};

    if (result != Core::SystemResultStatus::Success) {
        switch (result) {
        case Core::SystemResultStatus::ErrorGetLoader:
            LOG_CRITICAL(Frontend, "Failed to obtain loader for {}!", filename.toStdString());
            QtCommon::Frontend::Critical(tr("Error while loading ROM!"),
                                  tr("The ROM format is not supported."));
            break;
        case Core::SystemResultStatus::ErrorVideoCore:
            QtCommon::Frontend::Critical(
                tr("An error occurred initializing the video core."),
                tr("Eden has encountered an error while running the video core. "
                   "This is usually caused by outdated GPU drivers, including integrated ones. "
                   "Please see the log for more details. "
                   "For more information on accessing the log, please see the following page: "
                   "<a href='https://yuzu-mirror.github.io/help/reference/log-files/'>"
                   "How to Upload the Log File</a>. "));
            break;
        default:
            if (result > Core::SystemResultStatus::ErrorLoader) {
                const u16 loader_id = static_cast<u16>(Core::SystemResultStatus::ErrorLoader);
                const u16 error_id = static_cast<u16>(result) - loader_id;
                const std::string error_code = fmt::format("({:04X}-{:04X})", loader_id, error_id);
                LOG_CRITICAL(Frontend, "Failed to load ROM! {}", error_code);

                const auto title =
                    tr("Error while loading ROM! %1", "%1 signifies a numeric error code.")
                        .arg(QString::fromStdString(error_code));
                const auto description =
                    tr("%1<br>Please redump your files or ask on Discord/Revolt for help.",
                       "%1 signifies an error string.")
                        .arg(QString::fromStdString(
                            GetResultStatusString(static_cast<Loader::ResultStatus>(error_id))));

                QtCommon::Frontend::Critical(title, description);
            } else {
                QtCommon::Frontend::Critical(
                    tr("Error while loading ROM!"),
                    tr("An unknown error occurred. Please see the log for more details."));
            }
            break;
        }
        return false;
    }
    current_game_path = filename;

    return true;
}

QString MainWindowInterface::lookup(QtCommon::StringLookup::StringKey key) {
    return QtCommon::StringLookup::Lookup(key);
}

void MainWindowInterface::shutdownGame() {
    if (!emulation_running) {
        return;
    }

    // STUBBED

   // TODO(crueter): make this common as well (frontend_common?)
    // play_time_manager->Stop();
    // OnShutdownBegin();
    // OnEmulationStopTimeExpired();
    // OnEmulationStopped();
}

/// PROPERTIES ///
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

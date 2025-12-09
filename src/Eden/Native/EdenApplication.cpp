// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Eden/Interface/MainWindowInterface.h"
#include "EdenApplication.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "CarboxylApplication.h"

#include "Eden/Interface/QMLConfig.h"
#include "Eden/Interface/SettingsInterface.h"
#include "Eden/Interface/TitleManager.h"
#include "Eden/Models/GameListModel.h"

#include "common/settings_enums.h"
#include "qt_common/config/uisettings.h"
#include "qt_common/qt_common.h"
#include "qt_common/util/content.h"

#include <QQuickStyle>
#include <QWidget>

static constexpr const int EXIT_RELOAD = -2;

EdenApplication::EdenApplication(int &argc, char *argv[])
    : QApplication(argc, argv)
{
    QCoreApplication::setOrganizationName(QStringLiteral("eden-emu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));
    QApplication::setDesktopFileName(QStringLiteral("dev.eden-emu.eden"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/eden.svg")));

    /// QtCommon
    QtCommon::Init(new QWidget);

    QtCommon::SetupContentProviders();

    // Check for orphaned profiles and reset profile data if necessary
    QtCommon::Content::FixProfiles();

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    config = new QMLConfig;

    // TODO: Save all values on launch and per game etc
    connect(this, &QCoreApplication::aboutToQuit, this, [this]() { config->save(); });
}

void EdenApplication::reload()
{
    exit(EXIT_RELOAD);
}

int EdenApplication::run() {
    int ret = EXIT_SUCCESS;
    do {
        if (ret == EXIT_RELOAD)
            qmlClearTypeRegistrations();

        QQmlApplicationEngine engine;

        // carboxyl setup
        auto translations = ConfigurationShared::ComboboxEnumeration(this);

        const auto enumeration = &translations->at(Settings::EnumMetadata<Settings::Style>::Index());
        QString style;
        for (const auto &[idx, name] : *enumeration) {
            if (idx == (u32) UISettings::values.carboxyl_style.GetValue()) {
                style = name;
            }
        }

        CarboxylApplication *carboxylApp = new CarboxylApplication(*this,
                                                                   &engine,
                                                                   style,
                                                                   QStringLiteral("Helios"));
        carboxylApp->setParent(this);

        /// CONTEXT
        auto ctx = engine.rootContext();

        ctx->setContextProperty(QStringLiteral("QtConfig"), QVariant::fromValue(config));

        // Enums
        qmlRegisterUncreatableMetaObject(SettingsCategories::staticMetaObject,
                                         "Eden.Interface",
                                         1,
                                         0,
                                         "SettingsCategories",
                                         QString());

        // Directory List
        GameListModel *gameListModel = new GameListModel(this, &engine);
        ctx->setContextProperty(QStringLiteral("EdenGameList"), gameListModel);

        // Settings Interface
        SettingsInterface *interface = new SettingsInterface(&engine);
        ctx->setContextProperty(QStringLiteral("SettingsInterface"), interface);

        // Title Manager
        TitleManager *title = new TitleManager(&engine);
        ctx->setContextProperty(QStringLiteral("TitleManager"), title);

        // MainWindow interface
        MainWindowInterface* mwint = new MainWindowInterface(gameListModel, &engine);
        ctx->setContextProperty(QStringLiteral("MainWindowInterface"), mwint);

        // :)
        ctx->setContextProperty(QStringLiteral("EdenApplication"), this);

        /// LOAD
        QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            this,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);

        engine.loadFromModule("Eden.Main", "Main");

        ret = exec();

    } while (ret == EXIT_RELOAD);

    return ret;
}

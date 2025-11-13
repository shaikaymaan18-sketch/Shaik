// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// Qt on macOS doesn't define VMA shit
#if defined(QT_STATICPLUGIN) && !defined(__APPLE__)
#undef VMA_IMPLEMENTATION
#endif

#include <QApplication>
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

#include <QQuickStyle>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QCoreApplication::setOrganizationName(QStringLiteral("eden-emu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));
    QApplication::setDesktopFileName(QStringLiteral("dev.eden-emu.eden"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/eden.svg")));

    /// QtCommon
    QtCommon::Init(new QWidget);

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    QMLConfig *config = new QMLConfig;

    // TODO: Save all values on launch and per game etc
    app.connect(&app, &QCoreApplication::aboutToQuit, &app, [config]() {
        config->save();
    });

    // carboxyl setup
    auto translations = ConfigurationShared::ComboboxEnumeration(&app);

    const auto enumeration = &translations->at(Settings::EnumMetadata<Settings::Style>::Index());
    QString style;
    for (const auto &[idx, name] : *enumeration) {
        if (idx == (u32) UISettings::values.carboxyl_style.GetValue()) {
            style = name;
        }
    }

    CarboxylApplication *carboxylApp = new CarboxylApplication(app, &engine, style, QStringLiteral("Trioxide"));
    carboxylApp->setParent(&app);

    /// CONTEXT
    auto ctx = engine.rootContext();

    ctx->setContextProperty(QStringLiteral("QtConfig"), QVariant::fromValue(config));

    // Enums
    qmlRegisterUncreatableMetaObject(SettingsCategories::staticMetaObject, "Eden.Interface", 1, 0, "SettingsCategories", QString());

    // Directory List
    GameListModel *gameListModel = new GameListModel(&app, &engine);
    ctx->setContextProperty(QStringLiteral("EdenGameList"), gameListModel);

    // Settings Interface
    SettingsInterface *interface = new SettingsInterface(&engine);
    ctx->setContextProperty(QStringLiteral("SettingsInterface"), interface);

    // Title Manager
    TitleManager *title = new TitleManager(&engine);
    ctx->setContextProperty(QStringLiteral("TitleManager"), title);

    /// LOAD
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Eden.Main", "Main");

    return app.exec();
}

#if !defined(QT_STATICPLUGIN) || defined(__APPLE__)
#define VMA_IMPLEMENTATION
#include "video_core/vulkan_common/vma.h"
#endif

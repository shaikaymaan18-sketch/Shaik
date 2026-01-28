// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Eden/Interface/MainWindowInterface.h"
#include "Eden/Models/DependencyModel.h"
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
#include "qt_common/qt_string_lookup.h"
#include "qt_common/util/content.h"
#include "qt_common/util/game.h"

#include <QProcess>
#include <QQuickStyle>
#include <QTimer>
#include <QWidget>
#include <qapplication.h>
#include <qqmlapplicationengine.h>

EdenApplication::EdenApplication(int& argc, char* argv[]) : QApplication(argc, argv) {
    QCoreApplication::setOrganizationName(QStringLiteral("eden-emu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));
    QApplication::setDesktopFileName(QStringLiteral("dev.eden-emu.eden"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/eden.svg")));

    /// QtCommon
    m_widget = new QWidget;
    QtCommon::Init(m_widget);

    QtCommon::SetupContentProviders();

    // Check for orphaned profiles and reset profile data if necessary
    QtCommon::Content::FixProfiles();

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    config = new QMLConfig;

    // TODO: Save all values on launch and per game etc
    connect(this, &QCoreApplication::aboutToQuit, this, [this]() { config->save(); });

    m_engine = new QQmlApplicationEngine(this);

    // carboxyl setup
    auto translations = ConfigurationShared::ComboboxEnumeration(this);

    const auto enumeration = &translations->at(Settings::EnumMetadata<Settings::Style>::Index());
    QString style;
    for (const auto& [idx, name] : *enumeration) {
        if (idx == (u32)UISettings::values.carboxyl_style.GetValue()) {
            style = name;
        }
    }

    CarboxylApplication* carboxylApp =
        new CarboxylApplication(*this, m_engine, style, QStringLiteral("Graphide"));
    carboxylApp->setParent(this);

    /// CONTEXT
    auto ctx = m_engine->rootContext();

    ctx->setContextProperty(QStringLiteral("QtConfig"), QVariant::fromValue(config));

    // Enums
    qmlRegisterUncreatableMetaObject(SettingsCategories::staticMetaObject, "Eden.Interface", 1, 0,
                                     "SettingsCategories", QString());

    qmlRegisterUncreatableMetaObject(QtCommon::StringLookup::staticMetaObject, "Eden.Interface", 1,
                                     0, "StringKey", QString());

    qmlRegisterUncreatableMetaObject(QtCommon::Game::staticMetaObject, "Eden.Interface", 1, 0,
                                     "DataDirectory", QString());

    // Directory List
    gameListModel = new GameListModel(this, m_engine);
    ctx->setContextProperty(QStringLiteral("EdenGameList"), gameListModel);

    // Dependency Model
    DependencyModel* depModel = new DependencyModel(this);
    ctx->setContextProperty(QStringLiteral("DependencyModel"), depModel);

    // Settings Interface
    SettingsInterface* interface = new SettingsInterface(m_engine);
    ctx->setContextProperty(QStringLiteral("SettingsInterface"), interface);

    // Title Manager
    TitleManager* title = new TitleManager(m_engine);
    ctx->setContextProperty(QStringLiteral("TitleManager"), title);

    // :)
    ctx->setContextProperty(QStringLiteral("EdenApplication"), this);

    /// LOAD
    QObject::connect(
        m_engine, &QQmlApplicationEngine::objectCreationFailed, this,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
}

void EdenApplication::reload() {
    QString program = QApplication::applicationFilePath();
    QStringList args = QApplication::arguments().mid(1);

    config->save();

    QProcess process;
    process.setProgram(program);
    process.setArguments(args);

    // Sometimes IDEs will just straight-up eat your process if you keep the same output device :/
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());

    bool success = process.startDetached();

    if (success) {
        quit();
    }
}

int EdenApplication::run() {
    m_engine->loadFromModule("Eden.Main", "Main");

    // MainWindow interface
    QObject* root = m_engine->rootObjects()[0];
    QQuickWindow* window = qobject_cast<QQuickWindow*>(root);

    if (!window) {
        qFatal("Error: Your root item has to be a window.");
        return -1;
    }

    MainWindowInterface* mwint = new MainWindowInterface(gameListModel, config, window, m_engine);
    m_engine->rootContext()->setContextProperty(QStringLiteral("MainWindowInterface"), mwint);

    int ret = exec();
    m_widget->deleteLater();
    return ret;
}

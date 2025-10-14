#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Interface/QMLConfig.h"
#include "Interface/SettingsInterface.h"
#include "Interface/TitleManager.h"
#include "Models/GameListModel.h"
#include "core/core.h"
#include "qt_common/qt_common.h"

#include <QQuickStyle>
#include <qwidget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickStyle::setStyle(QObject::tr("Material"));

    QCoreApplication::setOrganizationName(QStringLiteral("eden-emu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));
    QApplication::setDesktopFileName(QStringLiteral("org.eden-emu.eden"));
    QGuiApplication::setWindowIcon(QIcon(":/icons/eden.svg"));

    /// QtCommon
    QtCommon::Init(new QWidget);

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    QMLConfig *config = new QMLConfig;

    // TODO: Save all values on launch and per game etc
    app.connect(&app, &QCoreApplication::aboutToQuit, &app, [config]() {
        config->save();
    });

    /// Expose Enums

    // Core
    std::unique_ptr<Core::System> system = std::make_unique<Core::System>();

    /// CONTEXT
    QQmlApplicationEngine engine;
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

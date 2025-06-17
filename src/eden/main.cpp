#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core/core.h"
#include "interface/QMLConfig.h"
#include "models/GameListModel.h"
#include "interface/SettingsInterface.h"

#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickStyle::setStyle(QObject::tr("Material"));

    QCoreApplication::setOrganizationName(QStringLiteral("eden-emu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));
    QApplication::setDesktopFileName(QStringLiteral("org.eden-emu.eden"));

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    QMLConfig *config = new QMLConfig;

    // // TODO: Save all values on launch and per game etc
    // app.connect(&app, &QCoreApplication::aboutToQuit, &app, [config]() {
    //     config->save();
    // });

    /// Expose Enums

    // Core
    std::unique_ptr<Core::System> system = std::make_unique<Core::System>();

    /// CONTEXT
    QQmlApplicationEngine engine;
    auto ctx = engine.rootContext();

    ctx->setContextProperty(QStringLiteral("QtConfig"), QVariant::fromValue(config));

    // Enums
    qmlRegisterUncreatableMetaObject(SettingsCategories::staticMetaObject, "org.eden_emu.interface", 1, 0, "SettingsCategories", QString());

    // Directory List
    GameListModel *gameListModel = new GameListModel(&app);
    ctx->setContextProperty(QStringLiteral("EdenGameList"), gameListModel);

    /// LOAD
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("org.eden_emu.main", "Main");

    return app.exec();
}

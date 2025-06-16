#include "core/core.h"
#include "interface/SettingsInterface.h"
#include "interface/qt_config.h"
#include "models/GameListModel.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickStyle::setStyle(QObject::tr("Material"));

    QCoreApplication::setOrganizationName(QStringLiteral("yuzu"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));

    /// Settings, etc
    Settings::SetConfiguringGlobal(true);
    QtConfig *config = new QtConfig;
    config->SaveAllValues();

    // TODO: Save all values on launch and per game etc
    app.connect(&app, &QCoreApplication::aboutToQuit, &app, [config]() {
        config->SaveAllValues();
    });

    /// Expose Enums

    // Core
    std::unique_ptr<Core::System> system = std::make_unique<Core::System>();

    /// CONTEXT
    QQmlApplicationEngine engine;

    // Enums
    qmlRegisterUncreatableMetaObject(SettingsCategories::staticMetaObject, "org.eden_emu.interface", 1, 0, "SettingsCategories", QString());

    // Directory List
    GameListModel *gameListModel = new GameListModel(&app);
    engine.rootContext()->setContextProperty(QStringLiteral("EdenGameList"), gameListModel);

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

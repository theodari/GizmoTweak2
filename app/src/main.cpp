#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDir>
#include <QtQml/qqmlextensionplugin.h>

#include "gizmotweaklib2.h"
#include "engine/LaserEngine.h"
#include "ExcaliburEngine.h"
#include "RecentFilesManager.h"

Q_IMPORT_QML_PLUGIN(GizmoTweakLib2Plugin)

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName("GizmoTweak2 (Excalibur)");
    app.setApplicationVersion(gizmotweak2::version());
    app.setOrganizationName("Excalibur Laser Systems");

    QQuickStyle::setStyle("Basic");

    // Create the laser engine
    auto* laserEngine = new gizmotweak2::ExcaliburEngine();

    // Auto-connect at startup
    laserEngine->connect();

    // Create the recent files manager
    auto* recentFilesManager = new RecentFilesManager();

    QQmlApplicationEngine engine;

    // Expose the laser engine to QML
    engine.rootContext()->setContextProperty("laserEngine", laserEngine);

    // Expose the recent files manager to QML
    engine.rootContext()->setContextProperty("recentFiles", recentFilesManager);

    // Ajouter le chemin vers les modules QML de la lib
    engine.addImportPath(QCoreApplication::applicationDirPath());
    engine.addImportPath("qrc:/");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    return app.exec();
}

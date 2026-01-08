#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDir>
#include <QtQml/qqmlextensionplugin.h>

#include "gizmotweaklib2.h"

Q_IMPORT_QML_PLUGIN(GizmoTweakLib2Plugin)

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName("GizmoTweak2");
    app.setApplicationVersion(gizmotweak2::version());
    app.setOrganizationName("Excalibur Laser Systems");

    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;

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

#pragma once

#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

/**
 * @brief Abstract interface for laser output engines.
 *
 * This class defines the interface that must be implemented by
 * concrete laser engine implementations (e.g., IkkonixEngine, ExcaliburEngine).
 *
 * The engine is responsible for:
 * - Discovering and listing available output zones
 * - Sending frame data to the laser hardware
 * - Managing connection state
 */
class LaserEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("LaserEngine is an abstract interface")

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList zones READ zones NOTIFY zonesChanged)
    Q_PROPERTY(QString engineName READ engineName CONSTANT)

public:
    explicit LaserEngine(QObject* parent = nullptr) : QObject(parent) {}
    ~LaserEngine() override = default;

    // Engine identification
    virtual QString engineName() const = 0;

    // Connection management
    virtual bool isConnected() const = 0;
    Q_INVOKABLE virtual bool connect() = 0;
    Q_INVOKABLE virtual void disconnect() = 0;

    // Zone management
    virtual QStringList zones() const = 0;
    Q_INVOKABLE virtual int zoneCount() const = 0;

    // Frame output
    // Points are in normalized coordinates [-1, +1] with RGB [0, 1]
    Q_INVOKABLE virtual bool sendFrame(int zoneIndex, const QVariantList& points) = 0;

    // Laser control
    Q_INVOKABLE virtual void setLaserEnabled(int zoneIndex, bool enabled) = 0;
    Q_INVOKABLE virtual bool isLaserEnabled(int zoneIndex) const = 0;

signals:
    void connectedChanged();
    void zonesChanged();
    void errorOccurred(const QString& message);
};

} // namespace gizmotweak2

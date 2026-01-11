#pragma once

#include "engine/LaserEngine.h"

namespace gizmotweak2
{

/**
 * @brief Excalibur laser engine implementation.
 *
 * This engine interfaces with Excalibur laser controllers.
 */
class ExcaliburEngine : public LaserEngine
{
    Q_OBJECT

public:
    explicit ExcaliburEngine(QObject* parent = nullptr);
    ~ExcaliburEngine() override;

    // LaserEngine interface
    QString engineName() const override { return QStringLiteral("Excalibur"); }

    bool isConnected() const override;
    bool connect() override;
    void disconnect() override;

    QStringList zones() const override;
    int zoneCount() const override;

    bool sendFrame(int zoneIndex, const QVariantList& points) override;

    void setLaserEnabled(int zoneIndex, bool enabled) override;
    bool isLaserEnabled(int zoneIndex) const override;

private:
    void discoverZones();

    bool _connected{false};
    QStringList _zones;
    QList<bool> _laserEnabled;
};

} // namespace gizmotweak2

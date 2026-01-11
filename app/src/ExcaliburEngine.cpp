#include "ExcaliburEngine.h"

#include <QDebug>

namespace gizmotweak2
{

ExcaliburEngine::ExcaliburEngine(QObject* parent)
    : LaserEngine(parent)
{
    // TODO: Initialize Excalibur SDK here
    qDebug() << "ExcaliburEngine created";
}

ExcaliburEngine::~ExcaliburEngine()
{
    disconnect();
}

bool ExcaliburEngine::isConnected() const
{
    return _connected;
}

bool ExcaliburEngine::connect()
{
    if (_connected)
    {
        return true;
    }

    qDebug() << "ExcaliburEngine: Connecting...";

    // TODO: Actual Excalibur SDK connection
    // For now, simulate connection with dummy zones
    _connected = true;
    discoverZones();

    emit connectedChanged();
    return true;
}

void ExcaliburEngine::disconnect()
{
    if (!_connected)
    {
        return;
    }

    qDebug() << "ExcaliburEngine: Disconnecting...";

    // TODO: Actual Excalibur SDK disconnection
    _connected = false;
    _zones.clear();
    _laserEnabled.clear();

    emit connectedChanged();
    emit zonesChanged();
}

void ExcaliburEngine::discoverZones()
{
    _zones.clear();
    _laserEnabled.clear();

    // TODO: Query actual zones from Excalibur hardware
    // For now, create dummy zones for testing
    _zones << QStringLiteral("Excalibur Zone 1")
           << QStringLiteral("Excalibur Zone 2")
           << QStringLiteral("Excalibur Zone 3")
           << QStringLiteral("Excalibur Zone 4");

    for (int i = 0; i < _zones.size(); ++i)
    {
        _laserEnabled.append(false);
    }

    emit zonesChanged();
}

QStringList ExcaliburEngine::zones() const
{
    return _zones;
}

int ExcaliburEngine::zoneCount() const
{
    return _zones.size();
}

bool ExcaliburEngine::sendFrame(int zoneIndex, const QVariantList& points)
{
    if (!_connected || zoneIndex < 0 || zoneIndex >= _zones.size())
    {
        return false;
    }

    if (!_laserEnabled.at(zoneIndex))
    {
        return false;
    }

    // TODO: Convert points to Excalibur format and send
    // Points format: [{x, y, r, g, b}, ...]
    // x, y in [-1, +1], r, g, b in [0, 1]

    Q_UNUSED(points)

    return true;
}

void ExcaliburEngine::setLaserEnabled(int zoneIndex, bool enabled)
{
    if (zoneIndex < 0 || zoneIndex >= _laserEnabled.size())
    {
        return;
    }

    if (_laserEnabled.at(zoneIndex) != enabled)
    {
        _laserEnabled[zoneIndex] = enabled;
        qDebug() << "ExcaliburEngine: Zone" << zoneIndex << "laser" << (enabled ? "ON" : "OFF");
    }
}

bool ExcaliburEngine::isLaserEnabled(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _laserEnabled.size())
    {
        return false;
    }
    return _laserEnabled.at(zoneIndex);
}

} // namespace gizmotweak2

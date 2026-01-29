#include "ExcaliburEngine.h"

#include <QDebug>

namespace gizmotweak2
{

ExcaliburEngine::ExcaliburEngine(QObject* parent)
    : QObject(parent)
{
    // Setup reconnect timer
    _reconnectTimer.setSingleShot(true);
    QObject::connect(&_reconnectTimer, &QTimer::timeout, this, &ExcaliburEngine::reconnect);
}

ExcaliburEngine::~ExcaliburEngine()
{
    _reconnectTimer.stop();
    disconnect();
}

bool ExcaliburEngine::isConnected() const
{
    return _connected;
}

void ExcaliburEngine::setConnectionStatus(ConnectionStatus status)
{
    if (_connectionStatus != status)
    {
        _connectionStatus = status;
        emit connectionStatusChanged();
    }
}

void ExcaliburEngine::setLastError(const QString& error)
{
    if (_lastError != error)
    {
        _lastError = error;
        emit lastErrorChanged();
        if (!error.isEmpty())
        {
            emit errorOccurred(error);
        }
    }
}

bool ExcaliburEngine::connect()
{
    if (_connected)
    {
        return true;
    }

    setConnectionStatus(ConnectionStatus::Connecting);
    setLastError(QString());
    // TODO: Actual Excalibur SDK connection
    // For now, simulate successful connection with dummy zones
    bool success = true;

    if (success)
    {
        _connected = true;
        _reconnectAttempts = 0;
        discoverZones();
        setConnectionStatus(ConnectionStatus::Connected);
        emit connectedChanged();
        return true;
    }
    else
    {
        setConnectionStatus(ConnectionStatus::Error);
        setLastError(tr("Failed to connect to Excalibur device"));

        // Schedule automatic reconnection
        if (_reconnectAttempts < MaxReconnectAttempts)
        {
            _reconnectAttempts++;
            _reconnectTimer.start(2000);  // Retry in 2 seconds
        }
        return false;
    }
}

void ExcaliburEngine::disconnect()
{
    _reconnectTimer.stop();
    _reconnectAttempts = 0;

    if (!_connected)
    {
        return;
    }

    // TODO: Actual Excalibur SDK disconnection
    _connected = false;
    _zones.clear();
    _laserEnabled.clear();

    setConnectionStatus(ConnectionStatus::Disconnected);
    setLastError(QString());
    emit connectedChanged();
    emit zonesChanged();
}

void ExcaliburEngine::reconnect()
{
    if (_connected)
    {
        return;
    }

    connect();
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
    // Validate connection
    if (!_connected)
    {
        // Don't spam errors, just return false
        return false;
    }

    // Validate zone index
    if (zoneIndex < 0 || zoneIndex >= _zones.size())
    {
        qWarning() << "ExcaliburEngine::sendFrame: Invalid zone index" << zoneIndex;
        return false;
    }

    // Check if laser is enabled for this zone
    if (!_laserEnabled.at(zoneIndex))
    {
        return false;
    }

    // Validate points
    if (points.isEmpty())
    {
        return true;  // Empty frame is valid, just nothing to display
    }

    // TODO: Convert points to Excalibur format and send
    // Points format: [{x, y, r, g, b}, ...]
    // x, y in [-1, +1], r, g, b in [0, 1]

    // Simulate frame sending - in real implementation this would call Excalibur API
    // bool success = excaliburSendFrame(zoneIndex, convertedPoints);
    bool success = true;

    if (!success)
    {
        setLastError(tr("Failed to send frame to zone %1").arg(zoneIndex + 1));
        return false;
    }

    return true;
}

void ExcaliburEngine::setLaserEnabled(int zoneIndex, bool enabled)
{
    if (zoneIndex < 0 || zoneIndex >= _laserEnabled.size())
    {
        qWarning() << "ExcaliburEngine::setLaserEnabled: Invalid zone index" << zoneIndex;
        return;
    }

    if (_laserEnabled.at(zoneIndex) != enabled)
    {
        _laserEnabled[zoneIndex] = enabled;

        // If disabling, ensure we send a blank frame to stop output
        if (!enabled && _connected)
        {
            // TODO: Send blank frame to zone
        }
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

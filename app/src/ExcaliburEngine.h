#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

/**
 * @brief Excalibur laser engine implementation.
 *
 * This engine interfaces with Excalibur laser controllers.
 */
class ExcaliburEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList zones READ zones NOTIFY zonesChanged)
    Q_PROPERTY(QString engineName READ engineName CONSTANT)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum class ConnectionStatus
    {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    Q_ENUM(ConnectionStatus)

    explicit ExcaliburEngine(QObject* parent = nullptr);
    ~ExcaliburEngine() override;

    QString engineName() const { return QStringLiteral("Excalibur"); }

    bool isConnected() const;
    ConnectionStatus connectionStatus() const { return _connectionStatus; }
    QString lastError() const { return _lastError; }

    Q_INVOKABLE bool connect();
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void reconnect();

    QStringList zones() const;
    Q_INVOKABLE int zoneCount() const;

    Q_INVOKABLE bool sendFrame(int zoneIndex, const QVariantList& points);

    Q_INVOKABLE void setLaserEnabled(int zoneIndex, bool enabled);
    Q_INVOKABLE bool isLaserEnabled(int zoneIndex) const;

signals:
    void connectedChanged();
    void zonesChanged();
    void connectionStatusChanged();
    void lastErrorChanged();
    void errorOccurred(const QString& message);

private:
    void discoverZones();
    void setConnectionStatus(ConnectionStatus status);
    void setLastError(const QString& error);

    bool _connected{false};
    ConnectionStatus _connectionStatus{ConnectionStatus::Disconnected};
    QString _lastError;
    QStringList _zones;
    QList<bool> _laserEnabled;
    QTimer _reconnectTimer;
    int _reconnectAttempts{0};
    static constexpr int MaxReconnectAttempts = 3;
};

} // namespace gizmotweak2

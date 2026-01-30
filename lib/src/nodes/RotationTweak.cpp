#include "RotationTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

RotationTweak::RotationTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Rotation"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D
    addInput(QStringLiteral("center"), Port::DataType::Position);  // Center override

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Rotation track with angle (0)
    auto* rotationTrack = createAutomationTrack(QStringLiteral("Rotation"), 1, QColor(255, 140, 0));
    rotationTrack->setupParameter(0, -360.0, 360.0, _angle, tr("Angle"), 1.0, QStringLiteral("\u00B0"));

    auto* centerTrack = createAutomationTrack(QStringLiteral("Center"), 2, QColor(186, 85, 211));
    centerTrack->setupParameter(0, -1.0, 1.0, _centerX, tr("Center X"), 100.0, QStringLiteral("%"));
    centerTrack->setupParameter(1, -1.0, 1.0, _centerY, tr("Center Y"), 100.0, QStringLiteral("%"));
}

void RotationTweak::setAngle(qreal a)
{
    if (!qFuzzyCompare(_angle, a))
    {
        _angle = a;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Rotation"));
        if (track) track->setInitialValue(0, a);
        emit angleChanged();
        emitPropertyChanged();
    }
}

void RotationTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(0, cx);
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void RotationTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(1, cy);
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void RotationTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;

        // Show/hide ratio port based on followGizmo
        auto* ratioPort = inputAt(1);  // ratio port is at index 1
        if (ratioPort)
        {
            ratioPort->setVisible(follow);
            if (!follow && ratioPort->isConnected())
            {
                emit requestDisconnectPort(ratioPort);
            }
        }

        emit followGizmoChanged();
        emitPropertyChanged();
    }
}

QPointF RotationTweak::apply(qreal x, qreal y, qreal ratio,
                             qreal gizmoX, qreal gizmoY) const
{
    // Effective angle based on ratio
    qreal effectiveAngle = _angle * ratio;
    qreal radians = qDegreesToRadians(effectiveAngle);

    // Center = own automatable center + position cable offset (additive)
    qreal cx = _centerX + gizmoX;
    qreal cy = _centerY + gizmoY;

    // Translate to origin (center)
    qreal dx = x - cx;
    qreal dy = y - cy;

    // Rotate
    qreal cosA = qCos(radians);
    qreal sinA = qSin(radians);

    qreal rotatedX = dx * cosA - dy * sinA;
    qreal rotatedY = dx * sinA + dy * cosA;

    // Translate back
    return QPointF(cx + rotatedX, cy + rotatedY);
}

QJsonObject RotationTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["angle"] = _angle;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void RotationTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("angle")) setAngle(json["angle"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void RotationTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* rotationTrack = automationTrack(QStringLiteral("Rotation"));
    if (rotationTrack && rotationTrack->isAutomated())
    {
        _angle = rotationTrack->timedValue(timeMs, 0);
    }

    auto* centerTrack = automationTrack(QStringLiteral("Center"));
    if (centerTrack && centerTrack->isAutomated())
    {
        _centerX = centerTrack->timedValue(timeMs, 0);
        _centerY = centerTrack->timedValue(timeMs, 1);
    }
}

} // namespace gizmotweak2

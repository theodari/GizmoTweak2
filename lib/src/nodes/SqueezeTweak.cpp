#include "SqueezeTweak.h"
#include "core/Port.h"

#include <QtMath>
#include <cmath>

namespace gizmotweak2
{

SqueezeTweak::SqueezeTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Squeeze"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D
    addInput(QStringLiteral("center"), Port::DataType::Position);  // Center override

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Squeeze track with intensity (0) and angle (1)
    auto* squeezeTrack = createAutomationTrack(QStringLiteral("Squeeze"), 2, QColor(210, 105, 30));
    squeezeTrack->setupParameter(0, -2.0, 2.0, _intensity, tr("Intensity"), 100.0, QStringLiteral("%"));
    squeezeTrack->setupParameter(1, 0.0, 360.0, _angle, tr("Angle"), 1.0, QStringLiteral("\u00B0"));

    auto* centerTrack = createAutomationTrack(QStringLiteral("Center"), 2, QColor(186, 85, 211));
    centerTrack->setupParameter(0, -1.0, 1.0, _centerX, tr("Center X"), 100.0, QStringLiteral("%"));
    centerTrack->setupParameter(1, -1.0, 1.0, _centerY, tr("Center Y"), 100.0, QStringLiteral("%"));
}

void SqueezeTweak::setIntensity(qreal i)
{
    i = qBound(-2.0, i, 2.0);
    if (!qFuzzyCompare(_intensity, i))
    {
        _intensity = i;
        auto* track = automationTrack(QStringLiteral("Squeeze"));
        if (track) track->setInitialValue(0, i);
        emit intensityChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setAngle(qreal a)
{
    // Normalize angle to 0-360
    while (a < 0.0) a += 360.0;
    while (a >= 360.0) a -= 360.0;
    if (!qFuzzyCompare(_angle, a))
    {
        _angle = a;
        auto* track = automationTrack(QStringLiteral("Squeeze"));
        if (track) track->setInitialValue(1, a);
        emit angleChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(0, cx);
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(1, cy);
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setFollowGizmo(bool follow)
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

QPointF SqueezeTweak::apply(qreal x, qreal y, qreal ratio,
                            qreal gizmoX, qreal gizmoY) const
{
    if (qFuzzyIsNull(_intensity) || qFuzzyIsNull(ratio))
    {
        return QPointF(x, y);
    }

    // Center = own automatable center + position cable offset (additive)
    qreal cx = _centerX + gizmoX;
    qreal cy = _centerY + gizmoY;

    // Convert to coordinates relative to center
    qreal dx = x - cx;
    qreal dy = y - cy;

    // Effective intensity based on ratio
    qreal effectiveIntensity = _intensity * ratio;

    // Hyperbolic transformation (squeeze/stretch)
    // x' = x * cosh(k) + y * sinh(k)
    // y' = x * sinh(k) + y * cosh(k)
    // This preserves area and creates hyperbolic distortion

    qreal k = effectiveIntensity;
    qreal coshK = std::cosh(k);
    qreal sinhK = std::sinh(k);

    // Apply hyperbolic transformation
    qreal newX = dx * coshK + dy * sinhK;
    qreal newY = dx * sinhK + dy * coshK;

    // Apply rotation to orient the squeeze/stretch axes
    if (!qFuzzyIsNull(_angle))
    {
        qreal angleRad = qDegreesToRadians(_angle);
        qreal cosA = qCos(angleRad);
        qreal sinA = qSin(angleRad);

        // First rotate the point
        qreal rotX = dx * cosA - dy * sinA;
        qreal rotY = dx * sinA + dy * cosA;

        // Apply hyperbolic transformation
        qreal transX = rotX * coshK + rotY * sinhK;
        qreal transY = rotX * sinhK + rotY * coshK;

        // Rotate back
        newX = transX * cosA + transY * sinA;
        newY = -transX * sinA + transY * cosA;
    }

    // Convert back to absolute coordinates
    qreal resultX = cx + newX;
    qreal resultY = cy + newY;

    return QPointF(resultX, resultY);
}

QJsonObject SqueezeTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["intensity"] = _intensity;
    obj["angle"] = _angle;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void SqueezeTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("intensity")) setIntensity(json["intensity"].toDouble());
    if (json.contains("angle")) setAngle(json["angle"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void SqueezeTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* squeezeTrack = automationTrack(QStringLiteral("Squeeze"));
    if (squeezeTrack && squeezeTrack->isAutomated())
    {
        _intensity = squeezeTrack->timedValue(timeMs, 0);
        _angle = squeezeTrack->timedValue(timeMs, 1);
    }

    auto* centerTrack = automationTrack(QStringLiteral("Center"));
    if (centerTrack && centerTrack->isAutomated())
    {
        _centerX = centerTrack->timedValue(timeMs, 0);
        _centerY = centerTrack->timedValue(timeMs, 1);
    }
}

} // namespace gizmotweak2

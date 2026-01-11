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

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void SqueezeTweak::setIntensity(qreal i)
{
    i = qBound(-2.0, i, 2.0);
    if (!qFuzzyCompare(_intensity, i))
    {
        _intensity = i;
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
        emit angleChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void SqueezeTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
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
                            qreal /*gizmoX*/, qreal /*gizmoY*/) const
{
    if (qFuzzyIsNull(_intensity) || qFuzzyIsNull(ratio))
    {
        return QPointF(x, y);
    }

    // Always use centerX/centerY as transformation center
    // (followGizmo only controls whether ratio comes from gizmo or is 1.0)

    // Convert to coordinates relative to center
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

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
    qreal resultX = _centerX + newX;
    qreal resultY = _centerY + newY;

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

} // namespace gizmotweak2

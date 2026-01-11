#include "GizmoNode.h"
#include "core/Port.h"

#include <QtMath>
#include <QEasingCurve>

namespace gizmotweak2
{

GizmoNode::GizmoNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Gizmo"));
    addOutput(QStringLiteral("ratio"), Port::DataType::Ratio2D);
}

void GizmoNode::setShape(Shape s)
{
    if (_shape != s)
    {
        _shape = s;
        emit shapeChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setCenterX(qreal x)
{
    if (!qFuzzyCompare(_centerX, x))
    {
        _centerX = x;
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setCenterY(qreal y)
{
    if (!qFuzzyCompare(_centerY, y))
    {
        _centerY = y;
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setHorizontalBorder(qreal b)
{
    b = qMax(0.001, b);
    if (!qFuzzyCompare(_horizontalBorder, b))
    {
        _horizontalBorder = b;
        emit horizontalBorderChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setVerticalBorder(qreal b)
{
    b = qMax(0.001, b);
    if (!qFuzzyCompare(_verticalBorder, b))
    {
        _verticalBorder = b;
        emit verticalBorderChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setFalloff(qreal f)
{
    f = qBound(0.0, f, 1.0);
    if (!qFuzzyCompare(_falloff, f))
    {
        _falloff = f;
        emit falloffChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setFalloffCurve(int curve)
{
    if (_falloffCurve != curve)
    {
        _falloffCurve = curve;
        emit falloffCurveChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setHorizontalBend(qreal b)
{
    b = qBound(-1.0, b, 1.0);
    if (!qFuzzyCompare(_horizontalBend, b))
    {
        _horizontalBend = b;
        emit horizontalBendChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setVerticalBend(qreal b)
{
    b = qBound(-1.0, b, 1.0);
    if (!qFuzzyCompare(_verticalBend, b))
    {
        _verticalBend = b;
        emit verticalBendChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setAperture(qreal a)
{
    a = qBound(0.0, a, 360.0);
    if (!qFuzzyCompare(_aperture, a))
    {
        _aperture = a;
        emit apertureChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setPhase(qreal p)
{
    // Normalize to 0-360
    while (p < 0.0) p += 360.0;
    while (p >= 360.0) p -= 360.0;
    if (!qFuzzyCompare(_phase, p))
    {
        _phase = p;
        emit phaseChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setWaveCount(int count)
{
    count = qMax(1, count);
    if (_waveCount != count)
    {
        _waveCount = count;
        emit waveCountChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setNoiseIntensity(qreal intensity)
{
    intensity = qBound(0.0, intensity, 1.0);
    if (!qFuzzyCompare(_noiseIntensity, intensity))
    {
        _noiseIntensity = intensity;
        emit noiseIntensityChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setNoiseScale(qreal scale)
{
    scale = qMax(0.01, scale);
    if (!qFuzzyCompare(_noiseScale, scale))
    {
        _noiseScale = scale;
        emit noiseScaleChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setNoiseSpeed(qreal speed)
{
    speed = qMax(0.0, speed);
    if (!qFuzzyCompare(_noiseSpeed, speed))
    {
        _noiseSpeed = speed;
        emit noiseSpeedChanged();
        emitPropertyChanged();
    }
}

qreal GizmoNode::pseudoRandom(qreal x, qreal y) const
{
    // Deterministic pseudo-random based on position
    // Returns value in [-1, 1]
    qreal seed = x * 12.9898 + y * 78.233;
    qreal hash = qSin(seed) * 43758.5453;
    return (hash - qFloor(hash)) * 2.0 - 1.0;
}

qreal GizmoNode::applyNoise(qreal ratio, qreal x, qreal y, qreal time) const
{
    if (qFuzzyIsNull(_noiseIntensity))
    {
        return ratio;
    }

    // Scale coordinates for grain size (smaller scale = finer grain)
    qreal scaledX = x / _noiseScale;
    qreal scaledY = y / _noiseScale;

    // Add time offset for animation (speed controls how fast noise changes)
    qreal timeOffset = time * _noiseSpeed;
    scaledX += timeOffset;
    scaledY += timeOffset * 0.7;  // Slightly different offset for Y to avoid linear motion

    qreal noise = pseudoRandom(scaledX, scaledY);
    qreal noisyRatio = ratio * (1.0 + noise * _noiseIntensity);
    return qBound(0.0, noisyRatio, 1.0);
}

qreal GizmoNode::applyBend(qreal coord, qreal bend) const
{
    if (qFuzzyIsNull(bend))
        return coord;

    // Bend distorts the coordinate space
    // Positive bend creates convex effect, negative creates concave
    if (bend > 0)
    {
        // Convex: push coordinates toward edges
        return coord * (1.0 + bend * (1.0 - qAbs(coord)));
    }
    else
    {
        // Concave: push coordinates toward center
        return coord * (1.0 + bend * qAbs(coord));
    }
}

qreal GizmoNode::applyFalloffCurve(qreal t) const
{
    // t is 0 at inner edge, 1 at outer edge
    // We want to return the ratio (1 at center, 0 at edge)
    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));
    return 1.0 - curve.valueForProgress(t);
}

qreal GizmoNode::computeRatio(qreal x, qreal y, qreal time) const
{
    qreal ratio = 0.0;

    switch (_shape)
    {
    case Shape::Rectangle:
        ratio = computeRectangleRatio(x, y);
        break;
    case Shape::Ellipse:
        ratio = computeEllipseRatio(x, y);
        break;
    case Shape::Angle:
        ratio = computeAngleRatio(x, y);
        break;
    case Shape::LinearWave:
        ratio = computeLinearWaveRatio(x, y);
        break;
    case Shape::CircularWave:
        ratio = computeCircularWaveRatio(x, y);
        break;
    }

    // Apply noise if enabled
    return applyNoise(ratio, x, y, time);
}

qreal GizmoNode::computeEllipseRatio(qreal x, qreal y) const
{
    // Transform to local coordinates
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Apply bend distortion
    dx = applyBend(dx, _horizontalBend);
    dy = applyBend(dy, _verticalBend);

    // Normalize by border size (ellipse equation)
    qreal normalizedDist = qSqrt((dx * dx) / (_horizontalBorder * _horizontalBorder) +
                                  (dy * dy) / (_verticalBorder * _verticalBorder));

    // Inside inner radius (after falloff): full effect
    qreal innerRadius = 1.0 - _falloff;
    if (normalizedDist <= innerRadius)
    {
        return 1.0;
    }

    // Outside outer radius: no effect
    if (normalizedDist >= 1.0)
    {
        return 0.0;
    }

    // Falloff zone
    qreal t = (normalizedDist - innerRadius) / _falloff;
    return applyFalloffCurve(t);
}

qreal GizmoNode::computeRectangleRatio(qreal x, qreal y) const
{
    // Transform to local coordinates
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Apply bend distortion
    dx = applyBend(dx, _horizontalBend);
    dy = applyBend(dy, _verticalBend);

    // Normalize by border size
    qreal nx = qAbs(dx) / _horizontalBorder;
    qreal ny = qAbs(dy) / _verticalBorder;

    // Use Chebyshev distance (max of normalized distances)
    qreal normalizedDist = qMax(nx, ny);

    // Inside inner radius: full effect
    qreal innerRadius = 1.0 - _falloff;
    if (normalizedDist <= innerRadius)
    {
        return 1.0;
    }

    // Outside outer radius: no effect
    if (normalizedDist >= 1.0)
    {
        return 0.0;
    }

    // Falloff zone
    qreal t = (normalizedDist - innerRadius) / _falloff;
    return applyFalloffCurve(t);
}

qreal GizmoNode::computeAngleRatio(qreal x, qreal y) const
{
    // Transform to local coordinates
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Calculate angle from center (in degrees, 0 = right, counter-clockwise)
    qreal angle = qRadiansToDegrees(qAtan2(dy, dx));
    if (angle < 0) angle += 360.0;

    // Aperture is centered on phase
    qreal halfAperture = _aperture / 2.0;
    qreal startAngle = _phase - halfAperture;
    qreal endAngle = _phase + halfAperture;

    // Normalize angles
    while (startAngle < 0) startAngle += 360.0;
    while (endAngle < 0) endAngle += 360.0;

    // Check if point is within the angle sector
    bool inSector = false;
    if (startAngle <= endAngle)
    {
        inSector = (angle >= startAngle && angle <= endAngle);
    }
    else
    {
        // Wraps around 0
        inSector = (angle >= startAngle || angle <= endAngle);
    }

    if (!inSector)
    {
        return 0.0;
    }

    // Calculate angular distance from edges for falloff
    qreal distFromStart = angle - startAngle;
    if (distFromStart < 0) distFromStart += 360.0;
    qreal distFromEnd = endAngle - angle;
    if (distFromEnd < 0) distFromEnd += 360.0;

    qreal angularDist = qMin(distFromStart, distFromEnd);
    qreal falloffAngle = halfAperture * _falloff;

    if (angularDist >= falloffAngle)
    {
        return 1.0;
    }

    // In falloff zone
    qreal t = 1.0 - (angularDist / falloffAngle);
    return applyFalloffCurve(t);
}

qreal GizmoNode::computeLinearWaveRatio(qreal x, qreal y) const
{
    // Transform to local coordinates
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Apply bend distortion
    dx = applyBend(dx, _horizontalBend);
    dy = applyBend(dy, _verticalBend);

    // Phase in radians
    qreal phaseRad = qDegreesToRadians(_phase);

    // Project onto wave direction (perpendicular to phase angle)
    qreal waveDir = dx * qCos(phaseRad) + dy * qSin(phaseRad);

    // Scale by border (use average of horizontal/vertical)
    qreal avgBorder = (_horizontalBorder + _verticalBorder) / 2.0;
    qreal normalizedDist = waveDir / avgBorder;

    // Generate wave
    qreal waveValue = qSin(normalizedDist * _waveCount * M_PI);

    // Convert from [-1, 1] to [0, 1] and apply falloff
    qreal ratio = (waveValue + 1.0) / 2.0;

    // Apply distance attenuation
    qreal distFromCenter = qSqrt(dx * dx + dy * dy) / avgBorder;
    if (distFromCenter > 1.0)
    {
        return 0.0;
    }

    qreal attenuation = 1.0 - distFromCenter * (1.0 - (1.0 - _falloff));
    return ratio * qMax(0.0, attenuation);
}

qreal GizmoNode::computeCircularWaveRatio(qreal x, qreal y) const
{
    // Transform to local coordinates
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Apply bend distortion
    dx = applyBend(dx, _horizontalBend);
    dy = applyBend(dy, _verticalBend);

    // Normalize by border size
    qreal normalizedDist = qSqrt((dx * dx) / (_horizontalBorder * _horizontalBorder) +
                                  (dy * dy) / (_verticalBorder * _verticalBorder));

    // Outside outer radius: no effect
    if (normalizedDist >= 1.0)
    {
        return 0.0;
    }

    // Phase offset in normalized distance
    qreal phaseOffset = _phase / 360.0;

    // Generate circular wave (ripple)
    qreal waveValue = qSin((normalizedDist + phaseOffset) * _waveCount * 2.0 * M_PI);

    // Convert from [-1, 1] to [0, 1]
    qreal ratio = (waveValue + 1.0) / 2.0;

    // Apply edge attenuation
    qreal edgeAttenuation = 1.0;
    qreal fadeStart = 1.0 - _falloff;
    if (normalizedDist > fadeStart)
    {
        qreal t = (normalizedDist - fadeStart) / _falloff;
        edgeAttenuation = applyFalloffCurve(t);
    }

    return ratio * edgeAttenuation;
}

QJsonObject GizmoNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["shape"] = static_cast<int>(_shape);
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["horizontalBorder"] = _horizontalBorder;
    obj["verticalBorder"] = _verticalBorder;
    obj["falloff"] = _falloff;
    obj["falloffCurve"] = _falloffCurve;
    obj["horizontalBend"] = _horizontalBend;
    obj["verticalBend"] = _verticalBend;
    obj["aperture"] = _aperture;
    obj["phase"] = _phase;
    obj["waveCount"] = _waveCount;
    obj["noiseIntensity"] = _noiseIntensity;
    obj["noiseScale"] = _noiseScale;
    obj["noiseSpeed"] = _noiseSpeed;
    return obj;
}

void GizmoNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("shape")) setShape(static_cast<Shape>(json["shape"].toInt()));
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("horizontalBorder")) setHorizontalBorder(json["horizontalBorder"].toDouble());
    if (json.contains("verticalBorder")) setVerticalBorder(json["verticalBorder"].toDouble());
    if (json.contains("falloff")) setFalloff(json["falloff"].toDouble());
    if (json.contains("falloffCurve")) setFalloffCurve(json["falloffCurve"].toInt());
    if (json.contains("horizontalBend")) setHorizontalBend(json["horizontalBend"].toDouble());
    if (json.contains("verticalBend")) setVerticalBend(json["verticalBend"].toDouble());
    if (json.contains("aperture")) setAperture(json["aperture"].toDouble());
    if (json.contains("phase")) setPhase(json["phase"].toDouble());
    if (json.contains("waveCount")) setWaveCount(json["waveCount"].toInt());
    if (json.contains("noiseIntensity")) setNoiseIntensity(json["noiseIntensity"].toDouble());
    if (json.contains("noiseScale")) setNoiseScale(json["noiseScale"].toDouble());
    if (json.contains("noiseSpeed")) setNoiseSpeed(json["noiseSpeed"].toDouble());

    // Legacy compatibility: if old "radius" exists
    if (json.contains("radius") && !json.contains("horizontalBorder"))
    {
        qreal r = json["radius"].toDouble();
        setHorizontalBorder(r);
        setVerticalBorder(r);
    }
}

} // namespace gizmotweak2

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

    // Automation: Scale track with scaleX (0) and scaleY (1)
    auto* scaleTrack = createAutomationTrack(QStringLiteral("Scale"), 2, QColor(255, 165, 0));
    scaleTrack->setupParameter(0, 0.01, 3.0, _scaleX, tr("Scale X"), 100.0, QStringLiteral("%"));
    scaleTrack->setupParameter(1, 0.01, 3.0, _scaleY, tr("Scale Y"), 100.0, QStringLiteral("%"));

    // Automation: Position track with centerX (0) and centerY (1)
    auto* centerTrack = createAutomationTrack(QStringLiteral("Position"), 2, QColor(186, 85, 211));
    centerTrack->setupParameter(0, -1.0, 1.0, _centerX, tr("Position X"), 100.0, QStringLiteral("%"));
    centerTrack->setupParameter(1, -1.0, 1.0, _centerY, tr("Position Y"), 100.0, QStringLiteral("%"));

    // Automation: Border track with horizontalBorder (0), horizontalBend (1), verticalBorder (2), verticalBend (3)
    auto* borderTrack = createAutomationTrack(QStringLiteral("Border"), 4, QColor(32, 178, 170));
    borderTrack->setupParameter(0, 0.0, 1.0, _horizontalBorder, tr("H Border"), 100.0, QStringLiteral("%"));
    borderTrack->setupParameter(1, -1.0, 1.0, _horizontalBend, tr("H Bend"), 100.0, QStringLiteral("%"));
    borderTrack->setupParameter(2, 0.0, 1.0, _verticalBorder, tr("V Border"), 100.0, QStringLiteral("%"));
    borderTrack->setupParameter(3, -1.0, 1.0, _verticalBend, tr("V Bend"), 100.0, QStringLiteral("%"));

    // Automation: Aperture track with aperture (0)
    auto* apertureTrack = createAutomationTrack(QStringLiteral("Aperture"), 1, QColor(255, 99, 71));
    apertureTrack->setupParameter(0, 0.0, 360.0, _aperture, tr("Aperture"), 1.0, QStringLiteral("\u00B0"));

    // Automation: Phase track with phase (0)
    auto* phaseTrack = createAutomationTrack(QStringLiteral("Phase"), 1, QColor(30, 144, 255));
    phaseTrack->setupParameter(0, 0.0, 360.0, _phase, tr("Phase"), 1.0, QStringLiteral("\u00B0"));

    // Automation: WaveCount track with waveCount (0)
    auto* waveTrack = createAutomationTrack(QStringLiteral("WaveCount"), 1, QColor(138, 43, 226));
    waveTrack->setupParameter(0, 1.0, 20.0, _waveCount, tr("Wave Count"), 1.0, QString());

    // Automation: Noise track with noiseIntensity (0), noiseScale (1), noiseSpeed (2)
    auto* noiseTrack = createAutomationTrack(QStringLiteral("Noise"), 3, QColor(128, 128, 0));
    noiseTrack->setupParameter(0, 0.0, 1.0, _noiseIntensity, tr("Intensity"), 100.0, QStringLiteral("%"));
    noiseTrack->setupParameter(1, 0.01, 2.0, _noiseScale, tr("Scale"), 100.0, QStringLiteral("%"));
    noiseTrack->setupParameter(2, 0.0, 10.0, _noiseSpeed, tr("Speed"), 1.0, QString());
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

void GizmoNode::setScaleX(qreal sx)
{
    sx = qBound(0.01, sx, 3.0);
    if (!qFuzzyCompare(_scaleX, sx))
    {
        _scaleX = sx;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Scale"));
        if (track) track->setInitialValue(0, sx);
        emit scaleXChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setScaleY(qreal sy)
{
    sy = qBound(0.01, sy, 3.0);
    if (!qFuzzyCompare(_scaleY, sy))
    {
        _scaleY = sy;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Scale"));
        if (track) track->setInitialValue(1, sy);
        emit scaleYChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setCenterX(qreal x)
{
    if (!qFuzzyCompare(_centerX, x))
    {
        _centerX = x;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(0, x);
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void GizmoNode::setCenterY(qreal y)
{
    if (!qFuzzyCompare(_centerY, y))
    {
        _centerY = y;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(1, y);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Border"));
        if (track && track->paramCount() >= 4) track->setInitialValue(0, b);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Border"));
        if (track && track->paramCount() >= 4) track->setInitialValue(2, b);
        emit verticalBorderChanged();
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Border"));
        if (track && track->paramCount() >= 4) track->setInitialValue(1, b);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Border"));
        if (track && track->paramCount() >= 4) track->setInitialValue(3, b);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Aperture"));
        if (track) track->setInitialValue(0, a);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Phase"));
        if (track) track->setInitialValue(0, p);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("WaveCount"));
        if (track) track->setInitialValue(0, count);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Noise"));
        if (track) track->setInitialValue(0, intensity);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Noise"));
        if (track) track->setInitialValue(1, scale);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Noise"));
        if (track) track->setInitialValue(2, speed);
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


qreal GizmoNode::computeRatio(qreal x, qreal y, qreal time) const
{
    if (qFuzzyIsNull(_scaleX) || qFuzzyIsNull(_scaleY))
        return 0.0;

    // Normalize to local coordinates: translate by center, then divide by scale
    auto x1 = (x - _centerX) / _scaleX;
    auto y1 = (y - _centerY) / _scaleY;

    qreal ratio = 0.0;

    switch (_shape)
    {
    case Shape::Rectangle:
        ratio = computeRectangleRatio(x1, y1);
        break;
    case Shape::Ellipse:
        ratio = computeEllipseRatio(x1, y1);
        break;
    case Shape::Angle:
        ratio = computeAngleRatio(x1, y1);
        break;
    case Shape::LinearWave:
        ratio = computeLinearWaveRatio(x1, y1);
        break;
    case Shape::CircularWave:
        ratio = computeCircularWaveRatio(x1, y1);
        break;
    }

    // Apply noise if enabled
    return applyNoise(ratio, x, y, time);
}

qreal GizmoNode::computeRectangleRatio(qreal x1, qreal y1) const
{
    // x1, y1 are already in local normalized coordinates [-1, 1]

    // Compute asymmetric slopes from border + bend (original GizmoTweak formula)
    auto rightSlope = qMax(_horizontalBorder * (1.0 - _horizontalBend), 1e-6);
    auto leftSlope  = qMax(_horizontalBorder * (1.0 + _horizontalBend), 1e-6);
    auto horizontalCentralPoint = _horizontalBend * _horizontalBorder;

    auto topSlope    = qMax(_verticalBorder * (1.0 - _verticalBend), 1e-6);
    auto bottomSlope = qMax(_verticalBorder * (1.0 + _verticalBend), 1e-6);
    auto verticalCentralPoint = _verticalBend * _verticalBorder;

    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));

    double xOmega;
    if (x1 > horizontalCentralPoint)
        xOmega = qBound(0.0, (1.0 - x1) / rightSlope, 1.0);
    else
        xOmega = qBound(0.0, (1.0 + x1) / leftSlope, 1.0);
    auto xResult = curve.valueForProgress(xOmega);

    double yOmega;
    if (y1 > verticalCentralPoint)
        yOmega = qBound(0.0, (1.0 - y1) / topSlope, 1.0);
    else
        yOmega = qBound(0.0, (1.0 + y1) / bottomSlope, 1.0);
    auto yResult = curve.valueForProgress(yOmega);

    return qMin(xResult, yResult);
}

qreal GizmoNode::computeEllipseRatio(qreal x1, qreal y1) const
{
    // x1, y1 are already in local normalized coordinates

    auto distance = qSqrt(x1 * x1 + y1 * y1);
    if (distance >= 1.0)
        return 0.0;

    if (qFuzzyIsNull(_horizontalBorder) && qFuzzyIsNull(_verticalBorder))
        return 1.0;

    auto x1Abs = qAbs(x1);
    auto y1Abs = qAbs(y1);
    auto ellipseHalfWidth = 1.0 - _horizontalBorder;
    auto ellipseHalfHeight = 1.0 - _verticalBorder;

    // Inside inner ellipse: full effect
    if (ellipseHalfWidth > 1e-6 && ellipseHalfHeight > 1e-6)
    {
        if (x1 * x1 / (ellipseHalfWidth * ellipseHalfWidth) +
            y1 * y1 / (ellipseHalfHeight * ellipseHalfHeight) < 1.0)
            return 1.0;
    }

    // Compute angle from center and project onto unit circle
    auto angle = qAtan2(y1Abs, x1Abs);
    auto normalizedX = qCos(angle);
    auto normalizedY = qSin(angle);
    auto ellipseX = normalizedX * ellipseHalfWidth;
    auto ellipseY = normalizedY * ellipseHalfHeight;

    auto outerDist = qSqrt((ellipseX - normalizedX) * (ellipseX - normalizedX) +
                            (ellipseY - normalizedY) * (ellipseY - normalizedY));

    double linearAlpha = 0.0;
    if (outerDist > 1e-6)
    {
        auto pointDist = qSqrt((x1Abs - _horizontalBend - normalizedX) * (x1Abs - _horizontalBend - normalizedX) +
                                (y1Abs - _verticalBend - normalizedY) * (y1Abs - _verticalBend - normalizedY));
        linearAlpha = pointDist / outerDist;
    }

    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));
    return curve.valueForProgress(qBound(0.0, linearAlpha, 1.0));
}

qreal GizmoNode::computeAngleRatio(qreal x1, qreal y1) const
{
    // x1, y1 are already in local normalized coordinates

    // Aperture in radians (half-aperture centered on phase)
    auto apertureRad = qDegreesToRadians(_aperture) / 2.0;
    auto phaseRad = qDegreesToRadians(_phase);

    // Angle relative to phase direction
    auto angle = qAtan2(y1, x1) - phaseRad;
    // Normalize to [-PI, PI]
    while (angle > M_PI) angle -= 2.0 * M_PI;
    while (angle < -M_PI) angle += 2.0 * M_PI;

    if (angle < -apertureRad || angle > apertureRad)
        return 0.0;

    if (qFuzzyIsNull(apertureRad))
        return 0.0;

    // Compute asymmetric slopes
    auto rightSlope = qMax(_horizontalBorder * (1.0 - _horizontalBend), 1e-6);
    auto leftSlope  = qMax(_horizontalBorder * (1.0 + _horizontalBend), 1e-6);
    auto horizontalCentralPoint = _horizontalBend * _horizontalBorder;

    // Normalize angle to [-1, 1] within aperture
    auto angleAlpha = angle * 2.0 / (apertureRad * 2.0);

    double omega;
    if (angle > horizontalCentralPoint)
        omega = qBound(0.0, (1.0 - angleAlpha) / rightSlope, 1.0);
    else
        omega = qBound(0.0, (1.0 + angleAlpha) / leftSlope, 1.0);

    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));

    auto angleSlope = (angleAlpha * rightSlope + (1.0 - angleAlpha) * leftSlope);
    omega = qMin(omega, curve.valueForProgress(qSqrt(x1 * x1 + y1 * y1)) * angleSlope);

    return curve.valueForProgress(omega);
}

qreal GizmoNode::computeLinearWaveRatio(qreal x1, qreal /*y1*/) const
{
    // x1 is already in local normalized coordinates

    // Phase offset (normalized)
    auto phaseOffset = _phase / 360.0;

    // Fold into [0, 1] triangle wave
    auto mod1 = std::fmod(qAbs((x1 - phaseOffset) * 2.0), 2.0);
    if (mod1 > 1.0)
        mod1 = 2.0 - mod1;

    // Compute asymmetric slopes
    auto rightSlope = qMax(_horizontalBorder * (1.0 - _horizontalBend), 1e-6);
    auto leftSlope  = qMax(_horizontalBorder * (1.0 + _horizontalBend), 1e-6);
    auto horizontalCentralPoint = _horizontalBend * _horizontalBorder;

    double xOmega;
    if (mod1 > horizontalCentralPoint)
        xOmega = qBound(0.0, (1.0 - mod1) / rightSlope, 1.0);
    else
        xOmega = qBound(0.0, (1.0 + mod1) / leftSlope, 1.0);

    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));
    return curve.valueForProgress(xOmega);
}

qreal GizmoNode::computeCircularWaveRatio(qreal x1, qreal y1) const
{
    // x1, y1 are already in local normalized coordinates

    auto dist = qSqrt(x1 * x1 + y1 * y1);

    // Phase offset (normalized)
    auto phaseOffset = _phase / 360.0;

    // Fold into [0, 1] triangle wave
    auto mod1 = std::fmod(qAbs((dist - phaseOffset) * 2.0), 2.0);
    if (mod1 > 1.0)
        mod1 = 2.0 - mod1;

    // Compute asymmetric slopes
    auto rightSlope = qMax(_horizontalBorder * (1.0 - _horizontalBend), 1e-6);
    auto leftSlope  = qMax(_horizontalBorder * (1.0 + _horizontalBend), 1e-6);
    auto horizontalCentralPoint = _horizontalBend * _horizontalBorder;

    double xOmega;
    if (mod1 > horizontalCentralPoint)
        xOmega = qBound(0.0, (1.0 - mod1) / rightSlope, 1.0);
    else
        xOmega = qBound(0.0, (1.0 + mod1) / leftSlope, 1.0);

    QEasingCurve curve(static_cast<QEasingCurve::Type>(_falloffCurve));
    return curve.valueForProgress(xOmega);
}

QJsonObject GizmoNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["shape"] = static_cast<int>(_shape);
    obj["scaleX"] = _scaleX;
    obj["scaleY"] = _scaleY;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["horizontalBorder"] = _horizontalBorder;
    obj["verticalBorder"] = _verticalBorder;
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
    if (json.contains("scaleX")) setScaleX(json["scaleX"].toDouble());
    if (json.contains("scaleY")) setScaleY(json["scaleY"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("horizontalBorder")) setHorizontalBorder(json["horizontalBorder"].toDouble());
    if (json.contains("verticalBorder")) setVerticalBorder(json["verticalBorder"].toDouble());
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

void GizmoNode::automationFromJson(const QJsonArray& json)
{
    // Load tracks from JSON (base implementation)
    Node::automationFromJson(json);

    // Migrate legacy format: old "Border" (2 params) + "Falloff" (1 param) + "Bend" (2 params)
    // -> new "Border" (4 params: H border, H bend, V border, V bend)
    auto* borderTrack = automationTrack(QStringLiteral("Border"));
    auto* falloffTrack = automationTrack(QStringLiteral("Falloff"));
    auto* bendTrack = automationTrack(QStringLiteral("Bend"));

    if (borderTrack && borderTrack->paramCount() == 2 && (falloffTrack || bendTrack))
    {
        // Old format detected — create new 4-param Border track
        auto hBend = bendTrack ? bendTrack->initialValue(0) : 0.0;
        auto vBend = bendTrack ? bendTrack->initialValue(1) : 0.0;

        auto* newBorderTrack = createAutomationTrack(QStringLiteral("Border_new"), 4, borderTrack->color());
        newBorderTrack->setInitialValue(0, borderTrack->initialValue(0));  // H border
        newBorderTrack->setInitialValue(1, hBend);                         // H bend
        newBorderTrack->setInitialValue(2, borderTrack->initialValue(1));  // V border
        newBorderTrack->setInitialValue(3, vBend);                         // V bend

        // Copy automation state
        if (borderTrack->isAutomated() || (bendTrack && bendTrack->isAutomated()))
        {
            newBorderTrack->setAutomated(true);
        }

        // Remove old tracks
        removeAutomationTrack(QStringLiteral("Border"));
        if (falloffTrack) removeAutomationTrack(QStringLiteral("Falloff"));
        if (bendTrack) removeAutomationTrack(QStringLiteral("Bend"));

        // Rename new track
        newBorderTrack->setTrackName(QStringLiteral("Border"));
    }
    else if (falloffTrack)
    {
        // Just remove orphaned Falloff track
        removeAutomationTrack(QStringLiteral("Falloff"));
    }
}

void GizmoNode::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* scaleTrack = automationTrack(QStringLiteral("Scale"));
    if (scaleTrack && scaleTrack->isAutomated())
    {
        _scaleX = scaleTrack->timedValue(timeMs, 0);
        _scaleY = scaleTrack->timedValue(timeMs, 1);
    }

    auto* centerTrack = automationTrack(QStringLiteral("Center"));
    if (centerTrack && centerTrack->isAutomated())
    {
        _centerX = centerTrack->timedValue(timeMs, 0);
        _centerY = centerTrack->timedValue(timeMs, 1);
    }

    auto* borderTrack = automationTrack(QStringLiteral("Border"));
    if (borderTrack && borderTrack->isAutomated())
    {
        if (borderTrack->paramCount() == 4)
        {
            _horizontalBorder = borderTrack->timedValue(timeMs, 0);
            _horizontalBend = borderTrack->timedValue(timeMs, 1);
            _verticalBorder = borderTrack->timedValue(timeMs, 2);
            _verticalBend = borderTrack->timedValue(timeMs, 3);
        }
        else if (borderTrack->paramCount() == 2)
        {
            // Legacy 2-param border track
            _horizontalBorder = borderTrack->timedValue(timeMs, 0);
            _verticalBorder = borderTrack->timedValue(timeMs, 1);
        }
    }

    auto* apertureTrack = automationTrack(QStringLiteral("Aperture"));
    if (apertureTrack && apertureTrack->isAutomated())
    {
        _aperture = apertureTrack->timedValue(timeMs, 0);
    }

    auto* phaseTrack = automationTrack(QStringLiteral("Phase"));
    if (phaseTrack && phaseTrack->isAutomated())
    {
        _phase = phaseTrack->timedValue(timeMs, 0);
    }

    auto* waveCountTrack = automationTrack(QStringLiteral("WaveCount"));
    if (waveCountTrack && waveCountTrack->isAutomated())
    {
        _waveCount = static_cast<int>(waveCountTrack->timedValue(timeMs, 0));
    }

    auto* noiseTrack = automationTrack(QStringLiteral("Noise"));
    if (noiseTrack && noiseTrack->isAutomated())
    {
        _noiseIntensity = noiseTrack->timedValue(timeMs, 0);
        _noiseScale = noiseTrack->timedValue(timeMs, 1);
        _noiseSpeed = noiseTrack->timedValue(timeMs, 2);
    }
}

} // namespace gizmotweak2

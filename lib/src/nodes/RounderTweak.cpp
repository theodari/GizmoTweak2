#include "RounderTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

RounderTweak::RounderTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Rounder"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Rounder track with amount (0), verticalShift (1), horizontalShift (2),
    // tighten (3), radialResize (4), radialShift (5)
    auto* rounderTrack = createAutomationTrack(QStringLiteral("Rounder"), 6, QColor(64, 224, 208));  // Turquoise
    rounderTrack->setupParameter(0, -2.0, 2.0, _amount, tr("Amount"), 100.0, QStringLiteral("%"));
    rounderTrack->setupParameter(1, -2.0, 2.0, _verticalShift, tr("V Shift"), 100.0, QStringLiteral("%"));
    rounderTrack->setupParameter(2, -2.0, 2.0, _horizontalShift, tr("H Shift"), 100.0, QStringLiteral("%"));
    rounderTrack->setupParameter(3, 0.0, 1.0, _tighten, tr("Tighten"), 100.0, QStringLiteral("%"));
    rounderTrack->setupParameter(4, 0.5, 2.0, _radialResize, tr("Radial Resize"), 100.0, QStringLiteral("%"));
    rounderTrack->setupParameter(5, -2.0, 2.0, _radialShift, tr("Radial Shift"), 100.0, QStringLiteral("%"));
}

void RounderTweak::setAmount(qreal value)
{
    value = qBound(-2.0, value, 2.0);
    if (!qFuzzyCompare(_amount, value))
    {
        _amount = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(0, value);
        emit amountChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setVerticalShift(qreal value)
{
    value = qBound(-2.0, value, 2.0);
    if (!qFuzzyCompare(_verticalShift, value))
    {
        _verticalShift = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(1, value);
        emit verticalShiftChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setHorizontalShift(qreal value)
{
    value = qBound(-2.0, value, 2.0);
    if (!qFuzzyCompare(_horizontalShift, value))
    {
        _horizontalShift = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(2, value);
        emit horizontalShiftChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setTighten(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_tighten, value))
    {
        _tighten = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(3, value);
        emit tightenChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setRadialResize(qreal value)
{
    value = qBound(0.5, value, 2.0);
    if (!qFuzzyCompare(_radialResize, value))
    {
        _radialResize = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(4, value);
        emit radialResizeChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setRadialShift(qreal value)
{
    value = qBound(-2.0, value, 2.0);
    if (!qFuzzyCompare(_radialShift, value))
    {
        _radialShift = value;
        auto* track = automationTrack(QStringLiteral("Rounder"));
        if (track) track->setInitialValue(5, value);
        emit radialShiftChanged();
        emitPropertyChanged();
    }
}

void RounderTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;
        auto* ratioPort = inputAt(1);
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

QPointF RounderTweak::apply(qreal x, qreal y, qreal ratio) const
{
    if (qFuzzyIsNull(ratio) || qFuzzyIsNull(_amount))
    {
        return QPointF(x, y);
    }

    // Apply ratio to all parameters
    qreal effectiveAmount = _amount * ratio;
    qreal effectiveVShift = _verticalShift * ratio;
    qreal effectiveHShift = _horizontalShift * ratio;
    qreal effectiveTighten = _tighten;  // Tighten is already normalized 0-1
    qreal effectiveRadialResize = 1.0 + (_radialResize - 1.0) * ratio;  // Interpolate from 1.0
    qreal effectiveRadialShift = _radialShift * ratio;

    // Precalculate constants (matching original GizmoTweak algorithm)
    qreal absLimAmount = qMin(1.0, qAbs(effectiveAmount));
    qreal rounderAmountRad = effectiveAmount * M_PI;

    // rounderRR = clamp(0.5, 1.0, 1 - |amount| + |amount| * radialResize)
    qreal rounderRR = qMin(1.0, qMax(0.5, 1.0 - qAbs(effectiveAmount)
                                        + qAbs(effectiveAmount) * effectiveRadialResize));

    // rounderTT = tighten * (1 - 2 * absLimAmount) + absLimAmount
    qreal rounderTT = effectiveTighten - 2.0 * effectiveTighten * absLimAmount + absLimAmount;

    qreal rounderYOffset = effectiveRadialShift * effectiveAmount;

    // Apply transformation
    qreal shiftedX = x - effectiveHShift;
    qreal shiftedY = y - effectiveVShift + rounderYOffset;
    qreal rounderAngle = shiftedX * -rounderAmountRad;

    qreal rounderSin = qSin(rounderAngle);
    qreal rounderCos = qCos(rounderAngle);

    qreal outX = effectiveHShift - rounderSin * shiftedY * rounderRR + shiftedX * rounderTT;
    qreal outY = effectiveVShift + rounderCos * shiftedY * rounderRR;

    return QPointF(outX, outY);
}

QJsonObject RounderTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["amount"] = _amount;
    obj["verticalShift"] = _verticalShift;
    obj["horizontalShift"] = _horizontalShift;
    obj["tighten"] = _tighten;
    obj["radialResize"] = _radialResize;
    obj["radialShift"] = _radialShift;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void RounderTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("amount")) setAmount(json["amount"].toDouble());
    if (json.contains("verticalShift")) setVerticalShift(json["verticalShift"].toDouble());
    if (json.contains("horizontalShift")) setHorizontalShift(json["horizontalShift"].toDouble());
    if (json.contains("tighten")) setTighten(json["tighten"].toDouble());
    if (json.contains("radialResize")) setRadialResize(json["radialResize"].toDouble());
    if (json.contains("radialShift")) setRadialShift(json["radialShift"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void RounderTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active
    auto* rounderTrack = automationTrack(QStringLiteral("Rounder"));
    if (rounderTrack && rounderTrack->isAutomated())
    {
        _amount = rounderTrack->timedValue(timeMs, 0);
        _verticalShift = rounderTrack->timedValue(timeMs, 1);
        _horizontalShift = rounderTrack->timedValue(timeMs, 2);
        _tighten = rounderTrack->timedValue(timeMs, 3);
        _radialResize = rounderTrack->timedValue(timeMs, 4);
        _radialShift = rounderTrack->timedValue(timeMs, 5);
    }
}

} // namespace gizmotweak2

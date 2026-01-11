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
}

void RounderTweak::setAmount(qreal value)
{
    value = qBound(-2.0, value, 2.0);
    if (!qFuzzyCompare(_amount, value))
    {
        _amount = value;
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

} // namespace gizmotweak2

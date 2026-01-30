#include "PolarTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

PolarTweak::PolarTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Polar"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D
    addInput(QStringLiteral("center"), Port::DataType::Position);  // Center override

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Expansion track with expansion (0), ringRadius (1) - matches GizmoTweak v1
    auto* expansionTrack = createAutomationTrack(QStringLiteral("Expansion"), 2, QColor(255, 127, 80));
    expansionTrack->setupParameter(0, -2.0, 2.0, _expansion, tr("Expansion"), 100.0, QStringLiteral("%"));
    expansionTrack->setupParameter(1, 0.01, 2.0, _ringRadius, tr("Radius"), 100.0, QStringLiteral("%"));

    auto* ringScaleTrack = createAutomationTrack(QStringLiteral("RingScale"), 1, QColor(138, 43, 226));
    ringScaleTrack->setupParameter(0, -1.0, 1.0, _ringScale, tr("Ring Scale"), 100.0, QStringLiteral("%"));
}

void PolarTweak::setExpansion(qreal e)
{
    if (!qFuzzyCompare(_expansion, e))
    {
        _expansion = e;
        auto* track = automationTrack(QStringLiteral("Expansion"));
        if (track) track->setInitialValue(0, e);
        emit expansionChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setRingRadius(qreal r)
{
    r = qMax(0.0, r);
    if (!qFuzzyCompare(_ringRadius, r))
    {
        _ringRadius = r;
        auto* track = automationTrack(QStringLiteral("Expansion"));
        if (track) track->setInitialValue(1, r);
        emit ringRadiusChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setRingScale(qreal s)
{
    if (!qFuzzyCompare(_ringScale, s))
    {
        _ringScale = s;
        auto* track = automationTrack(QStringLiteral("RingScale"));
        if (track) track->setInitialValue(0, s);
        emit ringScaleChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setCrossOver(bool co)
{
    if (_crossOver != co)
    {
        _crossOver = co;
        emit crossOverChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setTargetted(bool t)
{
    if (_targetted != t)
    {
        _targetted = t;
        emit targettedChanged();
        emitPropertyChanged();
    }
}

void PolarTweak::setFollowGizmo(bool follow)
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

QPointF PolarTweak::apply(qreal x, qreal y, qreal ratioX, qreal ratioY,
                          qreal gizmoX, qreal gizmoY) const
{
    // Determine which ratio to use
    qreal rX, rY;
    if (_crossOver)
    {
        rX = ratioY;
        rY = ratioX;
    }
    else
    {
        rX = ratioX;
        rY = ratioY;
    }

    // Combined ratio for effects
    qreal ratio = (rX + rY) / 2.0;

    // Center = own automatable center + position cable offset (additive)
    qreal cx = _centerX + gizmoX;
    qreal cy = _centerY + gizmoY;

    // Convert to polar coordinates relative to center
    qreal dx = x - cx;
    qreal dy = y - cy;
    qreal distance = qSqrt(dx * dx + dy * dy);
    qreal angle = qAtan2(dy, dx);

    if (distance < 0.0001)
    {
        return QPointF(x, y);
    }

    // Apply expansion effect
    qreal newDistance = distance;
    if (!qFuzzyIsNull(_expansion))
    {
        qreal expansionAmount = _expansion * ratio;
        if (_targetted)
        {
            newDistance = distance * (1.0 - expansionAmount);
        }
        else
        {
            newDistance = distance * (1.0 + expansionAmount);
        }
    }

    // Apply ring effect
    if (!qFuzzyIsNull(_ringScale) && _ringRadius > 0.0)
    {
        qreal ringPhase = (distance / _ringRadius) * 2.0 * M_PI;
        qreal ringOffset = qSin(ringPhase) * _ringScale * ratio;
        newDistance += ringOffset;
    }

    newDistance = qMax(0.0, newDistance);

    // Convert back to Cartesian
    qreal resultX = cx + newDistance * qCos(angle);
    qreal resultY = cy + newDistance * qSin(angle);

    return QPointF(resultX, resultY);
}

QJsonObject PolarTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["expansion"] = _expansion;
    obj["ringRadius"] = _ringRadius;
    obj["ringScale"] = _ringScale;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["crossOver"] = _crossOver;
    obj["targetted"] = _targetted;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void PolarTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("expansion")) setExpansion(json["expansion"].toDouble());
    if (json.contains("ringRadius")) setRingRadius(json["ringRadius"].toDouble());
    if (json.contains("ringScale")) setRingScale(json["ringScale"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("crossOver")) setCrossOver(json["crossOver"].toBool());
    if (json.contains("targetted")) setTargetted(json["targetted"].toBool());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void PolarTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    // Matches GizmoTweak v1: Expansion track (expansion, radius) + RingScale track (scale)
    auto* expansionTrack = automationTrack(QStringLiteral("Expansion"));
    if (expansionTrack && expansionTrack->isAutomated())
    {
        _expansion = expansionTrack->timedValue(timeMs, 0);
        _ringRadius = expansionTrack->timedValue(timeMs, 1);
    }

    auto* ringScaleTrack = automationTrack(QStringLiteral("RingScale"));
    if (ringScaleTrack && ringScaleTrack->isAutomated())
    {
        _ringScale = ringScaleTrack->timedValue(timeMs, 0);
    }
}

} // namespace gizmotweak2

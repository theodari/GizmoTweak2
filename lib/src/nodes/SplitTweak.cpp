#include "SplitTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

SplitTweak::SplitTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Split"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Threshold track with splitThreshold (0)
    auto* thresholdTrack = createAutomationTrack(QStringLiteral("Threshold"), 1, QColor(244, 164, 96));  // Sandy brown
    thresholdTrack->setupParameter(0, 0.001, 4.0, _splitThreshold, tr("Threshold"), 1.0, QString());
}

void SplitTweak::setSplitThreshold(qreal threshold)
{
    threshold = qBound(0.001, threshold, 4.0);
    if (!qFuzzyCompare(_splitThreshold, threshold))
    {
        _splitThreshold = threshold;
        auto* track = automationTrack(QStringLiteral("Threshold"));
        if (track) track->setInitialValue(0, threshold);
        emit splitThresholdChanged();
        emitPropertyChanged();
    }
}

void SplitTweak::setFollowGizmo(bool follow)
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

bool SplitTweak::shouldSplit(qreal x1, qreal y1, qreal x2, qreal y2, qreal ratio) const
{
    if (ratio <= 0.0)
    {
        return false;  // No effect when ratio is 0
    }

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
    qreal distance = qSqrt(dx * dx + dy * dy);

    return distance > effectiveThreshold(ratio);
}

qreal SplitTweak::effectiveThreshold(qreal ratio) const
{
    if (ratio <= 0.0)
    {
        return 999.0;  // Effectively infinite (no splitting)
    }

    // Higher ratio = lower threshold = more splitting
    // At ratio 1.0, threshold is _splitThreshold
    // At ratio 0.5, threshold is _splitThreshold * 2
    // At ratio 0.0, threshold is infinite (no splitting)
    return _splitThreshold / ratio;
}

QJsonObject SplitTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["splitThreshold"] = _splitThreshold;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void SplitTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("splitThreshold")) setSplitThreshold(json["splitThreshold"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void SplitTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active
    auto* thresholdTrack = automationTrack(QStringLiteral("Threshold"));
    if (thresholdTrack && thresholdTrack->isAutomated())
    {
        _splitThreshold = thresholdTrack->timedValue(timeMs, 0);
    }
}

} // namespace gizmotweak2

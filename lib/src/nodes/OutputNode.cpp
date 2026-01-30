#include "OutputNode.h"
#include "core/Port.h"

#include <QJsonObject>

namespace gizmotweak2
{

OutputNode::OutputNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Output"));
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
}

void OutputNode::setZoneIndex(int index)
{
    if (index >= 0 && _zoneIndex != index)
    {
        _zoneIndex = index;
        emit zoneIndexChanged();
        emitPropertyChanged();
    }
}

void OutputNode::setEnabled(bool enabled)
{
    if (_enabled != enabled)
    {
        _enabled = enabled;
        emit enabledChanged();
        emitPropertyChanged();
    }
}

void OutputNode::setLineBreakThreshold(qreal threshold)
{
    threshold = qBound(0.0, threshold, 3.0);
    if (!qFuzzyCompare(_lineBreakThreshold, threshold))
    {
        _lineBreakThreshold = threshold;
        emit lineBreakThresholdChanged();
        emitPropertyChanged();
    }
}

QJsonObject OutputNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["zoneIndex"] = _zoneIndex;
    obj["enabled"] = _enabled;
    obj["lineBreakThreshold"] = _lineBreakThreshold;
    return obj;
}

void OutputNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("zoneIndex")) setZoneIndex(json["zoneIndex"].toInt());
    if (json.contains("enabled")) setEnabled(json["enabled"].toBool());
    if (json.contains("lineBreakThreshold")) setLineBreakThreshold(json["lineBreakThreshold"].toDouble());
}

} // namespace gizmotweak2

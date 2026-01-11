#include "OutputNode.h"
#include "core/Port.h"

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

} // namespace gizmotweak2

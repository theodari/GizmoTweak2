#include "InputNode.h"
#include "core/Port.h"

namespace gizmotweak2
{

InputNode::InputNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Input"));
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

} // namespace gizmotweak2

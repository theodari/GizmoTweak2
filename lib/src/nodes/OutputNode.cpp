#include "OutputNode.h"
#include "core/Port.h"

namespace gizmotweak2
{

OutputNode::OutputNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Output"));
    addInput(QStringLiteral("frame"), Port::DataType::Frame);
}

} // namespace gizmotweak2

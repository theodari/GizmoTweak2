#include "Port.h"
#include "Node.h"

namespace gizmotweak2
{

Port::Port(Node* parent,
           const QString& name,
           Direction direction,
           DataType dataType,
           int index)
    : QObject(parent)
    , _node(parent)
    , _name(name)
    , _direction(direction)
    , _dataType(dataType)
    , _index(index)
{
}

void Port::setScenePosition(const QPointF& pos)
{
    if (_scenePosition != pos)
    {
        _scenePosition = pos;
        emit scenePositionChanged();
    }
}

void Port::setConnected(bool connected)
{
    if (_connected != connected)
    {
        _connected = connected;
        emit connectedChanged();
    }
}

bool Port::canConnectTo(Port* other) const
{
    if (!other || other == this)
    {
        return false;
    }

    // Directions must be opposite
    if (_direction == other->_direction)
    {
        return false;
    }

    // Cannot connect to same node
    if (_node == other->_node)
    {
        return false;
    }

    // Data types must be compatible
    // Frame -> Frame only
    // Ratio2D -> Ratio2D only
    // Ratio1D -> Ratio1D only
    if (_dataType != other->_dataType)
    {
        return false;
    }

    // Connection rules:
    // - All Input ports (Direction::In) can only have one connection
    // - Frame Output ports can only have one connection (linear chain)
    // - Ratio Output ports can have multiple connections (fan-out from Shapes)

    // Check input port
    const Port* inputPort = (_direction == Direction::In) ? this : other;
    if (inputPort->isConnected())
    {
        return false;
    }

    // Check output port - Frame outputs are also single-connection
    const Port* outputPort = (_direction == Direction::Out) ? this : other;
    if (outputPort->_dataType == DataType::Frame && outputPort->isConnected())
    {
        return false;
    }

    return true;
}

} // namespace gizmotweak2

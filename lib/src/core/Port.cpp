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
    return _dataType == other->_dataType;
}

} // namespace gizmotweak2

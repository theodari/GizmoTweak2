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
        bool wasSatisfied = isSatisfied();
        _connected = connected;
        if (!connected && _dataType == DataType::RatioAny)
        {
            // Reset to RatioAny when disconnected
            setConnectedDataType(DataType::RatioAny);
        }
        emit connectedChanged();
        if (wasSatisfied != isSatisfied())
        {
            emit satisfiedChanged();
        }
    }
}

void Port::setRequired(bool required)
{
    if (_required != required)
    {
        bool wasSatisfied = isSatisfied();
        _required = required;
        if (wasSatisfied != isSatisfied())
        {
            emit satisfiedChanged();
        }
    }
}

void Port::setVisible(bool visible)
{
    if (_visible != visible)
    {
        _visible = visible;
        emit visibleChanged();
    }
}

Port::DataType Port::effectiveDataType() const
{
    if (_dataType == DataType::RatioAny && _connected)
    {
        return _connectedDataType;
    }
    return _dataType;
}

void Port::setConnectedDataType(DataType type)
{
    if (_connectedDataType != type)
    {
        _connectedDataType = type;
        emit effectiveDataTypeChanged();
    }
}

// Helper to check if two data types are compatible
static bool areTypesCompatible(Port::DataType a, Port::DataType b)
{
    if (a == b) return true;

    // RatioAny is compatible with Ratio1D, Ratio2D, and RatioAny
    if (a == Port::DataType::RatioAny)
    {
        return b == Port::DataType::Ratio1D ||
               b == Port::DataType::Ratio2D ||
               b == Port::DataType::RatioAny;
    }
    if (b == Port::DataType::RatioAny)
    {
        return a == Port::DataType::Ratio1D ||
               a == Port::DataType::Ratio2D ||
               a == Port::DataType::RatioAny;
    }

    return false;
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
    if (!areTypesCompatible(_dataType, other->_dataType))
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

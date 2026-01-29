#include "Commands.h"
#include "NodeGraph.h"
#include "Node.h"
#include "Port.h"
#include "Connection.h"

namespace gizmotweak2
{

// ============================================================================
// CreateNodeCommand
// ============================================================================

CreateNodeCommand::CreateNodeCommand(NodeGraph* graph, const QString& nodeType, QPointF position,
                                     QUndoCommand* parent)
    : QUndoCommand(parent)
    , _graph(graph)
    , _nodeType(nodeType)
    , _position(position)
{
    setText(QObject::tr("Create %1").arg(nodeType));
}

void CreateNodeCommand::undo()
{
    if (!_nodeUuid.isEmpty())
    {
        auto node = _graph->nodeByUuid(_nodeUuid);
        if (node)
        {
            // Save node state for redo
            _nodeData = node->propertiesToJson();
        }
        _graph->removeNodeInternal(_nodeUuid);
    }
}

void CreateNodeCommand::redo()
{
    if (_firstRedo)
    {
        auto node = _graph->createNodeInternal(_nodeType, _position);
        if (node)
        {
            _nodeUuid = node->uuid();
        }
        _firstRedo = false;
    }
    else
    {
        // Restore node with saved data
        auto node = _graph->createNodeInternal(_nodeType, _position);
        if (node && !_nodeData.isEmpty())
        {
            node->propertiesFromJson(_nodeData);
        }
        // Note: UUID will be different, but we update it
        if (node)
        {
            _nodeUuid = node->uuid();
        }
    }
}

// ============================================================================
// DeleteNodeCommand
// ============================================================================

DeleteNodeCommand::DeleteNodeCommand(NodeGraph* graph, const QString& nodeUuid,
                                     QUndoCommand* parent)
    : QUndoCommand(parent)
    , _graph(graph)
    , _nodeUuid(nodeUuid)
{
    auto node = graph->nodeByUuid(nodeUuid);
    if (node)
    {
        _nodeType = node->type();
        _position = node->position();
        _displayName = node->displayName();
        _properties = node->propertiesToJson();

        setText(QObject::tr("Delete %1").arg(_displayName));

        // Save incoming connections (to this node's inputs)
        for (auto port : node->inputs())
        {
            auto conn = graph->connectionForPort(port);
            if (conn)
            {
                auto sourcePort = conn->sourcePort();
                _incomingConnections.append({
                    sourcePort->node()->uuid(),
                    sourcePort->name()
                });
            }
        }

        // Save outgoing connections (from this node's outputs)
        for (auto port : node->outputs())
        {
            for (auto conn : graph->connections())
            {
                if (conn->sourcePort() == port)
                {
                    auto targetPort = conn->targetPort();
                    _outgoingConnections.append({
                        targetPort->node()->uuid(),
                        targetPort->name()
                    });
                }
            }
        }
    }
}

void DeleteNodeCommand::undo()
{
    // Recreate the node
    auto node = _graph->createNodeInternal(_nodeType, _position);
    if (node)
    {
        node->setDisplayName(_displayName);
        node->propertiesFromJson(_properties);

        // The new node has a different UUID, so we need to update our reference
        auto newUuid = node->uuid();

        // Restore incoming connections
        for (const auto& conn : _incomingConnections)
        {
            auto sourceNode = _graph->nodeByUuid(conn.first);
            if (sourceNode)
            {
                Port* sourcePort = nullptr;
                for (auto p : sourceNode->outputs())
                {
                    if (p->name() == conn.second)
                    {
                        sourcePort = p;
                        break;
                    }
                }

                // Find the corresponding input port on new node
                // We assume same port order/name as before
                Port* targetPort = nullptr;
                int portIndex = 0;
                for (auto p : node->inputs())
                {
                    if (portIndex < _incomingConnections.size())
                    {
                        targetPort = p;
                        break;
                    }
                    portIndex++;
                }

                if (sourcePort && targetPort)
                {
                    _graph->connectInternal(sourcePort, targetPort);
                }
            }
        }

        // Restore outgoing connections
        for (const auto& conn : _outgoingConnections)
        {
            auto targetNode = _graph->nodeByUuid(conn.first);
            if (targetNode)
            {
                Port* targetPort = nullptr;
                for (auto p : targetNode->inputs())
                {
                    if (p->name() == conn.second)
                    {
                        targetPort = p;
                        break;
                    }
                }

                // Find first output port of new node
                Port* sourcePort = nullptr;
                if (!node->outputs().isEmpty())
                {
                    sourcePort = node->outputs().first();
                }

                if (sourcePort && targetPort)
                {
                    _graph->connectInternal(sourcePort, targetPort);
                }
            }
        }

        _nodeUuid = newUuid;
    }
}

void DeleteNodeCommand::redo()
{
    if (_firstRedo)
    {
        _firstRedo = false;
    }
    _graph->removeNodeInternal(_nodeUuid);
}

// ============================================================================
// MoveNodeCommand
// ============================================================================

MoveNodeCommand::MoveNodeCommand(NodeGraph* graph, const QString& nodeUuid,
                                 QPointF oldPos, QPointF newPos,
                                 QUndoCommand* parent)
    : QUndoCommand(parent)
    , _graph(graph)
    , _nodeUuid(nodeUuid)
    , _oldPos(oldPos)
    , _newPos(newPos)
{
    setText(QObject::tr("Move node"));
}

void MoveNodeCommand::undo()
{
    auto node = _graph->nodeByUuid(_nodeUuid);
    if (node)
    {
        node->setPosition(_oldPos);
    }
}

void MoveNodeCommand::redo()
{
    auto node = _graph->nodeByUuid(_nodeUuid);
    if (node)
    {
        node->setPosition(_newPos);
    }
}

bool MoveNodeCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id())
    {
        return false;
    }

    auto moveCmd = static_cast<const MoveNodeCommand*>(other);
    if (moveCmd->_nodeUuid != _nodeUuid)
    {
        return false;
    }

    _newPos = moveCmd->_newPos;
    return true;
}

// ============================================================================
// ConnectCommand
// ============================================================================

ConnectCommand::ConnectCommand(NodeGraph* graph,
                               const QString& sourceNodeUuid, const QString& sourcePortName,
                               const QString& targetNodeUuid, const QString& targetPortName,
                               QUndoCommand* parent)
    : QUndoCommand(parent)
    , _graph(graph)
    , _sourceNodeUuid(sourceNodeUuid)
    , _sourcePortName(sourcePortName)
    , _targetNodeUuid(targetNodeUuid)
    , _targetPortName(targetPortName)
{
    setText(QObject::tr("Connect"));
}

void ConnectCommand::undo()
{
    auto sourceNode = _graph->nodeByUuid(_sourceNodeUuid);
    auto targetNode = _graph->nodeByUuid(_targetNodeUuid);

    if (!sourceNode || !targetNode)
    {
        return;
    }

    Port* sourcePort = nullptr;
    for (auto p : sourceNode->outputs())
    {
        if (p->name() == _sourcePortName)
        {
            sourcePort = p;
            break;
        }
    }

    Port* targetPort = nullptr;
    for (auto p : targetNode->inputs())
    {
        if (p->name() == _targetPortName)
        {
            targetPort = p;
            break;
        }
    }

    if (sourcePort && targetPort)
    {
        // Find and remove the connection
        for (auto conn : _graph->connections())
        {
            if (conn->sourcePort() == sourcePort && conn->targetPort() == targetPort)
            {
                _graph->disconnectInternal(conn);
                break;
            }
        }
    }
}

void ConnectCommand::redo()
{
    auto sourceNode = _graph->nodeByUuid(_sourceNodeUuid);
    auto targetNode = _graph->nodeByUuid(_targetNodeUuid);

    if (!sourceNode || !targetNode)
    {
        return;
    }

    Port* sourcePort = nullptr;
    for (auto p : sourceNode->outputs())
    {
        if (p->name() == _sourcePortName)
        {
            sourcePort = p;
            break;
        }
    }

    Port* targetPort = nullptr;
    for (auto p : targetNode->inputs())
    {
        if (p->name() == _targetPortName)
        {
            targetPort = p;
            break;
        }
    }

    if (sourcePort && targetPort)
    {
        _graph->connectInternal(sourcePort, targetPort);
    }
}

// ============================================================================
// DisconnectCommand
// ============================================================================

DisconnectCommand::DisconnectCommand(NodeGraph* graph, Connection* connection,
                                     QUndoCommand* parent)
    : QUndoCommand(parent)
    , _graph(graph)
{
    if (connection)
    {
        _sourceNodeUuid = connection->sourcePort()->node()->uuid();
        _sourcePortName = connection->sourcePort()->name();
        _targetNodeUuid = connection->targetPort()->node()->uuid();
        _targetPortName = connection->targetPort()->name();
    }
    setText(QObject::tr("Disconnect"));
}

void DisconnectCommand::undo()
{
    auto sourceNode = _graph->nodeByUuid(_sourceNodeUuid);
    auto targetNode = _graph->nodeByUuid(_targetNodeUuid);

    if (!sourceNode || !targetNode)
    {
        return;
    }

    Port* sourcePort = nullptr;
    for (auto p : sourceNode->outputs())
    {
        if (p->name() == _sourcePortName)
        {
            sourcePort = p;
            break;
        }
    }

    Port* targetPort = nullptr;
    for (auto p : targetNode->inputs())
    {
        if (p->name() == _targetPortName)
        {
            targetPort = p;
            break;
        }
    }

    if (sourcePort && targetPort)
    {
        _graph->connectInternal(sourcePort, targetPort);
    }
}

void DisconnectCommand::redo()
{
    auto sourceNode = _graph->nodeByUuid(_sourceNodeUuid);
    auto targetNode = _graph->nodeByUuid(_targetNodeUuid);

    if (!sourceNode || !targetNode)
    {
        return;
    }

    Port* sourcePort = nullptr;
    for (auto p : sourceNode->outputs())
    {
        if (p->name() == _sourcePortName)
        {
            sourcePort = p;
            break;
        }
    }

    Port* targetPort = nullptr;
    for (auto p : targetNode->inputs())
    {
        if (p->name() == _targetPortName)
        {
            targetPort = p;
            break;
        }
    }

    if (sourcePort && targetPort)
    {
        for (auto conn : _graph->connections())
        {
            if (conn->sourcePort() == sourcePort && conn->targetPort() == targetPort)
            {
                _graph->disconnectInternal(conn);
                break;
            }
        }
    }
}

// ============================================================================
// MarkModifiedCommand
// ============================================================================

MarkModifiedCommand::MarkModifiedCommand(QUndoCommand* parent)
    : QUndoCommand(parent)
{
    setText(QObject::tr("Modify keyframes"));
}

} // namespace gizmotweak2

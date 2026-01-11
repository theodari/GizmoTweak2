#include "NodeGraph.h"
#include "Node.h"
#include "Port.h"
#include "Connection.h"
#include "Commands.h"
#include "GraphEvaluator.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"
#include "nodes/PolarTweak.h"
#include "nodes/SparkleTweak.h"
#include "nodes/FuzzynessTweak.h"
#include "nodes/ColorFuzzynessTweak.h"
#include "nodes/SplitTweak.h"
#include "nodes/RounderTweak.h"
#include "nodes/WaveTweak.h"
#include "nodes/SqueezeTweak.h"
#include "nodes/TimeShiftNode.h"
#include "nodes/SurfaceFactoryNode.h"
#include "nodes/MirrorNode.h"

#include <QDebug>
#include <QJsonArray>
#include <QSet>
#include <limits>

namespace gizmotweak2
{

NodeGraph::NodeGraph(QObject* parent)
    : QAbstractListModel(parent)
{
    connectUndoSignals();
}

NodeGraph::~NodeGraph()
{
    // Clear connections first to avoid signaling destroyed ports
    // Delete connections before nodes are destroyed by QObject
    qDeleteAll(_connections);
    _connections.clear();

    // Clear nodes explicitly to ensure proper cleanup order
    qDeleteAll(_nodes);
    _nodes.clear();
}

void NodeGraph::connectUndoSignals()
{
    QObject::connect(&_undoStack, &QUndoStack::canUndoChanged, this, &NodeGraph::canUndoChanged);
    QObject::connect(&_undoStack, &QUndoStack::canRedoChanged, this, &NodeGraph::canRedoChanged);
    QObject::connect(&_undoStack, &QUndoStack::undoTextChanged, this, &NodeGraph::undoTextChanged);
    QObject::connect(&_undoStack, &QUndoStack::redoTextChanged, this, &NodeGraph::redoTextChanged);
}

int NodeGraph::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return _nodes.size();
}

QVariant NodeGraph::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= _nodes.size())
    {
        return QVariant();
    }

    auto node = _nodes.at(index.row());

    switch (role)
    {
    case UuidRole:
        return node->uuid();
    case TypeRole:
        return node->type();
    case CategoryRole:
        return QVariant::fromValue(node->category());
    case PositionRole:
        return node->position();
    case DisplayNameRole:
        return node->displayName();
    case SelectedRole:
        return node->isSelected();
    case NodeRole:
        return QVariant::fromValue(node);
    default:
        return QVariant();
    }
}

bool NodeGraph::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= _nodes.size())
    {
        return false;
    }

    auto node = _nodes.at(index.row());

    switch (role)
    {
    case PositionRole:
        node->setPosition(value.toPointF());
        emit dataChanged(index, index, {role});
        return true;
    case DisplayNameRole:
        node->setDisplayName(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case SelectedRole:
        node->setSelected(value.toBool());
        emit dataChanged(index, index, {role});
        return true;
    default:
        return false;
    }
}

QHash<int, QByteArray> NodeGraph::roleNames() const
{
    return {
        {UuidRole, "uuid"},
        {TypeRole, "type"},
        {CategoryRole, "category"},
        {PositionRole, "position"},
        {DisplayNameRole, "displayName"},
        {SelectedRole, "selected"},
        {NodeRole, "node"}
    };
}

Node* NodeGraph::createNode(const QString& type, QPointF position)
{
    auto cmd = new CreateNodeCommand(this, type, position);
    _undoStack.push(cmd);
    return nodeByUuid(cmd->nodeUuid());
}

Node* NodeGraph::createNodeInternal(const QString& type, QPointF position)
{
    Node* node = nullptr;

    if (type == QStringLiteral("Input"))
    {
        node = new InputNode(this);
    }
    else if (type == QStringLiteral("Output"))
    {
        node = new OutputNode(this);
    }
    else if (type == QStringLiteral("Gizmo"))
    {
        node = new GizmoNode(this);
    }
    else if (type == QStringLiteral("Transform"))
    {
        node = new GroupNode(this);
    }
    else if (type == QStringLiteral("PositionTweak"))
    {
        node = new PositionTweak(this);
    }
    else if (type == QStringLiteral("ScaleTweak"))
    {
        node = new ScaleTweak(this);
    }
    else if (type == QStringLiteral("RotationTweak"))
    {
        node = new RotationTweak(this);
    }
    else if (type == QStringLiteral("ColorTweak"))
    {
        node = new ColorTweak(this);
    }
    else if (type == QStringLiteral("PolarTweak"))
    {
        node = new PolarTweak(this);
    }
    else if (type == QStringLiteral("SparkleTweak"))
    {
        node = new SparkleTweak(this);
    }
    else if (type == QStringLiteral("FuzzynessTweak"))
    {
        node = new FuzzynessTweak(this);
    }
    else if (type == QStringLiteral("ColorFuzzynessTweak"))
    {
        node = new ColorFuzzynessTweak(this);
    }
    else if (type == QStringLiteral("SplitTweak"))
    {
        node = new SplitTweak(this);
    }
    else if (type == QStringLiteral("RounderTweak"))
    {
        node = new RounderTweak(this);
    }
    else if (type == QStringLiteral("WaveTweak"))
    {
        node = new WaveTweak(this);
    }
    else if (type == QStringLiteral("SqueezeTweak"))
    {
        node = new SqueezeTweak(this);
    }
    else if (type == QStringLiteral("TimeShift"))
    {
        node = new TimeShiftNode(this);
    }
    else if (type == QStringLiteral("SurfaceFactory"))
    {
        node = new SurfaceFactoryNode(this);
    }
    else if (type == QStringLiteral("Mirror"))
    {
        node = new MirrorNode(this);
    }

    if (node)
    {
        node->setPosition(position);
        addNode(node);
    }

    return node;
}

QStringList NodeGraph::availableNodeTypes() const
{
    return {
        QStringLiteral("Input"),
        QStringLiteral("Output"),
        QStringLiteral("Gizmo"),
        QStringLiteral("Transform"),
        QStringLiteral("PositionTweak"),
        QStringLiteral("ScaleTweak"),
        QStringLiteral("RotationTweak"),
        QStringLiteral("ColorTweak"),
        QStringLiteral("PolarTweak"),
        QStringLiteral("SparkleTweak"),
        QStringLiteral("FuzzynessTweak"),
        QStringLiteral("ColorFuzzynessTweak"),
        QStringLiteral("SplitTweak"),
        QStringLiteral("RounderTweak"),
        QStringLiteral("WaveTweak"),
        QStringLiteral("SqueezeTweak"),
        QStringLiteral("TimeShift"),
        QStringLiteral("SurfaceFactory"),
        QStringLiteral("Mirror")
    };
}

void NodeGraph::addNode(Node* node)
{
    if (!node || _nodes.contains(node))
    {
        return;
    }

    node->setParent(this);

    // Connect to selection changes to update hasSelection
    QObject::connect(node, &Node::selectedChanged, this, &NodeGraph::hasSelectionChanged);

    // Connect to property changes for preview updates
    QObject::connect(node, &Node::propertyChanged, this, &NodeGraph::nodePropertyChanged);

    // Connect to port disconnect requests (e.g., when followGizmo is disabled)
    QObject::connect(node, &Node::requestDisconnectPort, this, &NodeGraph::disconnectPortInternal);

    beginInsertRows(QModelIndex(), _nodes.size(), _nodes.size());
    _nodes.append(node);
    endInsertRows();

    emit nodeCountChanged();
    emit nodeAdded(node);
    emit graphValidityChanged();
}

void NodeGraph::removeNode(const QString& uuid)
{
    auto node = nodeByUuid(uuid);
    if (node)
    {
        _undoStack.push(new DeleteNodeCommand(this, uuid));
    }
}

void NodeGraph::removeNodeInternal(const QString& uuid)
{
    for (int i = 0; i < _nodes.size(); ++i)
    {
        if (_nodes.at(i)->uuid() == uuid)
        {
            auto node = _nodes.at(i);

            // Remove all connections to/from this node
            for (auto port : node->inputs())
            {
                disconnectPortInternal(port);
            }
            for (auto port : node->outputs())
            {
                disconnectPortInternal(port);
            }

            beginRemoveRows(QModelIndex(), i, i);
            _nodes.removeAt(i);
            endRemoveRows();

            emit nodeCountChanged();
            emit nodeRemoved(uuid);
            emit graphValidityChanged();

            node->deleteLater();
            return;
        }
    }
}

void NodeGraph::disconnectPortInternal(Port* port)
{
    if (!port)
    {
        return;
    }

    QList<Connection*> toRemove;
    for (auto conn : _connections)
    {
        if (conn->sourcePort() == port || conn->targetPort() == port)
        {
            toRemove.append(conn);
        }
    }

    for (auto conn : toRemove)
    {
        disconnectInternal(conn);
    }
}

Node* NodeGraph::nodeByUuid(const QString& uuid) const
{
    for (auto node : _nodes)
    {
        if (node->uuid() == uuid)
        {
            return node;
        }
    }
    return nullptr;
}

Node* NodeGraph::nodeAt(int index) const
{
    if (index < 0 || index >= _nodes.size())
    {
        return nullptr;
    }
    return _nodes.at(index);
}

QList<Node*> NodeGraph::selectedNodes() const
{
    QList<Node*> result;
    for (auto node : _nodes)
    {
        if (node->isSelected())
        {
            result.append(node);
        }
    }
    return result;
}

void NodeGraph::clearSelection()
{
    for (int i = 0; i < _nodes.size(); ++i)
    {
        if (_nodes.at(i)->isSelected())
        {
            _nodes.at(i)->setSelected(false);
            auto idx = index(i);
            emit dataChanged(idx, idx, {SelectedRole});
        }
    }
}

void NodeGraph::selectAll()
{
    for (int i = 0; i < _nodes.size(); ++i)
    {
        if (!_nodes.at(i)->isSelected())
        {
            _nodes.at(i)->setSelected(true);
            auto idx = index(i);
            emit dataChanged(idx, idx, {SelectedRole});
        }
    }
    emit hasSelectionChanged();
}

void NodeGraph::duplicateSelected()
{
    if (!hasSelection()) return;

    // Copy selected nodes
    copySelected();

    // Calculate center of selection for offset
    auto selected = selectedNodes();
    qreal centerX = 0, centerY = 0;
    for (auto* node : selected)
    {
        centerX += node->position().x();
        centerY += node->position().y();
    }
    centerX /= selected.size();
    centerY /= selected.size();

    // Paste at offset position (40 pixels down and right)
    QPointF pastePos(centerX + 40, centerY + 40);
    pasteAtPosition(pastePos);
}

xengine::Frame* NodeGraph::evaluate(xengine::Frame* input, qreal time)
{
    if (!_evaluator)
    {
        _evaluator = new GraphEvaluator(this);
        _evaluator->setGraph(this);
    }
    return _evaluator->evaluate(input, time);
}

QVariantList NodeGraph::evaluatePoints(const QVariantList& sourcePoints, qreal time)
{
    if (!_evaluator)
    {
        _evaluator = new GraphEvaluator(this);
        _evaluator->setGraph(this);
    }
    return _evaluator->evaluateToPoints(sourcePoints, time);
}

bool NodeGraph::isGraphComplete() const
{
    // Check all ports in all nodes for unsatisfied required ports
    for (auto node : _nodes)
    {
        for (auto port : node->inputs())
        {
            if (port->isRequired() && !port->isSatisfied())
            {
                return false;
            }
        }
    }
    return true;
}

Connection* NodeGraph::connect(Port* source, Port* target)
{
    if (!Connection::isValid(source, target))
    {
        return nullptr;
    }

    // Ensure source is Out and target is In
    if (source->direction() == Port::Direction::In)
    {
        std::swap(source, target);
    }

    // Check if connection already exists
    for (auto conn : _connections)
    {
        if ((conn->sourcePort() == source && conn->targetPort() == target) ||
            (conn->sourcePort() == target && conn->targetPort() == source))
        {
            return nullptr;
        }
    }

    _undoStack.push(new ConnectCommand(this,
        source->node()->uuid(), source->name(),
        target->node()->uuid(), target->name()));

    // Find and return the new connection
    for (auto conn : _connections)
    {
        if (conn->sourcePort() == source && conn->targetPort() == target)
        {
            return conn;
        }
    }
    return nullptr;
}

Connection* NodeGraph::connectInternal(Port* source, Port* target)
{
    if (!Connection::isValid(source, target))
    {
        return nullptr;
    }

    // Check if connection already exists
    for (auto conn : _connections)
    {
        if ((conn->sourcePort() == source && conn->targetPort() == target) ||
            (conn->sourcePort() == target && conn->targetPort() == source))
        {
            return nullptr;
        }
    }

    // Ensure source is Out and target is In
    if (source->direction() == Port::Direction::In)
    {
        std::swap(source, target);
    }

    auto connection = new Connection(source, target, this);
    _connections.append(connection);

    emit connectionCountChanged();
    emit connectionsChanged();
    emit connectionAdded(connection);
    emit graphValidityChanged();

    return connection;
}

void NodeGraph::disconnect(Connection* connection)
{
    if (!connection || !_connections.contains(connection))
    {
        return;
    }

    _undoStack.push(new DisconnectCommand(this, connection));
}

void NodeGraph::disconnectInternal(Connection* connection)
{
    if (!connection || !_connections.contains(connection))
    {
        return;
    }

    // Update port state immediately (before deleteLater)
    if (connection->sourcePort())
    {
        connection->sourcePort()->setConnected(false);
    }
    if (connection->targetPort())
    {
        connection->targetPort()->setConnected(false);
    }

    _connections.removeOne(connection);

    emit connectionCountChanged();
    emit connectionsChanged();
    emit connectionRemoved(connection);
    emit graphValidityChanged();

    connection->deleteLater();
}

void NodeGraph::disconnectPort(Port* port)
{
    if (!port)
    {
        return;
    }

    QList<Connection*> toRemove;
    for (auto conn : _connections)
    {
        if (conn->sourcePort() == port || conn->targetPort() == port)
        {
            toRemove.append(conn);
        }
    }

    for (auto conn : toRemove)
    {
        disconnect(conn);
    }
}

Connection* NodeGraph::connectionForPort(Port* port) const
{
    if (!port)
    {
        return nullptr;
    }

    for (auto conn : _connections)
    {
        if (conn->sourcePort() == port || conn->targetPort() == port)
        {
            return conn;
        }
    }
    return nullptr;
}

// Current file format version - increment when format changes
static constexpr int FILE_FORMAT_VERSION = 1;

QJsonObject NodeGraph::toJson() const
{
    QJsonObject root;
    root["version"] = FILE_FORMAT_VERSION;

    QJsonArray nodesArray;
    for (auto node : _nodes)
    {
        QJsonObject nodeObj;
        nodeObj["uuid"] = node->uuid();
        nodeObj["type"] = node->type();
        nodeObj["displayName"] = node->displayName();

        QJsonObject posObj;
        posObj["x"] = node->position().x();
        posObj["y"] = node->position().y();
        nodeObj["position"] = posObj;

        // Save node-specific properties
        auto props = node->propertiesToJson();
        if (!props.isEmpty())
        {
            nodeObj["properties"] = props;
        }

        // Save automation tracks
        auto automation = node->automationToJson();
        if (!automation.isEmpty())
        {
            nodeObj["automation"] = automation;
        }

        nodesArray.append(nodeObj);
    }
    root["nodes"] = nodesArray;

    QJsonArray connectionsArray;
    for (auto conn : _connections)
    {
        QJsonObject connObj;

        QJsonObject fromObj;
        fromObj["node"] = conn->sourcePort()->node()->uuid();
        fromObj["port"] = conn->sourcePort()->name();
        connObj["from"] = fromObj;

        QJsonObject toObj;
        toObj["node"] = conn->targetPort()->node()->uuid();
        toObj["port"] = conn->targetPort()->name();
        connObj["to"] = toObj;

        connectionsArray.append(connObj);
    }
    root["connections"] = connectionsArray;

    return root;
}

bool NodeGraph::fromJson(const QJsonObject& json)
{
    // Version check for backward compatibility
    int version = 0;
    if (json["version"].isDouble())
    {
        version = json["version"].toInt();
    }
    else if (json["version"].isString())
    {
        // Legacy support for old "0.x.y" format - treat as version 0
        version = 0;
    }

    if (version < 0 || version > FILE_FORMAT_VERSION)
    {
        qWarning() << "Unsupported file version:" << version;
        return false;
    }

    // Clear existing graph
    clear();

    // Map old UUIDs to new nodes
    QHash<QString, Node*> uuidMap;

    // Load nodes
    auto nodesArray = json["nodes"].toArray();
    for (const auto& nodeVal : nodesArray)
    {
        auto nodeObj = nodeVal.toObject();
        auto type = nodeObj["type"].toString();
        auto oldUuid = nodeObj["uuid"].toString();

        auto posObj = nodeObj["position"].toObject();
        QPointF position(posObj["x"].toDouble(), posObj["y"].toDouble());

        auto node = createNode(type, position);
        if (node)
        {
            auto displayName = nodeObj["displayName"].toString();
            if (!displayName.isEmpty())
            {
                node->setDisplayName(displayName);
            }

            // Load node-specific properties
            if (nodeObj.contains("properties"))
            {
                node->propertiesFromJson(nodeObj["properties"].toObject());
            }

            // Load automation tracks
            if (nodeObj.contains("automation"))
            {
                node->automationFromJson(nodeObj["automation"].toArray());
            }

            uuidMap[oldUuid] = node;
        }
    }

    // Load connections
    auto connectionsArray = json["connections"].toArray();
    for (const auto& connVal : connectionsArray)
    {
        auto connObj = connVal.toObject();

        auto fromObj = connObj["from"].toObject();
        auto fromNodeUuid = fromObj["node"].toString();
        auto fromPortName = fromObj["port"].toString();

        auto toObj = connObj["to"].toObject();
        auto toNodeUuid = toObj["node"].toString();
        auto toPortName = toObj["port"].toString();

        // Find nodes by old UUID
        auto fromNode = uuidMap.value(fromNodeUuid);
        auto toNode = uuidMap.value(toNodeUuid);

        if (!fromNode || !toNode)
        {
            continue;
        }

        // Find ports by name
        Port* fromPort = nullptr;
        Port* toPort = nullptr;

        for (auto port : fromNode->outputs())
        {
            if (port->name() == fromPortName)
            {
                fromPort = port;
                break;
            }
        }

        for (auto port : toNode->inputs())
        {
            if (port->name() == toPortName)
            {
                toPort = port;
                break;
            }
        }

        if (fromPort && toPort)
        {
            connect(fromPort, toPort);
        }
    }

    return true;
}

void NodeGraph::clear()
{
    // Clear undo stack first
    _undoStack.clear();

    // Remove all connections first (internally, without undo)
    while (!_connections.isEmpty())
    {
        disconnectInternal(_connections.first());
    }

    // Remove all nodes
    beginResetModel();
    for (auto node : _nodes)
    {
        emit nodeRemoved(node->uuid());
        node->deleteLater();
    }
    _nodes.clear();
    endResetModel();

    emit nodeCountChanged();
}

void NodeGraph::undo()
{
    _undoStack.undo();
}

void NodeGraph::redo()
{
    _undoStack.redo();
}

void NodeGraph::clearUndoStack()
{
    _undoStack.clear();
}

void NodeGraph::beginMoveNode(const QString& uuid)
{
    auto node = nodeByUuid(uuid);
    if (node)
    {
        _movingNodeUuid = uuid;
        _moveStartPos = node->position();
    }
}

void NodeGraph::endMoveNode(const QString& uuid, QPointF newPos)
{
    if (_movingNodeUuid == uuid && !_movingNodeUuid.isEmpty())
    {
        if (_moveStartPos != newPos)
        {
            _undoStack.push(new MoveNodeCommand(this, uuid, _moveStartPos, newPos));
        }
        _movingNodeUuid.clear();
    }
}

bool NodeGraph::hasSelection() const
{
    for (auto node : _nodes)
    {
        if (node->isSelected())
        {
            return true;
        }
    }
    return false;
}

void NodeGraph::copySelected()
{
    qDebug() << "copySelected() called";
    auto selected = selectedNodes();
    qDebug() << "Selected nodes count:" << selected.size();
    if (selected.isEmpty())
    {
        qDebug() << "No nodes selected, returning";
        return;
    }

    // Filter out Input/Output nodes (they can't be copied)
    QList<Node*> copyable;
    for (auto node : selected)
    {
        qDebug() << "Checking node:" << node->type() << node->displayName();
        if (node->type() != QStringLiteral("Input") && node->type() != QStringLiteral("Output"))
        {
            copyable.append(node);
        }
    }

    qDebug() << "Copyable nodes count:" << copyable.size();
    if (copyable.isEmpty())
    {
        qDebug() << "No copyable nodes, returning";
        return;
    }

    QJsonObject clipboard;
    QJsonArray nodesArray;
    QSet<QString> copiedUuids;

    // Find bounding box center for relative positioning
    qreal minX = std::numeric_limits<qreal>::max();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal maxY = std::numeric_limits<qreal>::lowest();

    for (auto node : copyable)
    {
        minX = qMin(minX, node->position().x());
        minY = qMin(minY, node->position().y());
        maxX = qMax(maxX, node->position().x());
        maxY = qMax(maxY, node->position().y());
        copiedUuids.insert(node->uuid());
    }

    qreal centerX = (minX + maxX) / 2.0;
    qreal centerY = (minY + maxY) / 2.0;

    // Serialize nodes with relative positions
    for (auto node : copyable)
    {
        QJsonObject nodeObj;
        nodeObj["uuid"] = node->uuid();
        nodeObj["type"] = node->type();
        nodeObj["displayName"] = node->displayName();

        QJsonObject posObj;
        posObj["x"] = node->position().x() - centerX;
        posObj["y"] = node->position().y() - centerY;
        nodeObj["position"] = posObj;

        auto props = node->propertiesToJson();
        if (!props.isEmpty())
        {
            nodeObj["properties"] = props;
        }

        // Save automation tracks
        auto automation = node->automationToJson();
        if (!automation.isEmpty())
        {
            nodeObj["automation"] = automation;
        }

        nodesArray.append(nodeObj);
    }

    clipboard["nodes"] = nodesArray;

    // Copy connections between selected nodes
    QJsonArray connectionsArray;
    for (auto conn : _connections)
    {
        auto sourceUuid = conn->sourcePort()->node()->uuid();
        auto targetUuid = conn->targetPort()->node()->uuid();

        // Only copy connections where both ends are in the selection
        if (copiedUuids.contains(sourceUuid) && copiedUuids.contains(targetUuid))
        {
            QJsonObject connObj;

            QJsonObject fromObj;
            fromObj["node"] = sourceUuid;
            fromObj["port"] = conn->sourcePort()->name();
            connObj["from"] = fromObj;

            QJsonObject toObj;
            toObj["node"] = targetUuid;
            toObj["port"] = conn->targetPort()->name();
            connObj["to"] = toObj;

            connectionsArray.append(connObj);
        }
    }

    clipboard["connections"] = connectionsArray;

    _clipboard = clipboard;
    qDebug() << "Clipboard set with" << nodesArray.size() << "nodes and" << connectionsArray.size() << "connections";
    qDebug() << "canPaste is now:" << canPaste();
    emit canPasteChanged();
}

void NodeGraph::pasteAtPosition(QPointF position)
{
    if (_clipboard.isEmpty())
    {
        return;
    }

    auto nodesArray = _clipboard["nodes"].toArray();
    if (nodesArray.isEmpty())
    {
        return;
    }

    // Map old UUIDs to new nodes
    QHash<QString, Node*> uuidMap;

    // Clear current selection
    clearSelection();

    // Create nodes
    for (const auto& nodeVal : nodesArray)
    {
        auto nodeObj = nodeVal.toObject();
        auto type = nodeObj["type"].toString();
        auto oldUuid = nodeObj["uuid"].toString();

        auto posObj = nodeObj["position"].toObject();
        QPointF relativePos(posObj["x"].toDouble(), posObj["y"].toDouble());
        QPointF newPos = position + relativePos;

        // Snap to grid
        int gridSize = 20;
        newPos.setX(qRound(newPos.x() / gridSize) * gridSize);
        newPos.setY(qRound(newPos.y() / gridSize) * gridSize);

        auto node = createNodeInternal(type, newPos);
        if (node)
        {
            auto displayName = nodeObj["displayName"].toString();
            if (!displayName.isEmpty())
            {
                node->setDisplayName(displayName);
            }

            if (nodeObj.contains("properties"))
            {
                node->propertiesFromJson(nodeObj["properties"].toObject());
            }

            // Load automation tracks
            if (nodeObj.contains("automation"))
            {
                node->automationFromJson(nodeObj["automation"].toArray());
            }

            // Select the pasted node
            node->setSelected(true);

            uuidMap[oldUuid] = node;
        }
    }

    // Recreate connections between pasted nodes
    auto connectionsArray = _clipboard["connections"].toArray();
    for (const auto& connVal : connectionsArray)
    {
        auto connObj = connVal.toObject();

        auto fromObj = connObj["from"].toObject();
        auto fromNodeUuid = fromObj["node"].toString();
        auto fromPortName = fromObj["port"].toString();

        auto toObj = connObj["to"].toObject();
        auto toNodeUuid = toObj["node"].toString();
        auto toPortName = toObj["port"].toString();

        auto fromNode = uuidMap.value(fromNodeUuid);
        auto toNode = uuidMap.value(toNodeUuid);

        if (!fromNode || !toNode)
        {
            continue;
        }

        Port* fromPort = nullptr;
        for (auto port : fromNode->outputs())
        {
            if (port->name() == fromPortName)
            {
                fromPort = port;
                break;
            }
        }

        Port* toPort = nullptr;
        for (auto port : toNode->inputs())
        {
            if (port->name() == toPortName)
            {
                toPort = port;
                break;
            }
        }

        if (fromPort && toPort)
        {
            connectInternal(fromPort, toPort);
        }
    }

    emit hasSelectionChanged();
}

void NodeGraph::cutSelected()
{
    copySelected();

    // Delete selected nodes (except Input/Output)
    auto selected = selectedNodes();
    for (auto node : selected)
    {
        if (node->type() != QStringLiteral("Input") && node->type() != QStringLiteral("Output"))
        {
            removeNode(node->uuid());
        }
    }
}

} // namespace gizmotweak2

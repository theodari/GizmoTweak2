#include "NodeGraph.h"
#include "Node.h"
#include "Port.h"
#include "Connection.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"
#include "nodes/TimeShiftNode.h"
#include "nodes/SurfaceFactoryNode.h"

#include <QJsonArray>

namespace gizmotweak2
{

NodeGraph::NodeGraph(QObject* parent)
    : QAbstractListModel(parent)
{
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
    else if (type == QStringLiteral("Group"))
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
    else if (type == QStringLiteral("TimeShift"))
    {
        node = new TimeShiftNode(this);
    }
    else if (type == QStringLiteral("SurfaceFactory"))
    {
        node = new SurfaceFactoryNode(this);
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
        QStringLiteral("Group"),
        QStringLiteral("PositionTweak"),
        QStringLiteral("ScaleTweak"),
        QStringLiteral("RotationTweak"),
        QStringLiteral("ColorTweak"),
        QStringLiteral("TimeShift"),
        QStringLiteral("SurfaceFactory")
    };
}

void NodeGraph::addNode(Node* node)
{
    if (!node || _nodes.contains(node))
    {
        return;
    }

    node->setParent(this);

    beginInsertRows(QModelIndex(), _nodes.size(), _nodes.size());
    _nodes.append(node);
    endInsertRows();

    emit nodeCountChanged();
    emit nodeAdded(node);
}

void NodeGraph::removeNode(const QString& uuid)
{
    for (int i = 0; i < _nodes.size(); ++i)
    {
        if (_nodes.at(i)->uuid() == uuid)
        {
            auto node = _nodes.at(i);

            // Remove all connections to/from this node
            for (auto port : node->inputs())
            {
                disconnectPort(port);
            }
            for (auto port : node->outputs())
            {
                disconnectPort(port);
            }

            beginRemoveRows(QModelIndex(), i, i);
            _nodes.removeAt(i);
            endRemoveRows();

            emit nodeCountChanged();
            emit nodeRemoved(uuid);

            node->deleteLater();
            return;
        }
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

Connection* NodeGraph::connect(Port* source, Port* target)
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

    return connection;
}

void NodeGraph::disconnect(Connection* connection)
{
    if (!connection || !_connections.contains(connection))
    {
        return;
    }

    _connections.removeOne(connection);

    emit connectionCountChanged();
    emit connectionsChanged();
    emit connectionRemoved(connection);

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

QJsonObject NodeGraph::toJson() const
{
    QJsonObject root;
    root["version"] = "0.2.0";

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
    auto version = json["version"].toString();
    if (version.isEmpty())
    {
        qWarning() << "No version in file, cannot load";
        return false;
    }

    // Parse version (format: "major.minor.patch")
    auto versionParts = version.split('.');
    int majorVersion = versionParts.size() > 0 ? versionParts[0].toInt() : 0;
    int minorVersion = versionParts.size() > 1 ? versionParts[1].toInt() : 0;

    // Current format version is 0.2.x
    // We support loading 0.2.x files
    if (majorVersion != 0 || minorVersion < 2)
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
    // Remove all connections first
    while (!_connections.isEmpty())
    {
        disconnect(_connections.first());
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

} // namespace gizmotweak2

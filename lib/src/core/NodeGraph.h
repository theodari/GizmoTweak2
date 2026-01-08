#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QJsonObject>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class Node;
class Port;
class Connection;

class NodeGraph : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY nodeCountChanged)
    Q_PROPERTY(int connectionCount READ connectionCount NOTIFY connectionCountChanged)
    Q_PROPERTY(QList<Connection*> connections READ connections NOTIFY connectionsChanged)

public:
    enum Roles
    {
        UuidRole = Qt::UserRole + 1,
        TypeRole,
        CategoryRole,
        PositionRole,
        DisplayNameRole,
        SelectedRole,
        NodeRole
    };

    explicit NodeGraph(QObject* parent = nullptr);
    ~NodeGraph() override = default;

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int nodeCount() const { return _nodes.size(); }
    int connectionCount() const { return _connections.size(); }
    QList<Connection*> connections() const { return _connections; }

    // Node factory
    Q_INVOKABLE Node* createNode(const QString& type, QPointF position);
    Q_INVOKABLE QStringList availableNodeTypes() const;

    // Node management
    Q_INVOKABLE void addNode(Node* node);
    Q_INVOKABLE void removeNode(const QString& uuid);
    Q_INVOKABLE Node* nodeByUuid(const QString& uuid) const;
    Q_INVOKABLE QList<Node*> selectedNodes() const;
    Q_INVOKABLE void clearSelection();

    // Connection management
    Q_INVOKABLE Connection* connect(Port* source, Port* target);
    Q_INVOKABLE void disconnect(Connection* connection);
    Q_INVOKABLE void disconnectPort(Port* port);
    Q_INVOKABLE Connection* connectionForPort(Port* port) const;

    // Persistence
    Q_INVOKABLE QJsonObject toJson() const;
    Q_INVOKABLE bool fromJson(const QJsonObject& json);
    Q_INVOKABLE void clear();

signals:
    void nodeCountChanged();
    void connectionCountChanged();
    void connectionsChanged();
    void nodeAdded(Node* node);
    void nodeRemoved(const QString& uuid);
    void connectionAdded(Connection* connection);
    void connectionRemoved(Connection* connection);

private:
    QList<Node*> _nodes;
    QList<Connection*> _connections;
};

} // namespace gizmotweak2

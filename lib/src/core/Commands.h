#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QString>
#include <QJsonObject>

namespace gizmotweak2
{

class NodeGraph;
class Node;
class Port;
class Connection;

// Command for creating a new node
class CreateNodeCommand : public QUndoCommand
{
public:
    CreateNodeCommand(NodeGraph* graph, const QString& nodeType, QPointF position,
                      QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    QString nodeUuid() const { return _nodeUuid; }

private:
    NodeGraph* _graph;
    QString _nodeType;
    QPointF _position;
    QString _nodeUuid;
    QJsonObject _nodeData;  // For restoring on redo after undo
    bool _firstRedo{true};
};

// Command for deleting a node
class DeleteNodeCommand : public QUndoCommand
{
public:
    DeleteNodeCommand(NodeGraph* graph, const QString& nodeUuid,
                      QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    NodeGraph* _graph;
    QString _nodeUuid;
    QString _nodeType;
    QPointF _position;
    QString _displayName;
    QJsonObject _properties;
    QList<QPair<QString, QString>> _incomingConnections;  // sourceNodeUuid, sourcePortName
    QList<QPair<QString, QString>> _outgoingConnections;  // targetNodeUuid, targetPortName
    bool _firstRedo{true};
};

// Command for moving a node
class MoveNodeCommand : public QUndoCommand
{
public:
    MoveNodeCommand(NodeGraph* graph, const QString& nodeUuid,
                    QPointF oldPos, QPointF newPos,
                    QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 1; }
    bool mergeWith(const QUndoCommand* other) override;

private:
    NodeGraph* _graph;
    QString _nodeUuid;
    QPointF _oldPos;
    QPointF _newPos;
};

// Command for creating a connection
class ConnectCommand : public QUndoCommand
{
public:
    ConnectCommand(NodeGraph* graph,
                   const QString& sourceNodeUuid, const QString& sourcePortName,
                   const QString& targetNodeUuid, const QString& targetPortName,
                   QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    NodeGraph* _graph;
    QString _sourceNodeUuid;
    QString _sourcePortName;
    QString _targetNodeUuid;
    QString _targetPortName;
};

// Command for removing a connection
class DisconnectCommand : public QUndoCommand
{
public:
    DisconnectCommand(NodeGraph* graph, Connection* connection,
                      QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    NodeGraph* _graph;
    QString _sourceNodeUuid;
    QString _sourcePortName;
    QString _targetNodeUuid;
    QString _targetPortName;
};

} // namespace gizmotweak2

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QJsonObject>
#include <QPointF>
#include <QUndoStack>
#include <QtQml/qqmlregistration.h>

#include <frame.h>

namespace gizmotweak2
{

class Node;
class Port;
class Connection;
class GraphEvaluator;

class NodeGraph : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY nodeCountChanged)
    Q_PROPERTY(int connectionCount READ connectionCount NOTIFY connectionCountChanged)
    Q_PROPERTY(QList<Connection*> connections READ connections NOTIFY connectionsChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoTextChanged)
    Q_PROPERTY(QString redoText READ redoText NOTIFY redoTextChanged)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY canPasteChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)
    Q_PROPERTY(bool isGraphComplete READ isGraphComplete NOTIFY graphValidityChanged)
    Q_PROPERTY(bool isModified READ isModified NOTIFY modifiedChanged)

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
    ~NodeGraph() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int nodeCount() const { return _nodes.size(); }
    int connectionCount() const { return _connections.size(); }
    QList<Connection*> connections() const { return _connections; }

    // Undo/Redo properties
    bool canUndo() const { return _undoStack.canUndo(); }
    bool canRedo() const { return _undoStack.canRedo(); }
    QString undoText() const { return _undoStack.undoText(); }
    QString redoText() const { return _undoStack.redoText(); }

    // Modified state (based on undo stack clean index)
    bool isModified() const { return !_undoStack.isClean(); }
    Q_INVOKABLE void setClean();  // Call after save to mark as unmodified
    Q_INVOKABLE void markAsModified();  // Call to mark graph as modified (for operations outside undo system)

    // Clipboard properties
    bool canPaste() const { return !_clipboard.isEmpty(); }
    bool hasSelection() const;

    // Node factory
    Q_INVOKABLE Node* createNode(const QString& type, QPointF position);
    Q_INVOKABLE QStringList availableNodeTypes() const;

    // Node management
    Q_INVOKABLE void addNode(Node* node);
    Q_INVOKABLE void removeNode(const QString& uuid);
    Q_INVOKABLE Node* nodeByUuid(const QString& uuid) const;
    Q_INVOKABLE Node* nodeAt(int index) const;
    Q_INVOKABLE QList<Node*> selectedNodes() const;
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void duplicateSelected();

    // Align/distribute selected nodes
    Q_INVOKABLE void alignSelected(const QString& mode);       // "left","center","right","top","middle","bottom"
    Q_INVOKABLE void distributeSelected(const QString& mode);   // "horizontal","vertical"

    // Graph evaluation - returns transformed Frame
    Q_INVOKABLE xengine::Frame* evaluate(xengine::Frame* input, qreal time = 0.0);

    // Evaluate graph up to (and including) a specific node
    xengine::Frame* evaluateUpTo(xengine::Frame* input, Node* stopNode, qreal time = 0.0);

    // Graph evaluation - returns transformed points array [{x,y,r,g,b}, ...]
    Q_INVOKABLE QVariantList evaluatePoints(const QVariantList& sourcePoints, qreal time = 0.0);

    // Graph validation
    bool isGraphComplete() const;

    // Connection management
    Q_INVOKABLE Connection* connect(Port* source, Port* target);
    Q_INVOKABLE void disconnect(Connection* connection);
    Q_INVOKABLE void disconnectPort(Port* port);
    Q_INVOKABLE Connection* connectionForPort(Port* port) const;

    // Persistence
    Q_INVOKABLE QJsonObject toJson() const;
    Q_INVOKABLE bool fromJson(const QJsonObject& json);
    Q_INVOKABLE void clear();

    // Undo/Redo
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clearUndoStack();

    // Move tracking for undo
    Q_INVOKABLE void beginMoveNode(const QString& uuid);
    Q_INVOKABLE void endMoveNode(const QString& uuid, QPointF newPos);

    // Clipboard operations
    Q_INVOKABLE void copySelected();
    Q_INVOKABLE void pasteAtPosition(QPointF position);
    Q_INVOKABLE void cutSelected();

    // Internal methods (used by undo commands)
    Node* createNodeInternal(const QString& type, QPointF position);
    void removeNodeInternal(const QString& uuid);
    Connection* connectInternal(Port* source, Port* target);
    void disconnectInternal(Connection* connection);
    void disconnectPortInternal(Port* port);

signals:
    void nodeCountChanged();
    void connectionCountChanged();
    void connectionsChanged();
    void nodeAdded(Node* node);
    void nodeRemoved(const QString& uuid);
    void connectionAdded(Connection* connection);
    void connectionRemoved(Connection* connection);
    void canUndoChanged();
    void nodePropertyChanged();  // Emitted when any node's property changes
    void canRedoChanged();
    void undoTextChanged();
    void redoTextChanged();
    void canPasteChanged();
    void hasSelectionChanged();
    void graphValidityChanged();
    void modifiedChanged();

private:
    void connectUndoSignals();

    QList<Node*> _nodes;
    QList<Connection*> _connections;
    QUndoStack _undoStack;

    // Move tracking
    QString _movingNodeUuid;
    QPointF _moveStartPos;

    // Clipboard
    QJsonObject _clipboard;

    // Evaluator
    GraphEvaluator* _evaluator{nullptr};
};

} // namespace gizmotweak2

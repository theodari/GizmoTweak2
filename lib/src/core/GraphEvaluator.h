#pragma once

#include <QObject>
#include <QList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <frame.h>

namespace gizmotweak2
{

class NodeGraph;
class Node;
class Port;

class GraphEvaluator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isGraphComplete READ isGraphComplete NOTIFY graphValidityChanged)
    Q_PROPERTY(QStringList validationErrors READ validationErrors NOTIFY graphValidityChanged)

public:
    explicit GraphEvaluator(QObject* parent = nullptr);
    ~GraphEvaluator() override = default;

    void setGraph(NodeGraph* graph);
    NodeGraph* graph() const { return _graph; }

    // Evaluate the graph and return the resulting Frame
    Q_INVOKABLE xengine::Frame* evaluate(xengine::Frame* input, qreal time = 0.0);

    // Evaluate and return points as QVariantList for QML (array of {x, y, r, g, b})
    Q_INVOKABLE QVariantList evaluateToPoints(const QVariantList& inputPoints, qreal time = 0.0);

    // Validation
    bool isGraphComplete() const;
    QStringList validationErrors() const;

signals:
    void graphValidityChanged();

private:
    // Build the execution order: Input → Tweaks → Output
    QList<Node*> buildFramePath();

    // Find a node by type
    Node* findNodeByType(const QString& type) const;

    // Get the node connected to a port
    Node* getConnectedNode(Port* port) const;

    // Evaluate ratio nodes recursively
    // Returns the ratio value for a given position and time
    qreal evaluateRatioChain(Port* ratioPort, qreal x, qreal y, qreal time) const;

    // Apply a single tweak to a point
    struct Point { qreal x, y, r, g, b; };
    Point applyTweak(Node* tweakNode, const Point& input, qreal ratio, qreal time, int sampleIndex,
                     qreal gizmoX, qreal gizmoY) const;

    // Find the first connected Gizmo's center coordinates for a tweak
    // Returns (0, 0) if no Gizmo is connected
    QPointF findConnectedGizmoCenter(Port* ratioPort) const;

    NodeGraph* _graph{nullptr};
    mutable QStringList _validationErrors;
};

} // namespace gizmotweak2

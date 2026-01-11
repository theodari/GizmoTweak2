#pragma once

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

#include <frame.h>
#include "core/Node.h"
#include "core/NodeGraph.h"

class FramePreviewItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    // Mode 1: Observe a single node (for InputNode mini-preview)
    Q_PROPERTY(gizmotweak2::Node* node READ node WRITE setNode NOTIFY nodeChanged)

    // Mode 2: Evaluate a graph at a given time (for PreviewPanel)
    Q_PROPERTY(gizmotweak2::NodeGraph* graph READ graph WRITE setGraph NOTIFY graphChanged)
    Q_PROPERTY(qreal time READ time WRITE setTime NOTIFY timeChanged)

    // Visual properties
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)

public:
    explicit FramePreviewItem(QQuickItem* parent = nullptr);
    ~FramePreviewItem() override;

    void paint(QPainter* painter) override;

    // Node mode - observe a single node
    gizmotweak2::Node* node() const { return _node; }
    void setNode(gizmotweak2::Node* node);

    // Graph mode - evaluate graph at time
    gizmotweak2::NodeGraph* graph() const { return _graph; }
    void setGraph(gizmotweak2::NodeGraph* graph);

    qreal time() const { return _time; }
    void setTime(qreal time);

    // Visual properties
    bool showGrid() const { return _showGrid; }
    void setShowGrid(bool show);

    QColor gridColor() const { return _gridColor; }
    void setGridColor(const QColor& color);

    QColor backgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const QColor& color);

    qreal lineWidth() const { return _lineWidth; }
    void setLineWidth(qreal width);

signals:
    void nodeChanged();
    void graphChanged();
    void timeChanged();
    void showGridChanged();
    void gridColorChanged();
    void backgroundColorChanged();
    void lineWidthChanged();

private slots:
    void onNodeFrameChanged();
    void onGraphChanged();

private:
    void connectToNode();
    void disconnectFromNode();
    void connectToGraph();
    void disconnectFromGraph();
    void drawGrid(QPainter* painter);

    // Get frame depending on mode (node or graph)
    xengine::Frame* getCurrentFrame();

    // Evaluate graph on main thread (called when graph or time changes)
    void evaluateGraph();

    // Find InputNode in graph
    gizmotweak2::Node* findInputNode() const;

    // Node mode
    gizmotweak2::Node* _node{nullptr};

    // Graph mode
    gizmotweak2::NodeGraph* _graph{nullptr};
    qreal _time{0.0};
    xengine::Frame* _evaluatedFrame{nullptr};  // Owned by this class when in graph mode

    // Visual properties
    bool _showGrid{true};
    QColor _gridColor{40, 40, 40};
    QColor _backgroundColor{0, 0, 0};
    qreal _lineWidth{2.0};
};

#pragma once

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
#include <QHash>
#include <QImage>

#include "core/Node.h"
#include "core/NodeGraph.h"
#include "core/Port.h"

// Cache for computed ratio grids - shared across all NodePreviewItem instances
// Key: node pointer, Value: cached grid data with metadata
struct RatioGridCache
{
    struct CacheEntry
    {
        QVector<qreal> ratios;  // resolution x resolution grid
        int resolution{0};
        qreal time{0.0};
        bool valid{false};
    };

    QHash<gizmotweak2::Node*, CacheEntry> entries;

    void clear() { entries.clear(); }
    void invalidate(gizmotweak2::Node* node) { entries.remove(node); }
};

class NodePreviewItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(gizmotweak2::Node* node READ node WRITE setNode NOTIFY nodeChanged)
    Q_PROPERTY(gizmotweak2::NodeGraph* graph READ graph WRITE setGraph NOTIFY graphChanged)
    Q_PROPERTY(qreal currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(int resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)

public:
    explicit NodePreviewItem(QQuickItem* parent = nullptr);
    ~NodePreviewItem() override = default;

    void paint(QPainter* painter) override;

    gizmotweak2::Node* node() const { return _node; }
    void setNode(gizmotweak2::Node* node);

    gizmotweak2::NodeGraph* graph() const { return _graph; }
    void setGraph(gizmotweak2::NodeGraph* graph);

    qreal currentTime() const { return _currentTime; }
    void setCurrentTime(qreal time);

    int resolution() const { return _resolution; }
    void setResolution(int res);

signals:
    void nodeChanged();
    void graphChanged();
    void currentTimeChanged();
    void resolutionChanged();

private:
    // Evaluate a node's ratio output at position (x, y) with time
    qreal evaluateNode(gizmotweak2::Node* node, qreal x, qreal y, qreal time,
                       int pointIndex = 0, int totalPoints = 1) const;

    // Follow a port connection and evaluate the source node
    qreal evaluatePort(gizmotweak2::Port* port, qreal x, qreal y, qreal time,
                       int pointIndex, int totalPoints) const;

    // Get the node connected to an input port
    gizmotweak2::Node* getConnectedNode(gizmotweak2::Port* port) const;

    // Convert ratio value to color (blue-cyan-green for positive, red-magenta for negative)
    QColor ratioToColor(qreal ratio) const;

    // Draw shape heatmap
    void paintShapeHeatmap(QPainter* painter);

    // Draw SurfaceFactory curve
    void paintSurfaceCurve(QPainter* painter);

    // Draw node-specific icons
    void paintGizmoIcon(QPainter* painter);
    void paintGroupIcon(QPainter* painter);
    void paintMirrorIcon(QPainter* painter);
    void paintTimeShiftIcon(QPainter* painter);
    void paintTweakIcon(QPainter* painter);

    // Compute or retrieve cached ratio grid for a shape node
    const QVector<qreal>& getOrComputeRatioGrid(gizmotweak2::Node* node, int resolution, qreal time) const;

    // Compute full ratio grid for a node
    QVector<qreal> computeRatioGrid(gizmotweak2::Node* node, int resolution, qreal time) const;

    gizmotweak2::Node* _node{nullptr};
    gizmotweak2::NodeGraph* _graph{nullptr};
    qreal _currentTime{0.0};
    int _resolution{16};

    // Static cache shared by all NodePreviewItem instances
    static RatioGridCache s_cache;
};

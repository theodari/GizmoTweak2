#pragma once

#include <QQuickImageProvider>

namespace gizmotweak2 { class NodeGraph; }

class PatternImageProvider : public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(gizmotweak2::NodeGraph* graph READ graph WRITE setGraph)

public:
    explicit PatternImageProvider();

    gizmotweak2::NodeGraph* graph() const { return _graph; }
    void setGraph(gizmotweak2::NodeGraph* graph);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    gizmotweak2::NodeGraph* _graph{nullptr};
};

#include "FramePreviewItem.h"

#include <QPainter>

#include "core/Node.h"
#include "core/NodeGraph.h"
#include "nodes/InputNode.h"
#include "ExcaliburEngine.h"

using namespace gizmotweak2;

FramePreviewItem::FramePreviewItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}

FramePreviewItem::~FramePreviewItem()
{
    delete _evaluatedFrame;
}

// ============================================================================
// Node mode (for InputNode mini-preview)
// ============================================================================

void FramePreviewItem::setNode(gizmotweak2::Node* node)
{
    if (_node != node)
    {
        disconnectFromNode();
        _node = node;
        connectToNode();
        emit nodeChanged();
        update();
    }
}

void FramePreviewItem::connectToNode()
{
    if (!_node)
        return;

    // Connect to InputNode's currentFrameChanged signal
    if (_node->type() == QStringLiteral("Input"))
    {
        auto* inputNode = qobject_cast<InputNode*>(_node);
        if (inputNode)
        {
            connect(inputNode, &InputNode::currentFrameChanged,
                    this, &FramePreviewItem::onNodeFrameChanged);
        }
    }

    // Also connect to node's propertyChanged for other updates
    connect(_node, &Node::propertyChanged,
            this, &FramePreviewItem::onNodeFrameChanged);
}

void FramePreviewItem::disconnectFromNode()
{
    if (!_node)
        return;

    disconnect(_node, nullptr, this, nullptr);
}

void FramePreviewItem::onNodeFrameChanged()
{
    update();
}

// ============================================================================
// Graph mode (for PreviewPanel - evaluates entire graph)
// ============================================================================

void FramePreviewItem::setGraph(gizmotweak2::NodeGraph* graph)
{
    if (_graph != graph)
    {
        disconnectFromGraph();
        _graph = graph;
        connectToGraph();
        emit graphChanged();
        evaluateGraph();  // Evaluate on main thread
        update();
    }
}

void FramePreviewItem::setTime(qreal time)
{
    if (!qFuzzyCompare(_time, time))
    {
        _time = time;
        emit timeChanged();
        evaluateGraph();  // Evaluate on main thread, not in paint()
        update();
    }
}

void FramePreviewItem::connectToGraph()
{
    if (!_graph)
        return;

    // Connect to graph changes
    connect(_graph, &NodeGraph::rowsInserted, this, &FramePreviewItem::onGraphChanged);
    connect(_graph, &NodeGraph::rowsRemoved, this, &FramePreviewItem::onGraphChanged);
    connect(_graph, &NodeGraph::dataChanged, this, &FramePreviewItem::onGraphChanged);
    connect(_graph, &NodeGraph::connectionsChanged, this, &FramePreviewItem::onGraphChanged);
    connect(_graph, &NodeGraph::nodePropertyChanged, this, &FramePreviewItem::onGraphChanged);
}

void FramePreviewItem::disconnectFromGraph()
{
    if (!_graph)
        return;

    disconnect(_graph, nullptr, this, nullptr);
}

void FramePreviewItem::onGraphChanged()
{
    evaluateGraph();  // Evaluate on main thread
    update();
}

void FramePreviewItem::evaluateGraph()
{
    // Only evaluate in graph mode
    if (!_graph)
        return;

    // Find InputNode to get source frame
    auto* inputNodeBase = findInputNode();
    if (!inputNodeBase)
    {
        delete _evaluatedFrame;
        _evaluatedFrame = nullptr;
        return;
    }

    auto* inputNode = qobject_cast<InputNode*>(inputNodeBase);
    if (!inputNode)
    {
        delete _evaluatedFrame;
        _evaluatedFrame = nullptr;
        return;
    }

    auto* sourceFrame = inputNode->currentFrame();
    if (!sourceFrame)
    {
        delete _evaluatedFrame;
        _evaluatedFrame = nullptr;
        return;
    }

    // Evaluate graph and store result
    auto* result = _graph->evaluate(sourceFrame, _time);

    // Replace the stored frame
    if (_evaluatedFrame != result)
    {
        delete _evaluatedFrame;
        _evaluatedFrame = result;
    }

    // Send to laser engine
    sendFrameToZone();
}

Node* FramePreviewItem::findInputNode() const
{
    if (!_graph)
        return nullptr;

    for (int i = 0; i < _graph->rowCount(); ++i)
    {
        auto* node = _graph->nodeAt(i);
        if (node && node->type() == QStringLiteral("Input"))
        {
            return node;
        }
    }
    return nullptr;
}

// ============================================================================
// Laser engine output
// ============================================================================

void FramePreviewItem::setLaserEngine(gizmotweak2::ExcaliburEngine* engine)
{
    if (_laserEngine != engine)
    {
        _laserEngine = engine;
        emit laserEngineChanged();
    }
}

void FramePreviewItem::setZoneIndex(int index)
{
    if (_zoneIndex != index)
    {
        _zoneIndex = index;
        emit zoneIndexChanged();
    }
}

void FramePreviewItem::sendFrameToZone()
{
    if (!_laserEngine || !_evaluatedFrame)
        return;

    QVariantList points;
    for (int i = 0; i < _evaluatedFrame->size(); ++i)
    {
        const auto& sample = _evaluatedFrame->at(i);
        QVariantMap point;
        point[QStringLiteral("x")] = sample.getX();
        point[QStringLiteral("y")] = sample.getY();
        point[QStringLiteral("r")] = sample.getR();
        point[QStringLiteral("g")] = sample.getG();
        point[QStringLiteral("b")] = sample.getB();
        points.append(point);
    }

    _laserEngine->sendFrame(_zoneIndex, points);
}

// ============================================================================
// Frame retrieval (handles both modes)
// ============================================================================

xengine::Frame* FramePreviewItem::getCurrentFrame()
{
    // Mode 1: Node mode - get frame directly from InputNode
    if (_node)
    {
        if (_node->type() == QStringLiteral("Input"))
        {
            auto* inputNode = qobject_cast<InputNode*>(_node);
            if (inputNode)
            {
                return inputNode->currentFrame();
            }
        }
        return nullptr;
    }

    // Mode 2: Graph mode - return cached evaluated frame
    // (evaluation is done on main thread in evaluateGraph())
    if (_graph)
    {
        return _evaluatedFrame;
    }

    return nullptr;
}

// ============================================================================
// Visual properties
// ============================================================================

void FramePreviewItem::setShowGrid(bool show)
{
    if (_showGrid != show)
    {
        _showGrid = show;
        emit showGridChanged();
        update();
    }
}

void FramePreviewItem::setGridColor(const QColor& color)
{
    if (_gridColor != color)
    {
        _gridColor = color;
        emit gridColorChanged();
        update();
    }
}

void FramePreviewItem::setBackgroundColor(const QColor& color)
{
    if (_backgroundColor != color)
    {
        _backgroundColor = color;
        emit backgroundColorChanged();
        update();
    }
}

void FramePreviewItem::setLineWidth(qreal width)
{
    if (!qFuzzyCompare(_lineWidth, width))
    {
        _lineWidth = width;
        emit lineWidthChanged();
        update();
    }
}

// ============================================================================
// Painting
// ============================================================================

void FramePreviewItem::drawGrid(QPainter* painter)
{
    int w = static_cast<int>(width());
    int h = static_cast<int>(height());

    painter->setPen(QPen(_gridColor, 1));

    // Draw 4x4 grid lines
    for (int i = 1; i < 4; ++i)
    {
        int x = w * i / 4;
        int y = h * i / 4;
        painter->drawLine(x, 0, x, h);
        painter->drawLine(0, y, w, y);
    }

    // Draw center cross (brighter)
    painter->setPen(QPen(_gridColor.lighter(150), 1));
    int cx = w / 2;
    int cy = h / 2;
    painter->drawLine(cx - 5, cy, cx + 5, cy);
    painter->drawLine(cx, cy - 5, cx, cy + 5);
}

void FramePreviewItem::paint(QPainter* painter)
{
    int w = static_cast<int>(width());
    int h = static_cast<int>(height());

    // Background
    painter->fillRect(0, 0, w, h, _backgroundColor);

    // Grid
    if (_showGrid)
    {
        drawGrid(painter);
    }

    // Get frame (from node or graph evaluation)
    auto* frame = getCurrentFrame();

    // Frame rendering - fills 100% of the square
    if (frame && frame->size() > 0)
    {
        frame->render(painter, 0, 0, w, h, _lineWidth);
    }
    else
    {
        // No data message
        painter->setPen(QColor(100, 100, 100));
        painter->setFont(QFont("sans-serif", 12));
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No data");
    }
}

#include "NodePreviewItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <cmath>

#include "core/Node.h"
#include "core/NodeGraph.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/SurfaceFactoryNode.h"
#include "nodes/TimeShiftNode.h"
#include "nodes/MirrorNode.h"

using namespace gizmotweak2;

// Static cache definition
RatioGridCache NodePreviewItem::s_cache;

NodePreviewItem::NodePreviewItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}

void NodePreviewItem::setNode(Node* node)
{
    if (_node != node)
    {
        if (_node)
        {
            disconnect(_node, nullptr, this, nullptr);
        }

        _node = node;

        if (_node)
        {
            connect(_node, &Node::propertyChanged, this, [this]() {
                // Invalidate cache for this node when its properties change
                s_cache.invalidate(_node);
                update();
            });
        }

        emit nodeChanged();
        update();
    }
}

void NodePreviewItem::setGraph(NodeGraph* graph)
{
    if (_graph != graph)
    {
        if (_graph)
        {
            disconnect(_graph, nullptr, this, nullptr);
        }

        _graph = graph;

        if (_graph)
        {
            // Clear cache when connections change (affects recursive evaluation)
            connect(_graph, &NodeGraph::connectionAdded, this, [this]() {
                s_cache.clear();
                update();
            });
            connect(_graph, &NodeGraph::connectionRemoved, this, [this]() {
                s_cache.clear();
                update();
            });
            // Update when any node's properties change (connected nodes might affect this preview)
            connect(_graph, &NodeGraph::nodePropertyChanged, this, [this]() {
                s_cache.clear();
                update();
            });
        }

        emit graphChanged();
        update();
    }
}

void NodePreviewItem::setCurrentTime(qreal time)
{
    if (!qFuzzyCompare(_currentTime, time))
    {
        _currentTime = time;
        // Clear entire cache when time changes - all grids need recomputation
        s_cache.clear();
        emit currentTimeChanged();
        update();
    }
}

void NodePreviewItem::setResolution(int res)
{
    res = qBound(4, res, 64);
    if (_resolution != res)
    {
        _resolution = res;
        emit resolutionChanged();
        update();
    }
}

Node* NodePreviewItem::getConnectedNode(Port* port) const
{
    if (!port || !_graph) return nullptr;

    const auto& connections = _graph->connections();
    for (auto* conn : connections)
    {
        if (port->direction() == Port::Direction::In)
        {
            if (conn->targetPort() == port)
            {
                return conn->sourcePort()->node();
            }
        }
        else
        {
            if (conn->sourcePort() == port)
            {
                return conn->targetPort()->node();
            }
        }
    }
    return nullptr;
}

qreal NodePreviewItem::evaluatePort(Port* port, qreal x, qreal y, qreal time,
                                     int pointIndex, int totalPoints) const
{
    if (!port) return 1.0;

    Node* sourceNode = getConnectedNode(port);
    if (!sourceNode) return 1.0;

    return evaluateNode(sourceNode, x, y, time, pointIndex, totalPoints);
}

qreal NodePreviewItem::evaluateNode(Node* node, qreal x, qreal y, qreal time,
                                     int pointIndex, int totalPoints) const
{
    if (!node) return 0.0;

    QString nodeType = node->type();

    // Gizmo - direct evaluation
    if (nodeType == QStringLiteral("Gizmo"))
    {
        auto* gizmo = qobject_cast<GizmoNode*>(node);
        if (gizmo)
        {
            return gizmo->computeRatio(x, y, time);
        }
    }

    // Transform - combine connected inputs with geometric transformation
    if (nodeType == QStringLiteral("Transform"))
    {
        auto* group = qobject_cast<GroupNode*>(node);
        if (group)
        {
            // Transform coordinates using GroupNode method
            qreal x1, y1;
            group->transformCoordinates(x, y, x1, y1);

            // Collect ratios from connected inputs using transformed coordinates
            QList<qreal> ratios;
            for (auto* input : node->inputs())
            {
                if (input->dataType() == Port::DataType::Ratio2D ||
                    input->dataType() == Port::DataType::Ratio1D ||
                    input->dataType() == Port::DataType::RatioAny)
                {
                    if (input->isConnected())
                    {
                        ratios.append(evaluatePort(input, x1, y1, time, pointIndex, totalPoints));
                    }
                }
            }

            // Combine using GroupNode method (exact formulas from GizmoTweak)
            return group->combine(ratios);
        }
    }

    // SurfaceFactory - uses normalized point index
    if (nodeType == QStringLiteral("SurfaceFactory"))
    {
        auto* surface = qobject_cast<SurfaceFactoryNode*>(node);
        if (surface)
        {
            qreal t = (totalPoints > 1) ? qreal(pointIndex) / qreal(totalPoints - 1) : 0.0;
            return surface->computeRatio(t);
        }
    }

    // TimeShift - shift time and evaluate input
    if (nodeType == QStringLiteral("TimeShift"))
    {
        auto* timeShift = qobject_cast<TimeShiftNode*>(node);
        if (timeShift)
        {
            qreal shiftedTime = timeShift->shiftTime(time);

            for (auto* input : node->inputs())
            {
                if (input->dataType() == Port::DataType::RatioAny ||
                    input->dataType() == Port::DataType::Ratio1D ||
                    input->dataType() == Port::DataType::Ratio2D)
                {
                    if (input->isConnected())
                    {
                        return evaluatePort(input, x, y, shiftedTime, pointIndex, totalPoints);
                    }
                }
            }
        }
    }

    // Mirror - evaluate input at mirrored coordinates
    if (nodeType == QStringLiteral("Mirror"))
    {
        auto* mirror = qobject_cast<MirrorNode*>(node);
        if (mirror)
        {
            // Get mirrored coordinates
            QPointF mirrored = mirror->mirror(x, y);

            // Evaluate input at mirrored position
            for (auto* input : node->inputs())
            {
                if (input->dataType() == Port::DataType::Ratio2D)
                {
                    if (input->isConnected())
                    {
                        return evaluatePort(input, mirrored.x(), mirrored.y(), time, pointIndex, totalPoints);
                    }
                }
            }
        }
    }

    return 0.0;
}

const QVector<qreal>& NodePreviewItem::getOrComputeRatioGrid(Node* node, int resolution, qreal time) const
{
    if (!node)
    {
        static QVector<qreal> empty;
        return empty;
    }

    // Check if cached entry is valid
    auto it = s_cache.entries.find(node);
    if (it != s_cache.entries.end())
    {
        const auto& entry = it.value();
        if (entry.valid && entry.resolution == resolution && qFuzzyCompare(entry.time, time))
        {
            return entry.ratios;
        }
    }

    // Compute and cache the grid
    auto grid = computeRatioGrid(node, resolution, time);

    RatioGridCache::CacheEntry entry;
    entry.ratios = std::move(grid);
    entry.resolution = resolution;
    entry.time = time;
    entry.valid = true;

    s_cache.entries[node] = std::move(entry);
    return s_cache.entries[node].ratios;
}

QVector<qreal> NodePreviewItem::computeRatioGrid(Node* node, int resolution, qreal time) const
{
    QVector<qreal> grid(resolution * resolution);

    for (int iy = 0; iy < resolution; ++iy)
    {
        for (int ix = 0; ix < resolution; ++ix)
        {
            // Map to normalized coordinates [-1, +1]
            qreal nx = (qreal(ix) / (resolution - 1)) * 2.0 - 1.0;
            qreal ny = 1.0 - (qreal(iy) / (resolution - 1)) * 2.0;  // Y inverted

            grid[iy * resolution + ix] = evaluateNode(node, nx, ny, time, 0, 1);
        }
    }

    return grid;
}

QColor NodePreviewItem::ratioToColor(qreal ratio) const
{
    // Near-zero values are black
    if (qAbs(ratio) < 0.004)  // ~1/255
    {
        return Qt::black;
    }

    if (ratio >= 0.0)
    {
        // Positive: Blue → Cyan → Green
        int pixValue = qMin(qMax(0, int(ratio * 255.0)), 255);

        if (pixValue >= 128)
        {
            // 128-255: Cyan (0,255,255) → Green (0,255,0)
            // Blue component decreases from 255 to 0
            int blue = int((1.0 - (pixValue - 128) / 127.0) * 255.0);
            return QColor(0, 255, blue);
        }
        else
        {
            // 0-127: Blue (0,0,255) → Cyan (0,255,255)
            // Green component increases from 0 to 255
            int green = int((pixValue / 127.0) * 255.0);
            return QColor(0, green, 255);
        }
    }
    else
    {
        // Negative: Red → Magenta
        int pixValue = qMin(qMax(0, int(-ratio * 255.0)), 255);

        if (pixValue >= 128)
        {
            // 128-255: Magenta (255,0,255) → Red (255,0,0)
            // Blue component decreases from 255 to 0
            int blue = int((1.0 - (pixValue - 128) / 127.0) * 255.0);
            return QColor(255, 0, blue);
        }
        else
        {
            // 0-127: Red (255,0,0) → Magenta (255,0,255)
            // Blue component increases from 0 to 255
            int blue = int((pixValue / 127.0) * 255.0);
            return QColor(255, 0, blue);
        }
    }
}

void NodePreviewItem::paintShapeHeatmap(QPainter* painter)
{
    int res = _resolution;
    qreal cellW = width() / res;
    qreal cellH = height() / res;

    // Use cached grid for performance
    const auto& grid = getOrComputeRatioGrid(_node, res, _currentTime);

    if (grid.isEmpty())
    {
        painter->fillRect(boundingRect(), Qt::black);
        return;
    }

    for (int iy = 0; iy < res; ++iy)
    {
        for (int ix = 0; ix < res; ++ix)
        {
            qreal ratio = grid[iy * res + ix];
            QColor color = ratioToColor(ratio);

            painter->fillRect(QRectF(ix * cellW, iy * cellH, cellW + 0.5, cellH + 0.5), color);
        }
    }
}

void NodePreviewItem::paintSurfaceCurve(QPainter* painter)
{
    auto* surface = qobject_cast<SurfaceFactoryNode*>(_node);
    if (!surface) return;

    // Background
    painter->fillRect(boundingRect(), Qt::black);

    // Draw the curve
    QPen curvePen(QColor(100, 255, 150));  // Green
    curvePen.setWidth(2);
    painter->setPen(curvePen);

    int steps = qMax(int(width()), 40);
    QPointF prevPoint;

    for (int i = 0; i <= steps; ++i)
    {
        qreal t = qreal(i) / steps;
        qreal ratio = surface->computeRatio(t);

        qreal px = t * width();
        qreal py = height() - ratio * height();

        QPointF point(px, py);
        if (i > 0)
        {
            painter->drawLine(prevPoint, point);
        }
        prevPoint = point;
    }

    // Draw time indicator (vertical white line)
    qreal timeX = _currentTime * width();
    QPen timePen(Qt::white);
    timePen.setWidth(2);
    painter->setPen(timePen);
    painter->drawLine(QPointF(timeX, 0), QPointF(timeX, height()));

    // Draw current value point
    qreal currentRatio = surface->computeRatio(_currentTime);
    qreal pointY = height() - currentRatio * height();
    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(timeX, pointY), 4, 4);
}

void NodePreviewItem::paintGizmoIcon(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    qreal centerX = width() / 2;
    qreal centerY = height() / 2;
    qreal rx = qMin(width(), height()) * 0.4;
    qreal ry = rx * 0.7;

    // Create radial gradient for ellipse
    QRadialGradient gradient(centerX, centerY, rx);
    gradient.setColorAt(0.0, QColor(100, 200, 255));   // Cyan center
    gradient.setColorAt(0.5, QColor(60, 120, 200));    // Blue middle
    gradient.setColorAt(1.0, QColor(20, 40, 80));      // Dark blue edge

    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);
    painter->drawEllipse(QPointF(centerX, centerY), rx, ry);

    // Add subtle highlight
    QRadialGradient highlight(centerX - rx * 0.3, centerY - ry * 0.3, rx * 0.4);
    highlight.setColorAt(0.0, QColor(255, 255, 255, 80));
    highlight.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter->setBrush(highlight);
    painter->drawEllipse(QPointF(centerX, centerY), rx, ry);
}

void NodePreviewItem::paintGroupIcon(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    qreal centerX = width() / 2;
    qreal centerY = height() / 2;
    qreal size = qMin(width(), height()) * 0.4;

    QColor gateColor(180, 220, 160);  // Light green for group
    QPen pen(gateColor);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // NAND gate body (D-shape)
    QPainterPath gatePath;
    gatePath.moveTo(centerX - size * 0.6, centerY - size * 0.7);
    gatePath.lineTo(centerX - size * 0.6, centerY + size * 0.7);
    gatePath.lineTo(centerX - size * 0.1, centerY + size * 0.7);
    gatePath.arcTo(QRectF(centerX - size * 0.1 - size * 0.7, centerY - size * 0.7,
                          size * 1.4, size * 1.4), -90, 180);
    gatePath.lineTo(centerX - size * 0.6, centerY - size * 0.7);
    painter->drawPath(gatePath);

    // NAND bubble (negation)
    painter->setBrush(Qt::black);
    painter->drawEllipse(QPointF(centerX + size * 0.65, centerY), size * 0.12, size * 0.12);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPointF(centerX + size * 0.65, centerY), size * 0.12, size * 0.12);

    // Input lines
    painter->drawLine(QPointF(centerX - size, centerY - size * 0.35),
                      QPointF(centerX - size * 0.6, centerY - size * 0.35));
    painter->drawLine(QPointF(centerX - size, centerY + size * 0.35),
                      QPointF(centerX - size * 0.6, centerY + size * 0.35));

    // Output line
    painter->drawLine(QPointF(centerX + size * 0.77, centerY),
                      QPointF(centerX + size, centerY));
}

void NodePreviewItem::paintMirrorIcon(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    qreal centerX = width() / 2;
    qreal centerY = height() / 2;
    qreal size = qMin(width(), height()) * 0.35;

    QColor mirrorColor(200, 180, 140);  // Light brown/gold for utility
    QPen pen(mirrorColor);
    pen.setWidth(2);
    painter->setPen(pen);

    // Central mirror line (vertical dashed)
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(QPointF(centerX, centerY - size), QPointF(centerX, centerY + size));

    // Left triangle
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    painter->setBrush(mirrorColor.darker(150));
    QPainterPath leftTriangle;
    leftTriangle.moveTo(centerX - size * 0.2, centerY);
    leftTriangle.lineTo(centerX - size * 0.8, centerY - size * 0.5);
    leftTriangle.lineTo(centerX - size * 0.8, centerY + size * 0.5);
    leftTriangle.closeSubpath();
    painter->drawPath(leftTriangle);

    // Right triangle (mirrored)
    painter->setBrush(mirrorColor.darker(120));
    QPainterPath rightTriangle;
    rightTriangle.moveTo(centerX + size * 0.2, centerY);
    rightTriangle.lineTo(centerX + size * 0.8, centerY - size * 0.5);
    rightTriangle.lineTo(centerX + size * 0.8, centerY + size * 0.5);
    rightTriangle.closeSubpath();
    painter->drawPath(rightTriangle);
}

void NodePreviewItem::paintTimeShiftIcon(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    qreal centerX = width() / 2;
    qreal centerY = height() / 2;
    qreal size = qMin(width(), height()) * 0.35;

    QColor clockColor(200, 180, 140);  // Light brown/gold for utility
    QPen pen(clockColor);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Clock circle
    painter->drawEllipse(QPointF(centerX, centerY), size, size);

    // Clock hands
    painter->drawLine(QPointF(centerX, centerY), QPointF(centerX, centerY - size * 0.6));
    painter->drawLine(QPointF(centerX, centerY), QPointF(centerX + size * 0.4, centerY + size * 0.2));

    // Time shift arrow (curved)
    QPainterPath arrowPath;
    arrowPath.moveTo(centerX + size * 1.1, centerY - size * 0.3);
    arrowPath.quadTo(centerX + size * 1.4, centerY,
                     centerX + size * 1.1, centerY + size * 0.3);
    painter->drawPath(arrowPath);

    // Arrowhead
    painter->drawLine(QPointF(centerX + size * 1.1, centerY + size * 0.3),
                      QPointF(centerX + size * 1.0, centerY + size * 0.15));
    painter->drawLine(QPointF(centerX + size * 1.1, centerY + size * 0.3),
                      QPointF(centerX + size * 1.25, centerY + size * 0.2));

    // Delta t label
    painter->setFont(QFont("Arial", int(size * 0.35), QFont::Bold));
    painter->drawText(QRectF(centerX + size * 0.9, centerY + size * 0.4, size, size * 0.5),
                      Qt::AlignLeft, "Δt");
}

void NodePreviewItem::paintTweakIcon(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    if (!_node) return;

    QString tweakType = _node->type();
    QColor iconColor(160, 140, 200);  // Light purple for tweaks

    qreal centerX = width() / 2;
    qreal centerY = height() / 2;
    qreal size = qMin(width(), height()) * 0.38;

    QPen pen(iconColor);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    if (tweakType == QStringLiteral("PositionTweak"))
    {
        // Four arrows pointing outward (cross pattern)
        qreal arrowLen = size * 0.8;
        qreal arrowHead = 5;

        // Draw four arrows
        for (int i = 0; i < 4; ++i)
        {
            qreal angle = i * M_PI / 2;
            qreal dx = qCos(angle) * arrowLen;
            qreal dy = qSin(angle) * arrowLen;

            painter->drawLine(QPointF(centerX, centerY), QPointF(centerX + dx, centerY + dy));

            // Arrowhead
            qreal tipX = centerX + dx;
            qreal tipY = centerY + dy;
            qreal perpX = -qSin(angle) * arrowHead;
            qreal perpY = qCos(angle) * arrowHead;
            qreal backX = -qCos(angle) * arrowHead;
            qreal backY = -qSin(angle) * arrowHead;

            painter->drawLine(QPointF(tipX, tipY), QPointF(tipX + backX + perpX, tipY + backY + perpY));
            painter->drawLine(QPointF(tipX, tipY), QPointF(tipX + backX - perpX, tipY + backY - perpY));
        }
    }
    else if (tweakType == QStringLiteral("ScaleTweak"))
    {
        // Concentric squares
        painter->drawRect(QRectF(centerX - size * 0.3, centerY - size * 0.3, size * 0.6, size * 0.6));
        painter->drawRect(QRectF(centerX - size * 0.6, centerY - size * 0.6, size * 1.2, size * 1.2));
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(QRectF(centerX - size * 0.9, centerY - size * 0.9, size * 1.8, size * 1.8));
    }
    else if (tweakType == QStringLiteral("RotationTweak"))
    {
        // Curved arrow
        QRectF arcRect(centerX - size, centerY - size, size * 2, size * 2);
        painter->drawArc(arcRect, 45 * 16, 270 * 16);

        // Arrowhead at end
        qreal endAngle = qDegreesToRadians(-45.0);
        qreal endX = centerX + qCos(endAngle) * size;
        qreal endY = centerY + qSin(endAngle) * size;
        painter->drawLine(QPointF(endX, endY), QPointF(endX + 6, endY - 4));
        painter->drawLine(QPointF(endX, endY), QPointF(endX + 4, endY + 6));
    }
    else if (tweakType == QStringLiteral("ColorTweak"))
    {
        // Color wheel (6 segments)
        int segments = 6;
        QColor colors[] = {
            QColor(255, 80, 80), QColor(255, 255, 80), QColor(80, 255, 80),
            QColor(80, 255, 255), QColor(80, 80, 255), QColor(255, 80, 255)
        };

        for (int i = 0; i < segments; ++i)
        {
            painter->setBrush(colors[i]);
            painter->setPen(Qt::NoPen);
            QPainterPath path;
            path.moveTo(centerX, centerY);
            QRectF pieRect(centerX - size, centerY - size, size * 2, size * 2);
            path.arcTo(pieRect, i * 60 - 90, 60);
            path.closeSubpath();
            painter->drawPath(path);
        }
    }
    else if (tweakType == QStringLiteral("PolarTweak"))
    {
        // Polar grid (circles + radial lines)
        painter->setPen(pen);
        for (int i = 1; i <= 3; ++i)
            painter->drawEllipse(QPointF(centerX, centerY), size * i / 3, size * i / 3);
        for (int i = 0; i < 8; ++i)
        {
            qreal angle = i * M_PI / 4;
            painter->drawLine(QPointF(centerX, centerY),
                              QPointF(centerX + qCos(angle) * size, centerY + qSin(angle) * size));
        }
    }
    else if (tweakType == QStringLiteral("WaveTweak"))
    {
        // Concentric ripple circles
        for (int i = 1; i <= 4; ++i)
        {
            pen.setWidth(3 - i * 0.5);
            painter->setPen(pen);
            painter->drawEllipse(QPointF(centerX, centerY), size * i / 4, size * i / 4);
        }
    }
    else if (tweakType == QStringLiteral("SqueezeTweak"))
    {
        // Pinch arrows ><
        qreal arrowSize = size * 0.7;
        // Left arrow >
        painter->drawLine(QPointF(centerX - arrowSize, centerY - arrowSize * 0.6),
                          QPointF(centerX - arrowSize * 0.3, centerY));
        painter->drawLine(QPointF(centerX - arrowSize, centerY + arrowSize * 0.6),
                          QPointF(centerX - arrowSize * 0.3, centerY));
        // Right arrow <
        painter->drawLine(QPointF(centerX + arrowSize, centerY - arrowSize * 0.6),
                          QPointF(centerX + arrowSize * 0.3, centerY));
        painter->drawLine(QPointF(centerX + arrowSize, centerY + arrowSize * 0.6),
                          QPointF(centerX + arrowSize * 0.3, centerY));
    }
    else if (tweakType == QStringLiteral("SparkleTweak"))
    {
        // 4-pointed star sparkle
        painter->setBrush(iconColor);
        QPainterPath path;
        for (int i = 0; i < 8; ++i)
        {
            qreal angle = i * M_PI / 4 - M_PI / 2;
            qreal r = (i % 2 == 0) ? size : size * 0.3;
            qreal x = centerX + r * qCos(angle);
            qreal y = centerY + r * qSin(angle);
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        path.closeSubpath();
        painter->drawPath(path);
    }
    else
    {
        // Generic: simple wave
        QPainterPath path;
        path.moveTo(centerX - size, centerY);
        for (int i = 0; i <= 20; ++i)
        {
            qreal t = i / 20.0;
            qreal x = centerX - size + t * size * 2;
            qreal y = centerY + qSin(t * M_PI * 2) * size * 0.5;
            path.lineTo(x, y);
        }
        painter->drawPath(path);
    }
}

void NodePreviewItem::paint(QPainter* painter)
{
    if (!_node)
    {
        painter->fillRect(boundingRect(), Qt::black);
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    QString nodeType = _node->type();
    Node::Category cat = _node->category();

    // Input and Output nodes - no preview here (handled in QML)
    if (nodeType == QStringLiteral("Input") || nodeType == QStringLiteral("Output"))
    {
        painter->fillRect(boundingRect(), Qt::black);
        return;
    }

    // SurfaceFactory - 1D curve visualization
    if (nodeType == QStringLiteral("SurfaceFactory"))
    {
        paintSurfaceCurve(painter);
        return;
    }

    // Shape nodes (Gizmo, Group, Mirror) - heatmap visualization
    if (cat == Node::Category::Shape)
    {
        paintShapeHeatmap(painter);
        return;
    }

    // Utility nodes (TimeShift) - heatmap visualization
    if (cat == Node::Category::Utility)
    {
        paintShapeHeatmap(painter);
        return;
    }

    // Tweaks - black background (icons shown separately in QML)
    if (cat == Node::Category::Tweak)
    {
        painter->fillRect(boundingRect(), Qt::black);
        return;
    }

    // Fallback
    painter->fillRect(boundingRect(), Qt::black);
}

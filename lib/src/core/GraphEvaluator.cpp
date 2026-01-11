#include "GraphEvaluator.h"
#include "NodeGraph.h"
#include "Node.h"
#include "Port.h"
#include "Connection.h"

#include <QtMath>
#include <cmath>

// Node includes for type-specific evaluation
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/SurfaceFactoryNode.h"
#include "nodes/TimeShiftNode.h"
#include "nodes/MirrorNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"
#include "nodes/PolarTweak.h"
#include "nodes/WaveTweak.h"
#include "nodes/SqueezeTweak.h"
#include "nodes/SparkleTweak.h"
#include "nodes/FuzzynessTweak.h"
#include "nodes/ColorFuzzynessTweak.h"
#include "nodes/SplitTweak.h"
#include "nodes/RounderTweak.h"

#include <QVariantMap>

namespace gizmotweak2
{

GraphEvaluator::GraphEvaluator(QObject* parent)
    : QObject(parent)
{
}

void GraphEvaluator::setGraph(NodeGraph* graph)
{
    if (_graph != graph)
    {
        _graph = graph;
        emit graphValidityChanged();
    }
}

Node* GraphEvaluator::findNodeByType(const QString& type) const
{
    if (!_graph) return nullptr;

    for (int i = 0; i < _graph->rowCount(); ++i)
    {
        auto* node = _graph->nodeAt(i);
        if (node && node->type() == type)
        {
            return node;
        }
    }
    return nullptr;
}

Node* GraphEvaluator::getConnectedNode(Port* port) const
{
    if (!port || !_graph) return nullptr;

    // Find the connection that involves this port
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

QList<Node*> GraphEvaluator::buildFramePath()
{
    QList<Node*> path;

    if (!_graph) return path;

    // Start from Input node
    Node* inputNode = findNodeByType(QStringLiteral("Input"));
    if (!inputNode) return path;

    // Follow the Frame connections
    Node* current = inputNode;
    path.append(current);

    while (current)
    {
        // Find the Frame output port
        Port* frameOutput = nullptr;
        for (auto* port : current->outputs())
        {
            if (port->dataType() == Port::DataType::Frame)
            {
                frameOutput = port;
                break;
            }
        }

        if (!frameOutput) break;

        // Find connected node
        Node* next = getConnectedNode(frameOutput);
        if (!next || path.contains(next)) break;  // Avoid cycles

        path.append(next);
        current = next;
    }

    return path;
}

qreal GraphEvaluator::evaluateRatioChain(Port* ratioPort, qreal x, qreal y, qreal time) const
{
    if (!ratioPort || !_graph) return 1.0;

    // Find the node connected to this ratio input
    Node* sourceNode = getConnectedNode(ratioPort);
    if (!sourceNode) return 1.0;  // No connection = full ratio

    QString nodeType = sourceNode->type();

    // Handle Gizmo
    if (nodeType == QStringLiteral("Gizmo"))
    {
        auto* gizmo = qobject_cast<GizmoNode*>(sourceNode);
        if (gizmo)
        {
            return gizmo->computeRatio(x, y, time);
        }
    }

    // Handle Transform - combine connected inputs with geometric transformation
    if (nodeType == QStringLiteral("Transform"))
    {
        auto* group = qobject_cast<GroupNode*>(sourceNode);
        if (group)
        {
            // Transform coordinates using GroupNode method
            qreal x1, y1;
            group->transformCoordinates(x, y, x1, y1);

            // In single input mode, just pass through the first input
            if (group->singleInputMode())
            {
                auto* input = sourceNode->inputAt(0);
                if (input && input->isConnected())
                {
                    return evaluateRatioChain(input, x1, y1, time);
                }
                return 0.0;
            }

            // Collect ratios from connected inputs using transformed coordinates
            QList<qreal> ratios;
            for (auto* input : sourceNode->inputs())
            {
                if (input->dataType() == Port::DataType::Ratio2D ||
                    input->dataType() == Port::DataType::Ratio1D ||
                    input->dataType() == Port::DataType::RatioAny)
                {
                    if (input->isConnected() && input->isVisible())
                    {
                        ratios.append(evaluateRatioChain(input, x1, y1, time));
                    }
                }
            }

            // Combine using GroupNode method (exact formulas from GizmoTweak)
            return group->combine(ratios);
        }
    }

    // Handle SurfaceFactory
    if (nodeType == QStringLiteral("SurfaceFactory"))
    {
        auto* surface = qobject_cast<SurfaceFactoryNode*>(sourceNode);
        if (surface)
        {
            // SurfaceFactory uses time as input
            return surface->computeRatio(time);
        }
    }

    // Handle TimeShift
    if (nodeType == QStringLiteral("TimeShift"))
    {
        auto* timeShift = qobject_cast<TimeShiftNode*>(sourceNode);
        if (timeShift)
        {
            // TimeShift modifies time and passes to its input
            qreal shiftedTime = timeShift->shiftTime(time);

            // Find the ratio input of TimeShift
            for (auto* input : sourceNode->inputs())
            {
                if (input->dataType() == Port::DataType::RatioAny ||
                    input->dataType() == Port::DataType::Ratio1D ||
                    input->dataType() == Port::DataType::Ratio2D)
                {
                    return evaluateRatioChain(input, x, y, shiftedTime);
                }
            }
        }
    }

    // Handle Mirror - evaluate input at mirrored coordinates
    if (nodeType == QStringLiteral("Mirror"))
    {
        auto* mirror = qobject_cast<MirrorNode*>(sourceNode);
        if (mirror)
        {
            // Get mirrored coordinates
            QPointF mirrored = mirror->mirror(x, y);

            // Evaluate input at mirrored position
            for (auto* input : sourceNode->inputs())
            {
                if (input->dataType() == Port::DataType::Ratio2D)
                {
                    return evaluateRatioChain(input, mirrored.x(), mirrored.y(), time);
                }
            }
        }
    }

    return 1.0;
}

QPointF GraphEvaluator::findConnectedGizmoCenter(Port* ratioPort) const
{
    if (!ratioPort || !_graph) return QPointF(0.0, 0.0);

    // Find the node connected to this ratio input
    Node* sourceNode = getConnectedNode(ratioPort);
    if (!sourceNode) return QPointF(0.0, 0.0);

    QString nodeType = sourceNode->type();

    // Direct connection to Gizmo
    if (nodeType == QStringLiteral("Gizmo"))
    {
        auto* gizmo = qobject_cast<GizmoNode*>(sourceNode);
        if (gizmo)
        {
            return QPointF(gizmo->centerX(), gizmo->centerY());
        }
    }

    // Transform, TimeShift, Mirror - recursively search their inputs for a Gizmo
    if (nodeType == QStringLiteral("Transform") ||
        nodeType == QStringLiteral("TimeShift") ||
        nodeType == QStringLiteral("Mirror"))
    {
        for (auto* input : sourceNode->inputs())
        {
            if (input->dataType() == Port::DataType::Ratio2D ||
                input->dataType() == Port::DataType::Ratio1D ||
                input->dataType() == Port::DataType::RatioAny)
            {
                if (input->isConnected())
                {
                    QPointF center = findConnectedGizmoCenter(input);
                    if (!center.isNull() || center != QPointF(0.0, 0.0))
                    {
                        return center;
                    }
                }
            }
        }
    }

    return QPointF(0.0, 0.0);
}

GraphEvaluator::Point GraphEvaluator::applyTweak(Node* tweakNode, const Point& input, qreal ratio, qreal time, int sampleIndex,
                                                   qreal gizmoX, qreal gizmoY) const
{
    if (!tweakNode) return input;

    Point result = input;
    QString nodeType = tweakNode->type();

    // PositionTweak
    if (nodeType == QStringLiteral("PositionTweak"))
    {
        auto* tweak = qobject_cast<PositionTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // ScaleTweak
    else if (nodeType == QStringLiteral("ScaleTweak"))
    {
        auto* tweak = qobject_cast<ScaleTweak*>(tweakNode);
        if (tweak)
        {
            // Use same ratio for X and Y (TODO: support Ratio2D with separate components)
            QPointF pos = tweak->apply(input.x, input.y, ratio, ratio, gizmoX, gizmoY);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // RotationTweak
    else if (nodeType == QStringLiteral("RotationTweak"))
    {
        auto* tweak = qobject_cast<RotationTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio, gizmoX, gizmoY);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // ColorTweak
    else if (nodeType == QStringLiteral("ColorTweak"))
    {
        auto* tweak = qobject_cast<ColorTweak*>(tweakNode);
        if (tweak)
        {
            QColor inputColor = QColor::fromRgbF(input.r, input.g, input.b);
            QColor outputColor = tweak->apply(inputColor, ratio);
            result.r = outputColor.redF();
            result.g = outputColor.greenF();
            result.b = outputColor.blueF();
        }
    }

    // PolarTweak
    else if (nodeType == QStringLiteral("PolarTweak"))
    {
        auto* tweak = qobject_cast<PolarTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio, ratio, gizmoX, gizmoY);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // WaveTweak
    else if (nodeType == QStringLiteral("WaveTweak"))
    {
        auto* tweak = qobject_cast<WaveTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio, gizmoX, gizmoY);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // SqueezeTweak
    else if (nodeType == QStringLiteral("SqueezeTweak"))
    {
        auto* tweak = qobject_cast<SqueezeTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio, gizmoX, gizmoY);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // SparkleTweak - handled at frame level, not per-sample
    // (sparkles are inserted between samples, not modifying individual points)

    // FuzzynessTweak
    else if (nodeType == QStringLiteral("FuzzynessTweak"))
    {
        auto* tweak = qobject_cast<FuzzynessTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(QPointF(input.x, input.y), ratio, sampleIndex);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // ColorFuzzynessTweak
    else if (nodeType == QStringLiteral("ColorFuzzynessTweak"))
    {
        auto* tweak = qobject_cast<ColorFuzzynessTweak*>(tweakNode);
        if (tweak)
        {
            QColor inputColor = QColor::fromRgbF(input.r, input.g, input.b);
            QColor outputColor = tweak->apply(inputColor, ratio, sampleIndex);
            result.r = outputColor.redF();
            result.g = outputColor.greenF();
            result.b = outputColor.blueF();
        }
    }

    // RounderTweak
    else if (nodeType == QStringLiteral("RounderTweak"))
    {
        auto* tweak = qobject_cast<RounderTweak*>(tweakNode);
        if (tweak)
        {
            QPointF pos = tweak->apply(input.x, input.y, ratio);
            result.x = pos.x();
            result.y = pos.y();
        }
    }

    // SplitTweak - handled at frame level, not per-point
    // (splits are inserted between samples, not modifying individual points)

    return result;
}

xengine::Frame* GraphEvaluator::evaluate(xengine::Frame* input, qreal time)
{
    if (!input || !_graph) return nullptr;

    // Build the path from Input to Output
    QList<Node*> path = buildFramePath();

    // Use double-buffered frames for frame-level tweaks
    xengine::Frame* currentFrame = new xengine::Frame();
    currentFrame->clone(*input);

    xengine::Frame* tempFrame = new xengine::Frame();

    // Apply tweaks in order
    for (auto* node : path)
    {
        if (node->category() != Node::Category::Tweak) continue;

        QString nodeType = node->type();

        // Check if this is a frame-level tweak
        if (nodeType == QStringLiteral("SparkleTweak"))
        {
            auto* sparkleTweak = qobject_cast<SparkleTweak*>(node);
            if (sparkleTweak)
            {
                QVariant followGizmoProp = node->property("followGizmo");
                bool followGizmo = followGizmoProp.isValid() ? followGizmoProp.toBool() : false;

                tempFrame->clear();

                if (followGizmo)
                {
                    // Find ratio port
                    Port* ratioPort = nullptr;
                    for (auto* port : node->inputs())
                    {
                        if (port->dataType() == Port::DataType::RatioAny ||
                            port->dataType() == Port::DataType::Ratio2D ||
                            port->dataType() == Port::DataType::Ratio1D)
                        {
                            ratioPort = port;
                            break;
                        }
                    }

                    if (ratioPort && ratioPort->isConnected())
                    {
                        // Use per-sample ratio evaluation
                        auto ratioEvaluator = [this, ratioPort, time](qreal x, qreal y) {
                            return evaluateRatioChain(ratioPort, x, y, time);
                        };
                        sparkleTweak->applyToFrame(currentFrame, tempFrame, ratioEvaluator);
                    }
                    else
                    {
                        // No ratio connected with followGizmo enabled: skip effect (no transformation)
                        continue;
                    }
                }
                else
                {
                    // followGizmo disabled, use full ratio
                    sparkleTweak->applyToFrame(currentFrame, tempFrame, 1.0);
                }

                // Swap buffers
                std::swap(currentFrame, tempFrame);
            }
            continue;
        }

        // Per-sample tweaks
        tempFrame->clear();

        // Find the ratio input port
        Port* ratioPort = nullptr;
        for (auto* port : node->inputs())
        {
            if (port->dataType() == Port::DataType::RatioAny ||
                port->dataType() == Port::DataType::Ratio2D ||
                port->dataType() == Port::DataType::Ratio1D)
            {
                ratioPort = port;
                break;
            }
        }

        // Check if this tweak has followGizmo property enabled
        QVariant followGizmoProp = node->property("followGizmo");
        bool followGizmo = followGizmoProp.isValid() ? followGizmoProp.toBool() : false;

        // If followGizmo is enabled but no ratio is connected, skip this tweak entirely
        // (no effect when disconnected)
        if (followGizmo && (!ratioPort || !ratioPort->isConnected()))
        {
            continue;
        }

        // Process each sample
        for (int i = 0; i < currentFrame->size(); ++i)
        {
            xengine::XSample sample = currentFrame->at(i);
            Point point{sample.getX(), sample.getY(), sample.getR(), sample.getG(), sample.getB()};

            // Calculate ratio if followGizmo is enabled
            qreal ratio = 1.0;
            if (followGizmo && ratioPort && ratioPort->isConnected())
            {
                ratio = evaluateRatioChain(ratioPort, point.x, point.y, time);
            }

            // Find gizmo center for tweaks that use it as transformation center
            qreal gizmoX = 0.0, gizmoY = 0.0;
            if (followGizmo)
            {
                QPointF gizmoCenter = findConnectedGizmoCenter(ratioPort);
                gizmoX = gizmoCenter.x();
                gizmoY = gizmoCenter.y();
            }

            // Apply the tweak
            point = applyTweak(node, point, ratio, time, i, gizmoX, gizmoY);

            tempFrame->addSample(point.x, point.y, 0.0, point.r, point.g, point.b, sample.getNb());
        }

        // Swap buffers
        std::swap(currentFrame, tempFrame);
    }

    // Clean up temp frame
    delete tempFrame;

    return currentFrame;
}

QVariantList GraphEvaluator::evaluateToPoints(const QVariantList& inputPoints, qreal time)
{
    QVariantList result;

    if (!_graph) return result;

    // Convert input points to Frame for processing
    xengine::Frame inputFrame;
    for (const auto& pointVar : inputPoints)
    {
        QVariantMap pointMap = pointVar.toMap();
        qreal x = pointMap.value(QStringLiteral("x"), 0.0).toReal();
        qreal y = pointMap.value(QStringLiteral("y"), 0.0).toReal();
        qreal r = pointMap.value(QStringLiteral("r"), 1.0).toReal();
        qreal g = pointMap.value(QStringLiteral("g"), 1.0).toReal();
        qreal b = pointMap.value(QStringLiteral("b"), 1.0).toReal();
        inputFrame.addSample(x, y, 0.0, r, g, b, 1);
    }

    // Use the frame-based evaluate
    xengine::Frame* outputFrame = evaluate(&inputFrame, time);
    if (!outputFrame) return result;

    // Convert output frame back to QVariantList
    for (int i = 0; i < outputFrame->size(); ++i)
    {
        const auto& sample = outputFrame->at(i);
        QVariantMap resultPoint;
        resultPoint[QStringLiteral("x")] = sample.getX();
        resultPoint[QStringLiteral("y")] = sample.getY();
        resultPoint[QStringLiteral("r")] = sample.getR();
        resultPoint[QStringLiteral("g")] = sample.getG();
        resultPoint[QStringLiteral("b")] = sample.getB();
        result.append(resultPoint);
    }

    delete outputFrame;
    return result;
}

bool GraphEvaluator::isGraphComplete() const
{
    _validationErrors.clear();

    if (!_graph)
    {
        _validationErrors << QStringLiteral("No graph");
        return false;
    }

    // Check for Input and Output nodes
    Node* inputNode = findNodeByType(QStringLiteral("Input"));
    Node* outputNode = findNodeByType(QStringLiteral("Output"));

    if (!inputNode)
    {
        _validationErrors << QStringLiteral("Missing Input node");
    }
    if (!outputNode)
    {
        _validationErrors << QStringLiteral("Missing Output node");
    }

    // Check if Output's Frame input is connected
    if (outputNode)
    {
        bool hasFrameInput = false;
        for (auto* port : outputNode->inputs())
        {
            if (port->dataType() == Port::DataType::Frame && port->isConnected())
            {
                hasFrameInput = true;
                break;
            }
        }
        if (!hasFrameInput)
        {
            _validationErrors << QStringLiteral("Output node has no Frame input");
        }
    }

    return _validationErrors.isEmpty();
}

QStringList GraphEvaluator::validationErrors() const
{
    isGraphComplete();  // Refresh errors
    return _validationErrors;
}

} // namespace gizmotweak2

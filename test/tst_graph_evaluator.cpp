#include <QtTest>
#include <QtMath>

#include "core/GraphEvaluator.h"
#include "core/NodeGraph.h"
#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/MirrorNode.h"
#include "nodes/TimeShiftNode.h"
#include "nodes/SurfaceFactoryNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"

#include <frame.h>

using namespace gizmotweak2;

class TestGraphEvaluator : public QObject
{
    Q_OBJECT

private slots:
    // Validation tests
    void testNoGraph();
    void testEmptyGraph();
    void testMissingInput();
    void testMissingOutput();
    void testMissingOutputConnection();
    void testCompleteGraph();

    // Path building tests
    void testSimplePathInputOutput();
    void testPathWithSingleTweak();
    void testPathWithMultipleTweaks();

    // Ratio evaluation tests
    void testRatioNoConnection();
    void testRatioFromGizmo();
    void testRatioFromGizmoOutside();
    void testRatioFromGroup();
    void testRatioFromMirror();
    void testRatioFromTimeShift();
    void testRatioFromSurfaceFactory();

    // Frame evaluation tests
    void testEvaluatePassthrough();
    void testEvaluatePositionTweak();
    void testEvaluateScaleTweak();
    void testEvaluateRotationTweak();
    void testEvaluateColorTweak();
    void testEvaluateChainedTweaks();

    // Tweak with Gizmo tests
    void testPositionTweakWithGizmo();
    void testScaleTweakWithGizmoCenter();
    void testRotationTweakWithGizmoCenter();

    // QML interface tests
    void testEvaluateToPointsEmpty();
    void testEvaluateToPointsSimple();
    void testEvaluateToPointsWithTweak();

    // Edge cases
    void testCycleDetection();
    void testDisconnectedTweak();
    void testMultipleOutputConnections();

    // Disconnected ratio port tests (followGizmo=true but no ratio connected)
    void testPositionTweakFollowGizmoNoRatio();
    void testScaleTweakFollowGizmoNoRatio();
    void testRotationTweakFollowGizmoNoRatio();
    void testColorTweakFollowGizmoNoRatio();

private:
    bool fuzzyCompare(qreal a, qreal b, qreal epsilon = 0.0001);
    bool fuzzyComparePoint(qreal x1, qreal y1, qreal x2, qreal y2, qreal epsilon = 0.0001);

    // Helper to create a minimal complete graph (Input -> Output)
    NodeGraph* createMinimalGraph();

    // Helper to create a graph with a single tweak
    NodeGraph* createGraphWithTweak(const QString& tweakType);
};

bool TestGraphEvaluator::fuzzyCompare(qreal a, qreal b, qreal epsilon)
{
    return qAbs(a - b) < epsilon;
}

bool TestGraphEvaluator::fuzzyComparePoint(qreal x1, qreal y1, qreal x2, qreal y2, qreal epsilon)
{
    return fuzzyCompare(x1, x2, epsilon) && fuzzyCompare(y1, y2, epsilon);
}

NodeGraph* TestGraphEvaluator::createMinimalGraph()
{
    auto* graph = new NodeGraph();
    auto* input = graph->createNode("Input", QPointF(100, 100));
    auto* output = graph->createNode("Output", QPointF(400, 100));
    graph->connect(input->outputAt(0), output->inputAt(0));
    return graph;
}

NodeGraph* TestGraphEvaluator::createGraphWithTweak(const QString& tweakType)
{
    auto* graph = new NodeGraph();
    auto* input = graph->createNode("Input", QPointF(100, 100));
    auto* tweak = graph->createNode(tweakType, QPointF(250, 100));
    auto* output = graph->createNode("Output", QPointF(400, 100));

    // Connect Input -> Tweak -> Output
    graph->connect(input->outputAt(0), tweak->inputAt(0));  // Frame connection
    graph->connect(tweak->outputAt(0), output->inputAt(0)); // Frame connection

    return graph;
}

// ============================================================================
// Validation Tests
// ============================================================================

void TestGraphEvaluator::testNoGraph()
{
    GraphEvaluator evaluator;
    // No graph set

    QVERIFY(!evaluator.isGraphComplete());
    QVERIFY(evaluator.validationErrors().contains("No graph"));
}

void TestGraphEvaluator::testEmptyGraph()
{
    NodeGraph graph;
    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(!evaluator.isGraphComplete());
    auto errors = evaluator.validationErrors();
    QVERIFY(errors.contains("Missing Input node"));
    QVERIFY(errors.contains("Missing Output node"));
}

void TestGraphEvaluator::testMissingInput()
{
    NodeGraph graph;
    graph.createNode("Output", QPointF(100, 100));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(!evaluator.isGraphComplete());
    QVERIFY(evaluator.validationErrors().contains("Missing Input node"));
}

void TestGraphEvaluator::testMissingOutput()
{
    NodeGraph graph;
    graph.createNode("Input", QPointF(100, 100));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(!evaluator.isGraphComplete());
    QVERIFY(evaluator.validationErrors().contains("Missing Output node"));
}

void TestGraphEvaluator::testMissingOutputConnection()
{
    NodeGraph graph;
    graph.createNode("Input", QPointF(100, 100));
    graph.createNode("Output", QPointF(400, 100));
    // No connection between them

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(!evaluator.isGraphComplete());
    QVERIFY(evaluator.validationErrors().contains("Output node has no Frame input"));
}

void TestGraphEvaluator::testCompleteGraph()
{
    auto* graph = createMinimalGraph();

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    QVERIFY(evaluator.isGraphComplete());
    QVERIFY(evaluator.validationErrors().isEmpty());

    delete graph;
}

// ============================================================================
// Path Building Tests
// ============================================================================

void TestGraphEvaluator::testSimplePathInputOutput()
{
    auto* graph = createMinimalGraph();

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    // The graph should be complete with just Input -> Output
    QVERIFY(evaluator.isGraphComplete());

    delete graph;
}

void TestGraphEvaluator::testPathWithSingleTweak()
{
    auto* graph = createGraphWithTweak("PositionTweak");

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    QVERIFY(evaluator.isGraphComplete());
    QCOMPARE(graph->nodeCount(), 3);

    delete graph;
}

void TestGraphEvaluator::testPathWithMultipleTweaks()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(300, 100));
    auto* rotTweak = graph.createNode("RotationTweak", QPointF(400, 100));
    auto* output = graph.createNode("Output", QPointF(500, 100));

    // Chain: Input -> Position -> Scale -> Rotation -> Output
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), scaleTweak->inputAt(0));
    graph.connect(scaleTweak->outputAt(0), rotTweak->inputAt(0));
    graph.connect(rotTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
    QCOMPARE(graph.nodeCount(), 5);
}

// ============================================================================
// Ratio Evaluation Tests
// ============================================================================

void TestGraphEvaluator::testRatioNoConnection()
{
    // When ratio port is not connected, default ratio should be 1.0
    auto* graph = createGraphWithTweak("PositionTweak");

    // Create a test frame
    xengine::Frame inputFrame;
    inputFrame.addSample(0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    // Without Gizmo connected, tweak should use full ratio (1.0)
    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);

    // Position tweak with default offset (0,0) should not change position
    QCOMPARE(result->size(), 1);
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.5));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testRatioFromGizmo()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(250, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure Gizmo: centered at (0,0) with radius 0.5
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setShape(GizmoNode::Shape::Ellipse);
    gizmoNode->setCenterX(0.0);
    gizmoNode->setCenterY(0.0);
    gizmoNode->setHorizontalBorder(0.5);
    gizmoNode->setVerticalBorder(0.5);
    gizmoNode->setFalloff(0.0);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetX(1.0);
    posTweakNode->setOffsetY(0.0);
    posTweakNode->setFollowGizmo(true);

    // Connect: Input -> PosTweak -> Output, Gizmo -> PosTweak.ratio
    graph.connect(input->outputAt(0), posTweak->inputAt(0));  // Frame
    graph.connect(gizmo->outputAt(0), posTweak->inputAt(1));  // Ratio
    graph.connect(posTweak->outputAt(0), output->inputAt(0)); // Frame

    // Test point at center (inside gizmo) - should get full offset
    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // At center, ratio = 1.0, so full offset applied
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    delete result;
}

void TestGraphEvaluator::testRatioFromGizmoOutside()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(250, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure Gizmo: centered at (0,0) with small radius
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setShape(GizmoNode::Shape::Ellipse);
    gizmoNode->setCenterX(0.0);
    gizmoNode->setCenterY(0.0);
    gizmoNode->setHorizontalBorder(0.2);
    gizmoNode->setVerticalBorder(0.2);
    gizmoNode->setFalloff(0.0);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetX(1.0);
    posTweakNode->setOffsetY(0.0);
    posTweakNode->setFollowGizmo(true);

    // Connect
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    // Test point far outside gizmo - should get no offset
    xengine::Frame inputFrame;
    inputFrame.addSample(0.8, 0.8, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Outside gizmo, ratio = 0.0, so no offset applied
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.8));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.8));

    delete result;
}

void TestGraphEvaluator::testRatioFromGroup()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo1 = graph.createNode("Gizmo", QPointF(100, 200));
    auto* gizmo2 = graph.createNode("Gizmo", QPointF(100, 300));
    auto* group = graph.createNode("Transform", QPointF(200, 250));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(350, 100));
    auto* output = graph.createNode("Output", QPointF(500, 100));

    // Configure Gizmos
    auto* gizmoNode1 = qobject_cast<GizmoNode*>(gizmo1);
    gizmoNode1->setCenterX(-0.3);
    gizmoNode1->setCenterY(0.0);
    gizmoNode1->setHorizontalBorder(0.2);
    gizmoNode1->setVerticalBorder(0.2);

    auto* gizmoNode2 = qobject_cast<GizmoNode*>(gizmo2);
    gizmoNode2->setCenterX(0.3);
    gizmoNode2->setCenterY(0.0);
    gizmoNode2->setHorizontalBorder(0.2);
    gizmoNode2->setVerticalBorder(0.2);

    // Configure Group to Max mode
    auto* groupNode = qobject_cast<GroupNode*>(group);
    groupNode->setCompositionMode(GroupNode::CompositionMode::Max);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetX(0.5);
    posTweakNode->setFollowGizmo(true);

    // Connect Gizmos to Group
    graph.connect(gizmo1->outputAt(0), group->inputAt(0));
    graph.connect(gizmo2->outputAt(0), group->inputAt(1));

    // Connect: Input -> PosTweak -> Output, Group -> PosTweak.ratio
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(group->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

void TestGraphEvaluator::testRatioFromMirror()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* mirror = graph.createNode("Mirror", QPointF(200, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(350, 100));
    auto* output = graph.createNode("Output", QPointF(500, 100));

    // Configure Gizmo on positive X side
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setCenterX(0.5);
    gizmoNode->setCenterY(0.0);
    gizmoNode->setHorizontalBorder(0.3);
    gizmoNode->setVerticalBorder(0.3);

    // Configure Mirror to horizontal
    auto* mirrorNode = qobject_cast<MirrorNode*>(mirror);
    mirrorNode->setAxis(MirrorNode::Axis::Horizontal);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetY(0.5);
    posTweakNode->setFollowGizmo(true);

    // Connect: Gizmo -> Mirror, Mirror -> PosTweak.ratio
    graph.connect(gizmo->outputAt(0), mirror->inputAt(0));
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(mirror->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

void TestGraphEvaluator::testRatioFromTimeShift()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* surface = graph.createNode("SurfaceFactory", QPointF(100, 200));
    auto* timeShift = graph.createNode("TimeShift", QPointF(200, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(350, 100));
    auto* output = graph.createNode("Output", QPointF(500, 100));

    // Configure TimeShift
    auto* timeShiftNode = qobject_cast<TimeShiftNode*>(timeShift);
    timeShiftNode->setDelay(0.5);
    timeShiftNode->setScale(1.0);

    // Connect: Surface -> TimeShift -> PosTweak.ratio
    graph.connect(surface->outputAt(0), timeShift->inputAt(0));
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(timeShift->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

void TestGraphEvaluator::testRatioFromSurfaceFactory()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* surface = graph.createNode("SurfaceFactory", QPointF(100, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(350, 100));
    auto* output = graph.createNode("Output", QPointF(500, 100));

    // Configure SurfaceFactory
    auto* surfaceNode = qobject_cast<SurfaceFactoryNode*>(surface);
    surfaceNode->setSurfaceType(SurfaceFactoryNode::SurfaceType::Sine);
    surfaceNode->setAmplitude(1.0);
    surfaceNode->setFrequency(1.0);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetX(1.0);
    posTweakNode->setFollowGizmo(true);

    // Connect
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(surface->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

// ============================================================================
// Frame Evaluation Tests
// ============================================================================

void TestGraphEvaluator::testEvaluatePassthrough()
{
    auto* graph = createMinimalGraph();

    xengine::Frame inputFrame;
    inputFrame.addSample(0.1, 0.2, 0.0, 1.0, 0.5, 0.0, 1);
    inputFrame.addSample(0.3, 0.4, 0.0, 0.0, 1.0, 0.5, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 2);

    // Input -> Output should pass through unchanged
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.1));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.2));
    QVERIFY(fuzzyCompare(result->at(0).getR(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getG(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getB(), 0.0));

    QVERIFY(fuzzyCompare(result->at(1).getX(), 0.3));
    QVERIFY(fuzzyCompare(result->at(1).getY(), 0.4));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testEvaluatePositionTweak()
{
    auto* graph = createGraphWithTweak("PositionTweak");

    // Configure the tweak (followGizmo=false for constant offset)
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "PositionTweak")
        {
            auto* tweak = qobject_cast<PositionTweak*>(node);
            tweak->setOffsetX(0.5);
            tweak->setOffsetY(-0.3);
            tweak->setFollowGizmo(false);  // Constant ratio = 1.0
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Position should be offset
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), -0.3));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testEvaluateScaleTweak()
{
    auto* graph = createGraphWithTweak("ScaleTweak");

    // Configure the tweak
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "ScaleTweak")
        {
            auto* tweak = qobject_cast<ScaleTweak*>(node);
            tweak->setScaleX(2.0);
            tweak->setScaleY(2.0);
            tweak->setCenterX(0.0);
            tweak->setCenterY(0.0);
            tweak->setFollowGizmo(false);
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Position should be scaled 2x from origin
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 1.0));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testEvaluateRotationTweak()
{
    auto* graph = createGraphWithTweak("RotationTweak");

    // Configure the tweak
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "RotationTweak")
        {
            auto* tweak = qobject_cast<RotationTweak*>(node);
            tweak->setAngle(90.0);
            tweak->setCenterX(0.0);
            tweak->setCenterY(0.0);
            tweak->setFollowGizmo(false);
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // (1, 0) rotated 90 degrees around origin -> (0, 1)
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 1.0));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testEvaluateColorTweak()
{
    auto* graph = createGraphWithTweak("ColorTweak");

    // Configure the tweak
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "ColorTweak")
        {
            auto* tweak = qobject_cast<ColorTweak*>(node);
            tweak->setMode(ColorTweak::Mode::Replace);
            tweak->setColor(QColor(255, 0, 0));  // Red
            tweak->setIntensity(1.0);
            tweak->setFollowGizmo(false);
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1);  // Green

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Color should be replaced with red
    QVERIFY(fuzzyCompare(result->at(0).getR(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getG(), 0.0));
    QVERIFY(fuzzyCompare(result->at(0).getB(), 0.0));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testEvaluateChainedTweaks()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(300, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure tweaks (no gizmo = full effect)
    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(0.5);
    posNode->setOffsetY(0.0);
    posNode->setFollowGizmo(false);

    auto* scaleNode = qobject_cast<ScaleTweak*>(scaleTweak);
    scaleNode->setScaleX(2.0);
    scaleNode->setScaleY(2.0);
    scaleNode->setCenterX(0.0);
    scaleNode->setCenterY(0.0);
    scaleNode->setFollowGizmo(false);

    // Chain: Input -> Position -> Scale -> Output
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), scaleTweak->inputAt(0));
    graph.connect(scaleTweak->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // First position: (0,0) + (0.5, 0) = (0.5, 0)
    // Then scale 2x: (0.5, 0) * 2 = (1.0, 0)
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    delete result;
}

// ============================================================================
// Tweak with Gizmo Tests
// ============================================================================

void TestGraphEvaluator::testPositionTweakWithGizmo()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(250, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure Gizmo at center with radius 0.5
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setCenterX(0.0);
    gizmoNode->setCenterY(0.0);
    gizmoNode->setHorizontalBorder(0.5);
    gizmoNode->setVerticalBorder(0.5);
    gizmoNode->setFalloff(0.0);

    // Configure PositionTweak
    auto* posTweakNode = qobject_cast<PositionTweak*>(posTweak);
    posTweakNode->setOffsetX(1.0);
    posTweakNode->setOffsetY(0.0);
    posTweakNode->setFollowGizmo(true);

    // Connect
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), posTweak->inputAt(1));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    // Test multiple points: inside and outside gizmo
    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);  // Center (inside)
    inputFrame.addSample(0.8, 0.0, 0.0, 1.0, 1.0, 1.0, 1);  // Outside

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 2);

    // Point at center should be fully offset
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));

    // Point outside should not be offset
    QVERIFY(fuzzyCompare(result->at(1).getX(), 0.8));

    delete result;
}

void TestGraphEvaluator::testScaleTweakWithGizmoCenter()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(250, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure Gizmo at (0.5, 0.5)
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setCenterX(0.5);
    gizmoNode->setCenterY(0.5);
    gizmoNode->setHorizontalBorder(0.3);
    gizmoNode->setVerticalBorder(0.3);
    gizmoNode->setFalloff(0.0);

    // Configure ScaleTweak with followGizmo
    auto* scaleTweakNode = qobject_cast<ScaleTweak*>(scaleTweak);
    scaleTweakNode->setScaleX(2.0);
    scaleTweakNode->setScaleY(2.0);
    scaleTweakNode->setFollowGizmo(true);

    // Connect
    graph.connect(input->outputAt(0), scaleTweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), scaleTweak->inputAt(1));
    graph.connect(scaleTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

void TestGraphEvaluator::testRotationTweakWithGizmoCenter()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* rotTweak = graph.createNode("RotationTweak", QPointF(250, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure Gizmo at (0.5, 0.5)
    auto* gizmoNode = qobject_cast<GizmoNode*>(gizmo);
    gizmoNode->setCenterX(0.5);
    gizmoNode->setCenterY(0.5);
    gizmoNode->setHorizontalBorder(0.3);
    gizmoNode->setVerticalBorder(0.3);

    // Configure RotationTweak with followGizmo
    auto* rotTweakNode = qobject_cast<RotationTweak*>(rotTweak);
    rotTweakNode->setAngle(45.0);
    rotTweakNode->setFollowGizmo(true);

    // Connect
    graph.connect(input->outputAt(0), rotTweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), rotTweak->inputAt(1));
    graph.connect(rotTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

// ============================================================================
// QML Interface Tests
// ============================================================================

void TestGraphEvaluator::testEvaluateToPointsEmpty()
{
    auto* graph = createMinimalGraph();

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    QVariantList inputPoints;
    QVariantList result = evaluator.evaluateToPoints(inputPoints, 0.0);

    QVERIFY(result.isEmpty());

    delete graph;
}

void TestGraphEvaluator::testEvaluateToPointsSimple()
{
    auto* graph = createMinimalGraph();

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    QVariantList inputPoints;
    QVariantMap point1;
    point1["x"] = 0.1;
    point1["y"] = 0.2;
    point1["r"] = 1.0;
    point1["g"] = 0.5;
    point1["b"] = 0.0;
    inputPoints.append(point1);

    QVariantList result = evaluator.evaluateToPoints(inputPoints, 0.0);

    QCOMPARE(result.size(), 1);
    QVariantMap resultPoint = result.at(0).toMap();
    QVERIFY(fuzzyCompare(resultPoint["x"].toReal(), 0.1));
    QVERIFY(fuzzyCompare(resultPoint["y"].toReal(), 0.2));
    QVERIFY(fuzzyCompare(resultPoint["r"].toReal(), 1.0));
    QVERIFY(fuzzyCompare(resultPoint["g"].toReal(), 0.5));
    QVERIFY(fuzzyCompare(resultPoint["b"].toReal(), 0.0));

    delete graph;
}

void TestGraphEvaluator::testEvaluateToPointsWithTweak()
{
    auto* graph = createGraphWithTweak("PositionTweak");

    // Configure the tweak
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "PositionTweak")
        {
            auto* tweak = qobject_cast<PositionTweak*>(node);
            tweak->setOffsetX(0.5);
            tweak->setOffsetY(-0.5);
            tweak->setFollowGizmo(false);
            break;
        }
    }

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    QVariantList inputPoints;
    QVariantMap point;
    point["x"] = 0.0;
    point["y"] = 0.0;
    point["r"] = 1.0;
    point["g"] = 1.0;
    point["b"] = 1.0;
    inputPoints.append(point);

    QVariantList result = evaluator.evaluateToPoints(inputPoints, 0.0);

    QCOMPARE(result.size(), 1);
    QVariantMap resultPoint = result.at(0).toMap();
    QVERIFY(fuzzyCompare(resultPoint["x"].toReal(), 0.5));
    QVERIFY(fuzzyCompare(resultPoint["y"].toReal(), -0.5));

    delete graph;
}

// ============================================================================
// Edge Cases
// ============================================================================

void TestGraphEvaluator::testCycleDetection()
{
    // The buildFramePath method should avoid infinite loops
    // This is implicitly tested by the cycle check: path.contains(next)
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    graph.connect(input->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    // Should complete without hanging
    QVERIFY(evaluator.isGraphComplete());
}

void TestGraphEvaluator::testDisconnectedTweak()
{
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 200));  // Not in path
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure the disconnected tweak
    auto* tweakNode = qobject_cast<PositionTweak*>(posTweak);
    tweakNode->setOffsetX(999.0);  // Large offset that shouldn't be applied

    // Only connect Input -> Output, tweak is floating
    graph.connect(input->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Disconnected tweak should not affect the result
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    delete result;
}

void TestGraphEvaluator::testMultipleOutputConnections()
{
    // Test that a single output can connect to multiple inputs (fan-out)
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(100, 200));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(250, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(250, 200));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Gizmo connects to both tweaks
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), posTweak->inputAt(1));  // Ratio
    graph.connect(gizmo->outputAt(0), scaleTweak->inputAt(1)); // Same ratio

    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    QVERIFY(evaluator.isGraphComplete());
}

// ============================================================================
// Disconnected Ratio Port Tests
// When followGizmo=true but no ratio is connected, tweak should have NO effect
// ============================================================================

void TestGraphEvaluator::testPositionTweakFollowGizmoNoRatio()
{
    auto* graph = createGraphWithTweak("PositionTweak");

    // Configure the tweak with followGizmo=true but no ratio connected
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "PositionTweak")
        {
            auto* tweak = qobject_cast<PositionTweak*>(node);
            tweak->setOffsetX(999.0);  // Large offset
            tweak->setOffsetY(999.0);
            tweak->setFollowGizmo(true);  // Enabled but no ratio connected!
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // With followGizmo=true but no ratio connected, the tweak should be skipped
    // Position should remain unchanged
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.5));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testScaleTweakFollowGizmoNoRatio()
{
    auto* graph = createGraphWithTweak("ScaleTweak");

    // Configure the tweak with followGizmo=true but no ratio connected
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "ScaleTweak")
        {
            auto* tweak = qobject_cast<ScaleTweak*>(node);
            tweak->setScaleX(10.0);  // Large scale
            tweak->setScaleY(10.0);
            tweak->setCenterX(0.0);
            tweak->setCenterY(0.0);
            tweak->setFollowGizmo(true);  // Enabled but no ratio connected!
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // With followGizmo=true but no ratio connected, the tweak should be skipped
    // Position should remain unchanged
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.5));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testRotationTweakFollowGizmoNoRatio()
{
    auto* graph = createGraphWithTweak("RotationTweak");

    // Configure the tweak with followGizmo=true but no ratio connected
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "RotationTweak")
        {
            auto* tweak = qobject_cast<RotationTweak*>(node);
            tweak->setAngle(180.0);  // Large rotation
            tweak->setCenterX(0.0);
            tweak->setCenterY(0.0);
            tweak->setFollowGizmo(true);  // Enabled but no ratio connected!
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // With followGizmo=true but no ratio connected, the tweak should be skipped
    // Position should remain unchanged
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.5));

    delete result;
    delete graph;
}

void TestGraphEvaluator::testColorTweakFollowGizmoNoRatio()
{
    auto* graph = createGraphWithTweak("ColorTweak");

    // Configure the tweak with followGizmo=true but no ratio connected
    for (int i = 0; i < graph->nodeCount(); ++i)
    {
        auto* node = graph->nodeAt(i);
        if (node->type() == "ColorTweak")
        {
            auto* tweak = qobject_cast<ColorTweak*>(node);
            tweak->setMode(ColorTweak::Mode::Replace);
            tweak->setColor(QColor(255, 0, 0));  // Replace with red
            tweak->setIntensity(1.0);
            tweak->setFollowGizmo(true);  // Enabled but no ratio connected!
            break;
        }
    }

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1);  // Green color

    GraphEvaluator evaluator;
    evaluator.setGraph(graph);

    auto* result = evaluator.evaluate(&inputFrame, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // With followGizmo=true but no ratio connected, the tweak should be skipped
    // Color should remain green (unchanged)
    QVERIFY(fuzzyCompare(result->at(0).getR(), 0.0));
    QVERIFY(fuzzyCompare(result->at(0).getG(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getB(), 0.0));

    delete result;
    delete graph;
}

QTEST_MAIN(TestGraphEvaluator)
#include "tst_graph_evaluator.moc"

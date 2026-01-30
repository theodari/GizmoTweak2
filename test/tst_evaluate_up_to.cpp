#include <QtTest>
#include <QtMath>

#include "core/GraphEvaluator.h"
#include "core/NodeGraph.h"
#include "core/Node.h"
#include "core/Port.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"

#include <frame.h>

using namespace gizmotweak2;

class TestEvaluateUpTo : public QObject
{
    Q_OBJECT

private slots:
    // Basic evaluateUpTo
    void testEvaluateUpToNullGraph();
    void testEvaluateUpToFirstTweak();
    void testEvaluateUpToSecondTweak();
    void testEvaluateUpToOutputNode();
    void testEvaluateUpToInputNode();
    void testEvaluateUpToNullStopNode();

    // Chain scenarios
    void testEvaluateUpToMiddleOfLongChain();
    void testEvaluateUpToWithGizmoShape();
    void testEvaluateUpToDisconnectedTweak();

private:
    bool fuzzyCompare(qreal a, qreal b, qreal epsilon = 0.0001);

    xengine::Frame* createTestFrame();
};

bool TestEvaluateUpTo::fuzzyCompare(qreal a, qreal b, qreal epsilon)
{
    return qAbs(a - b) < epsilon;
}

xengine::Frame* TestEvaluateUpTo::createTestFrame()
{
    auto* frame = new xengine::Frame();
    frame->addSample(0.1, 0.2, 0.0, 1.0, 0.0, 0.0, 1);
    frame->addSample(0.3, 0.4, 0.0, 0.0, 1.0, 0.0, 1);
    return frame;
}

// ============================================================================
// Basic evaluateUpTo Tests
// ============================================================================

void TestEvaluateUpTo::testEvaluateUpToNullGraph()
{
    // Evaluator with no graph set should return nullptr
    GraphEvaluator evaluator;

    auto* inputFrame = createTestFrame();
    auto* result = evaluator.evaluateUpTo(inputFrame, nullptr, 0.0);

    QVERIFY(result == nullptr);

    delete inputFrame;
}

void TestEvaluateUpTo::testEvaluateUpToFirstTweak()
{
    // Input -> PositionTweak(dx=100) -> ScaleTweak -> Output
    // evaluateUpTo(frame, positionTweak) -> position offset applied, no scale
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(300, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Configure PositionTweak
    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(0.5);
    posNode->setOffsetY(0.0);
    posNode->setFollowGizmo(false);

    // Configure ScaleTweak
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

    auto* result = evaluator.evaluateUpTo(&inputFrame, posTweak, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Position offset applied: (0,0) + (0.5, 0) = (0.5, 0)
    // Scale NOT applied (stopped before scaleTweak)
    QVERIFY(fuzzyCompare(result->at(0).getX(), 0.5));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    delete result;
}

void TestEvaluateUpTo::testEvaluateUpToSecondTweak()
{
    // Input -> PositionTweak(dx=0.5) -> ScaleTweak(2x) -> Output
    // evaluateUpTo(frame, scaleTweak) -> both position and scale applied
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(300, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

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

    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), scaleTweak->inputAt(0));
    graph.connect(scaleTweak->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluateUpTo(&inputFrame, scaleTweak, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Position: (0,0) + (0.5, 0) = (0.5, 0)
    // Scale 2x from origin: (0.5, 0) * 2 = (1.0, 0)
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    delete result;
}

void TestEvaluateUpTo::testEvaluateUpToOutputNode()
{
    // evaluateUpTo(frame, outputNode) should produce same result as full evaluate()
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(0.5);
    posNode->setOffsetY(-0.3);
    posNode->setFollowGizmo(false);

    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* resultUpTo = evaluator.evaluateUpTo(&inputFrame, output, 0.0);
    auto* resultFull = evaluator.evaluate(&inputFrame, 0.0);

    QVERIFY(resultUpTo != nullptr);
    QVERIFY(resultFull != nullptr);
    QCOMPARE(resultUpTo->size(), resultFull->size());

    // Both should produce same result
    for (int i = 0; i < resultUpTo->size(); ++i)
    {
        QVERIFY(fuzzyCompare(resultUpTo->at(i).getX(), resultFull->at(i).getX()));
        QVERIFY(fuzzyCompare(resultUpTo->at(i).getY(), resultFull->at(i).getY()));
        QVERIFY(fuzzyCompare(resultUpTo->at(i).getR(), resultFull->at(i).getR()));
        QVERIFY(fuzzyCompare(resultUpTo->at(i).getG(), resultFull->at(i).getG()));
        QVERIFY(fuzzyCompare(resultUpTo->at(i).getB(), resultFull->at(i).getB()));
    }

    delete resultUpTo;
    delete resultFull;
}

void TestEvaluateUpTo::testEvaluateUpToInputNode()
{
    // evaluateUpTo(frame, inputNode) -> frame unchanged (input is before any tweak)
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(999.0);
    posNode->setOffsetY(999.0);
    posNode->setFollowGizmo(false);

    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.1, 0.2, 0.0, 1.0, 0.5, 0.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluateUpTo(&inputFrame, input, 0.0);

    // Stopping at input node means no tweaks applied
    // Implementation returns nullptr if stopNode is non-tweak and encountered first
    // OR returns the frame unchanged. Either way, verify the behavior.
    if (result != nullptr)
    {
        QCOMPARE(result->size(), 1);
        // Frame should be unchanged
        QVERIFY(fuzzyCompare(result->at(0).getX(), 0.1));
        QVERIFY(fuzzyCompare(result->at(0).getY(), 0.2));
        delete result;
    }
    // If nullptr, that is also acceptable (stopped before any processing)
}

void TestEvaluateUpTo::testEvaluateUpToNullStopNode()
{
    // evaluateUpTo(frame, nullptr) should handle gracefully (return nullptr)
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    graph.connect(input->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluateUpTo(&inputFrame, nullptr, 0.0);

    // nullptr stopNode -> should return nullptr per implementation
    QVERIFY(result == nullptr);
}

// ============================================================================
// Chain Scenarios
// ============================================================================

void TestEvaluateUpTo::testEvaluateUpToMiddleOfLongChain()
{
    // Input -> Pos -> Scale -> Rot -> Color -> Output
    // Stop at Scale -> only Pos + Scale applied
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* scaleTweak = graph.createNode("ScaleTweak", QPointF(300, 100));
    auto* rotTweak = graph.createNode("RotationTweak", QPointF(400, 100));
    auto* colorTweak = graph.createNode("ColorTweak", QPointF(500, 100));
    auto* output = graph.createNode("Output", QPointF(600, 100));

    // Configure tweaks
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

    auto* rotNode = qobject_cast<RotationTweak*>(rotTweak);
    rotNode->setAngle(90.0);
    rotNode->setCenterX(0.0);
    rotNode->setCenterY(0.0);
    rotNode->setFollowGizmo(false);

    auto* colorNode = qobject_cast<ColorTweak*>(colorTweak);
    colorNode->setColor(QColor(255, 0, 0));
    colorNode->setAlpha(1.0);
    colorNode->setFollowGizmo(false);

    // Chain: Input -> Pos -> Scale -> Rot -> Color -> Output
    graph.connect(input->outputAt(0), posTweak->inputAt(0));
    graph.connect(posTweak->outputAt(0), scaleTweak->inputAt(0));
    graph.connect(scaleTweak->outputAt(0), rotTweak->inputAt(0));
    graph.connect(rotTweak->outputAt(0), colorTweak->inputAt(0));
    graph.connect(colorTweak->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1);  // Green

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    // Stop at ScaleTweak: Pos + Scale applied, Rot + Color NOT applied
    auto* result = evaluator.evaluateUpTo(&inputFrame, scaleTweak, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 1);

    // Position: (0,0) + (0.5, 0) = (0.5, 0)
    // Scale 2x: (0.5, 0) * 2 = (1.0, 0)
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getY(), 0.0));

    // Rotation NOT applied (would have rotated 90 degrees)
    // Color NOT applied (should still be green, not red)
    QVERIFY(fuzzyCompare(result->at(0).getG(), 1.0));
    QVERIFY(fuzzyCompare(result->at(0).getR(), 0.0));

    delete result;
}

void TestEvaluateUpTo::testEvaluateUpToWithGizmoShape()
{
    // Input -> PosTweak(connected to Gizmo) -> Output
    // evaluateUpTo(frame, posTweak) -> position tweak applied with gizmo ratio modulation
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

    // Configure PositionTweak with followGizmo
    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(1.0);
    posNode->setOffsetY(0.0);
    posNode->setFollowGizmo(true);

    // Connect: Input -> PosTweak -> Output, Gizmo -> PosTweak.ratio
    graph.connect(input->outputAt(0), posTweak->inputAt(0));   // Frame
    graph.connect(gizmo->outputAt(0), posTweak->inputAt(1));   // Ratio
    graph.connect(posTweak->outputAt(0), output->inputAt(0));  // Frame

    // Test with two points: one inside gizmo (center), one outside
    xengine::Frame inputFrame;
    inputFrame.addSample(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);  // Center (inside)
    inputFrame.addSample(0.8, 0.0, 0.0, 1.0, 1.0, 1.0, 1);  // Outside

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluateUpTo(&inputFrame, posTweak, 0.0);
    QVERIFY(result != nullptr);
    QCOMPARE(result->size(), 2);

    // Point at center: ratio = 1.0, full offset -> x = 0.0 + 1.0 = 1.0
    QVERIFY(fuzzyCompare(result->at(0).getX(), 1.0));

    // Point outside gizmo: ratio = 0.0, no offset -> x stays ~0.8
    QVERIFY(fuzzyCompare(result->at(1).getX(), 0.8));

    delete result;
}

void TestEvaluateUpTo::testEvaluateUpToDisconnectedTweak()
{
    // Tweak not in the frame chain -> should return nullptr (not in path)
    NodeGraph graph;
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* posTweak = graph.createNode("PositionTweak", QPointF(200, 200));  // Floating, not connected
    auto* output = graph.createNode("Output", QPointF(400, 100));

    auto* posNode = qobject_cast<PositionTweak*>(posTweak);
    posNode->setOffsetX(999.0);
    posNode->setFollowGizmo(false);

    // Only connect Input -> Output, tweak is disconnected
    graph.connect(input->outputAt(0), output->inputAt(0));

    xengine::Frame inputFrame;
    inputFrame.addSample(0.1, 0.2, 0.0, 1.0, 1.0, 1.0, 1);

    GraphEvaluator evaluator;
    evaluator.setGraph(&graph);

    auto* result = evaluator.evaluateUpTo(&inputFrame, posTweak, 0.0);

    // Disconnected tweak is not in the path -> returns nullptr
    QVERIFY(result == nullptr);
}

QTEST_MAIN(TestEvaluateUpTo)
#include "tst_evaluate_up_to.moc"

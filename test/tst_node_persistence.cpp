#include <QtTest>
#include <QJsonDocument>

#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "core/NodeGraph.h"
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
#include "nodes/PolarTweak.h"
#include "nodes/SparkleTweak.h"
#include "nodes/FuzzynessTweak.h"
#include "nodes/ColorFuzzynessTweak.h"
#include "nodes/SplitTweak.h"
#include "nodes/RounderTweak.h"
#include "nodes/WaveTweak.h"
#include "nodes/SqueezeTweak.h"

using namespace gizmotweak2;

class TestNodePersistence : public QObject
{
    Q_OBJECT

private slots:
    // Individual node roundtrip tests
    void testGizmoNodeRoundtrip();
    void testGroupNodeRoundtrip();
    void testMirrorNodeRoundtrip();
    void testTimeShiftNodeRoundtrip();
    void testSurfaceFactoryNodeRoundtrip();
    void testPositionTweakRoundtrip();
    void testScaleTweakRoundtrip();
    void testRotationTweakRoundtrip();
    void testColorTweakRoundtrip();
    void testPolarTweakRoundtrip();
    void testSparkleTweakRoundtrip();
    void testWaveTweakRoundtrip();
    void testSqueezeTweakRoundtrip();
    void testFuzzynessTweakRoundtrip();
    void testColorFuzzynessTweakRoundtrip();
    void testSplitTweakRoundtrip();
    void testRounderTweakRoundtrip();

    // Full graph roundtrip tests
    void testEmptyGraphRoundtrip();
    void testSimpleGraphRoundtrip();
    void testComplexGraphRoundtrip();
    void testGraphWithAllNodeTypes();

    // Backward compatibility tests
    void testMissingFieldsUseDefaults();
    void testExtraFieldsIgnored();
    void testVersionCompatibility();

    // Edge cases
    void testSpecialCharactersInDisplayName();
    void testExtremePropertyValues();
    void testColorSerialization();

private:
    bool fuzzyCompare(qreal a, qreal b, qreal epsilon = 0.0001);
};

bool TestNodePersistence::fuzzyCompare(qreal a, qreal b, qreal epsilon)
{
    return qAbs(a - b) < epsilon;
}

// ============================================================================
// Gizmo Node Roundtrip
// ============================================================================

void TestNodePersistence::testGizmoNodeRoundtrip()
{
    // Create and configure
    GizmoNode original;
    original.setDisplayName("Test Gizmo");
    original.setShape(GizmoNode::Shape::Rectangle);
    original.setCenterX(0.3);
    original.setCenterY(-0.2);
    original.setHorizontalBorder(0.7);
    original.setVerticalBorder(0.4);
    original.setFalloff(0.15);
    original.setFalloffCurve(2);  // Different curve
    original.setHorizontalBend(0.1);
    original.setVerticalBend(-0.1);
    original.setNoiseIntensity(0.05);
    original.setNoiseScale(2.0);
    original.setNoiseSpeed(0.5);

    // Serialize
    QJsonObject json = original.propertiesToJson();

    // Deserialize into new node
    GizmoNode restored;
    restored.propertiesFromJson(json);

    // Verify
    QCOMPARE(restored.shape(), GizmoNode::Shape::Rectangle);
    QVERIFY(fuzzyCompare(restored.centerX(), 0.3));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.2));
    QVERIFY(fuzzyCompare(restored.horizontalBorder(), 0.7));
    QVERIFY(fuzzyCompare(restored.verticalBorder(), 0.4));
    QVERIFY(fuzzyCompare(restored.falloff(), 0.15));
    QCOMPARE(restored.falloffCurve(), 2);
    QVERIFY(fuzzyCompare(restored.horizontalBend(), 0.1));
    QVERIFY(fuzzyCompare(restored.verticalBend(), -0.1));
    QVERIFY(fuzzyCompare(restored.noiseIntensity(), 0.05));
    QVERIFY(fuzzyCompare(restored.noiseScale(), 2.0));
    QVERIFY(fuzzyCompare(restored.noiseSpeed(), 0.5));
}

// ============================================================================
// Group Node Roundtrip
// ============================================================================

void TestNodePersistence::testGroupNodeRoundtrip()
{
    GroupNode original;
    original.setCompositionMode(GroupNode::CompositionMode::Product);
    original.setPositionX(0.2);
    original.setPositionY(-0.3);
    original.setScaleX(1.5);
    original.setScaleY(0.8);
    original.setRotation(45.0);

    QJsonObject json = original.propertiesToJson();
    GroupNode restored;
    restored.propertiesFromJson(json);

    QCOMPARE(restored.compositionMode(), GroupNode::CompositionMode::Product);
    QVERIFY(fuzzyCompare(restored.positionX(), 0.2));
    QVERIFY(fuzzyCompare(restored.positionY(), -0.3));
    QVERIFY(fuzzyCompare(restored.scaleX(), 1.5));
    QVERIFY(fuzzyCompare(restored.scaleY(), 0.8));
    QVERIFY(fuzzyCompare(restored.rotation(), 45.0));
}

// ============================================================================
// Mirror Node Roundtrip
// ============================================================================

void TestNodePersistence::testMirrorNodeRoundtrip()
{
    MirrorNode original;
    original.setAxis(MirrorNode::Axis::Custom);
    original.setCustomAngle(37.5);

    QJsonObject json = original.propertiesToJson();
    MirrorNode restored;
    restored.propertiesFromJson(json);

    QCOMPARE(restored.axis(), MirrorNode::Axis::Custom);
    QVERIFY(fuzzyCompare(restored.customAngle(), 37.5));
}

// ============================================================================
// TimeShift Node Roundtrip
// ============================================================================

void TestNodePersistence::testTimeShiftNodeRoundtrip()
{
    TimeShiftNode original;
    original.setDelay(0.25);
    original.setScale(2.0);
    original.setLoop(true);
    original.setLoopDuration(1.5);

    QJsonObject json = original.propertiesToJson();
    TimeShiftNode restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.delay(), 0.25));
    QVERIFY(fuzzyCompare(restored.scale(), 2.0));
    QVERIFY(restored.loop());
    QVERIFY(fuzzyCompare(restored.loopDuration(), 1.5));
}

// ============================================================================
// SurfaceFactory Node Roundtrip
// ============================================================================

void TestNodePersistence::testSurfaceFactoryNodeRoundtrip()
{
    SurfaceFactoryNode original;
    original.setSurfaceType(SurfaceFactoryNode::SurfaceType::Triangle);
    original.setAmplitude(0.8);
    original.setFrequency(2.0);
    original.setPhase(45.0);
    original.setOffset(0.1);
    original.setClamp(false);

    QJsonObject json = original.propertiesToJson();
    SurfaceFactoryNode restored;
    restored.propertiesFromJson(json);

    QCOMPARE(restored.surfaceType(), SurfaceFactoryNode::SurfaceType::Triangle);
    QVERIFY(fuzzyCompare(restored.amplitude(), 0.8));
    QVERIFY(fuzzyCompare(restored.frequency(), 2.0));
    QVERIFY(fuzzyCompare(restored.phase(), 45.0));
    QVERIFY(fuzzyCompare(restored.offset(), 0.1));
    QVERIFY(!restored.clamp());
}

// ============================================================================
// Position Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testPositionTweakRoundtrip()
{
    PositionTweak original;
    original.setOffsetX(0.35);
    original.setOffsetY(-0.72);

    QJsonObject json = original.propertiesToJson();
    PositionTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.offsetX(), 0.35));
    QVERIFY(fuzzyCompare(restored.offsetY(), -0.72));
}

// ============================================================================
// Scale Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testScaleTweakRoundtrip()
{
    ScaleTweak original;
    // Set uniform=false FIRST to allow independent scaleX/Y values
    original.setUniform(false);
    original.setScaleX(2.5);
    original.setScaleY(0.5);
    original.setCenterX(0.1);
    original.setCenterY(-0.1);
    original.setCrossOver(true);
    original.setFollowGizmo(true);

    QJsonObject json = original.propertiesToJson();
    ScaleTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.scaleX(), 2.5));
    QVERIFY(fuzzyCompare(restored.scaleY(), 0.5));
    QVERIFY(!restored.uniform());
    QVERIFY(fuzzyCompare(restored.centerX(), 0.1));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.1));
    QVERIFY(restored.crossOver());
    QVERIFY(restored.followGizmo());
}

// ============================================================================
// Rotation Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testRotationTweakRoundtrip()
{
    RotationTweak original;
    original.setAngle(135.0);
    original.setCenterX(0.25);
    original.setCenterY(-0.25);
    original.setFollowGizmo(true);

    QJsonObject json = original.propertiesToJson();
    RotationTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.angle(), 135.0));
    QVERIFY(fuzzyCompare(restored.centerX(), 0.25));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.25));
    QVERIFY(restored.followGizmo());
}

// ============================================================================
// Color Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testColorTweakRoundtrip()
{
    ColorTweak original;
    original.setMode(ColorTweak::Mode::Multiply);
    original.setColor(QColor(128, 64, 255, 200));
    original.setIntensity(0.75);
    original.setAffectRed(false);
    original.setAffectGreen(true);
    original.setAffectBlue(true);
    original.setFilterRedMin(0.1);
    original.setFilterRedMax(0.9);
    original.setFilterGreenMin(0.2);
    original.setFilterGreenMax(0.8);
    original.setFilterBlueMin(0.3);
    original.setFilterBlueMax(0.7);

    QJsonObject json = original.propertiesToJson();
    ColorTweak restored;
    restored.propertiesFromJson(json);

    QCOMPARE(restored.mode(), ColorTweak::Mode::Multiply);
    QCOMPARE(restored.color().red(), 128);
    QCOMPARE(restored.color().green(), 64);
    QCOMPARE(restored.color().blue(), 255);
    QVERIFY(fuzzyCompare(restored.intensity(), 0.75));
    QVERIFY(!restored.affectRed());
    QVERIFY(restored.affectGreen());
    QVERIFY(restored.affectBlue());
    QVERIFY(fuzzyCompare(restored.filterRedMin(), 0.1));
    QVERIFY(fuzzyCompare(restored.filterRedMax(), 0.9));
}

// ============================================================================
// Polar Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testPolarTweakRoundtrip()
{
    PolarTweak original;
    original.setExpansion(0.8);
    original.setRingRadius(0.5);
    original.setRingScale(0.3);
    original.setCenterX(0.1);
    original.setCenterY(-0.1);
    original.setCrossOver(true);
    original.setTargetted(true);

    QJsonObject json = original.propertiesToJson();
    PolarTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.expansion(), 0.8));
    QVERIFY(fuzzyCompare(restored.ringRadius(), 0.5));
    QVERIFY(fuzzyCompare(restored.ringScale(), 0.3));
    QVERIFY(fuzzyCompare(restored.centerX(), 0.1));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.1));
    QVERIFY(restored.crossOver());
    QVERIFY(restored.targetted());
}

// ============================================================================
// Sparkle Tweak Roundtrip
// ============================================================================

void TestNodePersistence::testSparkleTweakRoundtrip()
{
    SparkleTweak original;
    original.setDensity(0.7);
    original.setRed(1.0);
    original.setGreen(0.5);
    original.setBlue(0.25);
    original.setAlpha(0.8);
    original.setFollowGizmo(false);

    QJsonObject json = original.propertiesToJson();
    SparkleTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.density(), 0.7));
    QVERIFY(fuzzyCompare(restored.red(), 1.0));
    QVERIFY(fuzzyCompare(restored.green(), 0.5));
    QVERIFY(fuzzyCompare(restored.blue(), 0.25));
    QVERIFY(fuzzyCompare(restored.alpha(), 0.8));
    QVERIFY(!restored.followGizmo());
}

// ============================================================================
// Other Tweak Roundtrips (simplified)
// ============================================================================

void TestNodePersistence::testWaveTweakRoundtrip()
{
    WaveTweak original;
    original.setAmplitude(0.15);
    original.setWavelength(0.3);
    original.setPhase(45.0);
    original.setAngle(90.0);
    original.setRadial(false);
    original.setCenterX(0.2);
    original.setCenterY(-0.2);

    QJsonObject json = original.propertiesToJson();
    WaveTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.amplitude(), 0.15));
    QVERIFY(fuzzyCompare(restored.wavelength(), 0.3));
    QVERIFY(fuzzyCompare(restored.phase(), 45.0));
    QVERIFY(fuzzyCompare(restored.angle(), 90.0));
    QVERIFY(!restored.radial());
    QVERIFY(fuzzyCompare(restored.centerX(), 0.2));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.2));
}

void TestNodePersistence::testSqueezeTweakRoundtrip()
{
    SqueezeTweak original;
    original.setIntensity(0.6);
    original.setAngle(90.0);
    original.setCenterX(0.15);
    original.setCenterY(-0.15);

    QJsonObject json = original.propertiesToJson();
    SqueezeTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.intensity(), 0.6));
    QVERIFY(fuzzyCompare(restored.angle(), 90.0));
    QVERIFY(fuzzyCompare(restored.centerX(), 0.15));
    QVERIFY(fuzzyCompare(restored.centerY(), -0.15));
}

void TestNodePersistence::testFuzzynessTweakRoundtrip()
{
    FuzzynessTweak original;
    original.setAmount(0.2);
    original.setAffectX(false);
    original.setAffectY(true);
    original.setSeed(123);
    original.setUseSeed(true);

    QJsonObject json = original.propertiesToJson();
    FuzzynessTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.amount(), 0.2));
    QVERIFY(!restored.affectX());
    QVERIFY(restored.affectY());
    QCOMPARE(restored.seed(), 123);
    QVERIFY(restored.useSeed());
}

void TestNodePersistence::testColorFuzzynessTweakRoundtrip()
{
    ColorFuzzynessTweak original;
    original.setAmount(0.3);
    original.setAffectRed(true);
    original.setAffectGreen(false);
    original.setAffectBlue(true);
    original.setSeed(456);
    original.setUseSeed(true);

    QJsonObject json = original.propertiesToJson();
    ColorFuzzynessTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.amount(), 0.3));
    QVERIFY(restored.affectRed());
    QVERIFY(!restored.affectGreen());
    QVERIFY(restored.affectBlue());
    QCOMPARE(restored.seed(), 456);
    QVERIFY(restored.useSeed());
}

void TestNodePersistence::testSplitTweakRoundtrip()
{
    SplitTweak original;
    original.setSplitThreshold(0.25);

    QJsonObject json = original.propertiesToJson();
    SplitTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.splitThreshold(), 0.25));
}

void TestNodePersistence::testRounderTweakRoundtrip()
{
    RounderTweak original;
    original.setAmount(0.5);
    original.setVerticalShift(0.1);
    original.setHorizontalShift(-0.1);
    original.setTighten(0.2);
    original.setRadialResize(1.5);
    original.setRadialShift(0.05);

    QJsonObject json = original.propertiesToJson();
    RounderTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.amount(), 0.5));
    QVERIFY(fuzzyCompare(restored.verticalShift(), 0.1));
    QVERIFY(fuzzyCompare(restored.horizontalShift(), -0.1));
    QVERIFY(fuzzyCompare(restored.tighten(), 0.2));
    QVERIFY(fuzzyCompare(restored.radialResize(), 1.5));
    QVERIFY(fuzzyCompare(restored.radialShift(), 0.05));
}

// ============================================================================
// Full Graph Roundtrip Tests
// ============================================================================

void TestNodePersistence::testEmptyGraphRoundtrip()
{
    NodeGraph original;

    QJsonObject json = original.toJson();

    NodeGraph restored;
    QVERIFY(restored.fromJson(json));

    QCOMPARE(restored.nodeCount(), 0);
    QCOMPARE(restored.connectionCount(), 0);
}

void TestNodePersistence::testSimpleGraphRoundtrip()
{
    NodeGraph original;

    auto* input = original.createNode("Input", QPointF(100, 100));
    auto* output = original.createNode("Output", QPointF(400, 100));
    input->setDisplayName("My Input");

    original.connect(input->outputAt(0), output->inputAt(0));

    QJsonObject json = original.toJson();

    NodeGraph restored;
    QVERIFY(restored.fromJson(json));

    QCOMPARE(restored.nodeCount(), 2);
    QCOMPARE(restored.connectionCount(), 1);

    // Verify positions preserved
    auto* restoredInput = restored.nodeAt(0);
    QVERIFY(restoredInput != nullptr);
    QCOMPARE(restoredInput->position(), QPointF(100, 100));
}

void TestNodePersistence::testComplexGraphRoundtrip()
{
    NodeGraph original;

    // Create a more complex graph
    auto* input = original.createNode("Input", QPointF(100, 200));
    auto* gizmo = original.createNode("Gizmo", QPointF(100, 400));
    auto* position = original.createNode("PositionTweak", QPointF(300, 200));
    auto* scale = original.createNode("ScaleTweak", QPointF(500, 200));
    auto* output = original.createNode("Output", QPointF(700, 200));

    // Configure nodes
    qobject_cast<GizmoNode*>(gizmo)->setCenterX(0.2);
    qobject_cast<PositionTweak*>(position)->setOffsetX(0.5);
    qobject_cast<ScaleTweak*>(scale)->setScaleX(2.0);

    // Connect
    original.connect(input->outputAt(0), position->inputAt(0));
    original.connect(gizmo->outputAt(0), position->inputAt(1));
    original.connect(position->outputAt(0), scale->inputAt(0));
    original.connect(scale->outputAt(0), output->inputAt(0));

    QJsonObject json = original.toJson();

    NodeGraph restored;
    QVERIFY(restored.fromJson(json));

    QCOMPARE(restored.nodeCount(), 5);
    QCOMPARE(restored.connectionCount(), 4);

    // Verify properties preserved
    bool foundGizmo = false;
    for (int i = 0; i < restored.nodeCount(); ++i)
    {
        auto* node = restored.nodeAt(i);
        if (node->type() == "Gizmo")
        {
            auto* g = qobject_cast<GizmoNode*>(node);
            QVERIFY(fuzzyCompare(g->centerX(), 0.2));
            foundGizmo = true;
        }
    }
    QVERIFY(foundGizmo);
}

void TestNodePersistence::testGraphWithAllNodeTypes()
{
    NodeGraph original;

    // Create one of each node type
    QStringList types = original.availableNodeTypes();
    int y = 100;
    for (const QString& type : types)
    {
        original.createNode(type, QPointF(100, y));
        y += 100;
    }

    QJsonObject json = original.toJson();

    NodeGraph restored;
    QVERIFY(restored.fromJson(json));

    QCOMPARE(restored.nodeCount(), types.size());
}

// ============================================================================
// Backward Compatibility Tests
// ============================================================================

void TestNodePersistence::testMissingFieldsUseDefaults()
{
    // Create JSON with missing fields
    QJsonObject json;
    // Only set some fields, leave others out

    ScaleTweak tweak;
    tweak.propertiesFromJson(json);

    // Should use default values
    QVERIFY(fuzzyCompare(tweak.scaleX(), 1.0));  // Default
    QVERIFY(fuzzyCompare(tweak.scaleY(), 1.0));  // Default
    QVERIFY(tweak.uniform());                     // Default is true
}

void TestNodePersistence::testExtraFieldsIgnored()
{
    QJsonObject json;
    json["uniform"] = false;  // Allow independent scaleX/Y
    json["scaleX"] = 2.0;
    json["scaleY"] = 3.0;
    json["unknownField"] = "should be ignored";
    json["anotherUnknown"] = 42;

    ScaleTweak tweak;
    tweak.propertiesFromJson(json);

    // Known fields loaded correctly
    QVERIFY(fuzzyCompare(tweak.scaleX(), 2.0));
    QVERIFY(fuzzyCompare(tweak.scaleY(), 3.0));
    // No crash from unknown fields
}

void TestNodePersistence::testVersionCompatibility()
{
    // Test loading current version
    QJsonObject json1;
    json1["version"] = 1;
    json1["nodes"] = QJsonArray();
    json1["connections"] = QJsonArray();

    NodeGraph graph1;
    QVERIFY(graph1.fromJson(json1));

    // Test that legacy string versions are accepted (treated as version 0)
    QJsonObject json2;
    json2["version"] = "0.2.0";  // Old format
    json2["nodes"] = QJsonArray();
    json2["connections"] = QJsonArray();

    NodeGraph graph2;
    QVERIFY(graph2.fromJson(json2));

    // Test that future versions are rejected
    QJsonObject json3;
    json3["version"] = 999;  // Future version
    json3["nodes"] = QJsonArray();
    json3["connections"] = QJsonArray();

    NodeGraph graph3;
    QVERIFY(!graph3.fromJson(json3));
}

// ============================================================================
// Edge Cases
// ============================================================================

void TestNodePersistence::testSpecialCharactersInDisplayName()
{
    NodeGraph original;
    auto* node = original.createNode("Gizmo", QPointF(100, 100));
    node->setDisplayName("Test \"Node\" with 'quotes' & <special> chars");

    QJsonObject json = original.toJson();

    NodeGraph restored;
    QVERIFY(restored.fromJson(json));

    auto* restoredNode = restored.nodeAt(0);
    QCOMPARE(restoredNode->displayName(), QString("Test \"Node\" with 'quotes' & <special> chars"));
}

void TestNodePersistence::testExtremePropertyValues()
{
    PositionTweak original;
    original.setOffsetX(1e10);
    original.setOffsetY(-1e10);

    QJsonObject json = original.propertiesToJson();
    PositionTweak restored;
    restored.propertiesFromJson(json);

    QVERIFY(fuzzyCompare(restored.offsetX(), 1e10, 1e5));
    QVERIFY(fuzzyCompare(restored.offsetY(), -1e10, 1e5));
}

void TestNodePersistence::testColorSerialization()
{
    ColorTweak original;

    // Test various color formats
    original.setColor(QColor(255, 0, 0));  // Pure red
    QJsonObject json1 = original.propertiesToJson();
    ColorTweak restored1;
    restored1.propertiesFromJson(json1);
    QCOMPARE(restored1.color().red(), 255);
    QCOMPARE(restored1.color().green(), 0);
    QCOMPARE(restored1.color().blue(), 0);

    // Test with alpha
    original.setColor(QColor(128, 64, 32, 200));
    QJsonObject json2 = original.propertiesToJson();
    ColorTweak restored2;
    restored2.propertiesFromJson(json2);
    QCOMPARE(restored2.color().red(), 128);
    QCOMPARE(restored2.color().green(), 64);
    QCOMPARE(restored2.color().blue(), 32);
    QCOMPARE(restored2.color().alpha(), 200);
}

QTEST_MAIN(TestNodePersistence)
#include "tst_node_persistence.moc"

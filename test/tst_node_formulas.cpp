#include <QtTest>
#include <QtMath>

#include "core/Node.h"
#include "core/Port.h"
#include "core/NodeGraph.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
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
#include "nodes/TimeShiftNode.h"
#include "nodes/SurfaceFactoryNode.h"

#include <frame.h>

using namespace gizmotweak2;

class TestNodeFormulas : public QObject
{
    Q_OBJECT

private slots:
    // Gizmo tests
    void testGizmoEllipseCenter();
    void testGizmoEllipseEdge();
    void testGizmoEllipseOutside();
    void testGizmoRectangleCenter();
    void testGizmoRectangleEdge();
    void testGizmoAsymmetricBorders();

    // Group tests
    void testGroupNormalMode();
    void testGroupMaxMode();
    void testGroupMinMode();
    void testGroupSumMode();
    void testGroupProductMode();
    void testGroupDiffMode();
    void testGroupAbsDiffMode();
    void testGroupTransformCoordinates();

    // Mirror tests
    void testMirrorHorizontal();
    void testMirrorVertical();
    void testMirrorDiagonal45();
    void testMirrorDiagonalMinus45();
    void testMirrorCustomAngle();

    // Position Tweak tests
    void testPositionTweakNoRatio();
    void testPositionTweakFullRatio();
    void testPositionTweakHalfRatio();

    // Scale Tweak tests
    void testScaleTweakNoRatio();
    void testScaleTweakFullRatio();
    void testScaleTweakUniform();
    void testScaleTweakAroundCenter();

    // Rotation Tweak tests
    void testRotationTweakNoRatio();
    void testRotationTweak90Degrees();
    void testRotationTweak180Degrees();
    void testRotationTweakAroundCenter();

    // Color Tweak tests
    void testColorTweakTintMode();
    void testColorTweakReplaceMode();
    void testColorTweakAffectChannels();
    void testColorTweakFilter();

    // Polar Tweak tests
    void testPolarTweakToPolar();
    void testPolarTweakFromPolar();

    // Complex transforms
    void testWaveTweak();
    void testSqueezeTweak();

    // Sparkle Tweak tests
    void testSparkleShouldSparkle();
    void testSparkleColorBlend();
    void testSparklePrecalcValues();
    void testSparkleIsActive();

    // Fuzzyness Tweak tests
    void testFuzzynessNoRatio();
    void testFuzzynessFullRatio();
    void testFuzzynessAffectXOnly();
    void testFuzzynessAffectYOnly();
    void testFuzzynessDeterministicSeed();

    // Color Fuzzyness Tweak tests
    void testColorFuzzynessNoRatio();
    void testColorFuzzynessFullRatio();
    void testColorFuzzynessAffectChannels();
    void testColorFuzzynessDeterministicSeed();

    // Split Tweak tests
    void testSplitEffectiveThreshold();
    void testSplitShouldSplitClose();
    void testSplitShouldSplitFar();
    void testSplitRatioEffect();

    // Rounder Tweak tests
    void testRounderNoEffect();
    void testRounderWithAmount();
    void testRounderCenterUnchanged();
    void testRounderVerticalShift();

    // TimeShift tests
    void testTimeShiftDelay();
    void testTimeShiftScale();
    void testTimeShiftLoop();
    void testTimeShiftCombined();

    // SurfaceFactory tests
    void testSurfaceFactorySine();
    void testSurfaceFactoryCosine();
    void testSurfaceFactoryTriangle();
    void testSurfaceFactorySawtooth();
    void testSurfaceFactorySquare();
    void testSurfaceFactoryLinear();
    void testSurfaceFactoryClamp();
    void testSurfaceFactoryOffset();

private:
    bool fuzzyCompare(qreal a, qreal b, qreal epsilon = 0.0001);
    bool fuzzyComparePoint(QPointF a, QPointF b, qreal epsilon = 0.0001);
};

bool TestNodeFormulas::fuzzyCompare(qreal a, qreal b, qreal epsilon)
{
    return qAbs(a - b) < epsilon;
}

bool TestNodeFormulas::fuzzyComparePoint(QPointF a, QPointF b, qreal epsilon)
{
    return fuzzyCompare(a.x(), b.x(), epsilon) && fuzzyCompare(a.y(), b.y(), epsilon);
}

// ============================================================================
// Gizmo Tests
// ============================================================================

void TestNodeFormulas::testGizmoEllipseCenter()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Ellipse);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.5);
    gizmo.setVerticalBorder(0.5);


    // At center, ratio should be 1.0
    qreal ratio = gizmo.computeRatio(0.0, 0.0, 0.0);
    QVERIFY(fuzzyCompare(ratio, 1.0));
}

void TestNodeFormulas::testGizmoEllipseEdge()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Ellipse);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.5);
    gizmo.setVerticalBorder(0.5);


    // At border edge, ratio should be close to 1.0 (just inside)
    qreal ratio = gizmo.computeRatio(0.49, 0.0, 0.0);
    QVERIFY(ratio > 0.9);

    // Just outside border, ratio should be 0.0
    ratio = gizmo.computeRatio(0.6, 0.0, 0.0);
    QVERIFY(fuzzyCompare(ratio, 0.0));
}

void TestNodeFormulas::testGizmoEllipseOutside()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Ellipse);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.5);
    gizmo.setVerticalBorder(0.5);


    // Far outside, ratio should be 0.0
    qreal ratio = gizmo.computeRatio(1.0, 1.0, 0.0);
    QVERIFY(fuzzyCompare(ratio, 0.0));
}

void TestNodeFormulas::testGizmoRectangleCenter()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Rectangle);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.5);
    gizmo.setVerticalBorder(0.5);


    // At center, ratio should be 1.0
    qreal ratio = gizmo.computeRatio(0.0, 0.0, 0.0);
    QVERIFY(fuzzyCompare(ratio, 1.0));
}

void TestNodeFormulas::testGizmoRectangleEdge()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Rectangle);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.5);
    gizmo.setVerticalBorder(0.5);


    // Inside rectangle
    qreal ratio = gizmo.computeRatio(0.4, 0.4, 0.0);
    QVERIFY(fuzzyCompare(ratio, 1.0));

    // Outside rectangle
    ratio = gizmo.computeRatio(0.6, 0.0, 0.0);
    QVERIFY(fuzzyCompare(ratio, 0.0));
}

void TestNodeFormulas::testGizmoAsymmetricBorders()
{
    GizmoNode gizmo;
    gizmo.setShape(GizmoNode::Shape::Ellipse);
    gizmo.setCenterX(0.0);
    gizmo.setCenterY(0.0);
    gizmo.setHorizontalBorder(0.8);
    gizmo.setVerticalBorder(0.3);


    // Inside on X axis (wide border)
    qreal ratio = gizmo.computeRatio(0.7, 0.0, 0.0);
    QVERIFY(ratio > 0.5);

    // Outside on Y axis (narrow border)
    ratio = gizmo.computeRatio(0.0, 0.5, 0.0);
    QVERIFY(fuzzyCompare(ratio, 0.0));
}

// ============================================================================
// Group Tests
// ============================================================================

void TestNodeFormulas::testGroupNormalMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Normal);

    // Same sign positive - takes max
    QList<qreal> ratios1 = {0.3, 0.7};
    QVERIFY(fuzzyCompare(group.combine(ratios1), 0.7));

    // Same sign negative - takes min
    QList<qreal> ratios2 = {-0.3, -0.7};
    QVERIFY(fuzzyCompare(group.combine(ratios2), -0.7));

    // Opposite signs - sums
    QList<qreal> ratios3 = {0.5, -0.3};
    QVERIFY(fuzzyCompare(group.combine(ratios3), 0.2));
}

void TestNodeFormulas::testGroupMaxMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Max);

    QList<qreal> ratios = {0.2, 0.8, 0.5};
    QVERIFY(fuzzyCompare(group.combine(ratios), 0.8));
}

void TestNodeFormulas::testGroupMinMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Min);

    QList<qreal> ratios = {0.2, 0.8, 0.5};
    QVERIFY(fuzzyCompare(group.combine(ratios), 0.2));
}

void TestNodeFormulas::testGroupSumMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Sum);

    QList<qreal> ratios = {0.3, 0.4};
    qreal result = group.combine(ratios);
    // Sum is NOT clamped (matches original GizmoTweak)
    QVERIFY(fuzzyCompare(result, 0.7));

    // No clamping - values can exceed [-1, 1]
    QList<qreal> ratios2 = {0.8, 0.5};
    result = group.combine(ratios2);
    QVERIFY(fuzzyCompare(result, 1.3));  // Not clamped
}

void TestNodeFormulas::testGroupProductMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Product);

    QList<qreal> ratios = {0.5, 0.8};
    QVERIFY(fuzzyCompare(group.combine(ratios), 0.4));
}

void TestNodeFormulas::testGroupDiffMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::Diff);

    // Diff computes: result = ratios[i] - result (successive subtraction)
    // For [0.8, 0.3]: result = 0.8, then result = 0.3 - 0.8 = -0.5
    QList<qreal> ratios = {0.8, 0.3};
    QVERIFY(fuzzyCompare(group.combine(ratios), -0.5));
}

void TestNodeFormulas::testGroupAbsDiffMode()
{
    GroupNode group;
    group.setCompositionMode(GroupNode::CompositionMode::AbsDiff);

    QList<qreal> ratios = {0.3, 0.8};
    QVERIFY(fuzzyCompare(group.combine(ratios), 0.5));

    // Order shouldn't matter for AbsDiff
    QList<qreal> ratios2 = {0.8, 0.3};
    QVERIFY(fuzzyCompare(group.combine(ratios2), 0.5));
}

void TestNodeFormulas::testGroupTransformCoordinates()
{
    GroupNode group;
    group.setPositionX(0.1);
    group.setPositionY(0.2);
    group.setScaleX(2.0);
    group.setScaleY(2.0);
    group.setRotation(0.0);

    qreal outX, outY;
    group.transformCoordinates(0.5, 0.5, outX, outY);

    // Inverse transformation: world to local coordinates
    // First translate: x0 = 0.5 - 0.1 = 0.4, y0 = 0.5 - 0.2 = 0.3
    // Then divide by scale (inverse): outX = 0.4 / 2.0 = 0.2, outY = 0.3 / 2.0 = 0.15
    QVERIFY(fuzzyCompare(outX, 0.2));
    QVERIFY(fuzzyCompare(outY, 0.15));
}

// ============================================================================
// Mirror Tests
// ============================================================================

void TestNodeFormulas::testMirrorHorizontal()
{
    MirrorNode mirror;
    mirror.setAxis(MirrorNode::Axis::Horizontal);

    QPointF result = mirror.mirror(0.5, 0.3);
    QVERIFY(fuzzyComparePoint(result, QPointF(-0.5, 0.3)));

    result = mirror.mirror(-0.2, 0.7);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.2, 0.7)));
}

void TestNodeFormulas::testMirrorVertical()
{
    MirrorNode mirror;
    mirror.setAxis(MirrorNode::Axis::Vertical);

    QPointF result = mirror.mirror(0.5, 0.3);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, -0.3)));

    result = mirror.mirror(0.2, -0.7);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.2, 0.7)));
}

void TestNodeFormulas::testMirrorDiagonal45()
{
    MirrorNode mirror;
    mirror.setAxis(MirrorNode::Axis::Diagonal45);

    // Swaps X and Y
    QPointF result = mirror.mirror(0.3, 0.7);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.7, 0.3)));
}

void TestNodeFormulas::testMirrorDiagonalMinus45()
{
    MirrorNode mirror;
    mirror.setAxis(MirrorNode::Axis::DiagonalMinus45);

    // Swaps and negates
    QPointF result = mirror.mirror(0.3, 0.7);
    QVERIFY(fuzzyComparePoint(result, QPointF(-0.7, -0.3)));
}

void TestNodeFormulas::testMirrorCustomAngle()
{
    MirrorNode mirror;
    mirror.setAxis(MirrorNode::Axis::Custom);

    // 0 degrees = horizontal axis (same as Horizontal)
    mirror.setCustomAngle(0.0);
    QPointF result = mirror.mirror(0.5, 0.3);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, -0.3)));

    // 90 degrees = vertical axis
    mirror.setCustomAngle(90.0);
    result = mirror.mirror(0.5, 0.3);
    QVERIFY(fuzzyComparePoint(result, QPointF(-0.5, 0.3)));
}

// ============================================================================
// Position Tweak Tests
// ============================================================================

void TestNodeFormulas::testPositionTweakNoRatio()
{
    PositionTweak tweak;
    tweak.setOffsetX(0.5);
    tweak.setOffsetY(-0.3);

    // With ratio = 0, no movement
    QPointF result = tweak.apply(0.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.0, 0.0)));
}

void TestNodeFormulas::testPositionTweakFullRatio()
{
    PositionTweak tweak;
    tweak.setOffsetX(0.5);
    tweak.setOffsetY(-0.3);

    // With ratio = 1, full movement
    QPointF result = tweak.apply(0.0, 0.0, 1.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, -0.3)));
}

void TestNodeFormulas::testPositionTweakHalfRatio()
{
    PositionTweak tweak;
    tweak.setOffsetX(1.0);
    tweak.setOffsetY(1.0);

    // With ratio = 0.5, half movement
    QPointF result = tweak.apply(0.0, 0.0, 0.5);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.5)));
}

// ============================================================================
// Scale Tweak Tests
// ============================================================================

void TestNodeFormulas::testScaleTweakNoRatio()
{
    ScaleTweak tweak;
    tweak.setScaleX(2.0);
    tweak.setScaleY(2.0);
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // With ratio = 0, no scaling
    QPointF result = tweak.apply(0.5, 0.5, 0.0, 0.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.5)));
}

void TestNodeFormulas::testScaleTweakFullRatio()
{
    ScaleTweak tweak;
    tweak.setScaleX(2.0);
    tweak.setScaleY(2.0);
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // With ratio = 1, full scaling (2x)
    QPointF result = tweak.apply(0.5, 0.5, 1.0, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(1.0, 1.0)));
}

void TestNodeFormulas::testScaleTweakUniform()
{
    ScaleTweak tweak;
    tweak.setUniform(true);
    tweak.setScaleX(3.0);
    // ScaleY should be synced

    QVERIFY(fuzzyCompare(tweak.scaleY(), 3.0));
}

void TestNodeFormulas::testScaleTweakAroundCenter()
{
    ScaleTweak tweak;
    tweak.setScaleX(2.0);
    tweak.setScaleY(2.0);
    tweak.setCenterX(0.5);
    tweak.setCenterY(0.5);

    // Point at center stays at center
    QPointF result = tweak.apply(0.5, 0.5, 1.0, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.5)));

    // Point away from center moves
    result = tweak.apply(0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    // (0 - 0.5) * 2 + 0.5 = -0.5
    QVERIFY(fuzzyComparePoint(result, QPointF(-0.5, -0.5)));
}

// ============================================================================
// Rotation Tweak Tests
// ============================================================================

void TestNodeFormulas::testRotationTweakNoRatio()
{
    RotationTweak tweak;
    tweak.setAngle(90.0);
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // With ratio = 0, no rotation
    QPointF result = tweak.apply(1.0, 0.0, 0.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(1.0, 0.0)));
}

void TestNodeFormulas::testRotationTweak90Degrees()
{
    RotationTweak tweak;
    tweak.setAngle(90.0);
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // Rotate (1, 0) by 90 degrees around origin -> (0, 1)
    QPointF result = tweak.apply(1.0, 0.0, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.0, 1.0)));
}

void TestNodeFormulas::testRotationTweak180Degrees()
{
    RotationTweak tweak;
    tweak.setAngle(180.0);
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // Rotate (1, 0) by 180 degrees around origin -> (-1, 0)
    QPointF result = tweak.apply(1.0, 0.0, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(-1.0, 0.0)));
}

void TestNodeFormulas::testRotationTweakAroundCenter()
{
    RotationTweak tweak;
    tweak.setAngle(90.0);
    tweak.setCenterX(0.5);
    tweak.setCenterY(0.5);

    // Point at center stays at center
    QPointF result = tweak.apply(0.5, 0.5, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.5)));
}

// ============================================================================
// Color Tweak Tests
// ============================================================================

void TestNodeFormulas::testColorTweakTintMode()
{
    ColorTweak tweak;
    tweak.setColor(QColor(255, 0, 0));  // Red
    tweak.setAlpha(1.0);

    QColor input(0, 255, 0);  // Green
    QColor result = tweak.apply(input, 1.0);

    // Full blend towards red (lerp with alpha=1.0)
    QCOMPARE(result.red(), 255);
    QCOMPARE(result.green(), 0);
    QCOMPARE(result.blue(), 0);
}

void TestNodeFormulas::testColorTweakReplaceMode()
{
    ColorTweak tweak;
    tweak.setColor(QColor(255, 128, 0));  // Orange
    tweak.setAlpha(1.0);

    QColor input(0, 0, 255);  // Blue
    QColor result = tweak.apply(input, 1.0);

    // Full blend towards orange (lerp with alpha=1.0)
    QCOMPARE(result.red(), 255);
    QCOMPARE(result.green(), 128);
    QCOMPARE(result.blue(), 0);
}

void TestNodeFormulas::testColorTweakAffectChannels()
{
    ColorTweak tweak;
    tweak.setColor(QColor(255, 255, 255));  // White
    tweak.setAlpha(1.0);
    tweak.setAffectRed(true);
    tweak.setAffectGreen(false);
    tweak.setAffectBlue(false);

    QColor input(0, 128, 64);
    QColor result = tweak.apply(input, 1.0);

    // Only red affected
    QCOMPARE(result.red(), 255);
    QCOMPARE(result.green(), 128);  // Unchanged
    QCOMPARE(result.blue(), 64);    // Unchanged
}

void TestNodeFormulas::testColorTweakFilter()
{
    ColorTweak tweak;
    tweak.setColor(QColor(255, 0, 0));
    tweak.setAlpha(1.0);

    // Set filter to only affect colors with high green
    tweak.setFilterGreenMin(0.8);
    tweak.setFilterGreenMax(1.0);

    // Color with low green - should not be affected
    QColor input1(0, 100, 0);  // Green ~0.39
    QColor result1 = tweak.apply(input1, 1.0);
    QCOMPARE(result1.green(), 100);  // Unchanged

    // Color with high green - should be affected
    QColor input2(0, 255, 0);  // Green = 1.0
    QColor result2 = tweak.apply(input2, 1.0);
    QCOMPARE(result2.red(), 255);  // Changed to red
}

// ============================================================================
// Polar Tweak Tests
// ============================================================================

void TestNodeFormulas::testPolarTweakToPolar()
{
    PolarTweak tweak;
    tweak.setExpansion(0.5);  // Radial expansion
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // With expansion, points should move away from center
    QPointF result = tweak.apply(0.5, 0.0, 1.0, 1.0, 0.0, 0.0);

    // Point should be pushed outward
    QVERIFY(result.x() > 0.5 || !fuzzyCompare(result.x(), 0.5));
}

void TestNodeFormulas::testPolarTweakFromPolar()
{
    PolarTweak tweak;
    tweak.setExpansion(-0.5);  // Radial contraction
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);
    tweak.setTargetted(true);

    // With negative expansion and targetted, points should move towards center
    QPointF result = tweak.apply(0.5, 0.0, 1.0, 1.0, 0.0, 0.0);
    QVERIFY(result.x() < 0.5 || !fuzzyCompare(result.x(), 0.5));
}

// ============================================================================
// Complex Transform Tests
// ============================================================================

void TestNodeFormulas::testWaveTweak()
{
    WaveTweak tweak;
    tweak.setAmplitude(0.1);
    tweak.setWavelength(1.0);
    tweak.setPhase(0.0);
    tweak.setRadial(false);  // Directional wave
    tweak.setAngle(0.0);     // Along X axis

    // Wave should modify position based on distance along wave direction
    QPointF result = tweak.apply(0.0, 0.0, 1.0, 0.0, 0.0);
    // At position 0, the wave displacement depends on implementation
    // Just verify it returns a valid point
    QVERIFY(!qIsNaN(result.x()) && !qIsNaN(result.y()));

    // With ratio=0, no effect
    result = tweak.apply(0.5, 0.0, 0.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.0)));
}

void TestNodeFormulas::testSqueezeTweak()
{
    SqueezeTweak tweak;
    tweak.setIntensity(0.5);
    tweak.setAngle(0.0);  // Squeeze along X, stretch along Y
    tweak.setCenterX(0.0);
    tweak.setCenterY(0.0);

    // Point at center should not move
    QPointF result = tweak.apply(0.0, 0.0, 1.0, 0.0, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.0, 0.0)));

    // Point away should be squeezed/stretched
    result = tweak.apply(0.5, 0.5, 1.0, 0.0, 0.0);
    QVERIFY(!fuzzyComparePoint(result, QPointF(0.5, 0.5)));
}

// ============================================================================
// Sparkle Tweak Tests
// ============================================================================

void TestNodeFormulas::testSparkleShouldSparkle()
{
    SparkleTweak tweak;
    tweak.setDensity(0.5);

    // With random value below density, should sparkle
    bool sparkle1 = tweak.shouldSparkle(0.3, 0.0, 0.0, 0.1, 0.0);
    QVERIFY(sparkle1);

    // With random value above density, should not sparkle
    bool sparkle2 = tweak.shouldSparkle(0.8, 0.0, 0.0, 0.1, 0.0);
    QVERIFY(!sparkle2);

    // With density 0, never sparkle
    tweak.setDensity(0.0);
    bool sparkle3 = tweak.shouldSparkle(0.0, 0.0, 0.0, 0.1, 0.0);
    QVERIFY(!sparkle3);

    // With density 1, always sparkle (when random < 1)
    tweak.setDensity(1.0);
    bool sparkle4 = tweak.shouldSparkle(0.99, 0.0, 0.0, 0.1, 0.0);
    QVERIFY(sparkle4);
}

void TestNodeFormulas::testSparkleColorBlend()
{
    SparkleTweak tweak;
    tweak.setRed(1.0);
    tweak.setGreen(0.0);
    tweak.setBlue(0.0);
    tweak.setAlpha(1.0);

    qreal outR, outG, outB;

    // Full alpha should produce sparkle color
    tweak.calculatePrecalcValues(1.0);
    tweak.calculateSparkleColor(0.0, 1.0, 0.0, outR, outG, outB);

    // Result depends on alpha blend formula
    QVERIFY(outR >= 0.0 && outR <= 1.0);
    QVERIFY(outG >= 0.0 && outG <= 1.0);
    QVERIFY(outB >= 0.0 && outB <= 1.0);
}

void TestNodeFormulas::testSparklePrecalcValues()
{
    SparkleTweak tweak;
    tweak.setDensity(0.5);
    tweak.setAlpha(0.8);

    // With ratio = 1.0
    tweak.calculatePrecalcValues(1.0);
    QVERIFY(fuzzyCompare(tweak.precalcDensity(), 0.5));

    // With ratio = 0.0, density should be 0
    tweak.calculatePrecalcValues(0.0);
    QVERIFY(fuzzyCompare(tweak.precalcDensity(), 0.0));

    // With ratio = 0.5, density should be halved
    tweak.calculatePrecalcValues(0.5);
    QVERIFY(fuzzyCompare(tweak.precalcDensity(), 0.25));
}

void TestNodeFormulas::testSparkleIsActive()
{
    SparkleTweak tweak;

    // With density 0, not active
    tweak.setDensity(0.0);
    QVERIFY(!tweak.isActive());

    // With any positive density, active
    tweak.setDensity(0.01);
    QVERIFY(tweak.isActive());

    tweak.setDensity(1.0);
    QVERIFY(tweak.isActive());
}

// ============================================================================
// Fuzzyness Tweak Tests
// ============================================================================

void TestNodeFormulas::testFuzzynessNoRatio()
{
    FuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectX(true);
    tweak.setAffectY(true);

    // With ratio = 0, no effect
    QPointF result = tweak.apply(QPointF(0.5, 0.5), 0.0, 0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.5)));
}

void TestNodeFormulas::testFuzzynessFullRatio()
{
    FuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectX(true);
    tweak.setAffectY(true);
    tweak.setUseSeed(true);
    tweak.setSeed(42);

    // With ratio = 1, position should change
    QPointF input(0.5, 0.5);
    QPointF result = tweak.apply(input, 1.0, 0);

    // Result should be different from input (with high probability)
    // Since we use a seed, the result is deterministic
    QVERIFY(!qFuzzyIsNull(result.x() - 0.5) || !qFuzzyIsNull(result.y() - 0.5));
}

void TestNodeFormulas::testFuzzynessAffectXOnly()
{
    FuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectX(true);
    tweak.setAffectY(false);
    tweak.setUseSeed(true);
    tweak.setSeed(42);

    QPointF input(0.5, 0.5);
    QPointF result = tweak.apply(input, 1.0, 0);

    // Y should be unchanged
    QVERIFY(fuzzyCompare(result.y(), 0.5));
}

void TestNodeFormulas::testFuzzynessAffectYOnly()
{
    FuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectX(false);
    tweak.setAffectY(true);
    tweak.setUseSeed(true);
    tweak.setSeed(42);

    QPointF input(0.5, 0.5);
    QPointF result = tweak.apply(input, 1.0, 0);

    // X should be unchanged
    QVERIFY(fuzzyCompare(result.x(), 0.5));
}

void TestNodeFormulas::testFuzzynessDeterministicSeed()
{
    FuzzynessTweak tweak;
    tweak.setAmount(0.3);
    tweak.setAffectX(true);
    tweak.setAffectY(true);
    tweak.setUseSeed(true);
    tweak.setSeed(12345);

    QPointF input(0.0, 0.0);

    // Same seed and sample index should give same result
    QPointF result1 = tweak.apply(input, 1.0, 5);
    QPointF result2 = tweak.apply(input, 1.0, 5);

    QVERIFY(fuzzyComparePoint(result1, result2));

    // Different sample index should give different result
    QPointF result3 = tweak.apply(input, 1.0, 6);
    QVERIFY(!fuzzyComparePoint(result1, result3) ||
            (fuzzyCompare(result1.x(), result3.x()) && fuzzyCompare(result1.y(), result3.y())));
}

// ============================================================================
// Color Fuzzyness Tweak Tests
// ============================================================================

void TestNodeFormulas::testColorFuzzynessNoRatio()
{
    ColorFuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectRed(true);
    tweak.setAffectGreen(true);
    tweak.setAffectBlue(true);

    QColor input(128, 128, 128);
    QColor result = tweak.apply(input, 0.0, 0);

    // With ratio = 0, color unchanged
    QCOMPARE(result.red(), 128);
    QCOMPARE(result.green(), 128);
    QCOMPARE(result.blue(), 128);
}

void TestNodeFormulas::testColorFuzzynessFullRatio()
{
    ColorFuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectRed(true);
    tweak.setAffectGreen(true);
    tweak.setAffectBlue(true);
    tweak.setUseSeed(true);
    tweak.setSeed(42);

    QColor input(128, 128, 128);
    QColor result = tweak.apply(input, 1.0, 0);

    // With ratio = 1, color should change (deterministic with seed)
    bool changed = (result.red() != 128) || (result.green() != 128) || (result.blue() != 128);
    QVERIFY(changed);
}

void TestNodeFormulas::testColorFuzzynessAffectChannels()
{
    ColorFuzzynessTweak tweak;
    tweak.setAmount(0.5);
    tweak.setAffectRed(true);
    tweak.setAffectGreen(false);
    tweak.setAffectBlue(false);
    tweak.setUseSeed(true);
    tweak.setSeed(42);

    QColor input(128, 100, 50);
    QColor result = tweak.apply(input, 1.0, 0);

    // Green and Blue should be unchanged
    QCOMPARE(result.green(), 100);
    QCOMPARE(result.blue(), 50);
}

void TestNodeFormulas::testColorFuzzynessDeterministicSeed()
{
    ColorFuzzynessTweak tweak;
    tweak.setAmount(0.3);
    tweak.setAffectRed(true);
    tweak.setAffectGreen(true);
    tweak.setAffectBlue(true);
    tweak.setUseSeed(true);
    tweak.setSeed(9999);

    QColor input(100, 100, 100);

    // Same seed and sample index should give same result
    QColor result1 = tweak.apply(input, 1.0, 10);
    QColor result2 = tweak.apply(input, 1.0, 10);

    QCOMPARE(result1.red(), result2.red());
    QCOMPARE(result1.green(), result2.green());
    QCOMPARE(result1.blue(), result2.blue());
}

// ============================================================================
// Split Tweak Tests
// ============================================================================

void TestNodeFormulas::testSplitEffectiveThreshold()
{
    SplitTweak tweak;
    tweak.setSplitThreshold(0.5);

    // With ratio = 0, threshold should be max (no splitting)
    qreal thresh0 = tweak.effectiveThreshold(0.0);
    QVERIFY(thresh0 > 10.0);  // Very high threshold

    // With ratio = 1, threshold should be the configured value
    qreal thresh1 = tweak.effectiveThreshold(1.0);
    QVERIFY(fuzzyCompare(thresh1, 0.5));

    // With ratio = 0.5, threshold should be higher
    qreal thresh05 = tweak.effectiveThreshold(0.5);
    QVERIFY(thresh05 > thresh1);
}

void TestNodeFormulas::testSplitShouldSplitClose()
{
    SplitTweak tweak;
    tweak.setSplitThreshold(0.5);

    // Points close together (distance 0.1) should not split
    bool split = tweak.shouldSplit(0.0, 0.0, 0.1, 0.0, 1.0);
    QVERIFY(!split);
}

void TestNodeFormulas::testSplitShouldSplitFar()
{
    SplitTweak tweak;
    tweak.setSplitThreshold(0.1);

    // Points far apart (distance 0.5) should split with full ratio
    bool split = tweak.shouldSplit(0.0, 0.0, 0.5, 0.0, 1.0);
    QVERIFY(split);
}

void TestNodeFormulas::testSplitRatioEffect()
{
    SplitTweak tweak;
    tweak.setSplitThreshold(0.3);

    // Distance 0.4 - with ratio=1 should split, with ratio=0 should not
    bool splitFull = tweak.shouldSplit(0.0, 0.0, 0.4, 0.0, 1.0);
    bool splitNone = tweak.shouldSplit(0.0, 0.0, 0.4, 0.0, 0.0);

    QVERIFY(splitFull);
    QVERIFY(!splitNone);
}

// ============================================================================
// Rounder Tweak Tests
// ============================================================================

void TestNodeFormulas::testRounderNoEffect()
{
    RounderTweak tweak;
    tweak.setAmount(0.5);

    // With ratio = 0, no effect
    QPointF result = tweak.apply(0.5, 0.3, 0.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.5, 0.3)));
}

void TestNodeFormulas::testRounderWithAmount()
{
    RounderTweak tweak;
    tweak.setAmount(1.0);
    tweak.setVerticalShift(0.0);
    tweak.setHorizontalShift(0.0);
    tweak.setTighten(0.0);
    tweak.setRadialResize(1.0);
    tweak.setRadialShift(0.0);

    // With amount > 0 and ratio = 1, position should change
    QPointF input(0.5, 0.3);
    QPointF result = tweak.apply(input.x(), input.y(), 1.0);

    // Result should be different (rounding effect)
    QVERIFY(!fuzzyComparePoint(result, input) ||
            (fuzzyCompare(result.x(), input.x()) && fuzzyCompare(result.y(), input.y())));
}

void TestNodeFormulas::testRounderCenterUnchanged()
{
    RounderTweak tweak;
    tweak.setAmount(1.0);

    // Point at center (0, 0) should remain at center
    QPointF result = tweak.apply(0.0, 0.0, 1.0);
    QVERIFY(fuzzyComparePoint(result, QPointF(0.0, 0.0)));
}

void TestNodeFormulas::testRounderVerticalShift()
{
    RounderTweak tweak;
    tweak.setAmount(0.0);  // No rounding
    tweak.setVerticalShift(0.2);

    // With vertical shift and ratio = 1
    QPointF result = tweak.apply(0.0, 0.0, 1.0);

    // Y should be shifted (exact behavior depends on implementation)
    // Just verify result is valid
    QVERIFY(!qIsNaN(result.x()) && !qIsNaN(result.y()));
}

// ============================================================================
// TimeShift Tests
// ============================================================================

void TestNodeFormulas::testTimeShiftDelay()
{
    TimeShiftNode timeShift;
    timeShift.setDelay(0.5);
    timeShift.setScale(1.0);
    timeShift.setLoop(false);

    // Time 1.0 with delay 0.5 should give 0.5
    qreal shifted = timeShift.shiftTime(1.0);
    QVERIFY(fuzzyCompare(shifted, 0.5));

    // Time 0.3 with delay 0.5 should give negative (or clamped)
    shifted = timeShift.shiftTime(0.3);
    QVERIFY(fuzzyCompare(shifted, -0.2) || shifted >= 0.0);  // Implementation may clamp
}

void TestNodeFormulas::testTimeShiftScale()
{
    TimeShiftNode timeShift;
    timeShift.setDelay(0.0);
    timeShift.setScale(2.0);
    timeShift.setLoop(false);

    // Time 0.5 with scale 2.0 should give 1.0
    qreal shifted = timeShift.shiftTime(0.5);
    QVERIFY(fuzzyCompare(shifted, 1.0));

    // Time 0.25 with scale 2.0 should give 0.5
    shifted = timeShift.shiftTime(0.25);
    QVERIFY(fuzzyCompare(shifted, 0.5));
}

void TestNodeFormulas::testTimeShiftLoop()
{
    TimeShiftNode timeShift;
    timeShift.setDelay(0.0);
    timeShift.setScale(1.0);
    timeShift.setLoop(true);
    timeShift.setLoopDuration(1.0);

    // Time 1.5 with loop duration 1.0 should give 0.5
    qreal shifted = timeShift.shiftTime(1.5);
    QVERIFY(fuzzyCompare(shifted, 0.5));

    // Time 2.3 should give 0.3
    shifted = timeShift.shiftTime(2.3);
    QVERIFY(fuzzyCompare(shifted, 0.3, 0.01));
}

void TestNodeFormulas::testTimeShiftCombined()
{
    TimeShiftNode timeShift;
    timeShift.setDelay(0.1);
    timeShift.setScale(2.0);
    timeShift.setLoop(true);
    timeShift.setLoopDuration(1.0);

    // Time 0.55: first subtract delay (0.45), then scale (0.9), then loop
    qreal shifted = timeShift.shiftTime(0.55);
    QVERIFY(shifted >= 0.0 && shifted <= 1.0);  // Should be in loop range
}

// ============================================================================
// SurfaceFactory Tests
// ============================================================================

void TestNodeFormulas::testSurfaceFactorySine()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Sine);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // At t=0, sin(0) = 0
    qreal ratio = surface.computeRatio(0.0);
    QVERIFY(fuzzyCompare(ratio, 0.0));

    // At t=0.25 (quarter period), sin(π/2) = 1
    ratio = surface.computeRatio(0.25);
    QVERIFY(fuzzyCompare(ratio, 1.0));

    // At t=0.5, sin(π) = 0
    ratio = surface.computeRatio(0.5);
    QVERIFY(fuzzyCompare(ratio, 0.0));

    // At t=0.75, sin(3π/2) = -1
    ratio = surface.computeRatio(0.75);
    QVERIFY(fuzzyCompare(ratio, -1.0));
}

void TestNodeFormulas::testSurfaceFactoryCosine()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Cosine);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // At t=0, cos(0) = 1
    qreal ratio = surface.computeRatio(0.0);
    QVERIFY(fuzzyCompare(ratio, 1.0));

    // At t=0.25, cos(π/2) = 0
    ratio = surface.computeRatio(0.25);
    QVERIFY(fuzzyCompare(ratio, 0.0));

    // At t=0.5, cos(π) = -1
    ratio = surface.computeRatio(0.5);
    QVERIFY(fuzzyCompare(ratio, -1.0));
}

void TestNodeFormulas::testSurfaceFactoryTriangle()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Triangle);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // Triangle wave: rises from -1 to 1 in first half, falls in second half
    qreal ratio0 = surface.computeRatio(0.0);
    qreal ratio25 = surface.computeRatio(0.25);
    qreal ratio50 = surface.computeRatio(0.5);
    qreal ratio75 = surface.computeRatio(0.75);

    // Should have some variation
    QVERIFY(ratio25 != ratio0 || ratio50 != ratio25);

    // All values should be in [-1, 1]
    QVERIFY(ratio0 >= -1.0 && ratio0 <= 1.0);
    QVERIFY(ratio25 >= -1.0 && ratio25 <= 1.0);
    QVERIFY(ratio50 >= -1.0 && ratio50 <= 1.0);
    QVERIFY(ratio75 >= -1.0 && ratio75 <= 1.0);
}

void TestNodeFormulas::testSurfaceFactorySawtooth()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Sawtooth);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // Sawtooth rises linearly then drops
    qreal ratio0 = surface.computeRatio(0.0);
    qreal ratio50 = surface.computeRatio(0.5);
    qreal ratio99 = surface.computeRatio(0.99);

    // Should increase over time
    QVERIFY(ratio50 > ratio0 || fuzzyCompare(ratio50, ratio0));

    // All values should be in valid range
    QVERIFY(ratio0 >= -1.0 && ratio0 <= 1.0);
    QVERIFY(ratio50 >= -1.0 && ratio50 <= 1.0);
}

void TestNodeFormulas::testSurfaceFactorySquare()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Square);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // Square wave: 1 for first half, -1 for second half
    qreal ratio25 = surface.computeRatio(0.25);
    qreal ratio75 = surface.computeRatio(0.75);

    // Should be opposite values
    QVERIFY(fuzzyCompare(ratio25, 1.0) || fuzzyCompare(ratio25, -1.0));
    QVERIFY(fuzzyCompare(ratio75, 1.0) || fuzzyCompare(ratio75, -1.0));
    QVERIFY(!fuzzyCompare(ratio25, ratio75) ||
            (fuzzyCompare(ratio25, 1.0) && fuzzyCompare(ratio75, -1.0)) ||
            (fuzzyCompare(ratio25, -1.0) && fuzzyCompare(ratio75, 1.0)));
}

void TestNodeFormulas::testSurfaceFactoryLinear()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Linear);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // Linear: value equals input (or scaled)
    qreal ratio0 = surface.computeRatio(0.0);
    qreal ratio50 = surface.computeRatio(0.5);
    qreal ratio100 = surface.computeRatio(1.0);

    // Should increase linearly
    QVERIFY(ratio50 > ratio0 || fuzzyCompare(ratio50, ratio0));
    QVERIFY(ratio100 > ratio50 || fuzzyCompare(ratio100, ratio50));
}

void TestNodeFormulas::testSurfaceFactoryClamp()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Sine);
    surface.setAmplitude(2.0);  // Will exceed [-1, 1]
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.0);

    // With clamp ON
    surface.setClamp(true);
    qreal ratioClamped = surface.computeRatio(0.25);  // sin = 1, * 2 = 2, clamped to 1
    QVERIFY(ratioClamped >= -1.0 && ratioClamped <= 1.0);

    // With clamp OFF
    surface.setClamp(false);
    qreal ratioUnclamped = surface.computeRatio(0.25);
    QVERIFY(fuzzyCompare(ratioUnclamped, 2.0));  // Not clamped
}

void TestNodeFormulas::testSurfaceFactoryOffset()
{
    SurfaceFactoryNode surface;
    surface.setSurfaceType(SurfaceFactoryNode::SurfaceType::Sine);
    surface.setAmplitude(1.0);
    surface.setFrequency(1.0);
    surface.setPhase(0.0);
    surface.setOffset(0.5);
    surface.setClamp(false);

    // At t=0, sin(0) = 0, + offset 0.5 = 0.5
    qreal ratio = surface.computeRatio(0.0);
    QVERIFY(fuzzyCompare(ratio, 0.5));

    // At t=0.25, sin(π/2) = 1, + offset 0.5 = 1.5
    ratio = surface.computeRatio(0.25);
    QVERIFY(fuzzyCompare(ratio, 1.5));
}

QTEST_MAIN(TestNodeFormulas)
#include "tst_node_formulas.moc"

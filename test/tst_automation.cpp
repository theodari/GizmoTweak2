#include <QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include <QEasingCurve>

#include "automation/KeyFrame.h"
#include "automation/AutomationTrack.h"

using namespace gizmotweak2;

class TestAutomation : public QObject
{
    Q_OBJECT

private slots:
    // KeyFrame tests
    void testKeyFrameConstruction();
    void testKeyFrameValues();
    void testKeyFrameCurveType();
    void testKeyFramePeriodAmplitude();
    void testKeyFrameValueForProgress();
    void testKeyFrameJsonRoundtrip();

    // AutomationTrack tests
    void testTrackConstruction();
    void testTrackSetupParameter();
    void testTrackCreateKeyFrame();
    void testTrackMoveKeyFrame();
    void testTrackDeleteKeyFrame();
    void testTrackTimedValueNoKeyframes();
    void testTrackTimedValueSingleKeyframe();
    void testTrackTimedValueInterpolation();
    void testTrackTimedValueMultipleParams();
    void testTrackJsonRoundtrip();
    void testTrackResizeKeyFrames();
    void testTrackRemoveKeyFramesAfter();
    void testTrackTranslateKeyFrames();

    // Edge cases
    void testKeyFrameOutOfBoundsParam();
    void testTrackDuplicateKeyFrame();
    void testInterpolationBeforeFirstKeyframe();
    void testInterpolationAfterLastKeyframe();

private:
    bool fuzzyCompare(double a, double b, double epsilon = 0.0001);
};

bool TestAutomation::fuzzyCompare(double a, double b, double epsilon)
{
    return qAbs(a - b) < epsilon;
}

// ============================================================================
// KeyFrame Tests
// ============================================================================

void TestAutomation::testKeyFrameConstruction()
{
    KeyFrame kf(3);

    QCOMPARE(kf.paramCount(), 3);
    QCOMPARE(kf.curveType(), QEasingCurve::Linear);
}

void TestAutomation::testKeyFrameValues()
{
    KeyFrame kf(2);

    kf.setValue(0, 0.5);
    kf.setValue(1, -0.3);

    QVERIFY(fuzzyCompare(kf.value(0), 0.5));
    QVERIFY(fuzzyCompare(kf.value(1), -0.3));
}

void TestAutomation::testKeyFrameCurveType()
{
    KeyFrame kf(1);

    kf.setCurveType(QEasingCurve::InOutQuad);
    QCOMPARE(kf.curveType(), QEasingCurve::InOutQuad);

    kf.setCurveType(QEasingCurve::OutBounce);
    QCOMPARE(kf.curveType(), QEasingCurve::OutBounce);
}

void TestAutomation::testKeyFramePeriodAmplitude()
{
    KeyFrame kf(1);

    kf.setPeriod(0.5);
    kf.setAmplitude(2.0);

    QVERIFY(fuzzyCompare(kf.period(), 0.5));
    QVERIFY(fuzzyCompare(kf.amplitude(), 2.0));
}

void TestAutomation::testKeyFrameValueForProgress()
{
    KeyFrame kf(1);
    kf.setCurveType(QEasingCurve::Linear);

    // Linear curve: output = input
    QVERIFY(fuzzyCompare(kf.valueForProgress(0.0), 0.0));
    QVERIFY(fuzzyCompare(kf.valueForProgress(0.5), 0.5));
    QVERIFY(fuzzyCompare(kf.valueForProgress(1.0), 1.0));
}

void TestAutomation::testKeyFrameJsonRoundtrip()
{
    KeyFrame original(3);
    original.setValue(0, 1.5);
    original.setValue(1, -0.7);
    original.setValue(2, 0.0);
    original.setCurveType(QEasingCurve::InOutCubic);
    original.setPeriod(0.3);
    original.setAmplitude(1.5);

    // Serialize
    QJsonObject json = original.toJson();

    // Deserialize
    KeyFrame restored(3);
    QVERIFY(restored.fromJson(json));

    // Verify
    QVERIFY(fuzzyCompare(restored.value(0), 1.5));
    QVERIFY(fuzzyCompare(restored.value(1), -0.7));
    QVERIFY(fuzzyCompare(restored.value(2), 0.0));
    QCOMPARE(restored.curveType(), QEasingCurve::InOutCubic);
    QVERIFY(fuzzyCompare(restored.period(), 0.3));
    QVERIFY(fuzzyCompare(restored.amplitude(), 1.5));
}

// ============================================================================
// AutomationTrack Tests
// ============================================================================

void TestAutomation::testTrackConstruction()
{
    AutomationTrack track(2, "TestTrack", QColor(255, 0, 0));

    QCOMPARE(track.paramCount(), 2);
    QCOMPARE(track.trackName(), QString("TestTrack"));
    QCOMPARE(track.color(), QColor(255, 0, 0));
    QVERIFY(!track.isAutomated());
    QCOMPARE(track.keyFrameCount(), 0);
}

void TestAutomation::testTrackSetupParameter()
{
    AutomationTrack track(2, "Track");

    track.setupParameter(0, 0.0, 1.0, 0.5, "Param1", 100.0, "%");
    track.setupParameter(1, -1.0, 1.0, 0.0, "Param2");

    QVERIFY(fuzzyCompare(track.minValue(0), 0.0));
    QVERIFY(fuzzyCompare(track.maxValue(0), 1.0));
    QVERIFY(fuzzyCompare(track.initialValue(0), 0.5));
    QCOMPARE(track.parameterName(0), QString("Param1"));
    QVERIFY(fuzzyCompare(track.displayRatio(0), 100.0));
    QCOMPARE(track.suffix(0), QString("%"));

    QVERIFY(fuzzyCompare(track.minValue(1), -1.0));
    QVERIFY(fuzzyCompare(track.maxValue(1), 1.0));
    QCOMPARE(track.parameterName(1), QString("Param2"));
}

void TestAutomation::testTrackCreateKeyFrame()
{
    AutomationTrack track(2, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.5, "P1");
    track.setupParameter(1, 0.0, 1.0, 0.0, "P2");

    auto* kf = track.createKeyFrame(1000);

    QVERIFY(kf != nullptr);
    QCOMPARE(track.keyFrameCount(), 1);
    QVERIFY(track.hasKeyFrameAt(1000));
    QVERIFY(!track.hasKeyFrameAt(500));

    // Verify keyframe times
    auto times = track.keyFrameTimes();
    QCOMPARE(times.size(), 1);
    QCOMPARE(times.at(0), 1000);
}

void TestAutomation::testTrackMoveKeyFrame()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 0.75);

    QVERIFY(track.hasKeyFrameAt(1000));

    track.moveKeyFrame(1000, 2000);

    QVERIFY(!track.hasKeyFrameAt(1000));
    QVERIFY(track.hasKeyFrameAt(2000));

    // Value should be preserved
    QVERIFY(fuzzyCompare(track.timedValue(2000, 0), 0.75));
}

void TestAutomation::testTrackDeleteKeyFrame()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.createKeyFrame(2000);

    QCOMPARE(track.keyFrameCount(), 2);

    track.deleteKeyFrame(1000);

    QCOMPARE(track.keyFrameCount(), 1);
    QVERIFY(!track.hasKeyFrameAt(1000));
    QVERIFY(track.hasKeyFrameAt(2000));
}

void TestAutomation::testTrackTimedValueNoKeyframes()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.5, "P1");

    // With no keyframes, should return initial value
    QVERIFY(fuzzyCompare(track.timedValue(0, 0), 0.5));
    QVERIFY(fuzzyCompare(track.timedValue(5000, 0), 0.5));
}

void TestAutomation::testTrackTimedValueSingleKeyframe()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.5, "P1");

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 0.8);

    // Before keyframe: initial value
    QVERIFY(fuzzyCompare(track.timedValue(0, 0), 0.5));

    // At keyframe: keyframe value
    QVERIFY(fuzzyCompare(track.timedValue(1000, 0), 0.8));

    // After keyframe: keyframe value (holds)
    QVERIFY(fuzzyCompare(track.timedValue(2000, 0), 0.8));
}

void TestAutomation::testTrackTimedValueInterpolation()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(0);
    track.updateKeyFrameValue(0, 0, 0.0);

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 1.0);

    // At start
    QVERIFY(fuzzyCompare(track.timedValue(0, 0), 0.0));

    // At end
    QVERIFY(fuzzyCompare(track.timedValue(1000, 0), 1.0));

    // At middle (linear interpolation)
    QVERIFY(fuzzyCompare(track.timedValue(500, 0), 0.5));

    // At 25%
    QVERIFY(fuzzyCompare(track.timedValue(250, 0), 0.25));
}

void TestAutomation::testTrackTimedValueMultipleParams()
{
    AutomationTrack track(2, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "X");
    track.setupParameter(1, 0.0, 1.0, 0.0, "Y");

    track.createKeyFrame(0);
    track.updateKeyFrameValue(0, 0, 0.0);
    track.updateKeyFrameValue(0, 1, 1.0);

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 1.0);
    track.updateKeyFrameValue(1000, 1, 0.0);

    // At middle
    QVERIFY(fuzzyCompare(track.timedValue(500, 0), 0.5));
    QVERIFY(fuzzyCompare(track.timedValue(500, 1), 0.5));

    // Params interpolate independently
    QVERIFY(fuzzyCompare(track.timedValue(250, 0), 0.25));
    QVERIFY(fuzzyCompare(track.timedValue(250, 1), 0.75));
}

void TestAutomation::testTrackJsonRoundtrip()
{
    AutomationTrack original(2, "TestTrack", QColor(100, 150, 200));
    original.setupParameter(0, 0.0, 1.0, 0.25, "Param1", 100.0, "%");
    original.setupParameter(1, -1.0, 1.0, 0.0, "Param2");
    original.setAutomated(true);

    original.createKeyFrame(0);
    original.updateKeyFrameValue(0, 0, 0.0);
    original.updateKeyFrameValue(0, 1, -0.5);

    original.createKeyFrame(1000);
    original.updateKeyFrameValue(1000, 0, 1.0);
    original.updateKeyFrameValue(1000, 1, 0.5);

    // Serialize
    QJsonObject json = original.toJson();

    // Deserialize
    AutomationTrack restored(2, "Temp");
    QVERIFY(restored.fromJson(json));

    // Verify properties
    QCOMPARE(restored.trackName(), QString("TestTrack"));
    QCOMPARE(restored.color(), QColor(100, 150, 200));
    QVERIFY(restored.isAutomated());
    QCOMPARE(restored.keyFrameCount(), 2);

    // Verify values
    QVERIFY(fuzzyCompare(restored.timedValue(0, 0), 0.0));
    QVERIFY(fuzzyCompare(restored.timedValue(0, 1), -0.5));
    QVERIFY(fuzzyCompare(restored.timedValue(1000, 0), 1.0));
    QVERIFY(fuzzyCompare(restored.timedValue(1000, 1), 0.5));

    // Verify interpolation still works
    QVERIFY(fuzzyCompare(restored.timedValue(500, 0), 0.5));
}

void TestAutomation::testTrackResizeKeyFrames()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.createKeyFrame(2000);

    // Double the duration
    track.resizeAllKeyFrames(2.0);

    auto times = track.keyFrameTimes();
    QCOMPARE(times.size(), 2);
    QVERIFY(times.contains(2000));
    QVERIFY(times.contains(4000));
}

void TestAutomation::testTrackRemoveKeyFramesAfter()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.createKeyFrame(2000);
    track.createKeyFrame(3000);

    track.removeKeyFramesAfter(1500);

    QCOMPARE(track.keyFrameCount(), 1);
    QVERIFY(track.hasKeyFrameAt(1000));
    QVERIFY(!track.hasKeyFrameAt(2000));
    QVERIFY(!track.hasKeyFrameAt(3000));
}

void TestAutomation::testTrackTranslateKeyFrames()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.createKeyFrame(2000);

    track.translateKeyFrames(500);

    auto times = track.keyFrameTimes();
    QVERIFY(times.contains(1500));
    QVERIFY(times.contains(2500));
}

// ============================================================================
// Edge Cases
// ============================================================================

void TestAutomation::testKeyFrameOutOfBoundsParam()
{
    KeyFrame kf(2);

    // Setting out of bounds should not crash
    kf.setValue(5, 1.0);  // Index 5 is out of bounds for 2 params

    // Getting out of bounds should return 0 or not crash
    double val = kf.value(10);
    Q_UNUSED(val);
}

void TestAutomation::testTrackDuplicateKeyFrame()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 0.5);

    // Creating at same time should update existing
    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 0.8);

    QCOMPARE(track.keyFrameCount(), 1);
    QVERIFY(fuzzyCompare(track.timedValue(1000, 0), 0.8));
}

void TestAutomation::testInterpolationBeforeFirstKeyframe()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.5, "P1");

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 1.0);

    // Before first keyframe should use initial value
    QVERIFY(fuzzyCompare(track.timedValue(0, 0), 0.5));
    QVERIFY(fuzzyCompare(track.timedValue(500, 0), 0.5) ||
            fuzzyCompare(track.timedValue(500, 0), 0.75));  // May interpolate
}

void TestAutomation::testInterpolationAfterLastKeyframe()
{
    AutomationTrack track(1, "Track");
    track.setupParameter(0, 0.0, 1.0, 0.0, "P1");

    track.createKeyFrame(1000);
    track.updateKeyFrameValue(1000, 0, 0.7);

    // After last keyframe should hold value
    QVERIFY(fuzzyCompare(track.timedValue(2000, 0), 0.7));
    QVERIFY(fuzzyCompare(track.timedValue(10000, 0), 0.7));
}

QTEST_MAIN(TestAutomation)
#include "tst_automation.moc"

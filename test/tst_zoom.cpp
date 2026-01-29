/**
 * @file tst_zoom.cpp
 * @brief Unit tests for zoom-to-cursor functionality
 *
 * Tests the mathematical properties of zoom operations:
 * - Point under cursor remains fixed after zoom
 * - Zoom bounds are respected
 * - Multiple zoom operations compose correctly
 */

#include <QTest>
#include <cmath>

/**
 * @class ZoomCalculator
 * @brief Encapsulates the zoom math from NodeCanvas.qml for testing
 *
 * This mirrors the zoom logic in NodeCanvas.qml WheelHandler
 */
class ZoomCalculator
{
public:
    // Viewport dimensions
    double viewportWidth = 800.0;
    double viewportHeight = 600.0;

    // Content dimensions (unscaled)
    double canvasWidth = 2400.0;
    double canvasHeight = 1600.0;

    // Current state
    double contentX = 0.0;
    double contentY = 0.0;
    double zoomScale = 1.0;

    // Zoom limits
    double minZoom = 0.5;
    double maxZoom = 2.0;

    // Zoom factor per wheel step (~3% as in NodeCanvas.qml)
    double zoomFactor = 1.033;

    /**
     * Get content coordinates from viewport coordinates
     */
    double toContentX(double viewportX) const
    {
        return (viewportX + contentX) / zoomScale;
    }

    double toContentY(double viewportY) const
    {
        return (viewportY + contentY) / zoomScale;
    }

    /**
     * Get viewport coordinates from content coordinates
     */
    double toViewportX(double contentXPos) const
    {
        return contentXPos * zoomScale - contentX;
    }

    double toViewportY(double contentYPos) const
    {
        return contentYPos * zoomScale - contentY;
    }

    /**
     * Perform zoom towards a point in viewport coordinates
     * @param viewportX Mouse X in viewport coordinates
     * @param viewportY Mouse Y in viewport coordinates
     * @param zoomIn true for zoom in, false for zoom out
     * @return true if zoom was applied, false if at limit
     */
    bool zoom(double viewportX, double viewportY, bool zoomIn)
    {
        double oldScale = zoomScale;

        if (zoomIn)
        {
            zoomScale = std::min(maxZoom, zoomScale * zoomFactor);
        }
        else
        {
            zoomScale = std::max(minZoom, zoomScale / zoomFactor);
        }

        if (oldScale == zoomScale)
        {
            return false; // At limit
        }

        // Calculate mouse position in content coordinates (before zoom)
        double mouseX = viewportX + contentX;
        double mouseY = viewportY + contentY;

        // Adjust content position to keep point under cursor
        double factor = zoomScale / oldScale;
        double newContentX = mouseX * factor - viewportX;
        double newContentY = mouseY * factor - viewportY;

        // Clamp to valid Flickable bounds
        double maxContentX = canvasWidth * zoomScale - viewportWidth;
        double maxContentY = canvasHeight * zoomScale - viewportHeight;
        contentX = std::max(0.0, std::min(maxContentX, newContentX));
        contentY = std::max(0.0, std::min(maxContentY, newContentY));

        return true;
    }

    /**
     * Reset to default state
     */
    void reset()
    {
        contentX = 0.0;
        contentY = 0.0;
        zoomScale = 1.0;
    }
};

class TestZoom : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        calc.reset();
    }

    /**
     * Test that a point under the cursor remains under the cursor after zoom in
     */
    void testZoomInKeepsPointUnderCursor()
    {
        // Click in center of viewport
        double mouseX = calc.viewportWidth / 2;
        double mouseY = calc.viewportHeight / 2;

        // Get content point under cursor before zoom
        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        // Zoom in
        calc.zoom(mouseX, mouseY, true);

        // Check that the same content point is still under the cursor
        double newViewportX = calc.toViewportX(contentPointX);
        double newViewportY = calc.toViewportY(contentPointY);

        QCOMPARE(newViewportX, mouseX);
        QCOMPARE(newViewportY, mouseY);
    }

    /**
     * Test that a point under the cursor remains under the cursor after zoom out
     */
    void testZoomOutKeepsPointUnderCursor()
    {
        // First zoom in and scroll to center so we have room to zoom out
        // without hitting the edge bounds
        calc.zoomScale = 1.5;
        // Scroll to center of content so clamping doesn't interfere
        calc.contentX = (calc.canvasWidth * calc.zoomScale - calc.viewportWidth) / 2;
        calc.contentY = (calc.canvasHeight * calc.zoomScale - calc.viewportHeight) / 2;

        double mouseX = calc.viewportWidth / 2;
        double mouseY = calc.viewportHeight / 2;

        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        calc.zoom(mouseX, mouseY, false);

        double newViewportX = calc.toViewportX(contentPointX);
        double newViewportY = calc.toViewportY(contentPointY);

        QVERIFY(qFuzzyCompare(newViewportX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(newViewportY + 1, mouseY + 1));
    }

    /**
     * Test zoom at corner positions
     */
    void testZoomAtCorners_data()
    {
        QTest::addColumn<double>("mouseX");
        QTest::addColumn<double>("mouseY");

        QTest::newRow("top-left") << 0.0 << 0.0;
        QTest::newRow("top-right") << 800.0 << 0.0;
        QTest::newRow("bottom-left") << 0.0 << 600.0;
        QTest::newRow("bottom-right") << 800.0 << 600.0;
    }

    void testZoomAtCorners()
    {
        QFETCH(double, mouseX);
        QFETCH(double, mouseY);

        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        calc.zoom(mouseX, mouseY, true);

        double newViewportX = calc.toViewportX(contentPointX);
        double newViewportY = calc.toViewportY(contentPointY);

        // Use fuzzy compare due to floating point
        QVERIFY(qFuzzyCompare(newViewportX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(newViewportY + 1, mouseY + 1));
    }

    /**
     * Test that zoom respects minimum bound
     */
    void testMinZoomBound()
    {
        calc.zoomScale = calc.minZoom;

        bool zoomed = calc.zoom(400, 300, false);

        QVERIFY(!zoomed);
        QCOMPARE(calc.zoomScale, calc.minZoom);
    }

    /**
     * Test that zoom respects maximum bound
     */
    void testMaxZoomBound()
    {
        calc.zoomScale = calc.maxZoom;

        bool zoomed = calc.zoom(400, 300, true);

        QVERIFY(!zoomed);
        QCOMPARE(calc.zoomScale, calc.maxZoom);
    }

    /**
     * Test multiple consecutive zoom operations
     */
    void testMultipleZooms()
    {
        double mouseX = 400;
        double mouseY = 300;

        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        // Zoom in 5 times
        for (int i = 0; i < 5; ++i)
        {
            calc.zoom(mouseX, mouseY, true);
        }

        // Point should still be under cursor
        double afterZoomInX = calc.toViewportX(contentPointX);
        double afterZoomInY = calc.toViewportY(contentPointY);

        QVERIFY(qFuzzyCompare(afterZoomInX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(afterZoomInY + 1, mouseY + 1));

        // Zoom out 5 times
        for (int i = 0; i < 5; ++i)
        {
            calc.zoom(mouseX, mouseY, false);
        }

        // Point should still be under cursor
        double afterZoomOutX = calc.toViewportX(contentPointX);
        double afterZoomOutY = calc.toViewportY(contentPointY);

        QVERIFY(qFuzzyCompare(afterZoomOutX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(afterZoomOutY + 1, mouseY + 1));
    }

    /**
     * Test zoom from scrolled position
     */
    void testZoomFromScrolledPosition()
    {
        // Scroll to a position
        calc.contentX = 200;
        calc.contentY = 150;

        double mouseX = 400;
        double mouseY = 300;

        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        calc.zoom(mouseX, mouseY, true);

        double newViewportX = calc.toViewportX(contentPointX);
        double newViewportY = calc.toViewportY(contentPointY);

        QVERIFY(qFuzzyCompare(newViewportX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(newViewportY + 1, mouseY + 1));
    }

    /**
     * Test that zoom factor is approximately 3.3% per step
     */
    void testZoomFactorValue()
    {
        double initial = calc.zoomScale;

        calc.zoom(400, 300, true);

        double percentChange = (calc.zoomScale - initial) / initial * 100;

        // Should be approximately 3.3%
        QVERIFY(percentChange > 3.0 && percentChange < 4.0);
    }

    /**
     * Test zoom with different initial scales
     */
    void testZoomAtDifferentScales_data()
    {
        QTest::addColumn<double>("initialScale");

        QTest::newRow("scale 0.5") << 0.5;
        QTest::newRow("scale 0.75") << 0.75;
        QTest::newRow("scale 1.0") << 1.0;
        QTest::newRow("scale 1.5") << 1.5;
        QTest::newRow("scale 1.9") << 1.9;
    }

    void testZoomAtDifferentScales()
    {
        QFETCH(double, initialScale);

        calc.zoomScale = initialScale;

        double mouseX = 400;
        double mouseY = 300;

        double contentPointX = calc.toContentX(mouseX);
        double contentPointY = calc.toContentY(mouseY);

        calc.zoom(mouseX, mouseY, true);

        double newViewportX = calc.toViewportX(contentPointX);
        double newViewportY = calc.toViewportY(contentPointY);

        QVERIFY(qFuzzyCompare(newViewportX + 1, mouseX + 1));
        QVERIFY(qFuzzyCompare(newViewportY + 1, mouseY + 1));
    }

    /**
     * Test that contentX/contentY are clamped to valid bounds when zooming at top-left
     */
    void testZoomClampingAtTopLeft()
    {
        // Position at top-left corner
        calc.contentX = 0;
        calc.contentY = 0;

        // Zoom out at top-left - this would make contentX/contentY negative without clamping
        calc.zoom(0, 0, false);

        // Should be clamped to 0
        QVERIFY(calc.contentX >= 0);
        QVERIFY(calc.contentY >= 0);
    }

    /**
     * Test that contentX/contentY are clamped when zooming at bottom-right edge
     */
    void testZoomClampingAtBottomRight()
    {
        // Position at max scroll (bottom-right)
        calc.contentX = calc.canvasWidth * calc.zoomScale - calc.viewportWidth;
        calc.contentY = calc.canvasHeight * calc.zoomScale - calc.viewportHeight;

        // Zoom in at bottom-right
        calc.zoom(calc.viewportWidth, calc.viewportHeight, true);

        // Should not exceed max bounds
        double maxX = calc.canvasWidth * calc.zoomScale - calc.viewportWidth;
        double maxY = calc.canvasHeight * calc.zoomScale - calc.viewportHeight;
        QVERIFY(calc.contentX <= maxX + 0.001);  // Small epsilon for floating point
        QVERIFY(calc.contentY <= maxY + 0.001);
    }

    /**
     * Test rapid zoom out at corner doesn't cause drift
     */
    void testRapidZoomOutAtCorner()
    {
        // Start at some scrolled position
        calc.contentX = 400;
        calc.contentY = 300;

        // Rapidly zoom out 20 times at top-left corner
        for (int i = 0; i < 20; ++i)
        {
            calc.zoom(0, 0, false);
        }

        // Content position should never go negative
        QVERIFY(calc.contentX >= 0);
        QVERIFY(calc.contentY >= 0);
    }

private:
    ZoomCalculator calc;
};

QTEST_MAIN(TestZoom)
#include "tst_zoom.moc"

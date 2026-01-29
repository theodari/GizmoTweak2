#pragma once

#include "core/Node.h"
#include <QColor>
#include <QRandomGenerator>
#include <QtQml/qqmlregistration.h>
#include <frame.h>
#include <functional>

namespace gizmotweak2
{

/**
 * @brief Sparkle effect - inserts bright points between samples
 *
 * This effect replicates GizmoTweak's sparkle behavior exactly:
 * - Randomly decides if a sparkle should appear between two samples
 * - Inserts transition samples + sparkle point (repeated 5x) + transition samples
 * - Maintains minimum distance between sparkles
 */
class SparkleTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal density READ density WRITE setDensity NOTIFY densityChanged)
    Q_PROPERTY(qreal red READ red WRITE setRed NOTIFY redChanged)
    Q_PROPERTY(qreal green READ green WRITE setGreen NOTIFY greenChanged)
    Q_PROPERTY(qreal blue READ blue WRITE setBlue NOTIFY blueChanged)
    Q_PROPERTY(qreal alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit SparkleTweak(QObject* parent = nullptr);
    ~SparkleTweak() override = default;

    QString type() const override { return QStringLiteral("SparkleTweak"); }
    Category category() const override { return Category::Tweak; }

    // Density - probability that a point sparkles (0-1)
    qreal density() const { return _density; }
    void setDensity(qreal d);

    // Sparkle color RGB (0-1 each)
    qreal red() const { return _red; }
    void setRed(qreal r);

    qreal green() const { return _green; }
    void setGreen(qreal g);

    qreal blue() const { return _blue; }
    void setBlue(qreal b);

    // Alpha - blend factor for sparkle color (0-1)
    qreal alpha() const { return _alpha; }
    void setAlpha(qreal a);

    // Follow gizmo - use gizmo's ratio when true
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Color as QColor for convenience
    QColor color() const { return QColor::fromRgbF(_red, _green, _blue); }
    void setColor(const QColor& c);

    // Check if sparkle should occur based on random value and minimum distance
    bool shouldSparkle(qreal random, qreal lastX, qreal lastY,
                       qreal currentX, qreal currentY, qreal minDistance = 0.001) const;

    // Calculate sparkle color blend from base color
    void calculateSparkleColor(qreal baseR, qreal baseG, qreal baseB,
                               qreal& outR, qreal& outG, qreal& outB) const;

    // Calculate precalc values based on ratio
    void calculatePrecalcValues(qreal ratio);

    // Precalculated values
    qreal precalcDensity() const { return _precalcDensity; }
    qreal precalcAlpha() const { return _precalcAlpha; }
    qreal precalcBeta() const { return _precalcBeta; }

    // Check if effect is active
    bool isActive() const { return !qFuzzyIsNull(_density); }

    // Apply sparkle effect to entire frame
    // This is the main entry point - processes input frame and returns output with sparkles inserted
    void applyToFrame(xengine::Frame* input, xengine::Frame* output, qreal ratio,
                      QRandomGenerator* rng = nullptr);

    // Overload with per-sample ratio evaluation (for followGizmo)
    // ratioEvaluator takes (x, y) and returns ratio at that position
    using RatioEvaluator = std::function<qreal(qreal x, qreal y)>;
    void applyToFrame(xengine::Frame* input, xengine::Frame* output,
                      const RatioEvaluator& ratioEvaluator,
                      QRandomGenerator* rng = nullptr);

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void densityChanged();
    void redChanged();
    void greenChanged();
    void blueChanged();
    void alphaChanged();
    void followGizmoChanged();

private:
    // Base parameters
    qreal _density{0.0};
    qreal _red{1.0};
    qreal _green{1.0};
    qreal _blue{1.0};
    qreal _alpha{1.0};
    bool _followGizmo{true};

    // Precalculated values (updated by calculatePrecalcValues)
    qreal _precalcDensity{0.0};
    qreal _precalcAlpha{1.0};
    qreal _precalcBeta{0.0};
};

} // namespace gizmotweak2

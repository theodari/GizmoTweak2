#include "SparkleTweak.h"
#include "core/Port.h"

#include <QtMath>
#include <QJsonObject>

namespace gizmotweak2
{

SparkleTweak::SparkleTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Sparkle"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);     // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Sparkle track with density (0), red (1), green (2), blue (3), alpha (4)
    // Matches GizmoTweak v1: single track with all 5 parameters
    auto* sparkleTrack = createAutomationTrack(QStringLiteral("Sparkle"), 5, QColor(255, 215, 0));
    sparkleTrack->setupParameter(0, 0.0, 1.0, _density, tr("Density"), 100.0, QStringLiteral("%"));
    sparkleTrack->setupParameter(1, 0.0, 1.0, _red, tr("Red"), 100.0, QStringLiteral("%"));
    sparkleTrack->setupParameter(2, 0.0, 1.0, _green, tr("Green"), 100.0, QStringLiteral("%"));
    sparkleTrack->setupParameter(3, 0.0, 1.0, _blue, tr("Blue"), 100.0, QStringLiteral("%"));
    sparkleTrack->setupParameter(4, 0.0, 1.0, _alpha, tr("Alpha"), 100.0, QStringLiteral("%"));
}

void SparkleTweak::setDensity(qreal d)
{
    d = qBound(0.0, d, 1.0);
    if (!qFuzzyCompare(_density, d))
    {
        _density = d;
        auto* track = automationTrack(QStringLiteral("Sparkle"));
        if (track) track->setInitialValue(0, d);
        emit densityChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setRed(qreal r)
{
    r = qBound(0.0, r, 1.0);
    if (!qFuzzyCompare(_red, r))
    {
        _red = r;
        auto* track = automationTrack(QStringLiteral("Sparkle"));
        if (track) track->setInitialValue(1, r);
        emit redChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setGreen(qreal g)
{
    g = qBound(0.0, g, 1.0);
    if (!qFuzzyCompare(_green, g))
    {
        _green = g;
        auto* track = automationTrack(QStringLiteral("Sparkle"));
        if (track) track->setInitialValue(2, g);
        emit greenChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setBlue(qreal b)
{
    b = qBound(0.0, b, 1.0);
    if (!qFuzzyCompare(_blue, b))
    {
        _blue = b;
        auto* track = automationTrack(QStringLiteral("Sparkle"));
        if (track) track->setInitialValue(3, b);
        emit blueChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setAlpha(qreal a)
{
    a = qBound(0.0, a, 1.0);
    if (!qFuzzyCompare(_alpha, a))
    {
        _alpha = a;
        auto* track = automationTrack(QStringLiteral("Sparkle"));
        if (track) track->setInitialValue(4, a);
        emit alphaChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;
        auto* ratioPort = inputAt(1);
        if (ratioPort)
        {
            ratioPort->setVisible(follow);
            if (!follow && ratioPort->isConnected())
            {
                emit requestDisconnectPort(ratioPort);
            }
        }
        emit followGizmoChanged();
        emitPropertyChanged();
    }
}

void SparkleTweak::setColor(const QColor& c)
{
    setRed(c.redF());
    setGreen(c.greenF());
    setBlue(c.blueF());
}

void SparkleTweak::calculatePrecalcValues(qreal ratio)
{
    if (_followGizmo)
    {
        _precalcDensity = ratio * _density;
        _precalcAlpha = ratio * _alpha;
        _precalcBeta = 1.0 - _precalcAlpha;
    }
    else
    {
        _precalcDensity = _density;
        _precalcAlpha = _alpha;
        _precalcBeta = 1.0 - _precalcAlpha;
    }
}

bool SparkleTweak::shouldSparkle(qreal random, qreal lastX, qreal lastY,
                                  qreal currentX, qreal currentY, qreal minDistance) const
{
    if (qFuzzyIsNull(_precalcDensity))
    {
        return false;
    }
    if (random >= _precalcDensity)
    {
        return false;
    }
    // Check minimum distance from last sparkle
    auto dx = currentX - lastX;
    auto dy = currentY - lastY;
    return (dx * dx + dy * dy) > (minDistance * minDistance);
}

void SparkleTweak::calculateSparkleColor(qreal baseR, qreal baseG, qreal baseB,
                                          qreal& outR, qreal& outG, qreal& outB) const
{
    outR = qBound(0.0, _precalcAlpha * _red + _precalcBeta * baseR, 1.0);
    outG = qBound(0.0, _precalcAlpha * _green + _precalcBeta * baseG, 1.0);
    outB = qBound(0.0, _precalcAlpha * _blue + _precalcBeta * baseB, 1.0);
}

void SparkleTweak::applyToFrame(xengine::Frame* input, xengine::Frame* output, qreal ratio,
                                 QRandomGenerator* rng)
{
    if (!input || !output)
    {
        return;
    }

    output->clear();

    const int n = input->size();
    if (n == 0)
    {
        return;
    }

    // Use provided RNG or global
    QRandomGenerator* generator = rng ? rng : QRandomGenerator::global();

    // Calculate precalc values based on ratio
    calculatePrecalcValues(ratio);

    // If sparkle is not active, just copy the frame
    if (!isActive() || qFuzzyIsNull(_precalcDensity))
    {
        output->clone(*input);
        return;
    }

    // Track last sparkle position for minimum distance check
    qreal lastSparkledX = 0.0;
    qreal lastSparkledY = 0.0;

    // Constants for sparkle insertion (matching GizmoTweak)
    const int nbBeginColoredSamples = 2;
    const int nbEndColoredSamples = 2;
    const int nbSparkleSamples = 5;

    // Process samples
    const auto& firstSample = (*input)[0];
    xengine::XSample lastSample(firstSample.getX(), firstSample.getY(), firstSample.getZ(),
                                 firstSample.getR(), firstSample.getG(), firstSample.getB(),
                                 firstSample.getRepeats());

    for (int i = 0; i < n; ++i)
    {
        const auto& currentSample = (*input)[i];

        // Check if sparkle should occur between last and current sample
        if (i > 0 && shouldSparkle(generator->generateDouble(),
                                    lastSparkledX, lastSparkledY,
                                    lastSample.getX(), lastSample.getY()))
        {
            // Calculate sparkle color
            qreal sparkleR, sparkleG, sparkleB;
            calculateSparkleColor(lastSample.getR(), lastSample.getG(), lastSample.getB(),
                                  sparkleR, sparkleG, sparkleB);

            // Random interpolation factor between last and current sample
            const qreal ra = generator->generateDouble();
            const qreal rb = 1.0 - ra;

            // Interpolated sparkle position
            qreal zx = lastSample.getX() * rb + currentSample.getX() * ra;
            qreal zy = lastSample.getY() * rb + currentSample.getY() * ra;
            qreal zz = lastSample.getZ() * rb + currentSample.getZ() * ra;

            // Interpolated transition colors
            qreal zr = lastSample.getR() * rb + sparkleR * ra;
            qreal zg = lastSample.getG() * rb + sparkleG * ra;
            qreal zb = lastSample.getB() * rb + sparkleB * ra;

            // Add transition IN samples (fade to sparkle)
            output->addSample(zx, zy, zz, zr, zg, zb, nbBeginColoredSamples);

            // Add sparkle point (repeated for brightness)
            output->addSample(zx, zy, zz, sparkleR, sparkleG, sparkleB, nbSparkleSamples);

            // Add transition OUT samples (fade from sparkle)
            output->addSample(zx, zy, zz, zr, zg, zb, nbEndColoredSamples);

            // Update last sparkle position
            lastSparkledX = lastSample.getX();
            lastSparkledY = lastSample.getY();
        }

        // Add the current sample to output
        output->addSample(currentSample.getX(), currentSample.getY(), currentSample.getZ(),
                          currentSample.getR(), currentSample.getG(), currentSample.getB(),
                          currentSample.getNb());

        // Update last sample
        lastSample = xengine::XSample(currentSample.getX(), currentSample.getY(), currentSample.getZ(),
                                       currentSample.getR(), currentSample.getG(), currentSample.getB(),
                                       currentSample.getRepeats());
    }
}

void SparkleTweak::applyToFrame(xengine::Frame* input, xengine::Frame* output,
                                 const RatioEvaluator& ratioEvaluator,
                                 QRandomGenerator* rng)
{
    if (!input || !output)
    {
        return;
    }

    output->clear();

    const int n = input->size();
    if (n == 0)
    {
        return;
    }

    // Use provided RNG or global
    QRandomGenerator* generator = rng ? rng : QRandomGenerator::global();

    // If sparkle is not active, just copy the frame
    if (!isActive())
    {
        output->clone(*input);
        return;
    }

    // Track last sparkle position for minimum distance check
    qreal lastSparkledX = 0.0;
    qreal lastSparkledY = 0.0;

    // Constants for sparkle insertion (matching GizmoTweak)
    const int nbBeginColoredSamples = 2;
    const int nbEndColoredSamples = 2;
    const int nbSparkleSamples = 5;

    // Process samples
    const auto& firstSample = (*input)[0];
    xengine::XSample lastSample(firstSample.getX(), firstSample.getY(), firstSample.getZ(),
                                 firstSample.getR(), firstSample.getG(), firstSample.getB(),
                                 firstSample.getRepeats());

    for (int i = 0; i < n; ++i)
    {
        const auto& currentSample = (*input)[i];

        // Evaluate ratio at current sample position
        qreal ratio = ratioEvaluator(currentSample.getX(), currentSample.getY());

        // Calculate density and alpha based on ratio (followGizmo behavior)
        qreal localDensity = ratio * _density;
        qreal localAlpha = ratio * _alpha;
        qreal localBeta = 1.0 - localAlpha;

        // Check if sparkle should occur between last and current sample
        bool doSparkle = false;
        if (i > 0 && !qFuzzyIsNull(localDensity))
        {
            qreal random = generator->generateDouble();
            if (random < localDensity)
            {
                // Check minimum distance from last sparkle
                qreal dx = currentSample.getX() - lastSparkledX;
                qreal dy = currentSample.getY() - lastSparkledY;
                if ((dx * dx + dy * dy) > 0.001 * 0.001)
                {
                    doSparkle = true;
                }
            }
        }

        if (doSparkle)
        {
            // Calculate sparkle color with local alpha
            qreal sparkleR = qBound(0.0, localAlpha * _red + localBeta * lastSample.getR(), 1.0);
            qreal sparkleG = qBound(0.0, localAlpha * _green + localBeta * lastSample.getG(), 1.0);
            qreal sparkleB = qBound(0.0, localAlpha * _blue + localBeta * lastSample.getB(), 1.0);

            // Random interpolation factor between last and current sample
            const qreal ra = generator->generateDouble();
            const qreal rb = 1.0 - ra;

            // Interpolated sparkle position
            qreal zx = lastSample.getX() * rb + currentSample.getX() * ra;
            qreal zy = lastSample.getY() * rb + currentSample.getY() * ra;
            qreal zz = lastSample.getZ() * rb + currentSample.getZ() * ra;

            // Interpolated transition colors
            qreal zr = lastSample.getR() * rb + sparkleR * ra;
            qreal zg = lastSample.getG() * rb + sparkleG * ra;
            qreal zb = lastSample.getB() * rb + sparkleB * ra;

            // Add transition IN samples (fade to sparkle)
            output->addSample(zx, zy, zz, zr, zg, zb, nbBeginColoredSamples);

            // Add sparkle point (repeated for brightness)
            output->addSample(zx, zy, zz, sparkleR, sparkleG, sparkleB, nbSparkleSamples);

            // Add transition OUT samples (fade from sparkle)
            output->addSample(zx, zy, zz, zr, zg, zb, nbEndColoredSamples);

            // Update last sparkle position
            lastSparkledX = lastSample.getX();
            lastSparkledY = lastSample.getY();
        }

        // Add the current sample to output
        output->addSample(currentSample.getX(), currentSample.getY(), currentSample.getZ(),
                          currentSample.getR(), currentSample.getG(), currentSample.getB(),
                          currentSample.getNb());

        // Update last sample
        lastSample = xengine::XSample(currentSample.getX(), currentSample.getY(), currentSample.getZ(),
                                       currentSample.getR(), currentSample.getG(), currentSample.getB(),
                                       currentSample.getRepeats());
    }
}

QJsonObject SparkleTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["density"] = _density;
    obj["red"] = _red;
    obj["green"] = _green;
    obj["blue"] = _blue;
    obj["alpha"] = _alpha;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void SparkleTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("density")) setDensity(json["density"].toDouble());
    if (json.contains("red")) setRed(json["red"].toDouble());
    if (json.contains("green")) setGreen(json["green"].toDouble());
    if (json.contains("blue")) setBlue(json["blue"].toDouble());
    if (json.contains("alpha")) setAlpha(json["alpha"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void SparkleTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active
    // Matches GizmoTweak v1: single Sparkle track with density, R, G, B, alpha
    auto* sparkleTrack = automationTrack(QStringLiteral("Sparkle"));
    if (sparkleTrack && sparkleTrack->isAutomated())
    {
        _density = sparkleTrack->timedValue(timeMs, 0);
        _red = sparkleTrack->timedValue(timeMs, 1);
        _green = sparkleTrack->timedValue(timeMs, 2);
        _blue = sparkleTrack->timedValue(timeMs, 3);
        _alpha = sparkleTrack->timedValue(timeMs, 4);
    }
}

} // namespace gizmotweak2

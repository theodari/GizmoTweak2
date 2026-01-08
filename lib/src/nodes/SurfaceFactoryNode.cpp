#include "SurfaceFactoryNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

SurfaceFactoryNode::SurfaceFactoryNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("SurfaceFactory"));

    // Input: normalized time (0-1)
    addInput(QStringLiteral("time"), Port::DataType::Ratio1D);

    // Output: computed ratio
    addOutput(QStringLiteral("ratio"), Port::DataType::Ratio1D);
}

void SurfaceFactoryNode::setSurfaceType(SurfaceType type)
{
    if (_surfaceType != type)
    {
        _surfaceType = type;
        emit surfaceTypeChanged();
    }
}

void SurfaceFactoryNode::setAmplitude(qreal amp)
{
    if (!qFuzzyCompare(_amplitude, amp))
    {
        _amplitude = amp;
        emit amplitudeChanged();
    }
}

void SurfaceFactoryNode::setFrequency(qreal freq)
{
    if (!qFuzzyCompare(_frequency, freq))
    {
        _frequency = freq;
        emit frequencyChanged();
    }
}

void SurfaceFactoryNode::setPhase(qreal ph)
{
    if (!qFuzzyCompare(_phase, ph))
    {
        _phase = ph;
        emit phaseChanged();
    }
}

void SurfaceFactoryNode::setOffset(qreal off)
{
    if (!qFuzzyCompare(_offset, off))
    {
        _offset = off;
        emit offsetChanged();
    }
}

void SurfaceFactoryNode::setClamp(bool c)
{
    if (_clamp != c)
    {
        _clamp = c;
        emit clampChanged();
    }
}

qreal SurfaceFactoryNode::computeRatio(qreal t) const
{
    // Apply frequency and phase
    qreal x = t * _frequency + _phase;

    qreal result = 0.0;

    switch (_surfaceType)
    {
    case SurfaceType::Linear:
        // Linear ramp from 0 to 1
        result = std::fmod(x, 1.0);
        if (result < 0.0) result += 1.0;
        break;

    case SurfaceType::Sine:
        // Sine wave normalized to 0-1 range
        result = (std::sin(x * 2.0 * M_PI) + 1.0) * 0.5;
        break;

    case SurfaceType::Cosine:
        // Cosine wave normalized to 0-1 range
        result = (std::cos(x * 2.0 * M_PI) + 1.0) * 0.5;
        break;

    case SurfaceType::Triangle:
        // Triangle wave
        {
            qreal frac = std::fmod(x, 1.0);
            if (frac < 0.0) frac += 1.0;
            result = (frac < 0.5) ? (frac * 2.0) : (2.0 - frac * 2.0);
        }
        break;

    case SurfaceType::Sawtooth:
        // Sawtooth wave (same as linear but explicit)
        result = std::fmod(x, 1.0);
        if (result < 0.0) result += 1.0;
        break;

    case SurfaceType::Square:
        // Square wave
        {
            qreal frac = std::fmod(x, 1.0);
            if (frac < 0.0) frac += 1.0;
            result = (frac < 0.5) ? 1.0 : 0.0;
        }
        break;

    case SurfaceType::Exponential:
        // Exponential curve
        {
            qreal frac = std::fmod(x, 1.0);
            if (frac < 0.0) frac += 1.0;
            result = (std::exp(frac) - 1.0) / (M_E - 1.0);
        }
        break;

    case SurfaceType::Logarithmic:
        // Logarithmic curve
        {
            qreal frac = std::fmod(x, 1.0);
            if (frac < 0.0) frac += 1.0;
            // Avoid log(0)
            result = std::log(1.0 + frac * (M_E - 1.0)) / 1.0;
        }
        break;
    }

    // Apply amplitude and offset
    result = result * _amplitude + _offset;

    // Clamp to 0-1 if enabled
    if (_clamp)
    {
        result = qBound(0.0, result, 1.0);
    }

    return result;
}

} // namespace gizmotweak2

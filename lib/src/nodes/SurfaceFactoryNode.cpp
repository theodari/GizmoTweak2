#include "SurfaceFactoryNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

SurfaceFactoryNode::SurfaceFactoryNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("SurfaceFactory"));

    // No input - SurfaceFactory is a generator (like Gizmo)
    // It produces time-based ratio curves autonomously

    // Output: computed ratio (outputs any ratio type)
    addOutput(QStringLiteral("ratio"), Port::DataType::RatioAny);
}

void SurfaceFactoryNode::setSurfaceType(SurfaceType type)
{
    if (_surfaceType != type)
    {
        _surfaceType = type;
        emit surfaceTypeChanged();
        emitPropertyChanged();
    }
}

void SurfaceFactoryNode::setAmplitude(qreal amp)
{
    if (!qFuzzyCompare(_amplitude, amp))
    {
        _amplitude = amp;
        emit amplitudeChanged();
        emitPropertyChanged();
    }
}

void SurfaceFactoryNode::setFrequency(qreal freq)
{
    if (!qFuzzyCompare(_frequency, freq))
    {
        _frequency = freq;
        emit frequencyChanged();
        emitPropertyChanged();
    }
}

void SurfaceFactoryNode::setPhase(qreal ph)
{
    if (!qFuzzyCompare(_phase, ph))
    {
        _phase = ph;
        emit phaseChanged();
        emitPropertyChanged();
    }
}

void SurfaceFactoryNode::setOffset(qreal off)
{
    if (!qFuzzyCompare(_offset, off))
    {
        _offset = off;
        emit offsetChanged();
        emitPropertyChanged();
    }
}

void SurfaceFactoryNode::setClamp(bool c)
{
    if (_clamp != c)
    {
        _clamp = c;
        emit clampChanged();
        emitPropertyChanged();
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

QJsonObject SurfaceFactoryNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["surfaceType"] = static_cast<int>(_surfaceType);
    obj["amplitude"] = _amplitude;
    obj["frequency"] = _frequency;
    obj["phase"] = _phase;
    obj["offset"] = _offset;
    obj["clamp"] = _clamp;
    return obj;
}

void SurfaceFactoryNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("surfaceType"))
    {
        setSurfaceType(static_cast<SurfaceType>(json["surfaceType"].toInt()));
    }
    if (json.contains("amplitude")) setAmplitude(json["amplitude"].toDouble());
    if (json.contains("frequency")) setFrequency(json["frequency"].toDouble());
    if (json.contains("phase")) setPhase(json["phase"].toDouble());
    if (json.contains("offset")) setOffset(json["offset"].toDouble());
    if (json.contains("clamp")) setClamp(json["clamp"].toBool());
}

} // namespace gizmotweak2

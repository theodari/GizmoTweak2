#include "TimeShiftNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

TimeShiftNode::TimeShiftNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("TimeShift"));

    // Input: time ratio from automation
    addInput(QStringLiteral("time"), Port::DataType::Ratio1D);

    // Output: shifted time ratio
    addOutput(QStringLiteral("shifted"), Port::DataType::Ratio1D);
}

void TimeShiftNode::setDelay(qreal d)
{
    if (!qFuzzyCompare(_delay, d))
    {
        _delay = d;
        emit delayChanged();
    }
}

void TimeShiftNode::setScale(qreal s)
{
    // Prevent zero or negative scale
    s = qMax(0.001, s);
    if (!qFuzzyCompare(_scale, s))
    {
        _scale = s;
        emit scaleChanged();
    }
}

void TimeShiftNode::setLoop(bool l)
{
    if (_loop != l)
    {
        _loop = l;
        emit loopChanged();
    }
}

void TimeShiftNode::setLoopDuration(qreal duration)
{
    // Minimum loop duration
    duration = qMax(0.001, duration);
    if (!qFuzzyCompare(_loopDuration, duration))
    {
        _loopDuration = duration;
        emit loopDurationChanged();
    }
}

qreal TimeShiftNode::shiftTime(qreal currentTime) const
{
    // Apply delay (positive = retard, negative = advance)
    qreal shifted = currentTime - _delay;

    // Apply scale (speed multiplier)
    shifted *= _scale;

    // Apply loop if enabled
    if (_loop && _loopDuration > 0.0)
    {
        // Use fmod for looping, handle negative values
        shifted = std::fmod(shifted, _loopDuration);
        if (shifted < 0.0)
        {
            shifted += _loopDuration;
        }
    }

    return shifted;
}

QJsonObject TimeShiftNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["delay"] = _delay;
    obj["scale"] = _scale;
    obj["loop"] = _loop;
    obj["loopDuration"] = _loopDuration;
    return obj;
}

void TimeShiftNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("delay")) setDelay(json["delay"].toDouble());
    if (json.contains("scale")) setScale(json["scale"].toDouble());
    if (json.contains("loop")) setLoop(json["loop"].toBool());
    if (json.contains("loopDuration")) setLoopDuration(json["loopDuration"].toDouble());
}

} // namespace gizmotweak2

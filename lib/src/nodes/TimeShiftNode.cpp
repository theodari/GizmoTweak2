#include "TimeShiftNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

TimeShiftNode::TimeShiftNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("TimeShift"));

    // Input: time ratio (accepts any ratio type)
    addInput(QStringLiteral("time"), Port::DataType::RatioAny);

    // Output: shifted time ratio (outputs any ratio type)
    addOutput(QStringLiteral("shifted"), Port::DataType::RatioAny);

    // Position pass-through
    addInput(QStringLiteral("center"), Port::DataType::Position);
    addOutput(QStringLiteral("center"), Port::DataType::Position);

    // Automation: Time track with delay (0) and scale (1)
    auto* timeTrack = createAutomationTrack(QStringLiteral("Time"), 2, QColor(65, 105, 225));  // Royal blue
    timeTrack->setupParameter(0, -10.0, 10.0, _delay, tr("Delay"), 1000.0, QStringLiteral(" ms"));
    timeTrack->setupParameter(1, 0.01, 10.0, _scale, tr("Scale"), 100.0, QStringLiteral("%"));

    // Automation: Loop track with loopDuration (0)
    auto* loopTrack = createAutomationTrack(QStringLiteral("Loop"), 1, QColor(34, 139, 34));  // Forest green
    loopTrack->setupParameter(0, 0.001, 60.0, _loopDuration, tr("Duration"), 1000.0, QStringLiteral(" ms"));
}

void TimeShiftNode::setDelay(qreal d)
{
    if (!qFuzzyCompare(_delay, d))
    {
        _delay = d;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Time"));
        if (track) track->setInitialValue(0, d);
        emit delayChanged();
        emitPropertyChanged();
    }
}

void TimeShiftNode::setScale(qreal s)
{
    // Prevent zero or negative scale
    s = qMax(0.001, s);
    if (!qFuzzyCompare(_scale, s))
    {
        _scale = s;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Time"));
        if (track) track->setInitialValue(1, s);
        emit scaleChanged();
        emitPropertyChanged();
    }
}

void TimeShiftNode::setLoop(bool l)
{
    if (_loop != l)
    {
        _loop = l;
        emit loopChanged();
        emitPropertyChanged();
    }
}

void TimeShiftNode::setLoopDuration(qreal duration)
{
    // Minimum loop duration
    duration = qMax(0.001, duration);
    if (!qFuzzyCompare(_loopDuration, duration))
    {
        _loopDuration = duration;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Loop"));
        if (track) track->setInitialValue(0, duration);
        emit loopDurationChanged();
        emitPropertyChanged();
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

void TimeShiftNode::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* timeTrack = automationTrack(QStringLiteral("Time"));
    if (timeTrack && timeTrack->isAutomated())
    {
        _delay = timeTrack->timedValue(timeMs, 0);
        _scale = timeTrack->timedValue(timeMs, 1);
    }

    auto* loopTrack = automationTrack(QStringLiteral("Loop"));
    if (loopTrack && loopTrack->isAutomated())
    {
        _loopDuration = loopTrack->timedValue(timeMs, 0);
    }
}

} // namespace gizmotweak2

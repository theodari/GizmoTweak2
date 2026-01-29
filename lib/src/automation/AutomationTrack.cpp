#include "AutomationTrack.h"
#include "core/Node.h"
#include <QDebug>

namespace gizmotweak2
{

AutomationTrack::AutomationTrack(int nbParams, const QString& trackName,
                                 const QColor& color, QObject* parent)
    : QObject(parent)
    , _nbParams(nbParams)
    , _trackName(trackName)
    , _color(color)
{
    _parameters.resize(nbParams);
}

AutomationTrack::AutomationTrack(const AutomationTrack& other)
    : QObject(other.parent())
    , _nbParams(other._nbParams)
    , _trackName(other._trackName)
    , _automated(other._automated)
    , _color(other._color)
{
    _parameters = other._parameters;

    // Deep copy keyframes
    for (auto it = other._keyFrames.constBegin(); it != other._keyFrames.constEnd(); ++it)
    {
        _keyFrames.insert(it.key(), new KeyFrame(*it.value()));
    }
}

AutomationTrack::~AutomationTrack()
{
    qDeleteAll(_keyFrames);
    _keyFrames.clear();
}

bool AutomationTrack::save(QDataStream& out) const
{
    quint64 reserved = 0;

    out << _nbParams << _trackName << _automated << reserved << reserved;

    for (int i = 0; i < _nbParams; ++i)
    {
        out << _parameters[i].initialValue;
        out << _parameters[i].minValue;
        out << _parameters[i].maxValue;
        out << _parameters[i].paramName;
        out << _parameters[i].displayRatio;
        out << _parameters[i].suffix;
    }

    out << static_cast<int>(_keyFrames.count());
    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        out << it.key();
        if (!it.value()->save(out))
        {
            return false;
        }
    }

    out << QChar('A');  // End marker
    return true;
}

bool AutomationTrack::load(QDataStream& in)
{
    quint64 reserved;
    in >> _nbParams >> _trackName >> _automated >> reserved >> reserved;

    if (_nbParams <= 0 || _nbParams > 16)
    {
        qWarning() << "AutomationTrack: Invalid param count:" << _nbParams;
        return false;
    }

    _parameters.resize(_nbParams);
    for (int i = 0; i < _nbParams; ++i)
    {
        in >> _parameters[i].initialValue;
        in >> _parameters[i].minValue;
        in >> _parameters[i].maxValue;
        in >> _parameters[i].paramName;
        in >> _parameters[i].displayRatio;
        in >> _parameters[i].suffix;
    }

    int keyFrameCount = 0;
    in >> keyFrameCount;

    if (keyFrameCount > 1000)
    {
        qWarning() << "AutomationTrack: Too many keyframes:" << keyFrameCount;
        return false;
    }

    qDeleteAll(_keyFrames);
    _keyFrames.clear();

    for (int i = 0; i < keyFrameCount; ++i)
    {
        int timeMs = 0;
        in >> timeMs;
        auto* kf = new KeyFrame(_nbParams);
        if (!kf->load(in))
        {
            delete kf;
            return false;
        }
        _keyFrames.insert(timeMs, kf);
    }

    QChar marker;
    in >> marker;
    if (marker != QChar('A'))
    {
        qWarning() << "AutomationTrack: Invalid end marker";
        return false;
    }

    emit keyFrameCountChanged();
    return true;
}

QJsonObject AutomationTrack::toJson() const
{
    QJsonObject obj;
    obj["trackName"] = _trackName;
    obj["automated"] = _automated;
    obj["color"] = _color.name();
    obj["paramCount"] = _nbParams;

    // Save parameters
    QJsonArray paramsArray;
    for (int i = 0; i < _nbParams; ++i)
    {
        QJsonObject paramObj;
        paramObj["minValue"] = _parameters[i].minValue;
        paramObj["maxValue"] = _parameters[i].maxValue;
        paramObj["initialValue"] = _parameters[i].initialValue;
        paramObj["paramName"] = _parameters[i].paramName;
        paramObj["displayRatio"] = _parameters[i].displayRatio;
        paramObj["suffix"] = _parameters[i].suffix;
        paramsArray.append(paramObj);
    }
    obj["parameters"] = paramsArray;

    // Save keyframes
    QJsonArray keyFramesArray;
    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        QJsonObject kfObj;
        kfObj["time"] = it.key();
        kfObj["data"] = it.value()->toJson();
        keyFramesArray.append(kfObj);
    }
    obj["keyFrames"] = keyFramesArray;

    return obj;
}

bool AutomationTrack::fromJson(const QJsonObject& json)
{
    if (!json.contains("trackName") || !json.contains("paramCount"))
    {
        return false;
    }

    _trackName = json["trackName"].toString();
    _automated = json["automated"].toBool();
    _color = QColor(json["color"].toString());
    _nbParams = json["paramCount"].toInt();

    // Load parameters
    _parameters.resize(_nbParams);
    auto paramsArray = json["parameters"].toArray();
    for (int i = 0; i < qMin(_nbParams, static_cast<int>(paramsArray.size())); ++i)
    {
        auto paramObj = paramsArray[i].toObject();
        _parameters[i].minValue = paramObj["minValue"].toDouble();
        _parameters[i].maxValue = paramObj["maxValue"].toDouble();
        _parameters[i].initialValue = paramObj["initialValue"].toDouble();
        _parameters[i].paramName = paramObj["paramName"].toString();
        _parameters[i].displayRatio = paramObj["displayRatio"].toDouble(1.0);
        _parameters[i].suffix = paramObj["suffix"].toString();
    }

    // Load keyframes
    qDeleteAll(_keyFrames);
    _keyFrames.clear();

    auto keyFramesArray = json["keyFrames"].toArray();
    for (const auto& kfVal : keyFramesArray)
    {
        auto kfObj = kfVal.toObject();
        int timeMs = kfObj["time"].toInt();
        auto* kf = new KeyFrame(_nbParams);
        if (kf->fromJson(kfObj["data"].toObject()))
        {
            _keyFrames.insert(timeMs, kf);
        }
        else
        {
            delete kf;
        }
    }

    emit keyFrameCountChanged();
    return true;
}

void AutomationTrack::keyframesFromJson(const QJsonObject& json)
{
    // Load keyframes, automated flag, and initial values from JSON
    // Preserve other parameter metadata (min, max, displayRatio, suffix, name) from setupParameter
    _automated = json["automated"].toBool();

    // Load initial values from JSON (but keep other metadata from setupParameter)
    auto paramsArray = json["parameters"].toArray();
    for (int i = 0; i < qMin(_nbParams, static_cast<int>(paramsArray.size())); ++i)
    {
        auto paramObj = paramsArray[i].toObject();
        // Only load initialValue, preserve min/max/displayRatio/suffix/name from setupParameter
        if (paramObj.contains("initialValue"))
        {
            _parameters[i].initialValue = paramObj["initialValue"].toDouble();
        }
    }

    // Clear and reload keyframes
    qDeleteAll(_keyFrames);
    _keyFrames.clear();

    auto keyFramesArray = json["keyFrames"].toArray();
    for (const auto& kfVal : keyFramesArray)
    {
        auto kfObj = kfVal.toObject();
        int timeMs = kfObj["time"].toInt();
        auto* kf = new KeyFrame(_nbParams);
        if (kf->fromJson(kfObj["data"].toObject()))
        {
            _keyFrames.insert(timeMs, kf);
        }
        else
        {
            delete kf;
        }
    }

    emit keyFrameCountChanged();
}

void AutomationTrack::setupParameter(int paramIndex, double minValue, double maxValue,
                                     double initialValue, const QString& paramName,
                                     double displayRatio, const QString& suffix)
{
    Q_ASSERT(paramIndex >= 0 && paramIndex < _parameters.count());
    _parameters[paramIndex].minValue = minValue;
    _parameters[paramIndex].maxValue = maxValue;
    _parameters[paramIndex].initialValue = initialValue;
    _parameters[paramIndex].paramName = paramName;
    _parameters[paramIndex].displayRatio = displayRatio;
    _parameters[paramIndex].suffix = suffix;
}

void AutomationTrack::setTrackName(const QString& name)
{
    if (_trackName != name)
    {
        _trackName = name;
        emit trackNameChanged();
    }
}

void AutomationTrack::setAutomated(bool automated)
{
    if (_automated != automated)
    {
        _automated = automated;
        if (!_automated)
        {
            qDeleteAll(_keyFrames);
            _keyFrames.clear();
            emit keyFrameCountChanged();
        }
        emit automatedChanged();
    }
}

void AutomationTrack::setColor(const QColor& color)
{
    if (_color != color)
    {
        _color = color;
        emit colorChanged();
    }
}

QString AutomationTrack::nodeType() const
{
    auto* node = qobject_cast<Node*>(parent());
    return node ? node->type() : QString();
}

QString AutomationTrack::nodeName() const
{
    auto* node = qobject_cast<Node*>(parent());
    return node ? node->displayName() : QString();
}

double AutomationTrack::minValue(int index) const
{
    if (index < 0 || index >= _nbParams) return 0.0;
    return _parameters[index].minValue;
}

double AutomationTrack::maxValue(int index) const
{
    if (index < 0 || index >= _nbParams) return 1.0;
    return _parameters[index].maxValue;
}

double AutomationTrack::initialValue(int index) const
{
    if (index < 0 || index >= _nbParams) return 0.0;
    return _parameters[index].initialValue;
}

void AutomationTrack::setInitialValue(int index, double value)
{
    if (index < 0 || index >= _nbParams) return;
    _parameters[index].initialValue = value;
}

QString AutomationTrack::parameterName(int index) const
{
    if (index < 0 || index >= _nbParams) return QString();
    return _parameters[index].paramName;
}

double AutomationTrack::displayRatio(int index) const
{
    if (index < 0 || index >= _nbParams) return 1.0;
    return _parameters[index].displayRatio;
}

QString AutomationTrack::suffix(int index) const
{
    if (index < 0 || index >= _nbParams) return QString();
    return _parameters[index].suffix;
}

KeyFrame* AutomationTrack::createKeyFrame(int timeMs)
{
    if (_keyFrames.contains(timeMs))
    {
        return _keyFrames.value(timeMs);
    }

    auto* kf = new KeyFrame(_nbParams);

    if (_keyFrames.isEmpty())
    {
        // First keyframe: use initial values
        for (int i = 0; i < _nbParams; ++i)
        {
            kf->setValue(i, _parameters[i].initialValue);
        }
    }
    else
    {
        // Interpolate values at this time
        for (int i = 0; i < _nbParams; ++i)
        {
            kf->setValue(i, timedValue(timeMs, i));
        }
    }

    _keyFrames.insert(timeMs, kf);
    emit keyFrameCountChanged();
    emit keyFrameModified(timeMs);
    return kf;
}

void AutomationTrack::moveKeyFrame(int oldTimeMs, int newTimeMs)
{
    if (!_keyFrames.contains(oldTimeMs) || _keyFrames.contains(newTimeMs))
    {
        return;
    }

    auto* kf = _keyFrames.take(oldTimeMs);
    _keyFrames.insert(newTimeMs, kf);
    emit keyFrameModified(newTimeMs);
}

void AutomationTrack::updateKeyFrameValue(int timeMs, int paramIndex, double value)
{
    if (paramIndex < 0 || paramIndex >= _nbParams) return;

    auto it = _keyFrames.find(timeMs);
    if (it != _keyFrames.end())
    {
        it.value()->setValue(paramIndex, value);
        emit keyFrameModified(timeMs);
    }
}

void AutomationTrack::deleteKeyFrame(int timeMs)
{
    if (_keyFrames.contains(timeMs))
    {
        delete _keyFrames.take(timeMs);
        emit keyFrameCountChanged();
    }
}

bool AutomationTrack::hasKeyFrameAt(int timeMs) const
{
    return _keyFrames.contains(timeMs);
}

double AutomationTrack::timedValue(int timeMs, int paramIndex) const
{
    // Defensive bounds check - return 0 for invalid indices instead of crashing
    if (paramIndex < 0 || paramIndex >= _nbParams)
    {
        return 0.0;
    }

    // Handle negative time (e.g., from TimeShift with delay) - return initial value
    // because at t<0, no animation has started yet
    if (timeMs < 0)
    {
        return _parameters[paramIndex].initialValue;
    }

    if (_keyFrames.isEmpty())
    {
        return _parameters[paramIndex].initialValue;
    }

    // Find surrounding keyframes
    int prevTime = 0;
    KeyFrame* prevKf = nullptr;
    KeyFrame* nextKf = nullptr;
    int nextTime = 0;

    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        if (it.key() <= timeMs)
        {
            prevTime = it.key();
            prevKf = it.value();
        }
        if (it.key() >= timeMs && !nextKf)
        {
            nextTime = it.key();
            nextKf = it.value();
            break;
        }
    }

    // Before first keyframe
    if (!prevKf && nextKf)
    {
        if (timeMs >= nextTime)
        {
            return nextKf->value(paramIndex);
        }
        double alpha = static_cast<double>(timeMs) / static_cast<double>(nextTime);
        double progress = nextKf->valueForProgress(alpha);
        return (1.0 - progress) * _parameters[paramIndex].initialValue
             + progress * nextKf->value(paramIndex);
    }

    // After last keyframe
    if (prevKf && !nextKf)
    {
        return prevKf->value(paramIndex);
    }

    // Between two keyframes
    if (prevKf && nextKf)
    {
        if (prevTime == nextTime)
        {
            return prevKf->value(paramIndex);
        }
        double alpha = static_cast<double>(timeMs - prevTime)
                     / static_cast<double>(nextTime - prevTime);
        double progress = nextKf->valueForProgress(alpha);
        return (1.0 - progress) * prevKf->value(paramIndex)
             + progress * nextKf->value(paramIndex);
    }

    return _parameters[paramIndex].initialValue;
}

void AutomationTrack::resizeAllKeyFrames(double factor)
{
    if (_keyFrames.isEmpty() || factor <= 0.0)
    {
        return;
    }

    QMap<int, KeyFrame*> newKeyFrames;
    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        int newTime = static_cast<int>(0.5 + it.key() * factor);
        if (!newKeyFrames.contains(newTime))
        {
            newKeyFrames.insert(newTime, it.value());
        }
        else
        {
            delete it.value();
        }
    }
    _keyFrames = newKeyFrames;
    emit keyFrameCountChanged();
}

void AutomationTrack::removeKeyFramesAfter(int timeMs)
{
    QList<int> toRemove;
    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        if (it.key() >= timeMs)
        {
            toRemove.append(it.key());
        }
    }
    for (int t : toRemove)
    {
        delete _keyFrames.take(t);
    }
    if (!toRemove.isEmpty())
    {
        emit keyFrameCountChanged();
    }
}

void AutomationTrack::translateKeyFrames(int deltaMs)
{
    QList<int> toRemove;
    QMap<int, KeyFrame*> toAdd;

    for (auto it = _keyFrames.constBegin(); it != _keyFrames.constEnd(); ++it)
    {
        int newTime = it.key() + deltaMs;
        if (newTime < 0)
        {
            toRemove.append(it.key());
        }
        else
        {
            toAdd.insert(newTime, it.value());
            toRemove.append(it.key());
        }
    }

    for (int t : toRemove)
    {
        if (!toAdd.values().contains(_keyFrames.value(t)))
        {
            delete _keyFrames.value(t);
        }
        _keyFrames.remove(t);
    }

    for (auto it = toAdd.constBegin(); it != toAdd.constEnd(); ++it)
    {
        _keyFrames.insert(it.key(), it.value());
    }

    emit keyFrameCountChanged();
}

QString AutomationTrack::toolTipText(int timeMs) const
{
    QString text = _trackName + "\n";
    for (int i = 0; i < _nbParams; ++i)
    {
        double val = timedValue(timeMs, i) * _parameters[i].displayRatio;
        text += _parameters[i].paramName + QString(" %1").arg(static_cast<int>(val))
              + _parameters[i].suffix;
        if (i < _nbParams - 1)
        {
            text += "\n";
        }
    }
    return text;
}

int AutomationTrack::keyFrameCurveType(int timeMs) const
{
    auto it = _keyFrames.find(timeMs);
    if (it != _keyFrames.end())
        return static_cast<int>(it.value()->curveType());
    return 0; // Linear
}

void AutomationTrack::setKeyFrameCurveType(int timeMs, int curveType)
{
    auto it = _keyFrames.find(timeMs);
    if (it != _keyFrames.end())
    {
        it.value()->setCurveType(static_cast<QEasingCurve::Type>(curveType));
        emit keyFrameModified(timeMs);
    }
}

} // namespace gizmotweak2

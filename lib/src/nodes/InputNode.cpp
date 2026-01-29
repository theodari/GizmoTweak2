#include "InputNode.h"
#include "core/Port.h"

#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QtMath>

namespace gizmotweak2
{

InputNode::InputNode(QObject* parent)
    : Node(parent)
    , _patternStack(false)  // Don't create empty frame
{
    setDisplayName(QStringLiteral("Input"));
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Try multiple resource paths (Qt6 QML module may prefix the path)
    QStringList paths = {
        QStringLiteral(":/resources/gizmoTweakPatterns.ild"),
        QStringLiteral(":/qt/qml/GizmoTweak2/resources/gizmoTweakPatterns.ild"),
        QStringLiteral(":/GizmoTweak2/resources/gizmoTweakPatterns.ild")
    };

    for (const auto& path : paths)
    {
        if (QFile::exists(path))
        {
            int result = _patternStack.ildaLoad(path);
            if (result == 0 && _patternStack.size() > 0)
            {
                break;
            }
        }
    }

    // If no patterns loaded, create default test patterns
    if (_patternStack.size() == 0)
    {
        createDefaultPatterns();
    }

    buildPatternNames();
    updateCurrentFrame();
}

void InputNode::setSourceType(SourceType type)
{
    if (_sourceType != type)
    {
        _sourceType = type;
        emit sourceTypeChanged();
        emitPropertyChanged();
        updateCurrentFrame();
    }
}

void InputNode::setPatternIndex(int index)
{
    if (index >= 0 && index < _patternStack.size() && _patternIndex != index)
    {
        _patternIndex = index;
        emit patternIndexChanged();
        emitPropertyChanged();
        updateCurrentFrame();
    }
}

void InputNode::setPreviewPatternIndex(int index)
{
    if (_previewPatternIndex != index)
    {
        _previewPatternIndex = index;
        emit previewPatternIndexChanged();
        updateCurrentFrame();
    }
}

xengine::Frame* InputNode::getPatternFrame(int index) const
{
    if (index >= 0 && index < _patternStack.size())
    {
        return _patternStack.get(index);
    }
    return nullptr;
}

bool InputNode::loadIldaFile(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        qWarning() << "InputNode::loadIldaFile: Empty file path";
        return false;
    }

    // Clear existing patterns and load new file
    _patternStack.deleteAllFrames(false);

    int result = _patternStack.ildaLoad(filePath);
    if (result != 0 || _patternStack.size() == 0)
    {
        qWarning() << "InputNode::loadIldaFile: Failed to load" << filePath << "error:" << result;
        // Restore default patterns on failure
        createDefaultPatterns();
        buildPatternNames();
        setPatternIndex(0);
        return false;
    }

    buildPatternNames();
    setPatternIndex(0);
    updateCurrentFrame();
    return true;
}

bool InputNode::saveIldaFile(const QString& filePath) const
{
    if (filePath.isEmpty())
    {
        qWarning() << "InputNode::saveIldaFile: Empty file path";
        return false;
    }

    if (_patternStack.size() == 0)
    {
        qWarning() << "InputNode::saveIldaFile: No patterns to save";
        return false;
    }

    int result = const_cast<xengine::Stack&>(_patternStack).ildaSave(filePath);
    if (result != 0)
    {
        qWarning() << "InputNode::saveIldaFile: Failed to save" << filePath << "error:" << result;
        return false;
    }

    return true;
}

void InputNode::buildPatternNames()
{
    _patternNames.clear();
    for (int i = 0; i < _patternStack.size(); ++i)
    {
        auto* frame = _patternStack.get(i);
        QString name = frame ? frame->getName() : QString();
        if (name.isEmpty())
        {
            name = QString("Pattern %1").arg(i + 1);
        }
        _patternNames.append(name);
    }
    emit patternNamesChanged();
}

void InputNode::updateCurrentFrame()
{
    xengine::Frame* newFrame = nullptr;

    if (_sourceType == SourceType::Pattern)
    {
        // Use preview pattern if set, otherwise use selected pattern
        int indexToUse = (_previewPatternIndex >= 0) ? _previewPatternIndex : _patternIndex;
        if (indexToUse >= 0 && indexToUse < _patternStack.size())
        {
            newFrame = _patternStack.get(indexToUse);
        }
    }

    if (_currentFrame != newFrame)
    {
        _currentFrame = newFrame;
        emit currentFrameChanged();
    }
}

void InputNode::createDefaultPatterns()
{
    // Pattern 1: Square
    {
        auto* frame = new xengine::Frame();
        frame->setName("Square");
        frame->addSample(-0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1);  // Red (bottom-left)
        frame->addSample( 0.5,  0.5, 0.0, 1.0, 1.0, 0.0, 1);  // Yellow (bottom-right)
        frame->addSample( 0.5, -0.5, 0.0, 0.0, 1.0, 0.0, 1);  // Green (top-right)
        frame->addSample(-0.5, -0.5, 0.0, 0.0, 1.0, 1.0, 1);  // Cyan (top-left)
        frame->addSample(-0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1);  // Back to start
        _patternStack.append(frame, -1);
    }

    // Pattern 2: Circle
    {
        auto* frame = new xengine::Frame();
        frame->setName("Circle");
        const int segments = 64;
        for (int i = 0; i <= segments; ++i)
        {
            qreal t = static_cast<qreal>(i) / segments;
            qreal angle = t * 2.0 * M_PI;
            qreal x = 0.6 * qCos(angle);
            qreal y = -0.6 * qSin(angle);  // Negate Y for Canvas convention
            // Rainbow color based on angle
            QColor color = QColor::fromHsvF(t, 1.0, 1.0);
            frame->addSample(x, y, 0.0, color.redF(), color.greenF(), color.blueF(), 1);
        }
        _patternStack.append(frame, -1);
    }

    // Pattern 3: Star
    {
        auto* frame = new xengine::Frame();
        frame->setName("Star");
        const int points = 5;
        const qreal outerR = 0.7;
        const qreal innerR = 0.3;
        for (int i = 0; i <= points * 2; ++i)
        {
            qreal angle = (i * M_PI / points) - M_PI / 2;
            qreal r = (i % 2 == 0) ? outerR : innerR;
            qreal x = r * qCos(angle);
            qreal y = -r * qSin(angle);  // Negate Y for Canvas convention
            qreal hue = static_cast<qreal>(i) / (points * 2);
            QColor color = QColor::fromHsvF(hue, 1.0, 1.0);
            frame->addSample(x, y, 0.0, color.redF(), color.greenF(), color.blueF(), 1);
        }
        _patternStack.append(frame, -1);
    }

    // Pattern 4: Grid 8x8
    {
        auto* frame = new xengine::Frame();
        frame->setName("Grid 8x8");
        const int gridSize = 8;
        for (int row = 0; row < gridSize; ++row)
        {
            for (int col = 0; col < gridSize; ++col)
            {
                qreal x = (col - (gridSize - 1) / 2.0) * 0.2;
                qreal y = -(row - (gridSize - 1) / 2.0) * 0.2;  // Negate Y for Canvas convention
                frame->addSample(x, y, 0.0, 1.0, 1.0, 1.0, 1);  // White points
            }
        }
        _patternStack.append(frame, -1);
    }

    // Pattern 5: Spiral
    {
        auto* frame = new xengine::Frame();
        frame->setName("Spiral");
        const int segments = 100;
        for (int i = 0; i <= segments; ++i)
        {
            qreal t = static_cast<qreal>(i) / segments;
            qreal angle = t * 4.0 * M_PI;  // 2 full rotations
            qreal r = 0.1 + t * 0.6;
            qreal x = r * qCos(angle);
            qreal y = -r * qSin(angle);  // Negate Y for Canvas convention
            QColor color = QColor::fromHsvF(t, 1.0, 1.0);
            frame->addSample(x, y, 0.0, color.redF(), color.greenF(), color.blueF(), 1);
        }
        _patternStack.append(frame, -1);
    }
}

void InputNode::setDuration(int ms)
{
    ms = qMax(100, ms);  // Minimum 100ms
    if (_duration != ms)
    {
        _duration = ms;
        emit durationChanged();
        emitPropertyChanged();
    }
}

void InputNode::setBpm(qreal bpm)
{
    bpm = qBound(1.0, bpm, 999.0);
    if (!qFuzzyCompare(_bpm, bpm))
    {
        _bpm = bpm;
        emit bpmChanged();

        // Update duration if using BPM timing
        if (_useBpmTiming)
        {
            int newDuration = calculateDurationFromBpm();
            if (_duration != newDuration)
            {
                _duration = newDuration;
                emit durationChanged();
            }
        }
        emitPropertyChanged();
    }
}

void InputNode::setBeatsPerMeasure(int beats)
{
    beats = qBound(1, beats, 32);
    if (_beatsPerMeasure != beats)
    {
        _beatsPerMeasure = beats;
        emit beatsPerMeasureChanged();

        // Update duration if using BPM timing
        if (_useBpmTiming)
        {
            int newDuration = calculateDurationFromBpm();
            if (_duration != newDuration)
            {
                _duration = newDuration;
                emit durationChanged();
            }
        }
        emitPropertyChanged();
    }
}

void InputNode::setMeasures(int count)
{
    count = qBound(1, count, 999);
    if (_measures != count)
    {
        _measures = count;
        emit measuresChanged();

        // Update duration if using BPM timing
        if (_useBpmTiming)
        {
            int newDuration = calculateDurationFromBpm();
            if (_duration != newDuration)
            {
                _duration = newDuration;
                emit durationChanged();
            }
        }
        emitPropertyChanged();
    }
}

void InputNode::setUseBpmTiming(bool use)
{
    if (_useBpmTiming != use)
    {
        _useBpmTiming = use;
        emit useBpmTimingChanged();

        // Update duration if switching to BPM timing
        if (_useBpmTiming)
        {
            int newDuration = calculateDurationFromBpm();
            if (_duration != newDuration)
            {
                _duration = newDuration;
                emit durationChanged();
            }
        }
        emitPropertyChanged();
    }
}

int InputNode::calculateDurationFromBpm() const
{
    // Duration = (measures * beatsPerMeasure) / bpm * 60 * 1000
    // = totalBeats / bpm * 60000
    int totalBeats = _measures * _beatsPerMeasure;
    return static_cast<int>((totalBeats / _bpm) * 60000.0);
}

int InputNode::beatToMs(int beat) const
{
    // ms = beat / bpm * 60 * 1000
    return static_cast<int>((beat / _bpm) * 60000.0);
}

int InputNode::measureToMs(int measure) const
{
    return beatToMs(measure * _beatsPerMeasure);
}

qreal InputNode::msToBeat(int ms) const
{
    // beat = ms / 60000 * bpm
    return (ms / 60000.0) * _bpm;
}

QJsonObject InputNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["sourceType"] = static_cast<int>(_sourceType);
    obj["patternIndex"] = _patternIndex;
    obj["duration"] = _duration;
    obj["bpm"] = _bpm;
    obj["beatsPerMeasure"] = _beatsPerMeasure;
    obj["measures"] = _measures;
    obj["useBpmTiming"] = _useBpmTiming;
    return obj;
}

void InputNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("sourceType"))
    {
        setSourceType(static_cast<SourceType>(json["sourceType"].toInt()));
    }
    if (json.contains("patternIndex"))
    {
        setPatternIndex(json["patternIndex"].toInt());
    }
    if (json.contains("useBpmTiming"))
    {
        setUseBpmTiming(json["useBpmTiming"].toBool());
    }
    if (json.contains("duration"))
    {
        setDuration(json["duration"].toInt());
    }
    if (json.contains("bpm"))
    {
        setBpm(json["bpm"].toDouble());
    }
    if (json.contains("beatsPerMeasure"))
    {
        setBeatsPerMeasure(json["beatsPerMeasure"].toInt());
    }
    if (json.contains("measures"))
    {
        setMeasures(json["measures"].toInt());
    }
}

} // namespace gizmotweak2

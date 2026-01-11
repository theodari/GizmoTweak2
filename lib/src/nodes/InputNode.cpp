#include "InputNode.h"
#include "core/Port.h"

#include <QFile>
#include <QDebug>
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
                qDebug() << "InputNode: Loaded" << _patternStack.size() << "patterns from" << path;
                break;
            }
        }
    }

    // If no patterns loaded, create default test patterns
    if (_patternStack.size() == 0)
    {
        qDebug() << "InputNode: No ILDA file found, creating default patterns";
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

} // namespace gizmotweak2

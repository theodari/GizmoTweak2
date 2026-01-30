#include "GroupNode.h"
#include "core/Port.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

namespace gizmotweak2
{

GroupNode::GroupNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Transform"));

    // Exactly 2 ratio inputs - accepts any ratio type
    addInput(QStringLiteral("ratio1"), Port::DataType::RatioAny);
    addInput(QStringLiteral("ratio2"), Port::DataType::RatioAny);

    // Output - outputs any ratio type
    addOutput(QStringLiteral("ratio"), Port::DataType::RatioAny);

    // Position pass-through
    addInput(QStringLiteral("center"), Port::DataType::Position);
    addOutput(QStringLiteral("center"), Port::DataType::Position);

    // Automation: Position track with positionX (0) and positionY (1)
    auto* positionTrack = createAutomationTrack(QStringLiteral("Position"), 2, QColor(70, 130, 180));
    positionTrack->setupParameter(0, -2.0, 2.0, _positionX, tr("Position X"), 100.0, QStringLiteral("%"));
    positionTrack->setupParameter(1, -2.0, 2.0, _positionY, tr("Position Y"), 100.0, QStringLiteral("%"));

    auto* scaleTrack = createAutomationTrack(QStringLiteral("Scale"), 2, QColor(60, 179, 113));
    scaleTrack->setupParameter(0, 0.01, 10.0, _scaleX, tr("Scale X"), 100.0, QStringLiteral("%"));
    scaleTrack->setupParameter(1, 0.01, 10.0, _scaleY, tr("Scale Y"), 100.0, QStringLiteral("%"));

    auto* rotationTrack = createAutomationTrack(QStringLiteral("Rotation"), 1, QColor(255, 140, 0));
    rotationTrack->setupParameter(0, -360.0, 360.0, _rotation, tr("Rotation"), 1.0, QStringLiteral("\u00B0"));
}

void GroupNode::setCompositionMode(CompositionMode mode)
{
    if (_compositionMode != mode)
    {
        _compositionMode = mode;
        emit compositionModeChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setSingleInputMode(bool enabled)
{
    if (_singleInputMode != enabled)
    {
        _singleInputMode = enabled;

        auto* input2 = inputAt(1);
        if (input2)
        {
            if (enabled)
            {
                // Disconnect input 2 if connected, then hide it
                if (input2->isConnected())
                {
                    requestDisconnectPort(input2);
                }
                input2->setVisible(false);
            }
            else
            {
                // Show input 2 again
                input2->setVisible(true);
            }
        }

        emit singleInputModeChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setPositionX(qreal x)
{
    if (!qFuzzyCompare(_positionX, x))
    {
        _positionX = x;
        emit positionXChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setPositionY(qreal y)
{
    if (!qFuzzyCompare(_positionY, y))
    {
        _positionY = y;
        emit positionYChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setScaleX(qreal sx)
{
    if (!qFuzzyCompare(_scaleX, sx))
    {
        _scaleX = sx;
        emit scaleXChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setScaleY(qreal sy)
{
    if (!qFuzzyCompare(_scaleY, sy))
    {
        _scaleY = sy;
        emit scaleYChanged();
        emitPropertyChanged();
    }
}

void GroupNode::setRotation(qreal r)
{
    if (!qFuzzyCompare(_rotation, r))
    {
        _rotation = r;
        emit rotationChanged();
        emitPropertyChanged();
    }
}

void GroupNode::transformCoordinates(qreal x, qreal y, qreal& outX, qreal& outY) const
{
    // Apply inverse geometric transformation to get local coordinates
    // Same as original GizmoTweak Group::getTweakRatio

    if (qFuzzyIsNull(_scaleX) || qFuzzyIsNull(_scaleY))
    {
        outX = 0.0;
        outY = 0.0;
        return;
    }

    qreal rotRad = qDegreesToRadians(_rotation);
    qreal myCos = std::cos(rotRad);
    qreal mySin = std::sin(rotRad);

    // Translate to local origin
    qreal x0 = x - _positionX;
    qreal y0 = y - _positionY;

    // Rotate and scale (inverse transformation)
    outX = (myCos * x0 - mySin * y0) / _scaleX;
    outY = (mySin * x0 + myCos * y0) / _scaleY;
}

qreal GroupNode::combine(const QList<qreal>& ratios) const
{
    // Exact formulas from original GizmoTweak Group::getTweakRatio
    qreal result = 0.0;
    int activeCount = ratios.size();

    switch (_compositionMode)
    {
    case CompositionMode::Normal:
        // Original formula: same signs take max/min, opposite signs sum
        result = 0.0;
        for (const qreal& tr : ratios)
        {
            if ((tr >= 0.0) && (result >= 0.0))
            {
                // Both positive: take max
                result = qMax(result, tr);
            }
            else if ((tr < 0.0) && (result < 0.0))
            {
                // Both negative: take min (most negative)
                result = qMin(result, tr);
            }
            else
            {
                // Opposite signs: sum
                result = result + tr;
            }
        }
        result = qBound(-1.0, result, 1.0);
        break;

    case CompositionMode::Max:
        if (activeCount == 0)
        {
            result = 0.0;
        }
        else
        {
            result = -1.0;
            for (const qreal& r : ratios)
            {
                result = qMax(result, r);
            }
        }
        break;

    case CompositionMode::Min:
        if (activeCount == 0)
        {
            result = 0.0;
        }
        else
        {
            result = 1.0;
            for (const qreal& r : ratios)
            {
                result = qMin(result, r);
            }
        }
        break;

    case CompositionMode::Sum:
        result = 0.0;
        for (const qreal& r : ratios)
        {
            result += r;
        }
        // Note: Original doesn't clamp Sum
        break;

    case CompositionMode::AbsDiff:
        if (activeCount == 0)
        {
            result = 0.0;
        }
        else if (activeCount >= 2)
        {
            result = ratios[0];
            for (int i = 1; i < ratios.size(); ++i)
            {
                result = qAbs(ratios[i] - result);
            }
        }
        else
        {
            result = ratios.first();
        }
        break;

    case CompositionMode::Diff:
        if (activeCount == 0)
        {
            result = 0.0;
        }
        else if (activeCount >= 2)
        {
            result = ratios[0];
            for (int i = 1; i < ratios.size(); ++i)
            {
                result = ratios[i] - result;
            }
        }
        else
        {
            result = ratios.first();
        }
        break;

    case CompositionMode::Product:
        if (activeCount == 0)
        {
            result = 0.0;
        }
        else
        {
            result = 1.0;
            for (const qreal& r : ratios)
            {
                result *= r;
            }
        }
        break;
    }

    return result;
}

QJsonObject GroupNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["compositionMode"] = static_cast<int>(_compositionMode);
    obj["singleInputMode"] = _singleInputMode;
    obj["positionX"] = _positionX;
    obj["positionY"] = _positionY;
    obj["scaleX"] = _scaleX;
    obj["scaleY"] = _scaleY;
    obj["rotation"] = _rotation;
    return obj;
}

void GroupNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("compositionMode"))
    {
        setCompositionMode(static_cast<CompositionMode>(json["compositionMode"].toInt()));
    }
    if (json.contains("singleInputMode"))
    {
        setSingleInputMode(json["singleInputMode"].toBool());
    }
    if (json.contains("positionX"))
    {
        setPositionX(json["positionX"].toDouble());
    }
    if (json.contains("positionY"))
    {
        setPositionY(json["positionY"].toDouble());
    }
    if (json.contains("scaleX"))
    {
        setScaleX(json["scaleX"].toDouble());
    }
    if (json.contains("scaleY"))
    {
        setScaleY(json["scaleY"].toDouble());
    }
    if (json.contains("rotation"))
    {
        setRotation(json["rotation"].toDouble());
    }
}

void GroupNode::syncToAnimatedValues(int timeMs)
{
    setPositionX(automatedValue(QStringLiteral("Position"), 0, timeMs));
    setPositionY(automatedValue(QStringLiteral("Position"), 1, timeMs));
    setScaleX(automatedValue(QStringLiteral("Scale"), 0, timeMs));
    setScaleY(automatedValue(QStringLiteral("Scale"), 1, timeMs));
    setRotation(automatedValue(QStringLiteral("Rotation"), 0, timeMs));
}

} // namespace gizmotweak2

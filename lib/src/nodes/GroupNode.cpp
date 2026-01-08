#include "GroupNode.h"
#include "core/Port.h"

#include <QtMath>
#include <algorithm>
#include <numeric>

namespace gizmotweak2
{

GroupNode::GroupNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Group"));

    // Initial ratio inputs (2 by default)
    for (int i = 0; i < _ratioInputCount; ++i)
    {
        addInput(QStringLiteral("ratio%1").arg(i + 1), Port::DataType::Ratio2D);
    }

    // Output
    addOutput(QStringLiteral("ratio"), Port::DataType::Ratio2D);
}

void GroupNode::setCompositionMode(CompositionMode mode)
{
    if (_compositionMode != mode)
    {
        _compositionMode = mode;
        emit compositionModeChanged();
    }
}

void GroupNode::setRatioInputCount(int count)
{
    count = qBound(2, count, 8);
    if (_ratioInputCount != count)
    {
        _ratioInputCount = count;
        updateInputPorts();
        emit ratioInputCountChanged();
    }
}

void GroupNode::setInvert(bool inv)
{
    if (_invert != inv)
    {
        _invert = inv;
        emit invertChanged();
    }
}

void GroupNode::updateInputPorts()
{
    // Clear existing input ports
    clearInputs();

    // Create new input ports
    for (int i = 0; i < _ratioInputCount; ++i)
    {
        addInput(QStringLiteral("ratio%1").arg(i + 1), Port::DataType::Ratio2D);
    }
}

qreal GroupNode::combine(const QList<qreal>& ratios) const
{
    if (ratios.isEmpty())
    {
        return _invert ? 1.0 : 0.0;
    }

    qreal result = 0.0;

    switch (_compositionMode)
    {
    case CompositionMode::Normal:
        result = ratios.first();
        break;

    case CompositionMode::Max:
        result = *std::max_element(ratios.begin(), ratios.end());
        break;

    case CompositionMode::Min:
        result = *std::min_element(ratios.begin(), ratios.end());
        break;

    case CompositionMode::Sum:
        result = std::accumulate(ratios.begin(), ratios.end(), 0.0);
        result = qMin(result, 1.0);
        break;

    case CompositionMode::Product:
        result = std::accumulate(ratios.begin(), ratios.end(), 1.0,
                                  std::multiplies<qreal>());
        break;

    case CompositionMode::Average:
        result = std::accumulate(ratios.begin(), ratios.end(), 0.0) / ratios.size();
        break;

    case CompositionMode::AbsDiff:
        if (ratios.size() >= 2)
        {
            result = qAbs(ratios[0] - ratios[1]);
        }
        else
        {
            result = ratios.first();
        }
        break;
    }

    // Apply inversion
    if (_invert)
    {
        result = 1.0 - result;
    }

    return qBound(0.0, result, 1.0);
}

QJsonObject GroupNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["compositionMode"] = static_cast<int>(_compositionMode);
    obj["ratioInputCount"] = _ratioInputCount;
    obj["invert"] = _invert;
    return obj;
}

void GroupNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("compositionMode"))
    {
        setCompositionMode(static_cast<CompositionMode>(json["compositionMode"].toInt()));
    }
    if (json.contains("ratioInputCount"))
    {
        setRatioInputCount(json["ratioInputCount"].toInt());
    }
    if (json.contains("invert"))
    {
        setInvert(json["invert"].toBool());
    }
}

} // namespace gizmotweak2

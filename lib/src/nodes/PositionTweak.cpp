#include "PositionTweak.h"
#include "core/Port.h"

namespace gizmotweak2
{

PositionTweak::PositionTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Position"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame);
    addInput(QStringLiteral("ratio"), Port::DataType::Ratio2D);

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void PositionTweak::setOffsetX(qreal x)
{
    if (!qFuzzyCompare(_offsetX, x))
    {
        _offsetX = x;
        emit offsetXChanged();
    }
}

void PositionTweak::setOffsetY(qreal y)
{
    if (!qFuzzyCompare(_offsetY, y))
    {
        _offsetY = y;
        emit offsetYChanged();
    }
}

void PositionTweak::setUseInitialPosition(bool use)
{
    if (_useInitialPosition != use)
    {
        _useInitialPosition = use;
        emit useInitialPositionChanged();
    }
}

void PositionTweak::setInitialX(qreal x)
{
    if (!qFuzzyCompare(_initialX, x))
    {
        _initialX = x;
        emit initialXChanged();
    }
}

void PositionTweak::setInitialY(qreal y)
{
    if (!qFuzzyCompare(_initialY, y))
    {
        _initialY = y;
        emit initialYChanged();
    }
}

QPointF PositionTweak::apply(qreal x, qreal y, qreal ratio) const
{
    qreal resultX = x;
    qreal resultY = y;

    if (_useInitialPosition)
    {
        // Interpolate from initial position to offset position
        resultX = x + ratio * (_offsetX - _initialX) + _initialX;
        resultY = y + ratio * (_offsetY - _initialY) + _initialY;
    }
    else
    {
        // Simple offset scaled by ratio
        resultX = x + _offsetX * ratio;
        resultY = y + _offsetY * ratio;
    }

    return QPointF(resultX, resultY);
}

} // namespace gizmotweak2

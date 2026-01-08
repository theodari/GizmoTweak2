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

QJsonObject PositionTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["offsetX"] = _offsetX;
    obj["offsetY"] = _offsetY;
    obj["useInitialPosition"] = _useInitialPosition;
    obj["initialX"] = _initialX;
    obj["initialY"] = _initialY;
    return obj;
}

void PositionTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("offsetX")) setOffsetX(json["offsetX"].toDouble());
    if (json.contains("offsetY")) setOffsetY(json["offsetY"].toDouble());
    if (json.contains("useInitialPosition")) setUseInitialPosition(json["useInitialPosition"].toBool());
    if (json.contains("initialX")) setInitialX(json["initialX"].toDouble());
    if (json.contains("initialY")) setInitialY(json["initialY"].toDouble());
}

} // namespace gizmotweak2

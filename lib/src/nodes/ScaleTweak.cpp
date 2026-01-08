#include "ScaleTweak.h"
#include "core/Port.h"

namespace gizmotweak2
{

ScaleTweak::ScaleTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Scale"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame);
    addInput(QStringLiteral("ratio"), Port::DataType::Ratio2D);

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void ScaleTweak::setScaleX(qreal sx)
{
    if (!qFuzzyCompare(_scaleX, sx))
    {
        _scaleX = sx;
        emit scaleXChanged();

        if (_uniform)
        {
            _scaleY = sx;
            emit scaleYChanged();
        }
    }
}

void ScaleTweak::setScaleY(qreal sy)
{
    if (!qFuzzyCompare(_scaleY, sy))
    {
        _scaleY = sy;
        emit scaleYChanged();

        if (_uniform)
        {
            _scaleX = sy;
            emit scaleXChanged();
        }
    }
}

void ScaleTweak::setUniform(bool u)
{
    if (_uniform != u)
    {
        _uniform = u;
        emit uniformChanged();

        if (_uniform)
        {
            _scaleY = _scaleX;
            emit scaleYChanged();
        }
    }
}

void ScaleTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
    }
}

void ScaleTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        emit centerYChanged();
    }
}

QPointF ScaleTweak::apply(qreal x, qreal y, qreal ratio) const
{
    // Interpolate scale factor based on ratio
    // ratio = 0 -> no scaling (factor = 1)
    // ratio = 1 -> full scaling (factor = scaleX/Y)
    qreal effectiveScaleX = 1.0 + (_scaleX - 1.0) * ratio;
    qreal effectiveScaleY = 1.0 + (_scaleY - 1.0) * ratio;

    // Scale around center point
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    qreal resultX = _centerX + dx * effectiveScaleX;
    qreal resultY = _centerY + dy * effectiveScaleY;

    return QPointF(resultX, resultY);
}

QJsonObject ScaleTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["scaleX"] = _scaleX;
    obj["scaleY"] = _scaleY;
    obj["uniform"] = _uniform;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    return obj;
}

void ScaleTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("scaleX")) setScaleX(json["scaleX"].toDouble());
    if (json.contains("scaleY")) setScaleY(json["scaleY"].toDouble());
    if (json.contains("uniform")) setUniform(json["uniform"].toBool());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
}

} // namespace gizmotweak2

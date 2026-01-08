#include "RotationTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

RotationTweak::RotationTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Rotation"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame);
    addInput(QStringLiteral("ratio"), Port::DataType::Ratio2D);

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void RotationTweak::setAngle(qreal a)
{
    if (!qFuzzyCompare(_angle, a))
    {
        _angle = a;
        emit angleChanged();
    }
}

void RotationTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
    }
}

void RotationTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        emit centerYChanged();
    }
}

QPointF RotationTweak::apply(qreal x, qreal y, qreal ratio) const
{
    // Effective angle based on ratio
    qreal effectiveAngle = _angle * ratio;
    qreal radians = qDegreesToRadians(effectiveAngle);

    // Translate to origin (center)
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;

    // Rotate
    qreal cosA = qCos(radians);
    qreal sinA = qSin(radians);

    qreal rotatedX = dx * cosA - dy * sinA;
    qreal rotatedY = dx * sinA + dy * cosA;

    // Translate back
    return QPointF(_centerX + rotatedX, _centerY + rotatedY);
}

QJsonObject RotationTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["angle"] = _angle;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    return obj;
}

void RotationTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("angle")) setAngle(json["angle"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
}

} // namespace gizmotweak2

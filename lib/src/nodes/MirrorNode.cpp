#include "MirrorNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

MirrorNode::MirrorNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Mirror"));

    // Input: 2D shape ratio
    addInput(QStringLiteral("shape"), Port::DataType::Ratio2D, true);  // Required

    // Output: mirrored shape
    addOutput(QStringLiteral("shape"), Port::DataType::Ratio2D);
}

void MirrorNode::setAxis(Axis a)
{
    if (_axis != a)
    {
        _axis = a;
        emit axisChanged();
        emitPropertyChanged();
    }
}

void MirrorNode::setCustomAngle(qreal angle)
{
    // Normalize angle to -180 to 180
    while (angle > 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;

    if (!qFuzzyCompare(_customAngle, angle))
    {
        _customAngle = angle;
        emit customAngleChanged();
        emitPropertyChanged();
    }
}

QPointF MirrorNode::mirror(qreal x, qreal y) const
{
    // Coordinate system is centered at (0, 0)
    qreal mirroredX, mirroredY;

    switch (_axis)
    {
    case Axis::Horizontal:
        // Mirror across vertical axis (flip X)
        mirroredX = -x;
        mirroredY = y;
        break;

    case Axis::Vertical:
        // Mirror across horizontal axis (flip Y)
        mirroredX = x;
        mirroredY = -y;
        break;

    case Axis::Diagonal45:
        // Mirror across +45° line (swap X and Y)
        mirroredX = y;
        mirroredY = x;
        break;

    case Axis::DiagonalMinus45:
        // Mirror across -45° line (swap and negate)
        mirroredX = -y;
        mirroredY = -x;
        break;

    case Axis::Custom:
        {
            // Mirror across line at custom angle through origin
            // Reflection matrix: [cos(2t), sin(2t); sin(2t), -cos(2t)]
            qreal theta = qDegreesToRadians(_customAngle);
            qreal cos2t = qCos(2.0 * theta);
            qreal sin2t = qSin(2.0 * theta);

            mirroredX = x * cos2t + y * sin2t;
            mirroredY = x * sin2t - y * cos2t;
        }
        break;

    default:
        mirroredX = x;
        mirroredY = y;
        break;
    }

    return QPointF(mirroredX, mirroredY);
}

QJsonObject MirrorNode::propertiesToJson() const
{
    QJsonObject obj;
    obj["axis"] = static_cast<int>(_axis);
    obj["customAngle"] = _customAngle;
    return obj;
}

void MirrorNode::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("axis"))
    {
        setAxis(static_cast<Axis>(json["axis"].toInt()));
    }
    if (json.contains("customAngle"))
    {
        setCustomAngle(json["customAngle"].toDouble());
    }
}

} // namespace gizmotweak2

#include "PositionTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

PositionTweak::PositionTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Position"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void PositionTweak::setOffsetX(qreal x)
{
    if (!qFuzzyCompare(_offsetX, x))
    {
        _offsetX = x;
        emit offsetXChanged();
        emitPropertyChanged();
    }
}

void PositionTweak::setOffsetY(qreal y)
{
    if (!qFuzzyCompare(_offsetY, y))
    {
        _offsetY = y;
        emit offsetYChanged();
        emitPropertyChanged();
    }
}

void PositionTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;

        // Show/hide ratio port based on followGizmo
        auto* ratioPort = inputAt(1);  // ratio port is at index 1
        if (ratioPort)
        {
            ratioPort->setVisible(follow);
            if (!follow && ratioPort->isConnected())
            {
                emit requestDisconnectPort(ratioPort);
            }
        }

        emit followGizmoChanged();
        emitPropertyChanged();
    }
}

QPointF PositionTweak::apply(qreal x, qreal y, qreal ratio) const
{
    return QPointF(x + _offsetX * ratio, y + _offsetY * ratio);
}

QJsonObject PositionTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["offsetX"] = _offsetX;
    obj["offsetY"] = _offsetY;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void PositionTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("offsetX")) setOffsetX(json["offsetX"].toDouble());
    if (json.contains("offsetY")) setOffsetY(json["offsetY"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

} // namespace gizmotweak2

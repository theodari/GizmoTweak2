#include "ScaleTweak.h"
#include "core/Port.h"

namespace gizmotweak2
{

ScaleTweak::ScaleTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Scale"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

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
        emitPropertyChanged();
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
        emitPropertyChanged();
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
        emitPropertyChanged();
    }
}

void ScaleTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void ScaleTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void ScaleTweak::setCrossOver(bool co)
{
    if (_crossOver != co)
    {
        _crossOver = co;
        emit crossOverChanged();
        emitPropertyChanged();
    }
}

void ScaleTweak::setFollowGizmo(bool follow)
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

QPointF ScaleTweak::apply(qreal x, qreal y, qreal ratioX, qreal ratioY,
                          qreal /*gizmoX*/, qreal /*gizmoY*/) const
{
    // Determine which ratio to use for each axis
    qreal rX, rY;
    if (_crossOver)
    {
        // CrossOver: X uses Y's ratio, Y uses X's ratio
        rX = ratioY;
        rY = ratioX;
    }
    else
    {
        rX = ratioX;
        rY = ratioY;
    }

    // Interpolate scale factor based on ratio
    // ratio = 0 -> no scaling (factor = 1)
    // ratio = 1 -> full scaling (factor = scaleX/Y)
    qreal effectiveScaleX = 1.0 + (_scaleX - 1.0) * rX;
    qreal effectiveScaleY = 1.0 + (_scaleY - 1.0) * rY;

    // Always use centerX/centerY as scale center
    // (followGizmo only controls whether ratio comes from gizmo or is 1.0)

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
    obj["crossOver"] = _crossOver;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void ScaleTweak::propertiesFromJson(const QJsonObject& json)
{
    // Load uniform first to prevent scale sync during scaleX/Y loading
    if (json.contains("uniform")) setUniform(json["uniform"].toBool());
    if (json.contains("scaleX")) setScaleX(json["scaleX"].toDouble());
    if (json.contains("scaleY")) setScaleY(json["scaleY"].toDouble());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("crossOver")) setCrossOver(json["crossOver"].toBool());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

} // namespace gizmotweak2

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

    // Automation: Scale track with scaleX (0) and scaleY (1)
    auto* scaleTrack = createAutomationTrack(QStringLiteral("Scale"), 2, QColor(60, 179, 113));
    scaleTrack->setupParameter(0, 0.01, 5.0, _scaleX, tr("Scale X"), 100.0, QStringLiteral("%"));
    scaleTrack->setupParameter(1, 0.01, 5.0, _scaleY, tr("Scale Y"), 100.0, QStringLiteral("%"));

    auto* centerTrack = createAutomationTrack(QStringLiteral("Center"), 2, QColor(186, 85, 211));
    centerTrack->setupParameter(0, -1.0, 1.0, _centerX, tr("Center X"), 100.0, QStringLiteral("%"));
    centerTrack->setupParameter(1, -1.0, 1.0, _centerY, tr("Center Y"), 100.0, QStringLiteral("%"));
}

void ScaleTweak::setScaleX(qreal sx)
{
    if (!qFuzzyCompare(_scaleX, sx))
    {
        _scaleX = sx;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Scale"));
        if (track) track->setInitialValue(0, sx);
        emit scaleXChanged();

        if (_uniform)
        {
            _scaleY = sx;
            if (track) track->setInitialValue(1, sx);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Scale"));
        if (track) track->setInitialValue(1, sy);
        emit scaleYChanged();

        if (_uniform)
        {
            _scaleX = sy;
            if (track) track->setInitialValue(0, sy);
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
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(0, cx);
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void ScaleTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Center"));
        if (track) track->setInitialValue(1, cy);
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

void ScaleTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* scaleTrack = automationTrack(QStringLiteral("Scale"));
    if (scaleTrack && scaleTrack->isAutomated())
    {
        _scaleX = scaleTrack->timedValue(timeMs, 0);
        _scaleY = scaleTrack->timedValue(timeMs, 1);
    }

    auto* centerTrack = automationTrack(QStringLiteral("Center"));
    if (centerTrack && centerTrack->isAutomated())
    {
        _centerX = centerTrack->timedValue(timeMs, 0);
        _centerY = centerTrack->timedValue(timeMs, 1);
    }
}

} // namespace gizmotweak2

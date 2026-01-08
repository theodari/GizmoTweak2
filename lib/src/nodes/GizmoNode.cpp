#include "GizmoNode.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

GizmoNode::GizmoNode(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Gizmo"));
    addOutput(QStringLiteral("ratio"), Port::DataType::Ratio2D);
}

void GizmoNode::setCenterX(qreal x)
{
    if (!qFuzzyCompare(_centerX, x))
    {
        _centerX = x;
        emit centerXChanged();
    }
}

void GizmoNode::setCenterY(qreal y)
{
    if (!qFuzzyCompare(_centerY, y))
    {
        _centerY = y;
        emit centerYChanged();
    }
}

void GizmoNode::setRadius(qreal r)
{
    r = qMax(0.001, r);
    if (!qFuzzyCompare(_radius, r))
    {
        _radius = r;
        emit radiusChanged();
    }
}

void GizmoNode::setFalloff(qreal f)
{
    f = qBound(0.0, f, 1.0);
    if (!qFuzzyCompare(_falloff, f))
    {
        _falloff = f;
        emit falloffChanged();
    }
}

qreal GizmoNode::computeRatio(qreal x, qreal y) const
{
    // Distance from center
    qreal dx = x - _centerX;
    qreal dy = y - _centerY;
    qreal distance = qSqrt(dx * dx + dy * dy);

    // Inside inner radius: full effect
    qreal innerRadius = _radius * (1.0 - _falloff);
    if (distance <= innerRadius)
    {
        return 1.0;
    }

    // Outside outer radius: no effect
    if (distance >= _radius)
    {
        return 0.0;
    }

    // Falloff zone: smooth interpolation
    qreal t = (distance - innerRadius) / (_radius - innerRadius);
    // Smooth step for nicer falloff
    return 1.0 - (t * t * (3.0 - 2.0 * t));
}

} // namespace gizmotweak2

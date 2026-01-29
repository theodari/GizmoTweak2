#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class PolarTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal expansion READ expansion WRITE setExpansion NOTIFY expansionChanged)
    Q_PROPERTY(qreal ringRadius READ ringRadius WRITE setRingRadius NOTIFY ringRadiusChanged)
    Q_PROPERTY(qreal ringScale READ ringScale WRITE setRingScale NOTIFY ringScaleChanged)
    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(bool crossOver READ crossOver WRITE setCrossOver NOTIFY crossOverChanged)
    Q_PROPERTY(bool targetted READ targetted WRITE setTargetted NOTIFY targettedChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit PolarTweak(QObject* parent = nullptr);
    ~PolarTweak() override = default;

    QString type() const override { return QStringLiteral("PolarTweak"); }
    Category category() const override { return Category::Tweak; }

    // Expansion - radial scaling (positive = expand, negative = contract)
    qreal expansion() const { return _expansion; }
    void setExpansion(qreal e);

    // Ring effect - creates concentric ring distortion
    qreal ringRadius() const { return _ringRadius; }
    void setRingRadius(qreal r);

    qreal ringScale() const { return _ringScale; }
    void setRingScale(qreal s);

    // Center of polar transformation
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal cx);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal cy);

    // CrossOver - X uses Y ratio component and vice versa
    bool crossOver() const { return _crossOver; }
    void setCrossOver(bool co);

    // Targetted - expansion moves towards center rather than outward
    bool targetted() const { return _targetted; }
    void setTargetted(bool t);

    // Follow Gizmo - use connected Gizmo center
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratioX, qreal ratioY,
                              qreal gizmoX = 0.0, qreal gizmoY = 0.0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void expansionChanged();
    void ringRadiusChanged();
    void ringScaleChanged();
    void centerXChanged();
    void centerYChanged();
    void crossOverChanged();
    void targettedChanged();
    void followGizmoChanged();

private:
    qreal _expansion{0.0};
    qreal _ringRadius{0.5};
    qreal _ringScale{0.0};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    bool _crossOver{false};
    bool _targetted{false};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

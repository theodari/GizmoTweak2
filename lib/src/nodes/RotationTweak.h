#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class RotationTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit RotationTweak(QObject* parent = nullptr);
    ~RotationTweak() override = default;

    QString type() const override { return QStringLiteral("RotationTweak"); }
    Category category() const override { return Category::Tweak; }

    // Rotation angle in degrees
    qreal angle() const { return _angle; }
    void setAngle(qreal a);

    // Center of rotation
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal cx);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal cy);

    // Follow Gizmo - use connected Gizmo center as rotation center
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio,
                              qreal gizmoX = 0.0, qreal gizmoY = 0.0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation sync
    void syncToAnimatedValues(int timeMs) override;

signals:
    void angleChanged();
    void centerXChanged();
    void centerYChanged();
    void followGizmoChanged();

private:
    qreal _angle{0.0};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

#pragma once

#include "core/Node.h"
#include <QPointF>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

/**
 * @brief Applies a cylindrical/spherical distortion that "rounds" the frame
 *
 * Creates effects similar to wrapping the image around a cylinder.
 * The amount controls how much the frame curves, while other parameters
 * control the center position, tightening, and radial behavior.
 */
class RounderTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(qreal verticalShift READ verticalShift WRITE setVerticalShift NOTIFY verticalShiftChanged)
    Q_PROPERTY(qreal horizontalShift READ horizontalShift WRITE setHorizontalShift NOTIFY horizontalShiftChanged)
    Q_PROPERTY(qreal tighten READ tighten WRITE setTighten NOTIFY tightenChanged)
    Q_PROPERTY(qreal radialResize READ radialResize WRITE setRadialResize NOTIFY radialResizeChanged)
    Q_PROPERTY(qreal radialShift READ radialShift WRITE setRadialShift NOTIFY radialShiftChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit RounderTweak(QObject* parent = nullptr);
    ~RounderTweak() override = default;

    QString type() const override { return QStringLiteral("RounderTweak"); }
    Category category() const override { return Category::Tweak; }

    // Properties
    qreal amount() const { return _amount; }
    void setAmount(qreal value);

    qreal verticalShift() const { return _verticalShift; }
    void setVerticalShift(qreal value);

    qreal horizontalShift() const { return _horizontalShift; }
    void setHorizontalShift(qreal value);

    qreal tighten() const { return _tighten; }
    void setTighten(qreal value);

    qreal radialResize() const { return _radialResize; }
    void setRadialResize(qreal value);

    qreal radialShift() const { return _radialShift; }
    void setRadialShift(qreal value);

    // Follow gizmo - use gizmo's ratio when true, full effect when false
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply the rounder effect to a coordinate
    // ratio modulates all parameters (0 = no effect, 1 = full effect)
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void amountChanged();
    void verticalShiftChanged();
    void horizontalShiftChanged();
    void tightenChanged();
    void radialResizeChanged();
    void radialShiftChanged();
    void followGizmoChanged();

private:
    qreal _amount{0.0};
    qreal _verticalShift{0.0};
    qreal _horizontalShift{0.0};
    qreal _tighten{0.0};
    qreal _radialResize{1.0};
    qreal _radialShift{0.0};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

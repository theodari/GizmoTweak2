#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class ScaleTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal scaleX READ scaleX WRITE setScaleX NOTIFY scaleXChanged)
    Q_PROPERTY(qreal scaleY READ scaleY WRITE setScaleY NOTIFY scaleYChanged)
    Q_PROPERTY(bool uniform READ uniform WRITE setUniform NOTIFY uniformChanged)
    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(bool crossOver READ crossOver WRITE setCrossOver NOTIFY crossOverChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit ScaleTweak(QObject* parent = nullptr);
    ~ScaleTweak() override = default;

    QString type() const override { return QStringLiteral("ScaleTweak"); }
    Category category() const override { return Category::Tweak; }

    // Scale properties
    qreal scaleX() const { return _scaleX; }
    void setScaleX(qreal sx);

    qreal scaleY() const { return _scaleY; }
    void setScaleY(qreal sy);

    bool uniform() const { return _uniform; }
    void setUniform(bool u);

    // Center of scaling
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal cx);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal cy);

    // CrossOver - X is scaled by Y ratio component and vice versa
    bool crossOver() const { return _crossOver; }
    void setCrossOver(bool co);

    // Follow Gizmo - use connected Gizmo center as scale center
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply tweak to a point
    // ratioX and ratioY are 2D ratio components (for crossover)
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratioX, qreal ratioY,
                              qreal gizmoX = 0.0, qreal gizmoY = 0.0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void scaleXChanged();
    void scaleYChanged();
    void uniformChanged();
    void centerXChanged();
    void centerYChanged();
    void crossOverChanged();
    void followGizmoChanged();

private:
    qreal _scaleX{1.0};
    qreal _scaleY{1.0};
    bool _uniform{true};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    bool _crossOver{false};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

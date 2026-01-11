#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class PositionTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal offsetX READ offsetX WRITE setOffsetX NOTIFY offsetXChanged)
    Q_PROPERTY(qreal offsetY READ offsetY WRITE setOffsetY NOTIFY offsetYChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit PositionTweak(QObject* parent = nullptr);
    ~PositionTweak() override = default;

    QString type() const override { return QStringLiteral("PositionTweak"); }
    Category category() const override { return Category::Tweak; }

    qreal offsetX() const { return _offsetX; }
    void setOffsetX(qreal x);

    qreal offsetY() const { return _offsetY; }
    void setOffsetY(qreal y);

    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void offsetXChanged();
    void offsetYChanged();
    void followGizmoChanged();

private:
    qreal _offsetX{0.0};
    qreal _offsetY{0.0};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

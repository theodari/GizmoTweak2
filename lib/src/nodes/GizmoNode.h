#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class GizmoNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qreal falloff READ falloff WRITE setFalloff NOTIFY falloffChanged)

public:
    explicit GizmoNode(QObject* parent = nullptr);
    ~GizmoNode() override = default;

    QString type() const override { return QStringLiteral("Gizmo"); }
    Category category() const override { return Category::Shape; }

    // Properties
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal x);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal y);

    qreal radius() const { return _radius; }
    void setRadius(qreal r);

    qreal falloff() const { return _falloff; }
    void setFalloff(qreal f);

    // Compute ratio at position
    Q_INVOKABLE qreal computeRatio(qreal x, qreal y) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void centerXChanged();
    void centerYChanged();
    void radiusChanged();
    void falloffChanged();

private:
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    qreal _radius{0.5};
    qreal _falloff{0.2};
};

} // namespace gizmotweak2

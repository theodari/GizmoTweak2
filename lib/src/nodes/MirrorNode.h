#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class MirrorNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Axis axis READ axis WRITE setAxis NOTIFY axisChanged)
    Q_PROPERTY(qreal customAngle READ customAngle WRITE setCustomAngle NOTIFY customAngleChanged)

public:
    enum class Axis
    {
        Horizontal,     // Mirror left <-> right
        Vertical,       // Mirror top <-> bottom
        Diagonal45,     // Mirror along +45° line
        DiagonalMinus45,// Mirror along -45° line
        Custom          // Mirror along custom angle
    };
    Q_ENUM(Axis)

    explicit MirrorNode(QObject* parent = nullptr);
    ~MirrorNode() override = default;

    QString type() const override { return QStringLiteral("Mirror"); }
    Category category() const override { return Category::Utility; }

    Axis axis() const { return _axis; }
    void setAxis(Axis a);

    qreal customAngle() const { return _customAngle; }
    void setCustomAngle(qreal angle);

    // Compute mirrored position
    // Returns the mirrored (x, y) coordinates
    Q_INVOKABLE QPointF mirror(qreal x, qreal y) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void axisChanged();
    void customAngleChanged();

private:
    Axis _axis{Axis::Horizontal};
    qreal _customAngle{0.0};  // In degrees, used when axis == Custom
};

} // namespace gizmotweak2

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

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio) const;

signals:
    void scaleXChanged();
    void scaleYChanged();
    void uniformChanged();
    void centerXChanged();
    void centerYChanged();

private:
    qreal _scaleX{1.0};
    qreal _scaleY{1.0};
    bool _uniform{true};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
};

} // namespace gizmotweak2

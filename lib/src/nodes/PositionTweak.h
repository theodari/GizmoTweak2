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
    Q_PROPERTY(bool useInitialPosition READ useInitialPosition WRITE setUseInitialPosition NOTIFY useInitialPositionChanged)
    Q_PROPERTY(qreal initialX READ initialX WRITE setInitialX NOTIFY initialXChanged)
    Q_PROPERTY(qreal initialY READ initialY WRITE setInitialY NOTIFY initialYChanged)

public:
    explicit PositionTweak(QObject* parent = nullptr);
    ~PositionTweak() override = default;

    QString type() const override { return QStringLiteral("PositionTweak"); }
    Category category() const override { return Category::Tweak; }

    // Offset properties
    qreal offsetX() const { return _offsetX; }
    void setOffsetX(qreal x);

    qreal offsetY() const { return _offsetY; }
    void setOffsetY(qreal y);

    // Initial position mode
    bool useInitialPosition() const { return _useInitialPosition; }
    void setUseInitialPosition(bool use);

    qreal initialX() const { return _initialX; }
    void setInitialX(qreal x);

    qreal initialY() const { return _initialY; }
    void setInitialY(qreal y);

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio) const;

signals:
    void offsetXChanged();
    void offsetYChanged();
    void useInitialPositionChanged();
    void initialXChanged();
    void initialYChanged();

private:
    qreal _offsetX{0.0};
    qreal _offsetY{0.0};
    bool _useInitialPosition{false};
    qreal _initialX{0.0};
    qreal _initialY{0.0};
};

} // namespace gizmotweak2

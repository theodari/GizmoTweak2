#pragma once

#include "core/Node.h"
#include <QList>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class GroupNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum class CompositionMode
    {
        Normal,     // Complex combination (original formula)
        Max,        // Maximum of all inputs
        Min,        // Minimum of all inputs
        Sum,        // Sum (unclamped)
        AbsDiff,    // Absolute difference
        Diff,       // Difference (subtraction)
        Product     // Multiplication
    };
    Q_ENUM(CompositionMode)

    Q_PROPERTY(CompositionMode compositionMode READ compositionMode WRITE setCompositionMode NOTIFY compositionModeChanged)
    Q_PROPERTY(bool singleInputMode READ singleInputMode WRITE setSingleInputMode NOTIFY singleInputModeChanged)

    // Geometric controls (like GizmoNode)
    Q_PROPERTY(qreal positionX READ positionX WRITE setPositionX NOTIFY positionXChanged)
    Q_PROPERTY(qreal positionY READ positionY WRITE setPositionY NOTIFY positionYChanged)
    Q_PROPERTY(qreal scaleX READ scaleX WRITE setScaleX NOTIFY scaleXChanged)
    Q_PROPERTY(qreal scaleY READ scaleY WRITE setScaleY NOTIFY scaleYChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    explicit GroupNode(QObject* parent = nullptr);
    ~GroupNode() override = default;

    QString type() const override { return QStringLiteral("Transform"); }
    Category category() const override { return Category::Shape; }

    // Composition mode
    CompositionMode compositionMode() const { return _compositionMode; }
    void setCompositionMode(CompositionMode mode);

    // Single input mode (bypass combination)
    bool singleInputMode() const { return _singleInputMode; }
    void setSingleInputMode(bool enabled);

    // Geometric properties
    qreal positionX() const { return _positionX; }
    void setPositionX(qreal x);

    qreal positionY() const { return _positionY; }
    void setPositionY(qreal y);

    qreal scaleX() const { return _scaleX; }
    void setScaleX(qreal sx);

    qreal scaleY() const { return _scaleY; }
    void setScaleY(qreal sy);

    qreal rotation() const { return _rotation; }
    void setRotation(qreal r);

    // Transform world coordinates to local coordinates
    void transformCoordinates(qreal x, qreal y, qreal& outX, qreal& outY) const;

    // Combine multiple ratios according to composition mode
    // This contains the exact formulas from original GizmoTweak
    Q_INVOKABLE qreal combine(const QList<qreal>& ratios) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void compositionModeChanged();
    void singleInputModeChanged();
    void positionXChanged();
    void positionYChanged();
    void scaleXChanged();
    void scaleYChanged();
    void rotationChanged();

private:
    CompositionMode _compositionMode{CompositionMode::Normal};
    bool _singleInputMode{false};

    // Geometric controls
    qreal _positionX{0.0};
    qreal _positionY{0.0};
    qreal _scaleX{1.0};
    qreal _scaleY{1.0};
    qreal _rotation{0.0};  // In degrees
};

} // namespace gizmotweak2

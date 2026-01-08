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
        Normal,     // First input only
        Max,        // Maximum of all inputs
        Min,        // Minimum of all inputs
        Sum,        // Sum (clamped to 1)
        Product,    // Multiplication
        Average,    // Average of all inputs
        AbsDiff     // Absolute difference (2 inputs)
    };
    Q_ENUM(CompositionMode)

    Q_PROPERTY(CompositionMode compositionMode READ compositionMode WRITE setCompositionMode NOTIFY compositionModeChanged)
    Q_PROPERTY(int inputCount READ ratioInputCount WRITE setRatioInputCount NOTIFY inputCountChanged)
    Q_PROPERTY(bool invert READ invert WRITE setInvert NOTIFY invertChanged)

public:
    explicit GroupNode(QObject* parent = nullptr);
    ~GroupNode() override = default;

    QString type() const override { return QStringLiteral("Group"); }
    Category category() const override { return Category::Shape; }

    // Properties
    CompositionMode compositionMode() const { return _compositionMode; }
    void setCompositionMode(CompositionMode mode);

    int ratioInputCount() const { return _ratioInputCount; }
    void setRatioInputCount(int count);

    bool invert() const { return _invert; }
    void setInvert(bool inv);

    // Combine multiple ratios
    Q_INVOKABLE qreal combine(const QList<qreal>& ratios) const;

signals:
    void compositionModeChanged();
    void inputCountChanged();
    void invertChanged();

private:
    void updateInputPorts();

    CompositionMode _compositionMode{CompositionMode::Max};
    int _ratioInputCount{2};
    bool _invert{false};
};

} // namespace gizmotweak2

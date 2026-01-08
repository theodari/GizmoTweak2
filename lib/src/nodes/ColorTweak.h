#pragma once

#include "core/Node.h"
#include <QColor>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class ColorTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum class Mode
    {
        Tint,       // Blend towards target color
        Multiply,   // Multiply by target color
        Add,        // Add target color
        Replace     // Replace with target color
    };
    Q_ENUM(Mode)

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(bool affectRed READ affectRed WRITE setAffectRed NOTIFY affectRedChanged)
    Q_PROPERTY(bool affectGreen READ affectGreen WRITE setAffectGreen NOTIFY affectGreenChanged)
    Q_PROPERTY(bool affectBlue READ affectBlue WRITE setAffectBlue NOTIFY affectBlueChanged)

public:
    explicit ColorTweak(QObject* parent = nullptr);
    ~ColorTweak() override = default;

    QString type() const override { return QStringLiteral("ColorTweak"); }
    Category category() const override { return Category::Tweak; }

    // Properties
    Mode mode() const { return _mode; }
    void setMode(Mode m);

    QColor color() const { return _color; }
    void setColor(const QColor& c);

    qreal intensity() const { return _intensity; }
    void setIntensity(qreal i);

    bool affectRed() const { return _affectRed; }
    void setAffectRed(bool affect);

    bool affectGreen() const { return _affectGreen; }
    void setAffectGreen(bool affect);

    bool affectBlue() const { return _affectBlue; }
    void setAffectBlue(bool affect);

    // Apply tweak to a color
    Q_INVOKABLE QColor apply(const QColor& input, qreal ratio) const;

signals:
    void modeChanged();
    void colorChanged();
    void intensityChanged();
    void affectRedChanged();
    void affectGreenChanged();
    void affectBlueChanged();

private:
    Mode _mode{Mode::Tint};
    QColor _color{Qt::white};
    qreal _intensity{1.0};
    bool _affectRed{true};
    bool _affectGreen{true};
    bool _affectBlue{true};
};

} // namespace gizmotweak2

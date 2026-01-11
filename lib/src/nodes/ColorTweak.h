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

    // Filter properties - only apply effect to samples within these color ranges
    Q_PROPERTY(qreal filterRedMin READ filterRedMin WRITE setFilterRedMin NOTIFY filterRedMinChanged)
    Q_PROPERTY(qreal filterRedMax READ filterRedMax WRITE setFilterRedMax NOTIFY filterRedMaxChanged)
    Q_PROPERTY(qreal filterGreenMin READ filterGreenMin WRITE setFilterGreenMin NOTIFY filterGreenMinChanged)
    Q_PROPERTY(qreal filterGreenMax READ filterGreenMax WRITE setFilterGreenMax NOTIFY filterGreenMaxChanged)
    Q_PROPERTY(qreal filterBlueMin READ filterBlueMin WRITE setFilterBlueMin NOTIFY filterBlueMinChanged)
    Q_PROPERTY(qreal filterBlueMax READ filterBlueMax WRITE setFilterBlueMax NOTIFY filterBlueMaxChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

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

    // Filter getters/setters
    qreal filterRedMin() const { return _filterRedMin; }
    void setFilterRedMin(qreal value);

    qreal filterRedMax() const { return _filterRedMax; }
    void setFilterRedMax(qreal value);

    qreal filterGreenMin() const { return _filterGreenMin; }
    void setFilterGreenMin(qreal value);

    qreal filterGreenMax() const { return _filterGreenMax; }
    void setFilterGreenMax(qreal value);

    qreal filterBlueMin() const { return _filterBlueMin; }
    void setFilterBlueMin(qreal value);

    qreal filterBlueMax() const { return _filterBlueMax; }
    void setFilterBlueMax(qreal value);

    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Check if a color passes the filter
    Q_INVOKABLE bool passesFilter(qreal r, qreal g, qreal b) const;

    // Apply tweak to a color
    Q_INVOKABLE QColor apply(const QColor& input, qreal ratio) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void modeChanged();
    void colorChanged();
    void intensityChanged();
    void affectRedChanged();
    void affectGreenChanged();
    void affectBlueChanged();
    void filterRedMinChanged();
    void filterRedMaxChanged();
    void filterGreenMinChanged();
    void filterGreenMaxChanged();
    void filterBlueMinChanged();
    void filterBlueMaxChanged();
    void followGizmoChanged();

private:
    Mode _mode{Mode::Tint};
    QColor _color{Qt::white};
    qreal _intensity{1.0};
    bool _affectRed{true};
    bool _affectGreen{true};
    bool _affectBlue{true};

    // Filter ranges (0.0 to 1.0)
    qreal _filterRedMin{0.0};
    qreal _filterRedMax{1.0};
    qreal _filterGreenMin{0.0};
    qreal _filterGreenMax{1.0};
    qreal _filterBlueMin{0.0};
    qreal _filterBlueMax{1.0};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

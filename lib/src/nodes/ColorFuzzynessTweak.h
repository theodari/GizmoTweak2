#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class ColorFuzzynessTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(bool affectRed READ affectRed WRITE setAffectRed NOTIFY affectRedChanged)
    Q_PROPERTY(bool affectGreen READ affectGreen WRITE setAffectGreen NOTIFY affectGreenChanged)
    Q_PROPERTY(bool affectBlue READ affectBlue WRITE setAffectBlue NOTIFY affectBlueChanged)
    Q_PROPERTY(int seed READ seed WRITE setSeed NOTIFY seedChanged)
    Q_PROPERTY(bool useSeed READ useSeed WRITE setUseSeed NOTIFY useSeedChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit ColorFuzzynessTweak(QObject* parent = nullptr);
    ~ColorFuzzynessTweak() override = default;

    QString type() const override { return QStringLiteral("ColorFuzzynessTweak"); }
    Category category() const override { return Category::Tweak; }

    // Properties
    qreal amount() const { return _amount; }
    void setAmount(qreal a);

    bool affectRed() const { return _affectRed; }
    void setAffectRed(bool affect);

    bool affectGreen() const { return _affectGreen; }
    void setAffectGreen(bool affect);

    bool affectBlue() const { return _affectBlue; }
    void setAffectBlue(bool affect);

    int seed() const { return _seed; }
    void setSeed(int s);

    bool useSeed() const { return _useSeed; }
    void setUseSeed(bool use);

    // Follow gizmo - use gizmo's ratio when true, full effect when false
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply fuzzyness to a color
    Q_INVOKABLE QColor apply(const QColor& input, qreal ratio, int sampleIndex = 0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void amountChanged();
    void affectRedChanged();
    void affectGreenChanged();
    void affectBlueChanged();
    void seedChanged();
    void useSeedChanged();
    void followGizmoChanged();

private:
    qreal _amount{0.1};
    bool _affectRed{true};
    bool _affectGreen{true};
    bool _affectBlue{true};
    int _seed{0};
    bool _useSeed{false};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

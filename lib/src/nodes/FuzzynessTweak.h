#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class FuzzynessTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(bool affectX READ affectX WRITE setAffectX NOTIFY affectXChanged)
    Q_PROPERTY(bool affectY READ affectY WRITE setAffectY NOTIFY affectYChanged)
    Q_PROPERTY(int seed READ seed WRITE setSeed NOTIFY seedChanged)
    Q_PROPERTY(bool useSeed READ useSeed WRITE setUseSeed NOTIFY useSeedChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit FuzzynessTweak(QObject* parent = nullptr);
    ~FuzzynessTweak() override = default;

    QString type() const override { return QStringLiteral("FuzzynessTweak"); }
    Category category() const override { return Category::Tweak; }

    // Properties
    qreal amount() const { return _amount; }
    void setAmount(qreal a);

    bool affectX() const { return _affectX; }
    void setAffectX(bool affect);

    bool affectY() const { return _affectY; }
    void setAffectY(bool affect);

    int seed() const { return _seed; }
    void setSeed(int s);

    bool useSeed() const { return _useSeed; }
    void setUseSeed(bool use);

    // Follow gizmo - use gizmo's ratio when true, full effect when false
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply fuzzyness to a position
    Q_INVOKABLE QPointF apply(const QPointF& input, qreal ratio, int sampleIndex = 0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void amountChanged();
    void affectXChanged();
    void affectYChanged();
    void seedChanged();
    void useSeedChanged();
    void followGizmoChanged();

private:
    qreal _amount{0.1};
    bool _affectX{true};
    bool _affectY{true};
    int _seed{0};
    bool _useSeed{false};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class SplitTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal splitThreshold READ splitThreshold WRITE setSplitThreshold NOTIFY splitThresholdChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit SplitTweak(QObject* parent = nullptr);
    ~SplitTweak() override = default;

    QString type() const override { return QStringLiteral("SplitTweak"); }
    Category category() const override { return Category::Tweak; }

    qreal splitThreshold() const { return _splitThreshold; }
    void setSplitThreshold(qreal threshold);

    // Follow gizmo - use gizmo's ratio when true, full effect when false
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Check if a segment should be split (distance > threshold * (1 - ratio))
    // Returns true if segment exceeds the effective threshold
    Q_INVOKABLE bool shouldSplit(qreal x1, qreal y1, qreal x2, qreal y2, qreal ratio) const;

    // Get effective threshold based on ratio (lower ratio = higher threshold)
    Q_INVOKABLE qreal effectiveThreshold(qreal ratio) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void splitThresholdChanged();
    void followGizmoChanged();

private:
    qreal _splitThreshold{0.5};  // Default threshold in normalized coordinates
    bool _followGizmo{true};
};

} // namespace gizmotweak2

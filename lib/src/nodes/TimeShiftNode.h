#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class TimeShiftNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged)
    Q_PROPERTY(qreal loopDuration READ loopDuration WRITE setLoopDuration NOTIFY loopDurationChanged)

public:
    explicit TimeShiftNode(QObject* parent = nullptr);
    ~TimeShiftNode() override = default;

    QString type() const override { return QStringLiteral("TimeShift"); }
    Category category() const override { return Category::Utility; }

    // Delay in seconds (positive = retard, negative = avance)
    qreal delay() const { return _delay; }
    void setDelay(qreal d);

    // Time scale factor (1.0 = normal, 2.0 = double speed, 0.5 = half speed)
    qreal scale() const { return _scale; }
    void setScale(qreal s);

    // Loop mode
    bool loop() const { return _loop; }
    void setLoop(bool l);

    qreal loopDuration() const { return _loopDuration; }
    void setLoopDuration(qreal duration);

    // Compute shifted time
    Q_INVOKABLE qreal shiftTime(qreal currentTime) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

signals:
    void delayChanged();
    void scaleChanged();
    void loopChanged();
    void loopDurationChanged();

private:
    qreal _delay{0.0};
    qreal _scale{1.0};
    bool _loop{false};
    qreal _loopDuration{1.0};
};

} // namespace gizmotweak2

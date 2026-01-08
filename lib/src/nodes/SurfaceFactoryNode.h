#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class SurfaceFactoryNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SurfaceType surfaceType READ surfaceType WRITE setSurfaceType NOTIFY surfaceTypeChanged)
    Q_PROPERTY(qreal amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(qreal frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(qreal phase READ phase WRITE setPhase NOTIFY phaseChanged)
    Q_PROPERTY(qreal offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(bool clamp READ clamp WRITE setClamp NOTIFY clampChanged)

public:
    enum class SurfaceType
    {
        Linear,
        Sine,
        Cosine,
        Triangle,
        Sawtooth,
        Square,
        Exponential,
        Logarithmic
    };
    Q_ENUM(SurfaceType)

    explicit SurfaceFactoryNode(QObject* parent = nullptr);
    ~SurfaceFactoryNode() override = default;

    QString type() const override { return QStringLiteral("SurfaceFactory"); }
    Category category() const override { return Category::Shape; }

    SurfaceType surfaceType() const { return _surfaceType; }
    void setSurfaceType(SurfaceType type);

    qreal amplitude() const { return _amplitude; }
    void setAmplitude(qreal amp);

    qreal frequency() const { return _frequency; }
    void setFrequency(qreal freq);

    qreal phase() const { return _phase; }
    void setPhase(qreal ph);

    qreal offset() const { return _offset; }
    void setOffset(qreal off);

    bool clamp() const { return _clamp; }
    void setClamp(bool c);

    // Compute ratio at given time t (0.0 to 1.0 normalized)
    Q_INVOKABLE qreal computeRatio(qreal t) const;

signals:
    void surfaceTypeChanged();
    void amplitudeChanged();
    void frequencyChanged();
    void phaseChanged();
    void offsetChanged();
    void clampChanged();

private:
    SurfaceType _surfaceType{SurfaceType::Sine};
    qreal _amplitude{1.0};
    qreal _frequency{1.0};
    qreal _phase{0.0};
    qreal _offset{0.0};
    bool _clamp{true};
};

} // namespace gizmotweak2

#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class WaveTweak : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(qreal wavelength READ wavelength WRITE setWavelength NOTIFY wavelengthChanged)
    Q_PROPERTY(qreal phase READ phase WRITE setPhase NOTIFY phaseChanged)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(bool radial READ radial WRITE setRadial NOTIFY radialChanged)
    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(bool followGizmo READ followGizmo WRITE setFollowGizmo NOTIFY followGizmoChanged)

public:
    explicit WaveTweak(QObject* parent = nullptr);
    ~WaveTweak() override = default;

    QString type() const override { return QStringLiteral("WaveTweak"); }
    Category category() const override { return Category::Tweak; }

    // Amplitude - strength of wave displacement
    qreal amplitude() const { return _amplitude; }
    void setAmplitude(qreal a);

    // Wavelength - distance between wave peaks
    qreal wavelength() const { return _wavelength; }
    void setWavelength(qreal wl);

    // Phase - offset for animation (0-360 degrees)
    qreal phase() const { return _phase; }
    void setPhase(qreal p);

    // Angle - direction of directional waves (0-360 degrees)
    qreal angle() const { return _angle; }
    void setAngle(qreal a);

    // Radial mode - concentric waves vs directional waves
    bool radial() const { return _radial; }
    void setRadial(bool r);

    // Center of transformation (for radial mode)
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal cx);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal cy);

    // Follow Gizmo - use connected Gizmo center
    bool followGizmo() const { return _followGizmo; }
    void setFollowGizmo(bool follow);

    // Apply tweak to a point
    Q_INVOKABLE QPointF apply(qreal x, qreal y, qreal ratio,
                              qreal gizmoX = 0.0, qreal gizmoY = 0.0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void amplitudeChanged();
    void wavelengthChanged();
    void phaseChanged();
    void angleChanged();
    void radialChanged();
    void centerXChanged();
    void centerYChanged();
    void followGizmoChanged();

private:
    qreal _amplitude{0.1};
    qreal _wavelength{0.5};
    qreal _phase{0.0};
    qreal _angle{0.0};
    bool _radial{true};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    bool _followGizmo{true};
};

} // namespace gizmotweak2

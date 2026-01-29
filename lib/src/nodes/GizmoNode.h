#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>
#include <QEasingCurve>

namespace gizmotweak2
{

class GizmoNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum class Shape
    {
        Rectangle,
        Ellipse,
        Angle,
        LinearWave,
        CircularWave
    };
    Q_ENUM(Shape)

private:
    Q_PROPERTY(Shape shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(qreal scaleX READ scaleX WRITE setScaleX NOTIFY scaleXChanged)
    Q_PROPERTY(qreal scaleY READ scaleY WRITE setScaleY NOTIFY scaleYChanged)
    Q_PROPERTY(qreal centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(qreal horizontalBorder READ horizontalBorder WRITE setHorizontalBorder NOTIFY horizontalBorderChanged)
    Q_PROPERTY(qreal verticalBorder READ verticalBorder WRITE setVerticalBorder NOTIFY verticalBorderChanged)
    Q_PROPERTY(int falloffCurve READ falloffCurve WRITE setFalloffCurve NOTIFY falloffCurveChanged)
    Q_PROPERTY(qreal horizontalBend READ horizontalBend WRITE setHorizontalBend NOTIFY horizontalBendChanged)
    Q_PROPERTY(qreal verticalBend READ verticalBend WRITE setVerticalBend NOTIFY verticalBendChanged)
    Q_PROPERTY(qreal aperture READ aperture WRITE setAperture NOTIFY apertureChanged)
    Q_PROPERTY(qreal phase READ phase WRITE setPhase NOTIFY phaseChanged)
    Q_PROPERTY(int waveCount READ waveCount WRITE setWaveCount NOTIFY waveCountChanged)
    Q_PROPERTY(qreal noiseIntensity READ noiseIntensity WRITE setNoiseIntensity NOTIFY noiseIntensityChanged)
    Q_PROPERTY(qreal noiseScale READ noiseScale WRITE setNoiseScale NOTIFY noiseScaleChanged)
    Q_PROPERTY(qreal noiseSpeed READ noiseSpeed WRITE setNoiseSpeed NOTIFY noiseSpeedChanged)

public:
    explicit GizmoNode(QObject* parent = nullptr);
    ~GizmoNode() override = default;

    QString type() const override { return QStringLiteral("Gizmo"); }
    Category category() const override { return Category::Shape; }

    // Shape
    Shape shape() const { return _shape; }
    void setShape(Shape s);

    // Scale (gizmo half-size)
    qreal scaleX() const { return _scaleX; }
    void setScaleX(qreal sx);

    qreal scaleY() const { return _scaleY; }
    void setScaleY(qreal sy);

    // Position
    qreal centerX() const { return _centerX; }
    void setCenterX(qreal x);

    qreal centerY() const { return _centerY; }
    void setCenterY(qreal y);

    // Asymmetric borders
    qreal horizontalBorder() const { return _horizontalBorder; }
    void setHorizontalBorder(qreal b);

    qreal verticalBorder() const { return _verticalBorder; }
    void setVerticalBorder(qreal b);

    // Falloff curve
    int falloffCurve() const { return _falloffCurve; }
    void setFalloffCurve(int curve);

    // Bend (distortion)
    qreal horizontalBend() const { return _horizontalBend; }
    void setHorizontalBend(qreal b);

    qreal verticalBend() const { return _verticalBend; }
    void setVerticalBend(qreal b);

    // Aperture (for Angle shape, in degrees 0-360)
    qreal aperture() const { return _aperture; }
    void setAperture(qreal a);

    // Phase (for wave shapes, in degrees 0-360)
    qreal phase() const { return _phase; }
    void setPhase(qreal p);

    // Wave count (for wave shapes)
    int waveCount() const { return _waveCount; }
    void setWaveCount(int count);

    // Noise
    qreal noiseIntensity() const { return _noiseIntensity; }
    void setNoiseIntensity(qreal intensity);

    qreal noiseScale() const { return _noiseScale; }
    void setNoiseScale(qreal scale);

    qreal noiseSpeed() const { return _noiseSpeed; }
    void setNoiseSpeed(qreal speed);

    // Compute ratio at position (time parameter for animated noise)
    Q_INVOKABLE qreal computeRatio(qreal x, qreal y, qreal time = 0.0) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;
    void automationFromJson(const QJsonArray& json) override;

    // Automation
    void syncToAnimatedValues(int timeMs) override;

    // Legacy compatibility
    qreal radius() const { return (_horizontalBorder + _verticalBorder) / 2.0; }
    void setRadius(qreal r) { setHorizontalBorder(r); setVerticalBorder(r); }

signals:
    void shapeChanged();
    void scaleXChanged();
    void scaleYChanged();
    void centerXChanged();
    void centerYChanged();
    void horizontalBorderChanged();
    void verticalBorderChanged();
    void falloffCurveChanged();
    void horizontalBendChanged();
    void verticalBendChanged();
    void apertureChanged();
    void phaseChanged();
    void waveCountChanged();
    void noiseIntensityChanged();
    void noiseScaleChanged();
    void noiseSpeedChanged();

private:
    qreal computeEllipseRatio(qreal x1, qreal y1) const;
    qreal computeRectangleRatio(qreal x1, qreal y1) const;
    qreal computeAngleRatio(qreal x1, qreal y1) const;
    qreal computeLinearWaveRatio(qreal x1, qreal y1) const;
    qreal computeCircularWaveRatio(qreal x1, qreal y1) const;
    qreal applyNoise(qreal ratio, qreal x, qreal y, qreal time) const;
    qreal pseudoRandom(qreal x, qreal y) const;

    Shape _shape{Shape::Ellipse};
    qreal _scaleX{1.0};
    qreal _scaleY{1.0};
    qreal _centerX{0.0};
    qreal _centerY{0.0};
    qreal _horizontalBorder{1.0};
    qreal _verticalBorder{1.0};
    int _falloffCurve{QEasingCurve::Linear};
    qreal _horizontalBend{0.0};
    qreal _verticalBend{0.0};
    qreal _aperture{90.0};   // Degrees for Angle shape
    qreal _phase{0.0};       // Degrees for wave shapes
    int _waveCount{4};       // Number of waves
    qreal _noiseIntensity{0.0};  // Noise intensity [0, 1]
    qreal _noiseScale{1.0};      // Noise scale (grain size)
    qreal _noiseSpeed{0.0};      // Noise animation speed (0 = static)
};

} // namespace gizmotweak2

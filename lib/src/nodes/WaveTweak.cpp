#include "WaveTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

WaveTweak::WaveTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Wave"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void WaveTweak::setAmplitude(qreal a)
{
    a = qBound(0.0, a, 2.0);
    if (!qFuzzyCompare(_amplitude, a))
    {
        _amplitude = a;
        emit amplitudeChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setWavelength(qreal wl)
{
    wl = qMax(0.01, wl);
    if (!qFuzzyCompare(_wavelength, wl))
    {
        _wavelength = wl;
        emit wavelengthChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setPhase(qreal p)
{
    // Normalize phase to 0-360
    while (p < 0.0) p += 360.0;
    while (p >= 360.0) p -= 360.0;
    if (!qFuzzyCompare(_phase, p))
    {
        _phase = p;
        emit phaseChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setAngle(qreal a)
{
    // Normalize angle to 0-360
    while (a < 0.0) a += 360.0;
    while (a >= 360.0) a -= 360.0;
    if (!qFuzzyCompare(_angle, a))
    {
        _angle = a;
        emit angleChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setRadial(bool r)
{
    if (_radial != r)
    {
        _radial = r;
        emit radialChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setCenterX(qreal cx)
{
    if (!qFuzzyCompare(_centerX, cx))
    {
        _centerX = cx;
        emit centerXChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setCenterY(qreal cy)
{
    if (!qFuzzyCompare(_centerY, cy))
    {
        _centerY = cy;
        emit centerYChanged();
        emitPropertyChanged();
    }
}

void WaveTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;

        // Show/hide ratio port based on followGizmo
        auto* ratioPort = inputAt(1);  // ratio port is at index 1
        if (ratioPort)
        {
            ratioPort->setVisible(follow);
            if (!follow && ratioPort->isConnected())
            {
                emit requestDisconnectPort(ratioPort);
            }
        }

        emit followGizmoChanged();
        emitPropertyChanged();
    }
}

QPointF WaveTweak::apply(qreal x, qreal y, qreal ratio,
                         qreal /*gizmoX*/, qreal /*gizmoY*/) const
{
    if (qFuzzyIsNull(_amplitude) || qFuzzyIsNull(ratio) || qFuzzyIsNull(_wavelength))
    {
        return QPointF(x, y);
    }

    // Effective amplitude based on ratio
    qreal effectiveAmplitude = _amplitude * ratio;

    // Convert phase to radians
    qreal phaseRad = qDegreesToRadians(_phase);

    qreal resultX = x;
    qreal resultY = y;

    if (_radial)
    {
        // Radial mode: concentric waves from center
        // Always use centerX/centerY as transformation center
        // (followGizmo only controls whether ratio comes from gizmo or is 1.0)

        qreal dx = x - _centerX;
        qreal dy = y - _centerY;
        qreal distance = qSqrt(dx * dx + dy * dy);

        if (distance > 0.0001)
        {
            // Calculate wave value based on distance
            qreal waveArg = (2.0 * M_PI * distance / _wavelength) + phaseRad;
            qreal waveValue = qSin(waveArg);

            // Apply displacement along the radial direction
            qreal displacement = effectiveAmplitude * waveValue;
            qreal angle = qAtan2(dy, dx);

            resultX = x + displacement * qCos(angle);
            resultY = y + displacement * qSin(angle);
        }
    }
    else
    {
        // Directional mode: parallel waves at specified angle
        qreal angleRad = qDegreesToRadians(_angle);

        // Project position onto wave direction
        qreal projection = x * qCos(angleRad) + y * qSin(angleRad);

        // Calculate wave value based on projection
        qreal waveArg = (2.0 * M_PI * projection / _wavelength) + phaseRad;
        qreal waveValue = qSin(waveArg);

        // Apply displacement perpendicular to wave direction
        qreal displacement = effectiveAmplitude * waveValue;
        qreal perpAngle = angleRad + M_PI / 2.0;

        resultX = x + displacement * qCos(perpAngle);
        resultY = y + displacement * qSin(perpAngle);
    }

    return QPointF(resultX, resultY);
}

QJsonObject WaveTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["amplitude"] = _amplitude;
    obj["wavelength"] = _wavelength;
    obj["phase"] = _phase;
    obj["angle"] = _angle;
    obj["radial"] = _radial;
    obj["centerX"] = _centerX;
    obj["centerY"] = _centerY;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void WaveTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("amplitude")) setAmplitude(json["amplitude"].toDouble());
    if (json.contains("wavelength")) setWavelength(json["wavelength"].toDouble());
    if (json.contains("phase")) setPhase(json["phase"].toDouble());
    if (json.contains("angle")) setAngle(json["angle"].toDouble());
    if (json.contains("radial")) setRadial(json["radial"].toBool());
    if (json.contains("centerX")) setCenterX(json["centerX"].toDouble());
    if (json.contains("centerY")) setCenterY(json["centerY"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

} // namespace gizmotweak2

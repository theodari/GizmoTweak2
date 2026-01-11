#include "ColorTweak.h"
#include "core/Port.h"

#include <QtMath>

namespace gizmotweak2
{

ColorTweak::ColorTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Color"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void ColorTweak::setMode(Mode m)
{
    if (_mode != m)
    {
        _mode = m;
        emit modeChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setColor(const QColor& c)
{
    if (_color != c)
    {
        _color = c;
        emit colorChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setIntensity(qreal i)
{
    i = qBound(0.0, i, 1.0);
    if (!qFuzzyCompare(_intensity, i))
    {
        _intensity = i;
        emit intensityChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setAffectRed(bool affect)
{
    if (_affectRed != affect)
    {
        _affectRed = affect;
        emit affectRedChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setAffectGreen(bool affect)
{
    if (_affectGreen != affect)
    {
        _affectGreen = affect;
        emit affectGreenChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setAffectBlue(bool affect)
{
    if (_affectBlue != affect)
    {
        _affectBlue = affect;
        emit affectBlueChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterRedMin(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterRedMin, value))
    {
        _filterRedMin = value;
        emit filterRedMinChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterRedMax(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterRedMax, value))
    {
        _filterRedMax = value;
        emit filterRedMaxChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterGreenMin(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterGreenMin, value))
    {
        _filterGreenMin = value;
        emit filterGreenMinChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterGreenMax(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterGreenMax, value))
    {
        _filterGreenMax = value;
        emit filterGreenMaxChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterBlueMin(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterBlueMin, value))
    {
        _filterBlueMin = value;
        emit filterBlueMinChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFilterBlueMax(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(_filterBlueMax, value))
    {
        _filterBlueMax = value;
        emit filterBlueMaxChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setFollowGizmo(bool follow)
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

bool ColorTweak::passesFilter(qreal r, qreal g, qreal b) const
{
    return (r >= _filterRedMin && r <= _filterRedMax &&
            g >= _filterGreenMin && g <= _filterGreenMax &&
            b >= _filterBlueMin && b <= _filterBlueMax);
}

QColor ColorTweak::apply(const QColor& input, qreal ratio) const
{
    qreal inR = input.redF();
    qreal inG = input.greenF();
    qreal inB = input.blueF();

    // Check if color passes the filter
    if (!passesFilter(inR, inG, inB))
    {
        return input;  // No effect on colors outside the filter range
    }

    qreal effectiveRatio = ratio * _intensity;

    qreal targetR = _color.redF();
    qreal targetG = _color.greenF();
    qreal targetB = _color.blueF();

    qreal outR = inR;
    qreal outG = inG;
    qreal outB = inB;

    switch (_mode)
    {
    case Mode::Tint:
        // Blend towards target color
        if (_affectRed)   outR = inR + (targetR - inR) * effectiveRatio;
        if (_affectGreen) outG = inG + (targetG - inG) * effectiveRatio;
        if (_affectBlue)  outB = inB + (targetB - inB) * effectiveRatio;
        break;

    case Mode::Multiply:
        // Multiply by target color
        if (_affectRed)   outR = inR * (1.0 + (targetR - 1.0) * effectiveRatio);
        if (_affectGreen) outG = inG * (1.0 + (targetG - 1.0) * effectiveRatio);
        if (_affectBlue)  outB = inB * (1.0 + (targetB - 1.0) * effectiveRatio);
        break;

    case Mode::Add:
        // Add target color
        if (_affectRed)   outR = inR + targetR * effectiveRatio;
        if (_affectGreen) outG = inG + targetG * effectiveRatio;
        if (_affectBlue)  outB = inB + targetB * effectiveRatio;
        break;

    case Mode::Replace:
        // Replace with target color
        if (_affectRed)   outR = inR * (1.0 - effectiveRatio) + targetR * effectiveRatio;
        if (_affectGreen) outG = inG * (1.0 - effectiveRatio) + targetG * effectiveRatio;
        if (_affectBlue)  outB = inB * (1.0 - effectiveRatio) + targetB * effectiveRatio;
        break;
    }

    // Clamp values
    outR = qBound(0.0, outR, 1.0);
    outG = qBound(0.0, outG, 1.0);
    outB = qBound(0.0, outB, 1.0);

    return QColor::fromRgbF(outR, outG, outB, input.alphaF());
}

QJsonObject ColorTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["mode"] = static_cast<int>(_mode);
    obj["color"] = _color.name(QColor::HexArgb);
    obj["intensity"] = _intensity;
    obj["affectRed"] = _affectRed;
    obj["affectGreen"] = _affectGreen;
    obj["affectBlue"] = _affectBlue;
    obj["filterRedMin"] = _filterRedMin;
    obj["filterRedMax"] = _filterRedMax;
    obj["filterGreenMin"] = _filterGreenMin;
    obj["filterGreenMax"] = _filterGreenMax;
    obj["filterBlueMin"] = _filterBlueMin;
    obj["filterBlueMax"] = _filterBlueMax;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void ColorTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("mode"))
    {
        setMode(static_cast<Mode>(json["mode"].toInt()));
    }
    if (json.contains("color"))
    {
        setColor(QColor(json["color"].toString()));
    }
    if (json.contains("intensity")) setIntensity(json["intensity"].toDouble());
    if (json.contains("affectRed")) setAffectRed(json["affectRed"].toBool());
    if (json.contains("affectGreen")) setAffectGreen(json["affectGreen"].toBool());
    if (json.contains("affectBlue")) setAffectBlue(json["affectBlue"].toBool());
    if (json.contains("filterRedMin")) setFilterRedMin(json["filterRedMin"].toDouble());
    if (json.contains("filterRedMax")) setFilterRedMax(json["filterRedMax"].toDouble());
    if (json.contains("filterGreenMin")) setFilterGreenMin(json["filterGreenMin"].toDouble());
    if (json.contains("filterGreenMax")) setFilterGreenMax(json["filterGreenMax"].toDouble());
    if (json.contains("filterBlueMin")) setFilterBlueMin(json["filterBlueMin"].toDouble());
    if (json.contains("filterBlueMax")) setFilterBlueMax(json["filterBlueMax"].toDouble());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

} // namespace gizmotweak2

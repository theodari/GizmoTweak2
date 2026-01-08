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
    addInput(QStringLiteral("frame"), Port::DataType::Frame);
    addInput(QStringLiteral("ratio"), Port::DataType::Ratio2D);

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void ColorTweak::setMode(Mode m)
{
    if (_mode != m)
    {
        _mode = m;
        emit modeChanged();
    }
}

void ColorTweak::setColor(const QColor& c)
{
    if (_color != c)
    {
        _color = c;
        emit colorChanged();
    }
}

void ColorTweak::setIntensity(qreal i)
{
    i = qBound(0.0, i, 1.0);
    if (!qFuzzyCompare(_intensity, i))
    {
        _intensity = i;
        emit intensityChanged();
    }
}

void ColorTweak::setAffectRed(bool affect)
{
    if (_affectRed != affect)
    {
        _affectRed = affect;
        emit affectRedChanged();
    }
}

void ColorTweak::setAffectGreen(bool affect)
{
    if (_affectGreen != affect)
    {
        _affectGreen = affect;
        emit affectGreenChanged();
    }
}

void ColorTweak::setAffectBlue(bool affect)
{
    if (_affectBlue != affect)
    {
        _affectBlue = affect;
        emit affectBlueChanged();
    }
}

QColor ColorTweak::apply(const QColor& input, qreal ratio) const
{
    qreal effectiveRatio = ratio * _intensity;

    qreal inR = input.redF();
    qreal inG = input.greenF();
    qreal inB = input.blueF();

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

} // namespace gizmotweak2

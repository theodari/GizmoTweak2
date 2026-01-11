#include "KeyFrame.h"

namespace gizmotweak2
{

KeyFrame::KeyFrame(int nbParams)
    : _nbParams(nbParams)
    , _curve(QEasingCurve::Linear)
{
    Q_ASSERT(nbParams >= 0 && nbParams <= 16);
    _values.resize(nbParams);
}

KeyFrame::KeyFrame(const KeyFrame& other)
    : _nbParams(other._nbParams)
    , _curve(other._curve.type())
{
    _curve.setAmplitude(other._curve.amplitude());
    _curve.setOvershoot(other._curve.overshoot());
    _curve.setPeriod(other._curve.period());
    _values = other._values;
}

bool KeyFrame::save(QDataStream& out) const
{
    out << _nbParams << static_cast<int>(_curve.type());
    for (int i = 0; i < _nbParams; ++i)
    {
        out << _values[i];
    }
    return true;
}

bool KeyFrame::load(QDataStream& in)
{
    int curveType;
    in >> _nbParams >> curveType;
    _curve.setType(static_cast<QEasingCurve::Type>(curveType));
    _values.resize(_nbParams);
    for (int i = 0; i < _nbParams; ++i)
    {
        in >> _values[i];
    }
    return true;
}

QJsonObject KeyFrame::toJson() const
{
    QJsonObject obj;
    obj["curveType"] = static_cast<int>(_curve.type());

    QJsonArray valuesArray;
    for (int i = 0; i < _nbParams; ++i)
    {
        valuesArray.append(_values[i]);
    }
    obj["values"] = valuesArray;

    return obj;
}

bool KeyFrame::fromJson(const QJsonObject& json)
{
    if (!json.contains("curveType") || !json.contains("values"))
    {
        return false;
    }

    _curve.setType(static_cast<QEasingCurve::Type>(json["curveType"].toInt()));

    auto valuesArray = json["values"].toArray();
    _nbParams = valuesArray.size();
    _values.resize(_nbParams);
    for (int i = 0; i < _nbParams; ++i)
    {
        _values[i] = valuesArray[i].toDouble();
    }

    return true;
}

double KeyFrame::value(int paramIndex) const
{
    Q_ASSERT(paramIndex >= 0 && paramIndex < _values.count());
    return _values[paramIndex];
}

void KeyFrame::setValue(int paramIndex, double value)
{
    Q_ASSERT(paramIndex >= 0 && paramIndex < _values.count());
    _values[paramIndex] = value;
}

QEasingCurve::Type KeyFrame::curveType() const
{
    return _curve.type();
}

void KeyFrame::setCurveType(QEasingCurve::Type type)
{
    _curve.setType(type);
}

double KeyFrame::period() const
{
    return _curve.period();
}

void KeyFrame::setPeriod(double period)
{
    _curve.setPeriod(period);
}

double KeyFrame::amplitude() const
{
    return _curve.amplitude();
}

void KeyFrame::setAmplitude(double amplitude)
{
    _curve.setAmplitude(amplitude);
}

double KeyFrame::valueForProgress(double progress) const
{
    return _curve.valueForProgress(qBound(0.0, progress, 1.0));
}

} // namespace gizmotweak2

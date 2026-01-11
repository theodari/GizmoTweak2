#pragma once

#include <QString>
#include <QDataStream>

namespace gizmotweak2
{

struct Param
{
    double minValue{0.0};
    double maxValue{1.0};
    double initialValue{0.0};
    QString paramName;
    double displayRatio{1.0};
    QString suffix;

    Param() = default;

    Param(double min, double max, double initial,
          const QString& name, double ratio = 1.0, const QString& suf = QString())
        : minValue(min)
        , maxValue(max)
        , initialValue(initial)
        , paramName(name)
        , displayRatio(ratio)
        , suffix(suf)
    {
    }

    bool save(QDataStream& out) const
    {
        out << minValue << maxValue << initialValue
            << paramName << displayRatio << suffix;
        return true;
    }

    bool load(QDataStream& in)
    {
        in >> minValue >> maxValue >> initialValue
           >> paramName >> displayRatio >> suffix;
        return true;
    }
};

} // namespace gizmotweak2

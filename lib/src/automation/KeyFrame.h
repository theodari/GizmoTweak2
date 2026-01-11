#pragma once

#include <QVector>
#include <QDataStream>
#include <QEasingCurve>
#include <QJsonObject>
#include <QJsonArray>

namespace gizmotweak2
{

class KeyFrame
{
public:
    explicit KeyFrame(int nbParams);
    KeyFrame(const KeyFrame& other);
    ~KeyFrame() = default;

    bool save(QDataStream& out) const;
    bool load(QDataStream& in);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    double value(int paramIndex) const;
    void setValue(int paramIndex, double value);

    int paramCount() const { return _nbParams; }

    QEasingCurve::Type curveType() const;
    void setCurveType(QEasingCurve::Type type);

    double period() const;
    void setPeriod(double period);

    double amplitude() const;
    void setAmplitude(double amplitude);

    double valueForProgress(double progress) const;

private:
    int _nbParams;
    QVector<double> _values;
    QEasingCurve _curve;
};

} // namespace gizmotweak2

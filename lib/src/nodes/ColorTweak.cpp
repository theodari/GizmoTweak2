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

    // Automation: Color track with R (0), G (1), B (2), Alpha (3)
    auto* colorTrack = createAutomationTrack(QStringLiteral("Color"), 4, QColor(220, 20, 60));
    colorTrack->setupParameter(0, 0.0, 1.0, _color.redF(), tr("Red"), 100.0, QStringLiteral("%"));
    colorTrack->setupParameter(1, 0.0, 1.0, _color.greenF(), tr("Green"), 100.0, QStringLiteral("%"));
    colorTrack->setupParameter(2, 0.0, 1.0, _color.blueF(), tr("Blue"), 100.0, QStringLiteral("%"));
    colorTrack->setupParameter(3, -2.0, 2.0, _alpha, tr("Alpha"), 100.0, QStringLiteral("%"));

    auto* filterTrack = createAutomationTrack(QStringLiteral("Filter"), 6, QColor(100, 149, 237));
    filterTrack->setupParameter(0, 0.0, 1.0, _filterRedMin, tr("R Min"), 100.0, QStringLiteral("%"));
    filterTrack->setupParameter(1, 0.0, 1.0, _filterRedMax, tr("R Max"), 100.0, QStringLiteral("%"));
    filterTrack->setupParameter(2, 0.0, 1.0, _filterGreenMin, tr("G Min"), 100.0, QStringLiteral("%"));
    filterTrack->setupParameter(3, 0.0, 1.0, _filterGreenMax, tr("G Max"), 100.0, QStringLiteral("%"));
    filterTrack->setupParameter(4, 0.0, 1.0, _filterBlueMin, tr("B Min"), 100.0, QStringLiteral("%"));
    filterTrack->setupParameter(5, 0.0, 1.0, _filterBlueMax, tr("B Max"), 100.0, QStringLiteral("%"));
}

void ColorTweak::setColor(const QColor& c)
{
    if (_color != c)
    {
        _color = c;
        // Sync to automation track initial values (R, G, B)
        auto* track = automationTrack(QStringLiteral("Color"));
        if (track)
        {
            track->setInitialValue(0, c.redF());
            track->setInitialValue(1, c.greenF());
            track->setInitialValue(2, c.blueF());
        }
        emit colorChanged();
        emitPropertyChanged();
    }
}

void ColorTweak::setAlpha(qreal a)
{
    a = qBound(-2.0, a, 2.0);
    if (!qFuzzyCompare(_alpha, a))
    {
        _alpha = a;
        // Sync to automation track initial value
        auto* track = automationTrack(QStringLiteral("Color"));
        if (track) track->setInitialValue(3, a);
        emit alphaChanged();
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(0, value);
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(1, value);
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(2, value);
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(3, value);
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(4, value);
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
        auto* track = automationTrack(QStringLiteral("Filter"));
        if (track) track->setInitialValue(5, value);
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

    // GizmoTweak formula: lerp with alpha
    // alpha = ratio * colorAlpha (range can be negative for inverse effect)
    // beta = 1 - alpha
    // out = beta * in + alpha * target
    qreal effectiveAlpha = ratio * _alpha;
    qreal beta = 1.0 - effectiveAlpha;

    qreal targetR = _color.redF();
    qreal targetG = _color.greenF();
    qreal targetB = _color.blueF();

    qreal outR = inR;
    qreal outG = inG;
    qreal outB = inB;

    if (_affectRed)   outR = beta * inR + effectiveAlpha * targetR;
    if (_affectGreen) outG = beta * inG + effectiveAlpha * targetG;
    if (_affectBlue)  outB = beta * inB + effectiveAlpha * targetB;

    // Clamp values
    outR = qBound(0.0, outR, 1.0);
    outG = qBound(0.0, outG, 1.0);
    outB = qBound(0.0, outB, 1.0);

    return QColor::fromRgbF(outR, outG, outB, input.alphaF());
}

QJsonObject ColorTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["color"] = _color.name(QColor::HexArgb);
    obj["alpha"] = _alpha;
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
    if (json.contains("color"))
    {
        setColor(QColor(json["color"].toString()));
    }
    if (json.contains("alpha")) setAlpha(json["alpha"].toDouble());
    // Backward compatibility with old "intensity" field
    else if (json.contains("intensity")) setAlpha(json["intensity"].toDouble());
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

void ColorTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* colorTrack = automationTrack(QStringLiteral("Color"));
    if (colorTrack && colorTrack->isAutomated())
    {
        qreal r = colorTrack->timedValue(timeMs, 0);
        qreal g = colorTrack->timedValue(timeMs, 1);
        qreal b = colorTrack->timedValue(timeMs, 2);
        _color = QColor::fromRgbF(r, g, b);
        _alpha = colorTrack->timedValue(timeMs, 3);
    }

    auto* filterTrack = automationTrack(QStringLiteral("Filter"));
    if (filterTrack && filterTrack->isAutomated())
    {
        _filterRedMin = filterTrack->timedValue(timeMs, 0);
        _filterRedMax = filterTrack->timedValue(timeMs, 1);
        _filterGreenMin = filterTrack->timedValue(timeMs, 2);
        _filterGreenMax = filterTrack->timedValue(timeMs, 3);
        _filterBlueMin = filterTrack->timedValue(timeMs, 4);
        _filterBlueMax = filterTrack->timedValue(timeMs, 5);
    }
}

} // namespace gizmotweak2

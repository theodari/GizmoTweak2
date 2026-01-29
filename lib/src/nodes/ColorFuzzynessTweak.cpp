#include "ColorFuzzynessTweak.h"
#include "core/Port.h"

#include <QtMath>
#include <QRandomGenerator>

namespace gizmotweak2
{

ColorFuzzynessTweak::ColorFuzzynessTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Color Fuzzyness"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);

    // Automation: Amount track with amount (0)
    auto* amountTrack = createAutomationTrack(QStringLiteral("Amount"), 1, QColor(255, 182, 193));
    amountTrack->setupParameter(0, 0.0, 2.0, _amount, tr("Amount"), 100.0, QStringLiteral("%"));

    auto* seedTrack = createAutomationTrack(QStringLiteral("Seed"), 1, QColor(169, 169, 169));
    seedTrack->setupParameter(0, 0.0, 999999.0, _seed, tr("Seed"), 1.0, QString());
}

void ColorFuzzynessTweak::setAmount(qreal a)
{
    a = qBound(0.0, a, 2.0);
    if (!qFuzzyCompare(_amount, a))
    {
        _amount = a;
        auto* track = automationTrack(QStringLiteral("Amount"));
        if (track) track->setInitialValue(0, a);
        emit amountChanged();
        emitPropertyChanged();
    }
}

void ColorFuzzynessTweak::setAffectRed(bool affect)
{
    if (_affectRed != affect)
    {
        _affectRed = affect;
        emit affectRedChanged();
    }
}

void ColorFuzzynessTweak::setAffectGreen(bool affect)
{
    if (_affectGreen != affect)
    {
        _affectGreen = affect;
        emit affectGreenChanged();
    }
}

void ColorFuzzynessTweak::setAffectBlue(bool affect)
{
    if (_affectBlue != affect)
    {
        _affectBlue = affect;
        emit affectBlueChanged();
    }
}

void ColorFuzzynessTweak::setSeed(int s)
{
    if (_seed != s)
    {
        _seed = s;
        auto* track = automationTrack(QStringLiteral("Seed"));
        if (track) track->setInitialValue(0, static_cast<qreal>(s));
        emit seedChanged();
        emitPropertyChanged();
    }
}

void ColorFuzzynessTweak::setUseSeed(bool use)
{
    if (_useSeed != use)
    {
        _useSeed = use;
        emit useSeedChanged();
    }
}

void ColorFuzzynessTweak::setFollowGizmo(bool follow)
{
    if (_followGizmo != follow)
    {
        _followGizmo = follow;
        auto* ratioPort = inputAt(1);
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

QColor ColorFuzzynessTweak::apply(const QColor& input, qreal ratio, int sampleIndex) const
{
    if (_amount <= 0.0 || ratio <= 0.0)
    {
        return input;
    }

    qreal effectiveAmount = _amount * ratio;

    QRandomGenerator* rng;
    QRandomGenerator seededRng;

    if (_useSeed)
    {
        // Create deterministic random based on seed and sample index
        seededRng.seed(_seed + sampleIndex);
        rng = &seededRng;
    }
    else
    {
        rng = QRandomGenerator::global();
    }

    qreal outR = input.redF();
    qreal outG = input.greenF();
    qreal outB = input.blueF();

    if (_affectRed)
    {
        qreal randomR = (rng->bounded(2.0) - 1.0) * effectiveAmount;
        outR = qBound(0.0, outR + randomR, 1.0);
    }

    if (_affectGreen)
    {
        qreal randomG = (rng->bounded(2.0) - 1.0) * effectiveAmount;
        outG = qBound(0.0, outG + randomG, 1.0);
    }

    if (_affectBlue)
    {
        qreal randomB = (rng->bounded(2.0) - 1.0) * effectiveAmount;
        outB = qBound(0.0, outB + randomB, 1.0);
    }

    return QColor::fromRgbF(outR, outG, outB, input.alphaF());
}

QJsonObject ColorFuzzynessTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["amount"] = _amount;
    obj["affectRed"] = _affectRed;
    obj["affectGreen"] = _affectGreen;
    obj["affectBlue"] = _affectBlue;
    obj["seed"] = _seed;
    obj["useSeed"] = _useSeed;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void ColorFuzzynessTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("amount")) setAmount(json["amount"].toDouble());
    if (json.contains("affectRed")) setAffectRed(json["affectRed"].toBool());
    if (json.contains("affectGreen")) setAffectGreen(json["affectGreen"].toBool());
    if (json.contains("affectBlue")) setAffectBlue(json["affectBlue"].toBool());
    if (json.contains("seed")) setSeed(json["seed"].toInt());
    if (json.contains("useSeed")) setUseSeed(json["useSeed"].toBool());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

void ColorFuzzynessTweak::syncToAnimatedValues(int timeMs)
{
    // Only sync if automation is active for each track
    auto* amountTrack = automationTrack(QStringLiteral("Amount"));
    if (amountTrack && amountTrack->isAutomated())
    {
        _amount = amountTrack->timedValue(timeMs, 0);
    }

    auto* seedTrack = automationTrack(QStringLiteral("Seed"));
    if (seedTrack && seedTrack->isAutomated())
    {
        _seed = static_cast<int>(seedTrack->timedValue(timeMs, 0));
    }
}

} // namespace gizmotweak2

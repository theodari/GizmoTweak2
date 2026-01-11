#include "FuzzynessTweak.h"
#include "core/Port.h"

#include <QtMath>
#include <QRandomGenerator>

namespace gizmotweak2
{

FuzzynessTweak::FuzzynessTweak(QObject* parent)
    : Node(parent)
{
    setDisplayName(QStringLiteral("Fuzzyness"));

    // Inputs
    addInput(QStringLiteral("frame"), Port::DataType::Frame, true);  // Required
    addInput(QStringLiteral("ratio"), Port::DataType::RatioAny);  // Accepts Ratio1D or Ratio2D

    // Output
    addOutput(QStringLiteral("frame"), Port::DataType::Frame);
}

void FuzzynessTweak::setAmount(qreal a)
{
    a = qBound(0.0, a, 2.0);
    if (!qFuzzyCompare(_amount, a))
    {
        _amount = a;
        emit amountChanged();
    }
}

void FuzzynessTweak::setAffectX(bool affect)
{
    if (_affectX != affect)
    {
        _affectX = affect;
        emit affectXChanged();
    }
}

void FuzzynessTweak::setAffectY(bool affect)
{
    if (_affectY != affect)
    {
        _affectY = affect;
        emit affectYChanged();
    }
}

void FuzzynessTweak::setSeed(int s)
{
    if (_seed != s)
    {
        _seed = s;
        emit seedChanged();
    }
}

void FuzzynessTweak::setUseSeed(bool use)
{
    if (_useSeed != use)
    {
        _useSeed = use;
        emit useSeedChanged();
    }
}

void FuzzynessTweak::setFollowGizmo(bool follow)
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

QPointF FuzzynessTweak::apply(const QPointF& input, qreal ratio, int sampleIndex) const
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

    qreal outX = input.x();
    qreal outY = input.y();

    if (_affectX)
    {
        // Random value in range [-1, 1] multiplied by amount
        qreal randomX = (rng->bounded(2.0) - 1.0) * effectiveAmount;
        outX += randomX;
    }

    if (_affectY)
    {
        qreal randomY = (rng->bounded(2.0) - 1.0) * effectiveAmount;
        outY += randomY;
    }

    return QPointF(outX, outY);
}

QJsonObject FuzzynessTweak::propertiesToJson() const
{
    QJsonObject obj;
    obj["amount"] = _amount;
    obj["affectX"] = _affectX;
    obj["affectY"] = _affectY;
    obj["seed"] = _seed;
    obj["useSeed"] = _useSeed;
    obj["followGizmo"] = _followGizmo;
    return obj;
}

void FuzzynessTweak::propertiesFromJson(const QJsonObject& json)
{
    if (json.contains("amount")) setAmount(json["amount"].toDouble());
    if (json.contains("affectX")) setAffectX(json["affectX"].toBool());
    if (json.contains("affectY")) setAffectY(json["affectY"].toBool());
    if (json.contains("seed")) setSeed(json["seed"].toInt());
    if (json.contains("useSeed")) setUseSeed(json["useSeed"].toBool());
    if (json.contains("followGizmo")) setFollowGizmo(json["followGizmo"].toBool());
}

} // namespace gizmotweak2

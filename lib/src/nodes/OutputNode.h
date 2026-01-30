#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class OutputNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int zoneIndex READ zoneIndex WRITE setZoneIndex NOTIFY zoneIndexChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(qreal lineBreakThreshold READ lineBreakThreshold WRITE setLineBreakThreshold NOTIFY lineBreakThresholdChanged)

public:
    explicit OutputNode(QObject* parent = nullptr);
    ~OutputNode() override = default;

    QString type() const override { return QStringLiteral("Output"); }
    Category category() const override { return Category::IO; }

    // Zone selection
    int zoneIndex() const { return _zoneIndex; }
    void setZoneIndex(int index);

    // Enable/disable output
    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled);

    // Line break threshold (0 = disabled)
    // When distance between two consecutive colored samples exceeds this,
    // insert blanking points to break the line (like an elastic snapping)
    qreal lineBreakThreshold() const { return _lineBreakThreshold; }
    void setLineBreakThreshold(qreal threshold);

    // Persistence
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void zoneIndexChanged();
    void enabledChanged();
    void lineBreakThresholdChanged();

private:
    int _zoneIndex{0};
    bool _enabled{true};
    qreal _lineBreakThreshold{3.0};  // default max (300%)
};

} // namespace gizmotweak2

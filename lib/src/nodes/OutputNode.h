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

signals:
    void zoneIndexChanged();
    void enabledChanged();

private:
    int _zoneIndex{0};
    bool _enabled{true};
};

} // namespace gizmotweak2

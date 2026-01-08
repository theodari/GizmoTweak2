#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class InputNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit InputNode(QObject* parent = nullptr);
    ~InputNode() override = default;

    QString type() const override { return QStringLiteral("Input"); }
    Category category() const override { return Category::IO; }
};

} // namespace gizmotweak2

#pragma once

#include "core/Node.h"
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class OutputNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OutputNode(QObject* parent = nullptr);
    ~OutputNode() override = default;

    QString type() const override { return QStringLiteral("Output"); }
    Category category() const override { return Category::IO; }
};

} // namespace gizmotweak2

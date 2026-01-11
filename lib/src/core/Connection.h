#pragma once

#include <QObject>
#include <QPointer>
#include <QUuid>
#include <QtQml/qqmlregistration.h>

#include "Port.h"

namespace gizmotweak2
{

class Connection : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Connection cannot be created from QML")

    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(Port* sourcePort READ sourcePort CONSTANT)
    Q_PROPERTY(Port* targetPort READ targetPort CONSTANT)

public:
    explicit Connection(Port* source, Port* target, QObject* parent = nullptr);
    ~Connection() override;

    QString uuid() const { return _uuid.toString(QUuid::WithoutBraces); }
    Port* sourcePort() const { return _sourcePort.data(); }
    Port* targetPort() const { return _targetPort.data(); }

    static bool isValid(Port* source, Port* target);

private:
    QUuid _uuid;
    QPointer<Port> _sourcePort;
    QPointer<Port> _targetPort;
};

} // namespace gizmotweak2

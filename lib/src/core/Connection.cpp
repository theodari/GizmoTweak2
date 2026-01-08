#include "Connection.h"
#include "Port.h"

namespace gizmotweak2
{

Connection::Connection(Port* source, Port* target, QObject* parent)
    : QObject(parent)
    , _uuid(QUuid::createUuid())
    , _sourcePort(source)
    , _targetPort(target)
{
    if (_sourcePort)
    {
        _sourcePort->setConnected(true);
    }
    if (_targetPort)
    {
        _targetPort->setConnected(true);
    }
}

Connection::~Connection()
{
    if (_sourcePort)
    {
        _sourcePort->setConnected(false);
    }
    if (_targetPort)
    {
        _targetPort->setConnected(false);
    }
}

bool Connection::isValid(Port* source, Port* target)
{
    if (!source || !target)
    {
        return false;
    }

    return source->canConnectTo(target);
}

} // namespace gizmotweak2

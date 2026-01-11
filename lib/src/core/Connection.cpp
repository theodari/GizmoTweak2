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
    if (_sourcePort && _targetPort)
    {
        // Determine the effective data type for RatioAny ports
        auto sourceType = _sourcePort->dataType();
        auto targetType = _targetPort->dataType();

        // If one is RatioAny and the other is specific, adopt the specific type
        if (sourceType == Port::DataType::RatioAny &&
            (targetType == Port::DataType::Ratio1D || targetType == Port::DataType::Ratio2D))
        {
            _sourcePort->setConnectedDataType(targetType);
        }
        else if (targetType == Port::DataType::RatioAny &&
                 (sourceType == Port::DataType::Ratio1D || sourceType == Port::DataType::Ratio2D))
        {
            _targetPort->setConnectedDataType(sourceType);
        }
        // If both are RatioAny, they stay as RatioAny

        _sourcePort->setConnected(true);
        _targetPort->setConnected(true);
    }
    else
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
}

Connection::~Connection()
{
    // Block signals during destruction to avoid crashes from signal handlers
    // accessing partially destroyed objects
    if (_sourcePort)
    {
        bool wasBlocked = _sourcePort->blockSignals(true);
        _sourcePort->setConnected(false);
        _sourcePort->blockSignals(wasBlocked);
    }
    if (_targetPort)
    {
        bool wasBlocked = _targetPort->blockSignals(true);
        _targetPort->setConnected(false);
        _targetPort->blockSignals(wasBlocked);
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

#include "Node.h"
#include "Port.h"

namespace gizmotweak2
{

Node::Node(QObject* parent)
    : QObject(parent)
    , _uuid(QUuid::createUuid())
{
}

void Node::setDisplayName(const QString& name)
{
    if (_displayName != name)
    {
        _displayName = name;
        emit displayNameChanged();
    }
}

void Node::setPosition(const QPointF& pos)
{
    if (_position != pos)
    {
        _position = pos;
        emit positionChanged();
    }
}

void Node::setSelected(bool selected)
{
    if (_selected != selected)
    {
        _selected = selected;
        emit selectedChanged();
    }
}

Port* Node::inputAt(int index) const
{
    if (index >= 0 && index < _inputs.size())
    {
        return _inputs.at(index);
    }
    return nullptr;
}

Port* Node::outputAt(int index) const
{
    if (index >= 0 && index < _outputs.size())
    {
        return _outputs.at(index);
    }
    return nullptr;
}

Port* Node::addInput(const QString& name, Port::DataType dataType)
{
    auto port = new Port(this, name, Port::Direction::In, dataType, _inputs.size());
    _inputs.append(port);
    return port;
}

Port* Node::addOutput(const QString& name, Port::DataType dataType)
{
    auto port = new Port(this, name, Port::Direction::Out, dataType, _outputs.size());
    _outputs.append(port);
    return port;
}

void Node::clearInputs()
{
    for (auto port : _inputs)
    {
        port->deleteLater();
    }
    _inputs.clear();
}

void Node::clearOutputs()
{
    for (auto port : _outputs)
    {
        port->deleteLater();
    }
    _outputs.clear();
}

QQmlListProperty<Port> Node::inputsProperty()
{
    return QQmlListProperty<Port>(this, &_inputs, &portCount, &portAt);
}

QQmlListProperty<Port> Node::outputsProperty()
{
    return QQmlListProperty<Port>(this, &_outputs, &portCount, &portAt);
}

qsizetype Node::portCount(QQmlListProperty<Port>* list)
{
    return static_cast<QList<Port*>*>(list->data)->size();
}

Port* Node::portAt(QQmlListProperty<Port>* list, qsizetype index)
{
    return static_cast<QList<Port*>*>(list->data)->at(index);
}

} // namespace gizmotweak2

#include "Node.h"
#include "Port.h"
#include <QJsonArray>

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

Port* Node::addInput(const QString& name, Port::DataType dataType, bool required)
{
    auto port = new Port(this, name, Port::Direction::In, dataType, _inputs.size());
    port->setRequired(required);
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

bool Node::hasAutomation() const
{
    for (auto* track : _automationTracks)
    {
        if (track->isAutomated())
        {
            return true;
        }
    }
    return false;
}

AutomationTrack* Node::automationTrack(const QString& trackName) const
{
    for (auto* track : _automationTracks)
    {
        if (track->trackName() == trackName)
        {
            return track;
        }
    }
    return nullptr;
}

AutomationTrack* Node::createAutomationTrack(const QString& trackName, int paramCount, const QColor& color)
{
    // Check if track already exists
    auto* existing = automationTrack(trackName);
    if (existing)
    {
        return existing;
    }

    auto* track = new AutomationTrack(paramCount, trackName, color, this);
    _automationTracks.append(track);
    emit automationTracksChanged();
    return track;
}

void Node::removeAutomationTrack(const QString& trackName)
{
    for (int i = 0; i < _automationTracks.size(); ++i)
    {
        if (_automationTracks[i]->trackName() == trackName)
        {
            _automationTracks[i]->deleteLater();
            _automationTracks.removeAt(i);
            emit automationTracksChanged();
            return;
        }
    }
}

double Node::automatedValue(const QString& trackName, int paramIndex, int timeMs) const
{
    auto* track = automationTrack(trackName);
    if (track && track->isAutomated())
    {
        return track->timedValue(timeMs, paramIndex);
    }
    // Return initial value if track exists but not automated
    if (track)
    {
        return track->initialValue(paramIndex);
    }
    return 0.0;
}

QJsonArray Node::automationToJson() const
{
    QJsonArray arr;
    for (auto* track : _automationTracks)
    {
        arr.append(track->toJson());
    }
    return arr;
}

void Node::automationFromJson(const QJsonArray& json)
{
    // Clear existing tracks
    for (auto* track : _automationTracks)
    {
        track->deleteLater();
    }
    _automationTracks.clear();

    // Load tracks from JSON
    for (const auto& trackVal : json)
    {
        auto trackObj = trackVal.toObject();
        int paramCount = trackObj["paramCount"].toInt();
        auto trackName = trackObj["trackName"].toString();
        auto color = QColor(trackObj["color"].toString());

        auto* track = new AutomationTrack(paramCount, trackName, color, this);
        if (track->fromJson(trackObj))
        {
            _automationTracks.append(track);
        }
        else
        {
            delete track;
        }
    }

    emit automationTracksChanged();
}

} // namespace gizmotweak2

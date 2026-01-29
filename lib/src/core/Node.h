#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <QList>
#include <QJsonObject>
#include <QQmlListProperty>
#include <QtQml/qqmlregistration.h>

#include "Port.h"
#include "automation/AutomationTrack.h"

namespace gizmotweak2
{

class Node : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Node cannot be created from QML")

public:
    enum class Category
    {
        IO,
        Shape,
        Utility,
        Tweak
    };
    Q_ENUM(Category)

    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(Category category READ category CONSTANT)
    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(QQmlListProperty<gizmotweak2::Port> inputs READ inputsProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<gizmotweak2::Port> outputs READ outputsProperty CONSTANT)
    Q_PROPERTY(QList<gizmotweak2::AutomationTrack*> automationTracks READ automationTracks NOTIFY automationTracksChanged)
    Q_PROPERTY(bool hasAutomation READ hasAutomation NOTIFY automationTracksChanged)

public:
    explicit Node(QObject* parent = nullptr);
    ~Node() override = default;

    QString uuid() const { return _uuid.toString(QUuid::WithoutBraces); }
    virtual QString type() const = 0;
    virtual Category category() const = 0;

    QString displayName() const { return _displayName; }
    void setDisplayName(const QString& name);

    QPointF position() const { return _position; }
    void setPosition(const QPointF& pos);

    bool isSelected() const { return _selected; }
    void setSelected(bool selected);

    QList<Port*> inputs() const { return _inputs; }
    QList<Port*> outputs() const { return _outputs; }

    Q_INVOKABLE Port* inputAt(int index) const;
    Q_INVOKABLE Port* outputAt(int index) const;
    Q_INVOKABLE int inputCount() const { return _inputs.size(); }
    Q_INVOKABLE int outputCount() const { return _outputs.size(); }

    // Serialization - override in derived classes to save/load specific properties
    Q_INVOKABLE virtual QJsonObject propertiesToJson() const { return QJsonObject(); }
    Q_INVOKABLE virtual void propertiesFromJson(const QJsonObject& json) { Q_UNUSED(json) }

    // Automation serialization
    QJsonArray automationToJson() const;
    virtual void automationFromJson(const QJsonArray& json);

    // Automation support
    QList<AutomationTrack*> automationTracks() const { return _automationTracks; }
    bool hasAutomation() const;
    Q_INVOKABLE AutomationTrack* automationTrack(const QString& trackName) const;
    Q_INVOKABLE AutomationTrack* createAutomationTrack(const QString& trackName, int paramCount, const QColor& color = QColor(80, 80, 80));
    Q_INVOKABLE void removeAutomationTrack(const QString& trackName);

    // Get automated value at time (returns initial value if not automated)
    Q_INVOKABLE double automatedValue(const QString& trackName, int paramIndex, int timeMs) const;

    // Sync properties to animated values at given time
    // Override in derived classes to apply automation to specific properties
    virtual void syncToAnimatedValues(int timeMs) { Q_UNUSED(timeMs) }

signals:
    void displayNameChanged();
    void positionChanged();
    void selectedChanged();
    void automationTracksChanged();
    void propertyChanged();  // Emitted when any effect property changes (for preview update)
    void requestDisconnectPort(gizmotweak2::Port* port);  // Request NodeGraph to disconnect this port

protected:
    // Call this from derived class setters to notify property changes
    void emitPropertyChanged() { emit propertyChanged(); }

public:
    Port* addInput(const QString& name, Port::DataType dataType, bool required = false);
    Port* addOutput(const QString& name, Port::DataType dataType);
    void clearInputs();
    void clearOutputs();

private:
    QQmlListProperty<Port> inputsProperty();
    QQmlListProperty<Port> outputsProperty();

    static qsizetype portCount(QQmlListProperty<Port>* list);
    static Port* portAt(QQmlListProperty<Port>* list, qsizetype index);

    QUuid _uuid;
    QString _displayName;
    QPointF _position;
    bool _selected{false};
    QList<Port*> _inputs;
    QList<Port*> _outputs;
    QList<AutomationTrack*> _automationTracks;
};

} // namespace gizmotweak2

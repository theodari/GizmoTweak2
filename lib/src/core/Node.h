#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <QList>
#include <QQmlListProperty>
#include <QtQml/qqmlregistration.h>

#include "Port.h"

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

signals:
    void displayNameChanged();
    void positionChanged();
    void selectedChanged();

protected:
    Port* addInput(const QString& name, Port::DataType dataType);
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
};

} // namespace gizmotweak2

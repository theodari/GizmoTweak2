#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class Node;

class Port : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Port cannot be created from QML")

public:
    enum class Direction
    {
        In,
        Out
    };
    Q_ENUM(Direction)

    enum class DataType
    {
        Frame,
        Ratio2D,
        Ratio1D,
        RatioAny,  // Accepts both Ratio1D and Ratio2D
        Position   // Center position (x, y) for tweak center override
    };
    Q_ENUM(DataType)

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Direction direction READ direction CONSTANT)
    Q_PROPERTY(DataType dataType READ dataType CONSTANT)
    Q_PROPERTY(DataType effectiveDataType READ effectiveDataType NOTIFY effectiveDataTypeChanged)
    Q_PROPERTY(Node* node READ node CONSTANT)
    Q_PROPERTY(int index READ index CONSTANT)
    Q_PROPERTY(QPointF scenePosition READ scenePosition WRITE setScenePosition NOTIFY scenePositionChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool required READ isRequired CONSTANT)
    Q_PROPERTY(bool satisfied READ isSatisfied NOTIFY satisfiedChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit Port(Node* parent,
                  const QString& name,
                  Direction direction,
                  DataType dataType,
                  int index);
    ~Port() override = default;

    QString name() const { return _name; }
    Direction direction() const { return _direction; }
    DataType dataType() const { return _dataType; }
    Node* node() const { return _node; }
    int index() const { return _index; }

    QPointF scenePosition() const { return _scenePosition; }
    void setScenePosition(const QPointF& pos);

    bool isConnected() const { return _connected; }
    void setConnected(bool connected);

    bool isRequired() const { return _required; }
    void setRequired(bool required);
    bool isSatisfied() const { return !_required || _connected; }

    bool isVisible() const { return _visible; }
    void setVisible(bool visible);

    // For RatioAny ports: returns the actual connected type, or RatioAny if not connected
    DataType effectiveDataType() const;
    void setConnectedDataType(DataType type);

    Q_INVOKABLE bool canConnectTo(Port* other) const;

signals:
    void scenePositionChanged();
    void connectedChanged();
    void effectiveDataTypeChanged();
    void satisfiedChanged();
    void visibleChanged();

private:
    Node* _node;
    QString _name;
    Direction _direction;
    DataType _dataType;
    DataType _connectedDataType{DataType::RatioAny};  // Actual type when connected
    int _index;
    QPointF _scenePosition;
    bool _connected{false};
    bool _required{false};
    bool _visible{true};
};

} // namespace gizmotweak2

#pragma once

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2 { class Node; }

class PatternThumbnail : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(gizmotweak2::Node* inputNode READ inputNode WRITE setInputNode NOTIFY inputNodeChanged)
    Q_PROPERTY(int patternIndex READ patternIndex WRITE setPatternIndex NOTIFY patternIndexChanged)

public:
    explicit PatternThumbnail(QQuickItem* parent = nullptr);

    gizmotweak2::Node* inputNode() const { return _inputNode; }
    void setInputNode(gizmotweak2::Node* node);

    int patternIndex() const { return _patternIndex; }
    void setPatternIndex(int index);

    void paint(QPainter* painter) override;

signals:
    void inputNodeChanged();
    void patternIndexChanged();

private:
    gizmotweak2::Node* _inputNode{nullptr};
    int _patternIndex{0};
};

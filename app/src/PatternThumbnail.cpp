#include "PatternThumbnail.h"

#include <QPainter>

#include "nodes/InputNode.h"

using namespace gizmotweak2;

PatternThumbnail::PatternThumbnail(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

void PatternThumbnail::setInputNode(Node* node)
{
    if (_inputNode != node)
    {
        _inputNode = node;
        emit inputNodeChanged();
        update();
    }
}

void PatternThumbnail::setPatternIndex(int index)
{
    if (_patternIndex != index)
    {
        _patternIndex = index;
        emit patternIndexChanged();
        update();
    }
}

void PatternThumbnail::paint(QPainter* painter)
{
    auto w = static_cast<int>(width());
    auto h = static_cast<int>(height());

    painter->fillRect(0, 0, w, h, QColor(20, 20, 20));

    if (!_inputNode)
        return;

    auto* inputNode = qobject_cast<InputNode*>(_inputNode);
    if (!inputNode)
        return;

    auto* frame = inputNode->getPatternFrame(_patternIndex);
    if (!frame || frame->size() == 0)
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    frame->render(painter, 0, 0, w, h, 1.5);
}

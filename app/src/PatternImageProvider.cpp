#include "PatternImageProvider.h"

#include "core/NodeGraph.h"
#include "nodes/InputNode.h"

#include <QPainter>

PatternImageProvider::PatternImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

void PatternImageProvider::setGraph(gizmotweak2::NodeGraph* graph)
{
    _graph = graph;
}

QImage PatternImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    int patternIndex = id.toInt();
    int w = requestedSize.width() > 0 ? requestedSize.width() : 64;
    int h = requestedSize.height() > 0 ? requestedSize.height() : 64;

    if (size)
    {
        *size = QSize(w, h);
    }

    QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(30, 30, 30));

    if (!_graph)
    {
        return image;
    }

    // Find InputNode
    gizmotweak2::InputNode* inputNode = nullptr;
    for (int i = 0; i < _graph->rowCount(); ++i)
    {
        auto* node = _graph->nodeAt(i);
        if (node && node->type() == QStringLiteral("Input"))
        {
            inputNode = qobject_cast<gizmotweak2::InputNode*>(node);
            break;
        }
    }

    if (!inputNode)
    {
        return image;
    }

    auto* frame = inputNode->getPatternFrame(patternIndex);
    if (!frame || frame->size() == 0)
    {
        return image;
    }

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    frame->render(&painter, 0, 0, w, h, 1.5);

    return image;
}

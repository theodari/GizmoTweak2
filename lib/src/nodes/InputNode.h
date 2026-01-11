#pragma once

#include "core/Node.h"
#include <stack.h>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class InputNode : public Node
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SourceType sourceType READ sourceType WRITE setSourceType NOTIFY sourceTypeChanged)
    Q_PROPERTY(int patternIndex READ patternIndex WRITE setPatternIndex NOTIFY patternIndexChanged)
    Q_PROPERTY(int previewPatternIndex READ previewPatternIndex WRITE setPreviewPatternIndex NOTIFY previewPatternIndexChanged)
    Q_PROPERTY(QStringList patternNames READ patternNames NOTIFY patternNamesChanged)
    Q_PROPERTY(xengine::Frame* currentFrame READ currentFrame NOTIFY currentFrameChanged)

public:
    enum class SourceType
    {
        Pattern,    // Internal ILDA patterns
        Frame,      // Single frame from caller
        Frames,     // Frame sequence from caller
        Stack,      // Bank/stack passed by caller
        Live        // Triple buffer live input
    };
    Q_ENUM(SourceType)

    explicit InputNode(QObject* parent = nullptr);
    ~InputNode() override = default;

    QString type() const override { return QStringLiteral("Input"); }
    Category category() const override { return Category::IO; }

    // Source type
    SourceType sourceType() const { return _sourceType; }
    void setSourceType(SourceType type);

    // Pattern selection
    int patternIndex() const { return _patternIndex; }
    void setPatternIndex(int index);
    int previewPatternIndex() const { return _previewPatternIndex; }
    void setPreviewPatternIndex(int index);
    QStringList patternNames() const { return _patternNames; }

    // Get frame for a specific pattern (for mini-previews)
    Q_INVOKABLE xengine::Frame* getPatternFrame(int index) const;

    // Current frame for preview
    xengine::Frame* currentFrame() const { return _currentFrame; }

    // Access to the pattern stack
    xengine::Stack* patternStack() { return &_patternStack; }

signals:
    void sourceTypeChanged();
    void patternIndexChanged();
    void previewPatternIndexChanged();
    void patternNamesChanged();
    void currentFrameChanged();

private:
    void updateCurrentFrame();
    void buildPatternNames();
    void createDefaultPatterns();

    SourceType _sourceType{SourceType::Pattern};
    int _patternIndex{0};
    int _previewPatternIndex{-1};  // -1 means no preview
    QStringList _patternNames;
    xengine::Stack _patternStack;
    xengine::Frame* _currentFrame{nullptr};
};

} // namespace gizmotweak2

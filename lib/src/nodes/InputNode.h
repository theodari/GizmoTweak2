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

    // Timeline settings
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(qreal bpm READ bpm WRITE setBpm NOTIFY bpmChanged)
    Q_PROPERTY(int beatsPerMeasure READ beatsPerMeasure WRITE setBeatsPerMeasure NOTIFY beatsPerMeasureChanged)
    Q_PROPERTY(int measures READ measures WRITE setMeasures NOTIFY measuresChanged)
    Q_PROPERTY(bool useBpmTiming READ useBpmTiming WRITE setUseBpmTiming NOTIFY useBpmTimingChanged)

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

    // ILDA file operations
    Q_INVOKABLE bool loadIldaFile(const QString& filePath);
    Q_INVOKABLE bool saveIldaFile(const QString& filePath) const;

    // Current frame for preview
    xengine::Frame* currentFrame() const { return _currentFrame; }

    // Access to the pattern stack
    xengine::Stack* patternStack() { return &_patternStack; }

    // Timeline settings
    int duration() const { return _duration; }
    void setDuration(int ms);

    qreal bpm() const { return _bpm; }
    void setBpm(qreal bpm);

    int beatsPerMeasure() const { return _beatsPerMeasure; }
    void setBeatsPerMeasure(int beats);

    int measures() const { return _measures; }
    void setMeasures(int count);

    bool useBpmTiming() const { return _useBpmTiming; }
    void setUseBpmTiming(bool use);

    // Calculate duration from BPM settings
    Q_INVOKABLE int calculateDurationFromBpm() const;

    // Time conversion utilities
    Q_INVOKABLE int beatToMs(int beat) const;
    Q_INVOKABLE int measureToMs(int measure) const;
    Q_INVOKABLE qreal msToBeat(int ms) const;

    // Serialization
    QJsonObject propertiesToJson() const override;
    void propertiesFromJson(const QJsonObject& json) override;

signals:
    void sourceTypeChanged();
    void patternIndexChanged();
    void previewPatternIndexChanged();
    void patternNamesChanged();
    void currentFrameChanged();
    void durationChanged();
    void bpmChanged();
    void beatsPerMeasureChanged();
    void measuresChanged();
    void useBpmTimingChanged();

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

    // Timeline settings
    int _duration{10000};        // Duration in milliseconds (default 10 seconds)
    qreal _bpm{120.0};           // Beats per minute
    int _beatsPerMeasure{4};     // Beats per measure (time signature)
    int _measures{8};            // Number of measures
    bool _useBpmTiming{true};    // Use BPM timing (true) or direct duration (false)
};

} // namespace gizmotweak2

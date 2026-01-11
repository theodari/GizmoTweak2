#pragma once

#include <QObject>
#include <QMap>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QtQml/qqmlregistration.h>

#include "Param.h"
#include "KeyFrame.h"

namespace gizmotweak2
{

class AutomationTrack : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("AutomationTrack is created via Node.createAutomationTrack()")

    Q_PROPERTY(QString trackName READ trackName WRITE setTrackName NOTIFY trackNameChanged)
    Q_PROPERTY(bool automated READ isAutomated WRITE setAutomated NOTIFY automatedChanged)
    Q_PROPERTY(int paramCount READ paramCount CONSTANT)
    Q_PROPERTY(int keyFrameCount READ keyFrameCount NOTIFY keyFrameCountChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit AutomationTrack(int nbParams, const QString& trackName,
                             const QColor& color = QColor(80, 80, 80),
                             QObject* parent = nullptr);
    AutomationTrack(const AutomationTrack& other);
    ~AutomationTrack() override;

    bool save(QDataStream& out) const;
    bool load(QDataStream& in);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    // Parameter setup
    Q_INVOKABLE void setupParameter(int paramIndex, double minValue, double maxValue,
                                    double initialValue, const QString& paramName,
                                    double displayRatio = 1.0, const QString& suffix = QString());

    // Track properties
    QString trackName() const { return _trackName; }
    void setTrackName(const QString& name);

    bool isAutomated() const { return _automated; }
    void setAutomated(bool automated);

    QColor color() const { return _color; }
    void setColor(const QColor& color);

    int paramCount() const { return _nbParams; }
    int keyFrameCount() const { return _keyFrames.count(); }

    // Parameter info
    Q_INVOKABLE double minValue(int index) const;
    Q_INVOKABLE double maxValue(int index) const;
    Q_INVOKABLE double initialValue(int index) const;
    Q_INVOKABLE void setInitialValue(int index, double value);
    Q_INVOKABLE QString parameterName(int index) const;
    Q_INVOKABLE double displayRatio(int index) const;
    Q_INVOKABLE QString suffix(int index) const;

    // Keyframe operations
    Q_INVOKABLE KeyFrame* createKeyFrame(int timeMs);
    Q_INVOKABLE void moveKeyFrame(int oldTimeMs, int newTimeMs);
    Q_INVOKABLE void updateKeyFrameValue(int timeMs, int paramIndex, double value);
    Q_INVOKABLE void deleteKeyFrame(int timeMs);
    Q_INVOKABLE bool hasKeyFrameAt(int timeMs) const;

    // Value interpolation
    Q_INVOKABLE double timedValue(int timeMs, int paramIndex) const;

    // Keyframe access
    const QMap<int, KeyFrame*>& keyFrames() const { return _keyFrames; }
    Q_INVOKABLE QList<int> keyFrameTimes() const { return _keyFrames.keys(); }

    // Resize/trim operations
    void resizeAllKeyFrames(double factor);
    void removeKeyFramesAfter(int timeMs);
    void translateKeyFrames(int deltaMs);

    // Tooltip
    QString toolTipText(int timeMs) const;

signals:
    void trackNameChanged();
    void automatedChanged();
    void colorChanged();
    void keyFrameCountChanged();
    void keyFrameModified(int timeMs);

private:
    int _nbParams;
    QString _trackName;
    QVector<Param> _parameters;
    QMap<int, KeyFrame*> _keyFrames;
    bool _automated{false};
    QColor _color;
};

} // namespace gizmotweak2

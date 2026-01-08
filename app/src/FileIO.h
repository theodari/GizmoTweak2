#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

namespace gizmotweak2
{

class FileIO : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit FileIO(QObject* parent = nullptr);

    Q_INVOKABLE QString readFile(const QUrl& fileUrl) const;
    Q_INVOKABLE bool writeFile(const QUrl& fileUrl, const QString& content) const;
    Q_INVOKABLE QString urlToLocalFile(const QUrl& fileUrl) const;
};

} // namespace gizmotweak2

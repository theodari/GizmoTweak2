#include "FileIO.h"

#include <QFile>
#include <QTextStream>

namespace gizmotweak2
{

FileIO::FileIO(QObject* parent)
    : QObject(parent)
{
}

QString FileIO::readFile(const QUrl& fileUrl) const
{
    QString localPath = fileUrl.toLocalFile();
    QFile file(localPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open file for reading:" << localPath;
        return QString();
    }

    QTextStream stream(&file);
    return stream.readAll();
}

bool FileIO::writeFile(const QUrl& fileUrl, const QString& content) const
{
    QString localPath = fileUrl.toLocalFile();
    QFile file(localPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open file for writing:" << localPath;
        return false;
    }

    QTextStream stream(&file);
    stream << content;
    return true;
}

QString FileIO::urlToLocalFile(const QUrl& fileUrl) const
{
    return fileUrl.toLocalFile();
}

} // namespace gizmotweak2

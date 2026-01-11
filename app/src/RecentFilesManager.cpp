#include "RecentFilesManager.h"

#include <QSettings>
#include <QFileInfo>

RecentFilesManager::RecentFilesManager(QObject* parent)
    : QObject(parent)
{
    loadSettings();
    validateRecentFiles();
}

void RecentFilesManager::loadSettings()
{
    QSettings settings("GizmoTweak", "GizmoTweak2");
    _recentFiles = settings.value("recentFiles").toStringList();
    _reloadLastFileOnStartup = settings.value("reloadLastFileOnStartup", true).toBool();
}

void RecentFilesManager::saveSettings()
{
    QSettings settings("GizmoTweak", "GizmoTweak2");
    settings.setValue("recentFiles", _recentFiles);
    settings.setValue("reloadLastFileOnStartup", _reloadLastFileOnStartup);
}

void RecentFilesManager::setReloadLastFileOnStartup(bool value)
{
    if (_reloadLastFileOnStartup != value)
    {
        _reloadLastFileOnStartup = value;
        saveSettings();
        emit reloadLastFileOnStartupChanged();
    }
}

void RecentFilesManager::addRecentFile(const QString& filePath)
{
    if (filePath.isEmpty())
        return;

    // Normalize path
    QString normalizedPath = QFileInfo(filePath).absoluteFilePath();

    // Remove if already exists (will be re-added at top)
    _recentFiles.removeAll(normalizedPath);

    // Add at the beginning
    _recentFiles.prepend(normalizedPath);

    // Limit the number of recent files
    while (_recentFiles.size() > _maxRecentFiles)
    {
        _recentFiles.removeLast();
    }

    saveSettings();
    emit recentFilesChanged();
}

void RecentFilesManager::removeRecentFile(const QString& filePath)
{
    QString normalizedPath = QFileInfo(filePath).absoluteFilePath();

    if (_recentFiles.removeAll(normalizedPath) > 0)
    {
        saveSettings();
        emit recentFilesChanged();
    }
}

void RecentFilesManager::clearRecentFiles()
{
    if (!_recentFiles.isEmpty())
    {
        _recentFiles.clear();
        saveSettings();
        emit recentFilesChanged();
    }
}

bool RecentFilesManager::fileExists(const QString& filePath) const
{
    return QFileInfo::exists(filePath);
}

QString RecentFilesManager::displayName(const QString& filePath) const
{
    return QFileInfo(filePath).fileName();
}

void RecentFilesManager::validateRecentFiles()
{
    QStringList validFiles;
    bool changed = false;

    for (const QString& filePath : _recentFiles)
    {
        if (QFileInfo::exists(filePath))
        {
            validFiles.append(filePath);
        }
        else
        {
            changed = true;
        }
    }

    if (changed)
    {
        _recentFiles = validFiles;
        saveSettings();
        emit recentFilesChanged();
    }
}

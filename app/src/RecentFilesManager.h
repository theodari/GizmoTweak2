#pragma once

#include <QObject>
#include <QStringList>

class RecentFilesManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(bool reloadLastFileOnStartup READ reloadLastFileOnStartup WRITE setReloadLastFileOnStartup NOTIFY reloadLastFileOnStartupChanged)
    Q_PROPERTY(QString lastFile READ lastFile NOTIFY recentFilesChanged)

public:
    explicit RecentFilesManager(QObject* parent = nullptr);
    ~RecentFilesManager() override = default;

    QStringList recentFiles() const { return _recentFiles; }
    QString lastFile() const { return _recentFiles.isEmpty() ? QString() : _recentFiles.first(); }

    bool reloadLastFileOnStartup() const { return _reloadLastFileOnStartup; }
    void setReloadLastFileOnStartup(bool value);

    // Add a file to recent list (moves to top if already exists)
    Q_INVOKABLE void addRecentFile(const QString& filePath);

    // Remove a specific file from the list
    Q_INVOKABLE void removeRecentFile(const QString& filePath);

    // Clear all recent files
    Q_INVOKABLE void clearRecentFiles();

    // Check if a file exists
    Q_INVOKABLE bool fileExists(const QString& filePath) const;

    // Get display name (filename without path)
    Q_INVOKABLE QString displayName(const QString& filePath) const;

    // Validate and remove non-existent files
    Q_INVOKABLE void validateRecentFiles();

signals:
    void recentFilesChanged();
    void reloadLastFileOnStartupChanged();

private:
    void loadSettings();
    void saveSettings();

    QStringList _recentFiles;
    bool _reloadLastFileOnStartup{true};
    int _maxRecentFiles{10};
};

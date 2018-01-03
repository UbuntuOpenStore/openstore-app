#ifndef PACKAGESCACHE_H
#define PACKAGESCACHE_H

#include <QObject>
#include <QHash>

class PackageItem;

class PackagesCache : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool updatingCache READ updatingCache NOTIFY updatingCacheChanged)

public:
    PackagesCache();
    static PackagesCache* instance();

    bool updatingCache() const { return m_updatingCache; }

    bool contains(const QString &appId) const;
    PackageItem* insert(const QString &appId, const QVariantMap &jsonMap);
    PackageItem* get(const QString &appId) const;

    int getLocalAppRevision(const QString &appId) const { return m_localAppRevision.value(appId, -1); }
    int getRemoteAppRevision(const QString &appId) const { return m_remoteAppRevision.value(appId, -1); }

    int numberOfInstalledAppsInStore() const { return m_remoteAppRevision.count(); }

    Q_INVOKABLE void getPackageDetails(const QString &appId);

Q_SIGNALS:
    void updatingCacheChanged();
    void packageDetailsReady(PackageItem* pkg);

private Q_SLOTS:
    void updateCacheRevisions();

private:
    QHash<QString, PackageItem*> m_cache;
    QHash<QString, int> m_remoteAppRevision; // appid, revision
    QHash<QString, int> m_localAppRevision; // appid, revision

    QString m_signature;
    bool m_updatingCache;
};

#endif // PACKAGESCACHE_H

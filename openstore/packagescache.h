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

Q_SIGNALS:
    void updatingCacheChanged();

private Q_SLOTS:
    void updateCacheRevisions();

private:
    QHash<QString, PackageItem*> m_cache;

    QString m_signature;
    bool m_updatingCache;
};

#endif // PACKAGESCACHE_H

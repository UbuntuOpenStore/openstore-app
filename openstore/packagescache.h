#ifndef PACKAGESCACHE_H
#define PACKAGESCACHE_H

#include <QObject>
#include <QHash>

class PackageItem;

class PackagesCache : public QObject
{
    Q_OBJECT
public:
    PackagesCache();
    static PackagesCache* instance();

    bool contains(const QString &appId) const;
    PackageItem* insert(const QString &appId, const QVariantMap &jsonMap);
    PackageItem* get(const QString &appId) const;

private:
    QHash<QString, PackageItem*> m_cache;
};

#endif // PACKAGESCACHE_H

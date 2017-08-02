#include "packagescache.h"
#include "package.h"

Q_GLOBAL_STATIC(PackagesCache, s_packagesCache)

PackagesCache::PackagesCache()
{ }

PackagesCache *PackagesCache::instance()
{
    return s_packagesCache();
}

bool PackagesCache::contains(const QString &appId) const
{
    return m_cache.contains(appId);
}

PackageItem* PackagesCache::insert(const QString &appId, const QVariantMap &jsonMap)
{
    PackageItem *pkg = new PackageItem(jsonMap, this);

    m_cache.insert(appId, pkg);
    return pkg;
}

PackageItem *PackagesCache::get(const QString &appId) const
{
    return m_cache.value(appId, Q_NULLPTR);
}

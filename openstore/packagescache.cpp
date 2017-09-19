#include "packagescache.h"
#include "package.h"

#include "platformintegration.h"
#include "openstorenetworkmanager.h"

Q_GLOBAL_STATIC(PackagesCache, s_packagesCache)

PackagesCache::PackagesCache()
{
    m_updatingCache = false;
    connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &PackagesCache::updateCacheRevisions);
}

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

void PackagesCache::updateCacheRevisions()
{
    m_updatingCache = true;
    Q_EMIT updatingCacheChanged();

    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply, [=](const OpenStoreReply &reply) {
        if (reply.signature != m_signature)
            return;

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.data, &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << Q_FUNC_INFO << "Error parsing json";
            return;
        }

        QVariantMap replyMap = jsonDoc.toVariant().toMap();

        if (!replyMap.value("success").toBool() || !replyMap.contains("data")) {
            qWarning() << Q_FUNC_INFO << "Response doesn't contain data";
            return;
        }

        QHash<QString, int> localAppRevision; // appid, revision
        QVariantList data = replyMap.value("data").toList();
        Q_FOREACH (QVariant d, data) {
            QVariantMap map = d.toMap();
            const QString &appId = map.value("id").toString();
            localAppRevision.insert(appId, map.value("revision").toInt());
        }

        Q_FOREACH (PackageItem* pkg, m_cache) {
            int pkgRevNo = localAppRevision.value(pkg->appId(), -1);
            if (pkgRevNo != -1) {
                pkg->updateLocalInfo(pkgRevNo, PlatformIntegration::instance()->appVersion(pkg->appId()));
            }
        }

        m_updatingCache = false;
        Q_EMIT updatingCacheChanged();
    });

    QStringList appIdsAtRevisionList;

    Q_FOREACH(const QString &appId, PlatformIntegration::instance()->installedAppIds()) {
        const QString &version = PlatformIntegration::instance()->appVersion(appId);
        appIdsAtRevisionList.append(QString("%1@%2").arg(appId, version));
    }

    m_signature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getRevisions(m_signature, appIdsAtRevisionList);
}

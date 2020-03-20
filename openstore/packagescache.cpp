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
    pkg->updateLocalInfo(m_localAppRevision.value(pkg->appId()), PlatformIntegration::instance()->appVersion(pkg->appId()));

    m_cache.insert(appId, pkg);
    return pkg;
}

PackageItem *PackagesCache::get(const QString &appId) const
{
    return m_cache.value(appId, Q_NULLPTR);
}

void PackagesCache::getPackageDetails(const QString &appId)
{
    if (contains(appId)) {
        Q_EMIT packageDetailsReady(get(appId));
    } else {
        const QString &signature = OpenStoreNetworkManager::instance()->generateNewSignature();

        connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply, [=](const OpenStoreReply &reply) {
            if (reply.signature != signature)
                return;

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.data, &error);

            if (error.error != QJsonParseError::NoError) {
                qWarning() << Q_FUNC_INFO << "Error parsing json";
                return;
            }

            QVariantMap replyMap = jsonDoc.toVariant().toMap();

            if (!replyMap.value("success").toBool() || !replyMap.contains("data")) {
                qWarning() << Q_FUNC_INFO << "Error retriving info from" << reply.url;

                Q_EMIT packageFetchError(appId);
                return;
            }

            QVariantMap pkg = replyMap.value("data").toMap();

            PackageItem* pkgItem = insert(appId, pkg);
            Q_EMIT packageDetailsReady(pkgItem);
        });

        OpenStoreNetworkManager::instance()->getAppDetails(signature, appId);
    }
}

void PackagesCache::updateCacheRevisions()
{
    m_updatingCache = true;
    Q_EMIT updatingCacheChanged();

    //qDebug() << Q_FUNC_INFO << "called";

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

        m_localAppRevision.clear();
        m_remoteAppRevision.clear();
        m_packageUrls.clear();

        QVariantList data = replyMap.value("data").toList();
        Q_FOREACH (QVariant d, data) {
            QVariantMap map = d.toMap();
            const QString &appId = map.value("id").toString();
            m_localAppRevision.insert(appId, map.value("revision").toInt());
            m_remoteAppRevision.insert(appId, map.value("latest_revision").toInt());
            m_packageUrls.insert(appId, map.value("download_url").toString());
        }

        Q_FOREACH (PackageItem* pkg, m_cache) {
            int pkgRevNo = m_localAppRevision.value(pkg->appId(), -1);
            pkg->updateLocalInfo(pkgRevNo, PlatformIntegration::instance()->appVersion(pkg->appId()));
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

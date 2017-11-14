#include "openstorenetworkmanager.h"
#include "platformintegration.h"

#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QUuid>

#include <QDebug>

Q_GLOBAL_STATIC(OpenStoreNetworkManager, s_openStoreNetworkManager)

OpenStoreNetworkManager::OpenStoreNetworkManager()
{    
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::networkAccessibleChanged, this, &OpenStoreNetworkManager::networkAccessibleChanged);
    connect(this, &OpenStoreNetworkManager::showNsfwChanged, this, &OpenStoreNetworkManager::deleteCache);

    // Default value
    m_showNsfw = false;

    // Cache result on disk
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(QStringLiteral("%1/api_cache").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
    m_manager->setCache(diskCache);

    deleteCache();
}

OpenStoreNetworkManager *OpenStoreNetworkManager::instance()
{
    return s_openStoreNetworkManager();
}

QString OpenStoreNetworkManager::generateNewSignature() const
{
    return QUuid::createUuid().toString();
}

QNetworkReply *OpenStoreNetworkManager::sendRequest(QNetworkRequest request)
{
    QUrl url = request.url();

    QUrlQuery q(url);
    q.addQueryItem("frameworks", PlatformIntegration::instance()->supportedFrameworks().join(','));
    q.addQueryItem("architecture", PlatformIntegration::instance()->supportedArchitecture());
    q.addQueryItem("lang", PlatformIntegration::instance()->systemLocale());
    q.addQueryItem("nsfw", QString(m_showNsfw ? "" : "false"));

    url.setQuery(q);
    request.setUrl(url);

    qDebug() << "Firing request for" << request.url();

    return m_manager->get(request);
}

void OpenStoreNetworkManager::emitReplySignal(QNetworkReply *reply, const QString &signature)
{
     if (reply->isFinished()) {
        disconnect(reply);

        OpenStoreReply r;

        r.signature = signature;
        r.data = reply->readAll();
        r.url = reply->url();

        reply->deleteLater();

        Q_EMIT newReply(r);
    }
}

bool OpenStoreNetworkManager::getDiscover(const QString &signature)
{
    QUrl url("https://open.uappexplorer.com/api/v1/apps/discover");
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    if (reply->isFinished()) {
        disconnect(reply);
        emitReplySignal(reply, signature);
    }

    return true;
}

bool OpenStoreNetworkManager::getAppDetails(const QString &signature, const QString &appId)
{
    QUrl url(QString("https://open.uappexplorer.com/api/v1/apps/%1").arg(appId));
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort)
{
    QUrl url("https://open.uappexplorer.com/api/v1/apps");

    QUrlQuery q(url);
    q.addQueryItem("skip", QString::number(skip));
    q.addQueryItem("limit", QString::number(limit));
    q.addQueryItem("sort", sort);
    q.addQueryItem("search", filterString);
    q.addQueryItem("category", category);

    url.setQuery(q);

    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getCategories(const QString &signature)
{
    QUrl url("https://open.uappexplorer.com/api/v2/categories");
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getUrl(const QString &signature, const QUrl &url)
{
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getUpdates(const QString &signature, const QStringList &appIds)
{
    QUrl url("https://open.uappexplorer.com/api/v2/apps/updates");

    QUrlQuery q(url);
    q.addQueryItem("apps", appIds.join(","));

    url.setQuery(q);

    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getRevisions(const QString &signature, const QStringList &appIdsAtVersion)
{
    QUrl url("https://open.uappexplorer.com/api/v2/apps/revision");

    QUrlQuery q(url);
    q.addQueryItem("apps", appIdsAtVersion.join(","));

    url.setQuery(q);

    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

void OpenStoreNetworkManager::deleteCache()
{
    if (m_manager == Q_NULLPTR)
        return;

    // Clear cache from previous sessions.
    m_manager->cache()->clear();

    Q_EMIT reloaded();
}

#include "openstorenetworkmanager.h"
#include "platformintegration.h"
#include "apiconstants.h"

#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QUuid>
#include <QJsonObject>

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
    q.addQueryItem("channel", PlatformIntegration::instance()->systemCodename());

    url.setQuery(q);
    request.setUrl(url);

    // qDebug() << "Firing request for" << request.url();

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
    QUrl url(API_BASEURL + API_DISCOVER_ENDPOINT);
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
    QUrl url(API_BASEURL + API_APPDETAILS_ENDPOINT.arg(appId));
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [=]() {
        emitReplySignal(reply, signature);
    });

    emitReplySignal(reply, signature);

    return true;
}

bool OpenStoreNetworkManager::getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort)
{
    QUrl url(API_BASEURL + API_SEARCH_ENDPOINT);

    QUrlQuery q(url);
    q.addQueryItem("skip", QString::number(skip));
    q.addQueryItem("limit", QString::number(limit));
    q.addQueryItem("sort", sort);
    q.addQueryItem("category", category);

    if (filterString.startsWith("author:")) {
        q.addQueryItem("author", filterString.right(filterString.size() - 7));
    }
    else {
        q.addQueryItem("search", filterString);
    }

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
    QUrl url(API_BASEURL + API_CATEGORIES_ENDPOINT);
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

bool OpenStoreNetworkManager::getRevisions(const QString &signature, const QStringList &appIdsAtVersion)
{
    QUrl url(API_BASEURL + API_REVISION_ENDPOINT);

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


bool OpenStoreNetworkManager::postReview(const QString &signature,
                                         const QString &appId,
                                         const QString &version,
                                         const QString &review,
                                         ReviewItem::Rating rating)
{
    QJsonObject createReview{
        { "body", review },
        { "version", version },
        { "rating", ReviewItem::ratingToString(rating) }
    };
    QJsonDocument jsonDocument(createReview);

    QUrl url(API_BASEURL + API_REVIEW_LIST_ENDPOINT.arg(appId));

    QUrlQuery q(url);
    q.addQueryItem("append", "true");

    url.setQuery(q);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->post(request, jsonDocument.toJson());

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

/*
 * Copyright (C) 2020 Brian Douglass
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "openstorenetworkmanager.h"
#include "platformintegration.h"
#include "apiconstants.h"

#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

#include <QDebug>

OpenStoreNetworkManager *OpenStoreNetworkManager::m_instance = nullptr;

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

QString OpenStoreNetworkManager::generateNewSignature() const
{
    return QUuid::createUuid().toString();
}

QString OpenStoreNetworkManager::getUrl() const
{
    return getUrl("");
}

QString OpenStoreNetworkManager::getUrl(QString route) const
{
    QString base = qgetenv("OPENSTORE_API");
    if (base.isEmpty())
    {
        base = API_BASEURL;
    }

    return base + route;
}

bool OpenStoreNetworkManager::isDifferentDomain() const
{
    return (getUrl() != API_BASEURL);
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

QNetworkReply *OpenStoreNetworkManager::postRequest(QNetworkRequest request, QJsonObject query)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    query.insert("frameworks", PlatformIntegration::instance()->supportedFrameworks().join(','));
    query.insert("architecture", PlatformIntegration::instance()->supportedArchitecture());
    query.insert("lang", PlatformIntegration::instance()->systemLocale());
    query.insert("nsfw", QString(m_showNsfw ? "" : "false"));
    query.insert("channel", PlatformIntegration::instance()->systemCodename());

    return m_manager->post(request, QJsonDocument(query).toJson());
}

void OpenStoreNetworkManager::parseReply(QNetworkReply *reply, const QString &signature)
{
    if (reply->isFinished())
    {
        disconnect(reply);

        QJsonParseError jsonError;
        QByteArray body = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &jsonError);
        if (jsonError.error != QJsonParseError::NoError)
        {
            if (reply->error() != QNetworkReply::NoError)
            {
                qWarning() << "network request failed with" << reply->errorString() << reply->error();
                Q_EMIT error(signature, reply->errorString());
            }
            else {
                qWarning() << Q_FUNC_INFO << "Error parsing json" << jsonError.errorString();
                Q_EMIT error(signature, jsonError.errorString());
            }

            reply->deleteLater();
            return;
        }

        QVariantMap replyMap = jsonDoc.toVariant().toMap();
        if (!replyMap.value("success").toBool() && !replyMap.value("message").toString().isEmpty())
        {
            qWarning() << Q_FUNC_INFO << "Error from api" << replyMap.value("message").toString() << reply->url();
            Q_EMIT error(signature, replyMap.value("message").toString());
            reply->deleteLater();
            return;
        }

        OpenStoreReply r;

        r.signature = signature;
        r.data = replyMap.value("data");
        r.url = reply->url();

        reply->deleteLater();

        Q_EMIT parsedReply(r);
    }
    else {
        connect(reply, &QNetworkReply::finished, [=]() {
            parseReply(reply, signature);
        });
    }
}

void OpenStoreNetworkManager::getDiscover(const QString &signature)
{
    QUrl url(getUrl(API_DISCOVER_ENDPOINT));
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getAppDetails(const QString &signature, const QString &appId)
{
    QUrl url(getUrl(API_APPDETAILS_ENDPOINT.arg(appId)));
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort, const QString &filterType)
{
    QUrl url(getUrl(API_SEARCH_ENDPOINT));

    QUrlQuery q(url);
    q.addQueryItem("skip", QString::number(skip));
    q.addQueryItem("limit", QString::number(limit));
    q.addQueryItem("sort", sort);
    q.addQueryItem("category", category);
    q.addQueryItem("type", filterType);

        if (filterString.startsWith("author:"))
    {
        q.addQueryItem("author", filterString.right(filterString.size() - 7));
    }
    else
    {
        q.addQueryItem("search", filterString);
    }

    url.setQuery(q);

    QNetworkReply *reply = sendRequest(QNetworkRequest(url));
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getCategories(const QString &signature)
{
    QUrl url(getUrl(API_CATEGORIES_ENDPOINT));
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getByUrl(const QString &signature, const QUrl &url)
{
    QNetworkReply *reply = sendRequest(QNetworkRequest(url));
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getRevisions(const QString &signature, const QStringList &appIdsAtVersion)
{
    QUrl url(getUrl(API_REVISION_ENDPOINT));

    QJsonObject q;
    q.insert("apps", QJsonValue::fromVariant(appIdsAtVersion));

    QNetworkReply *reply = postRequest(QNetworkRequest(url), q);
    parseReply(reply, signature);
}

void OpenStoreNetworkManager::postReview(const QString &signature,
                                         const QString &appId,
                                         const QString &version,
                                         const QString &review,
                                         Ratings::Rating rating,
                                         const QString &apikey,
                                         const bool &edit)
{
    QJsonObject createReview{
        {"body", review},
        {"version", version},
        {"rating", Ratings::ratingToString(rating)}};
    QJsonDocument jsonDocument(createReview);

    QUrl url(API_BASEURL + API_REVIEW_LIST_ENDPOINT.arg(appId));

    QUrlQuery q(url);
    q.addQueryItem("apikey", apikey);

    url.setQuery(q);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply;

    if (edit)
    {
        reply = m_manager->put(request, jsonDocument.toJson());
    }
    else
    {
        reply = m_manager->post(request, jsonDocument.toJson());
    }

    parseReply(reply, signature);
}

void OpenStoreNetworkManager::getReviews(const QString &signature,
                                         const QString &appId)
{
    QUrl url(API_BASEURL + API_REVIEW_LIST_ENDPOINT.arg(appId));

    QUrlQuery q(url);
    q.addQueryItem("limit", "10");
    url.setQuery(q);

    getReviewsByUrl(signature, url);
}

void OpenStoreNetworkManager::getReviews(const QString &signature,
                                         const QString &appId,
                                         const QString &apiKey)
{
    QUrl url(API_BASEURL + API_REVIEW_LIST_ENDPOINT.arg(appId));

    QUrlQuery q(url);
    q.addQueryItem("filter", "apikey");
    q.addQueryItem("apikey", apiKey);
    url.setQuery(q);

    getReviewsByUrl(signature, url);
}

void OpenStoreNetworkManager::getReviews(const QString &signature,
                                         const QString &appId,
                                         unsigned int limit,
                                         qlonglong fromDate)
{
    QUrl url(API_BASEURL + API_REVIEW_LIST_ENDPOINT.arg(appId));

    QUrlQuery q(url);
    q.addQueryItem("limit", QString::number(limit));
    q.addQueryItem("from", QString::number(fromDate));
    url.setQuery(q);

    getReviewsByUrl(signature, url);
}

void OpenStoreNetworkManager::getReviewsByUrl(const QString &signature, const QUrl &url)
{
    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));

    parseReply(reply, signature);
}

void OpenStoreNetworkManager::deleteCache()
{
    if (m_manager == Q_NULLPTR)
        return;

    // Clear cache from previous sessions.
    m_manager->cache()->clear();

    Q_EMIT reloaded();
}

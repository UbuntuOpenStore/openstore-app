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

#ifndef OPENSTORENETWORKMANAGER_H
#define OPENSTORENETWORKMANAGER_H

#include "review.h"

#include <QObject>
#include <QNetworkAccessManager>

#include <QUrlQuery>

    struct OpenStoreReply {
    QVariant data;
    QString signature;
    QUrl url;
};

class OpenStoreNetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    Q_PROPERTY(bool showNsfw MEMBER m_showNsfw NOTIFY showNsfwChanged)
    Q_PROPERTY(bool isDifferentDomain READ isDifferentDomain CONSTANT)
    Q_PROPERTY(QString domain READ getUrl CONSTANT)

public:
    explicit OpenStoreNetworkManager();
    static OpenStoreNetworkManager* instance();

    bool networkAccessible() const { return m_manager->networkAccessible() != QNetworkAccessManager::NotAccessible; }
    bool isDifferentDomain() const;
    QString getUrl() const;
    QString getUrl(QString route) const;

    QString generateNewSignature() const;

public Q_SLOTS:
    void getDiscover(const QString &signature);
    void getAppDetails(const QString &signature, const QString &appId);
    void getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort);
    void getCategories(const QString &signature);
    void getByUrl(const QString &signature, const QUrl &url);
    void getRevisions(const QString &signature, const QStringList &appIdsAtVersion);
    void postReview(const QString &signature, const QString &appId, const QString &version, const QString &review, Ratings::Rating rating, const QString &apiKey, const bool &edit);
    void getReviews(const QString &signature, const QString &appId);
    void getReviews(const QString &signature, const QString &appId, unsigned int limit, qlonglong fromDate);
    void getReviews(const QString &signature, const QString &appId, const QString &apiKey);

Q_SIGNALS:
    void networkAccessibleChanged();
    void parsedReply(const OpenStoreReply &reply);
    void showNsfwChanged();
    void reloaded();
    void error(const QString &signature, const QString &error);

private Q_SLOTS:
    void deleteCache();

private:
    QNetworkReply* sendRequest(QNetworkRequest request);
    void parseReply(QNetworkReply *reply, const QString &signature);
    void getReviewsByUrl(const QString &signature, const QUrl &url);

private:
    QNetworkAccessManager* m_manager;
    bool m_showNsfw;
};

#endif // OPENSTORENETWORKMANAGER_H

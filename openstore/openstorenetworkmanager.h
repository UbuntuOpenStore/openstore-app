#ifndef OPENSTORENETWORKMANAGER_H
#define OPENSTORENETWORKMANAGER_H

#include "review.h"

#include <QObject>
#include <QNetworkAccessManager>

#include <QUrlQuery>


struct OpenStoreReply {
    QByteArray data;
    QString signature;
    QUrl url;
};

class OpenStoreNetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    Q_PROPERTY(bool showNsfw MEMBER m_showNsfw NOTIFY showNsfwChanged)

public:
    explicit OpenStoreNetworkManager();
    static OpenStoreNetworkManager* instance();

    bool networkAccessible() const { return m_manager->networkAccessible() != QNetworkAccessManager::NotAccessible; }

    QString generateNewSignature() const;

public Q_SLOTS:
    bool getDiscover(const QString &signature);
    bool getAppDetails(const QString &signature, const QString &appId);
    bool getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort);
    bool getCategories(const QString &signature);
    bool getUrl(const QString &signature, const QUrl &url);
    bool getRevisions(const QString &signature, const QStringList &appIdsAtVersion);
    bool postReview(const QString &signature, const QString &appId, const QString &version, const QString &review, ReviewItem::Rating rating);

Q_SIGNALS:
    void networkAccessibleChanged();
    void newReply(const OpenStoreReply &reply);
    void showNsfwChanged();
    void reloaded();

private Q_SLOTS:
    void deleteCache();

private:
    QNetworkReply* sendRequest(QNetworkRequest request);
    void emitReplySignal(QNetworkReply* reply, const QString &signature);

private:
    QNetworkAccessManager* m_manager;
    bool m_showNsfw;
};

#endif // OPENSTORENETWORKMANAGER_H

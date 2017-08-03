#ifndef OPENSTORENETWORKMANAGER_H
#define OPENSTORENETWORKMANAGER_H

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
    Q_PROPERTY(QNetworkAccessManager::NetworkAccessibility networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)

public:   
    explicit OpenStoreNetworkManager();
    static OpenStoreNetworkManager* instance();

    QNetworkAccessManager::NetworkAccessibility networkAccessible() const { return m_manager->networkAccessible(); }

    QString generateNewSignature() const;

public Q_SLOTS:
    bool getDiscover(const QString &signature);
    bool getAppDetails(const QString &signature, const QString &appId);
    bool getSearch(const QString &signature, int skip, int limit, const QString &filterString, const QString &category, const QString &sort);
    bool getCategories(const QString &signature);
    bool getUrl(const QString &signature, const QUrl &url);
    bool getUpdates(const QString &signature, const QStringList &appIds);

Q_SIGNALS:
    void networkAccessibleChanged();
    void newReply(const OpenStoreReply &reply);

private:
    QNetworkReply* sendRequest(QNetworkRequest request);
    void emitReplySignal(QNetworkReply* reply, const QString &signature);

private:
    QNetworkAccessManager* m_manager;
};

#endif // OPENSTORENETWORKMANAGER_H

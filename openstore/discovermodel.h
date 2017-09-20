#ifndef DISCOVERMODEL_H
#define DISCOVERMODEL_H

#include <QAbstractListModel>

#include "openstorenetworkmanager.h"

class QNetworkReply;
class PackageItem;

struct DiscoverCategoryItem {
    QString name;
    QString tagline;
    QString queryUrl;
    QStringList appIds;
};

class DiscoverModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl highlightBannerUrl READ highlightBannerUrl NOTIFY updated)
    Q_PROPERTY(QString highlightAppId READ highlightAppId NOTIFY updated)

public:
    enum Roles {
        RoleName,
        RoleTagline,
        RoleQueryUrl,
        RoleAppIds
    };

    explicit DiscoverModel(QObject *parent = 0);

    QUrl highlightBannerUrl() const { return m_highlightBannerUrl; }
    QString highlightAppId() const { return m_highlightAppId; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_INVOKABLE PackageItem* getPackage(const QString &appId);

Q_SIGNALS:
    void updated();

private Q_SLOTS:
    void parseReply(OpenStoreReply reply);

private:
    QString m_requestSignature;

    QList<DiscoverCategoryItem> m_list;
//    QHash<QString, PackageItem*> m_packages;    // Cache

    QUrl m_highlightBannerUrl;
    QString m_highlightAppId;
};

#endif // DISCOVERMODEL_H

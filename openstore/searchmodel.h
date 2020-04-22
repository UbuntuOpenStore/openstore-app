#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractListModel>
#include <QUrl>

#include "openstorenetworkmanager.h"
#include "review.h"
#include "package.h"

struct SearchPackageItem {
    QString name;
    QString appId;
    QString icon;
    QPointer<Ratings> ratings;
    QString tagline;
    bool installed;
    bool updateAvailable;
    QStringList types;
};

class SearchModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY updated)

    Q_PROPERTY(QString filterString MEMBER m_filterString NOTIFY filterStringChanged)
    Q_PROPERTY(QString category MEMBER m_category NOTIFY categoryChanged)
    Q_PROPERTY(QString sortMode MEMBER m_sortMode NOTIFY sortModeChanged)
    Q_PROPERTY(QUrl queryUrl MEMBER m_queryUrl NOTIFY queryUrlChanged)

public:
    enum Roles {
        RoleName,
        RoleAppId,
        RoleIcon,
        RoleRatings,
        RoleTagline,
        RoleInstalled,
        RoleUpdateAvailable,
        RoleTypes,
    };

    explicit SearchModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_INVOKABLE int find(const QString &appId) const;

Q_SIGNALS:
    void filterStringChanged();
    void categoryChanged();
    void queryUrlChanged();
    void sortModeChanged();
    void updated();

protected:
    bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void update();
    void sendRequest(int skip = 0);
    void parseReply(OpenStoreReply reply);
    void refreshInstalledInfo();

private:
    QString m_filterString;
    QString m_category;
    QString m_sortMode;
    QUrl m_queryUrl;

    bool m_fetchedAll;

    QList<SearchPackageItem> m_list;
    QString m_requestSignature;
};

#endif // SEARCHMODEL_H

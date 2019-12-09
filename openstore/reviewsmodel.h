#ifndef REVIEWSMODEL_H
#define REVIEWSMODEL_H

#include <QAbstractListModel>

#include "openstorenetworkmanager.h"
#include "review.h"

class QNetworkReply;


class ReviewsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(unsigned int thumbsUpCounter MEMBER m_thumbsUpCount NOTIFY updated)
    Q_PROPERTY(unsigned int thumbsDownCounter MEMBER m_thumbsDownCount NOTIFY updated)
    Q_PROPERTY(unsigned int neutralCounter MEMBER m_neutralCount NOTIFY updated)
    Q_PROPERTY(unsigned int happyCounter MEMBER m_happyCount NOTIFY updated)
    Q_PROPERTY(unsigned int buggyCounter MEMBER m_buggyCount NOTIFY updated)
    Q_PROPERTY(unsigned int reviewCount READ reviewCount NOTIFY updated)

public:
    enum Roles {
        RoleId,
        RoleBody,
        RoleComment,
        RoleRedacted,
        RoleDate,
        RoleAuthor,
        RoleVersion,
        RoleRating
    };

    explicit ReviewsModel(const QString &appId, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    unsigned int reviewCount() const;

    bool postReview(const QString &version, const QString &review, ReviewItem::Rating rating, const QString &apiKey);

    Q_INVOKABLE void loadMore();

Q_SIGNALS:
    void updated();

private Q_SLOTS:
    void parseReply(OpenStoreReply reply);

private:
    QString m_requestSignature;

    QList<ReviewItem> m_list;
    QString m_appId;

    unsigned int m_thumbsUpCount;
    unsigned int m_thumbsDownCount;
    unsigned int m_neutralCount;
    unsigned int m_happyCount;
    unsigned int m_buggyCount;
};

#endif // REVIEWSMODEL_H

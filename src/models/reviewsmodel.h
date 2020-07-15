#ifndef REVIEWSMODEL_H
#define REVIEWSMODEL_H

#include <QAbstractListModel>

#include "../openstorenetworkmanager.h"
#include "../review.h"

class QNetworkReply;

class ReviewsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(unsigned int reviewCount READ reviewCount NOTIFY updated)

public:
    enum Roles
    {
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

    bool sendReview(const QString &version, const QString &review, Ratings::Rating rating, const QString &apiKey, const bool &edit);

    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void getOwnReview(const QString &apiKey);

Q_SIGNALS:
    void updated();
    void refresh();
    void error(QString text);
    void reviewPosted();
    void ownReviewResponse(QJsonObject review, int rating);

private Q_SLOTS:
    void parseReply(OpenStoreReply reply);
    void parseError(const QString &signature, const QString &error);
    void onRefresh();

private:
    QString m_postedSignature;
    QString m_appendSignature;
    QString m_resetSignature;
    QString m_ownSignature;

    QList<ReviewItem> m_list;
    QString m_appId;
    int m_reviewCount = 0;
    bool m_loadMorePending;

public:
    void handleReviewPosted(const QJsonObject &);
    void handleAppendReviews(const QJsonObject &data);
    void handleResetReviews(const QJsonObject &data);
    void handleOwnReview(const QJsonObject &data);
};

#endif // REVIEWSMODEL_H

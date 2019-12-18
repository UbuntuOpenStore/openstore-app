#ifndef REVIEWSMODEL_H
#define REVIEWSMODEL_H

#include <QAbstractListModel>

#include "openstorenetworkmanager.h"
#include "review.h"

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
    void ownReviewResponse(QJsonObject review);

private Q_SLOTS:
    void parseReply(OpenStoreReply reply);
    void onRefresh();

private:
    struct IReplyHandling
    {
        virtual void dispatch(ReviewsModel & model, const QJsonObject &data) = 0;
    } & m_replyHandling;

    template <class T>
    struct ReplyHandling: IReplyHandling
    {
        void dispatch(ReviewsModel & model, const QJsonObject &data)
        {
            model.handleReply(data, static_cast<T*>(this));
        }
    };

    struct HandleReviewPosted: ReplyHandling<HandleReviewPosted> {} m_handleReviewPosted;
    struct AppendReviews: ReplyHandling<AppendReviews> {} m_appendReviews;
    struct ResetReviews: ReplyHandling<ResetReviews> {} m_resetReviews;
    struct HandleOwnReview: ReplyHandling<HandleOwnReview> {} m_handleOwnReview;

    QString m_requestSignature;
    QList<ReviewItem> m_list;
    QString m_appId;
    int m_reviewCount;
    bool m_loadMorePending;

public:
    void handleReply(const QJsonObject &, const HandleReviewPosted *);
    void handleReply(const QJsonObject &data, const AppendReviews *);
    void handleReply(const QJsonObject &data, const ResetReviews *);
    void handleReply(const QJsonObject &data, const HandleOwnReview *);
};

#endif // REVIEWSMODEL_H

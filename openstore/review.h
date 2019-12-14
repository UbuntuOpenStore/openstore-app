#ifndef REVIEW_H
#define REVIEW_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class Ratings: public QObject
{
    Q_OBJECT
    Q_ENUMS(Rating)
    Q_PROPERTY(unsigned int thumbsUpCount MEMBER m_thumbsUpCount NOTIFY updated)
    Q_PROPERTY(unsigned int thumbsDownCount MEMBER m_thumbsDownCount NOTIFY updated)
    Q_PROPERTY(unsigned int neutralCount MEMBER m_neutralCount NOTIFY updated)
    Q_PROPERTY(unsigned int happyCount MEMBER m_happyCount NOTIFY updated)
    Q_PROPERTY(unsigned int buggyCount MEMBER m_buggyCount NOTIFY updated)

public:
    explicit Ratings(const QMap<QString, QVariant> &map, QObject * parent = Q_NULLPTR);
    explicit Ratings(QObject * parent = Q_NULLPTR);

    enum Rating {
        RatingThumbsUp = 0,
        RatingThumbsDown = 1,
        RatingNeutral = 2,
        RatingHappy = 3,
        RatingBuggy = 4,
    };

    static QString ratingToString(enum Rating rating);
    static Rating ratingFromString(const QString &rating);

Q_SIGNALS:
    void updated();

private:
    static QMap<QString, Rating> & stringToRatingMap();

    unsigned int m_thumbsUpCount;
    unsigned int m_thumbsDownCount;
    unsigned int m_neutralCount;
    unsigned int m_happyCount;
    unsigned int m_buggyCount;
};

typedef Ratings::Rating Rating;


class ReviewItem: public QObject
{
    Q_OBJECT

public:
    explicit ReviewItem(const QJsonObject &json, QObject * parent = Q_NULLPTR);
    // TODO: can we add objects derived from QObject to QLists properly?
    explicit ReviewItem(const ReviewItem &review);

    class Comment
    {
    public:
        QString m_body;
        unsigned int m_date;

        Comment() : m_body(""), m_date(0) {}
        Comment(const QString &body, unsigned int date)
            : m_body(body), m_date(date)
        {
        }

        Comment(const Comment &comment)
            : m_body(comment.m_body), m_date(comment.m_date)
        {
        }

        QString body() const
        {
            return m_body;
        }

        unsigned int date() const
        {
            return m_date;
        }
    };

    QString id() const;
    QString version() const;
    Rating rating() const;
    QString body() const;
    Comment comment() const;
    bool redacted() const;
    QString author() const;
    unsigned int date() const;

    QString m_reviewId;
    QString m_author;
    QString m_body;
    Rating m_rating;
    QString m_reviewedVersion;
    Comment m_comment;
    bool m_isRedacted;
    unsigned int m_date;
};

Q_DECLARE_METATYPE(ReviewItem::Comment);

#endif // REVIEW_H

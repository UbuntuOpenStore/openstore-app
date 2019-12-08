#ifndef REVIEW_H
#define REVIEW_H

#include <QObject>
#include <QJsonObject>

class ReviewItem: public QObject
{
    Q_OBJECT
    Q_ENUMS(Rating)

public:
    explicit ReviewItem(const QJsonObject &json, QObject * parent = Q_NULLPTR);
    explicit ReviewItem(const ReviewItem &review);

    enum Rating {
        RatingThumbsUp = 0,
        RatingThumbsDown = 1,
        RatingNeutral = 2,
        RatingHappy = 3,
        RatingBuggy = 4,
    };

    static QString ratingToString(enum Rating rating);

    QString id() const;
    QString version() const;
    Rating rating() const;
    QString body() const;
    QString comment() const;
    bool redacted() const;
    QString author() const;
    unsigned int date() const;

private:
    static Rating ratingFromString(const QString &rating);

    QString m_reviewId;
    QString m_body;
    QString m_comment;
    Rating m_rating;
    QString m_reviewedVersion;
    bool m_isRedacted;
    QString m_author;
    unsigned int m_date;
};

typedef ReviewItem::Rating Rating;
#endif // REVIEW_H

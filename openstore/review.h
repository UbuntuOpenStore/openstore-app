#ifndef REVIEW_H
#define REVIEW_H

#include <QObject>
#include <QVariantMap>
#include <QJsonDocument>
#include <QDebug>

class ReviewItem: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString body MEMBER m_body)
    Q_PROPERTY(QString comment MEMBER m_comment)
    Q_PROPERTY(Rating rating READ rating)
    Q_PROPERTY(QString reviewedVersion READ reviewedVersion)
    Q_PROPERTY(bool isReducted MEMBER m_isReducted)
    Q_PROPERTY(QString author MEMBER m_author)
    Q_PROPERTY(unsigned int date MEMBER m_date)

    Q_ENUMS(Rating)

public:
    enum Rating {
        RatingThumbsUp = 0,
        RatingThumbsDown = 1,
        RatingNeutral = 2,
        RatingHappy = 3,
        RatingBuggy = 4,
    };

    static QString ratingToString(enum Rating rating)
    {
        switch (rating)
        {
            case RatingThumbsUp:
                return "THUMBS_UP";
            case RatingThumbsDown:
                return "THUMBS_DOWN";
            case RatingNeutral:
                return "NEUTRAL";
            case RatingHappy:
                return "HAPPY";
            case RatingBuggy:
                return "BUGGY";
        }
        return "";
    }

    QString id() const
    {
        return m_reviewId;
    }

    QString reviewedVersion() const
    {
        return m_reviewedVersion;
    }

    Rating rating() const
    {
        return m_rating;
    }

//    explicit ReviewItem(const QVariantMap &json, QObject *parent = 0);
//    ~ReviewItem();

//    Q_INVOKABLE Hooks hooks(int index) const { return m_hooks.at(index).hooks; }
//
//Q_SIGNALS:
//    void updated();
//    void installedChanged();
//
//private Q_SLOTS:
//    void fillData(const QVariantMap &json);

private:
    QString m_reviewId;
    QString m_body;
    QString m_comment;
    Rating m_rating;
    QString m_reviewedVersion;
    bool m_isReducted;
    QString m_author;
    unsigned int m_date;
};

#endif // REVIEW_H

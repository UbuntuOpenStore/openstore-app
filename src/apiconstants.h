#ifndef APICONSTANTS_H
#define APICONSTANTS_H

#define STORE_DOMAIN QStringLiteral("open-store.io")
#define API_BASEURL QStringLiteral("https://open-store.io/")

#define API_DISCOVER_ENDPOINT QStringLiteral("api/v4/discover")
#define API_APPDETAILS_ENDPOINT QStringLiteral("api/v4/apps/%1")
#define API_SEARCH_ENDPOINT QStringLiteral("api/v4/apps")
#define API_CATEGORIES_ENDPOINT QStringLiteral("api/v3/categories")
#define API_REVISION_ENDPOINT QStringLiteral("api/v4/revisions")

#define API_REVIEW_LIST_ENDPOINT QStringLiteral("api/v3/apps/%1/reviews")
#define API_REVIEW_ENDPOINT QStringLiteral("api/v3/apps/%1/reviews/%2")
#define API_REVIEW_COMMENT_ENDPOINT QStringLiteral("api/v3/apps/%1/reviews/%2/comment")

#endif // APICONSTANTS_H
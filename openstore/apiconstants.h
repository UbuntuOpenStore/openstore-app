#ifndef APICONSTANTS_H
#define APICONSTANTS_H

#define STORE_DOMAIN QStringLiteral("open-store.io")
#define API_BASEURL QStringLiteral("https://open-store.io/api/")

#define API_DISCOVER_ENDPOINT QStringLiteral("v3/discover")
#define API_APPDETAILS_ENDPOINT QStringLiteral("v3/apps/%1")
#define API_SEARCH_ENDPOINT QStringLiteral("v3/apps")
#define API_CATEGORIES_ENDPOINT QStringLiteral("v3/categories")
#define API_REVISION_ENDPOINT QStringLiteral("v3/revisions")

#define API_REVIEW_LIST_ENDPOINT API_APPDETAILS_ENDPOINT + QStringLiteral("/reviews")
#define API_REVIEW_ENDPOINT API_APPDETAILS_ENDPOINT + QStringLiteral("/reviews/%2")
#define API_REVIEW_COMMENT_ENDPOINT API_APPDETAILS_ENDPOINT + QStringLiteral("/reviews/%2/comment")

#endif // APICONSTANTS_H

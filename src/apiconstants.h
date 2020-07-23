/*
 * Copyright (C) 2020 Brian Douglass
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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

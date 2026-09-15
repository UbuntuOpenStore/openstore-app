/*
 * Copyright (C) 2026 Brian Douglass
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

#include "xapianindex.h"
#include "appstreampool.h"

#include <AppStreamQt/component.h>
#include <AppStreamQt/icon.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>

#include <xapian.h>

namespace {
const int VALUE_ID = 0;
const int VALUE_NAME = 1;
const int VALUE_SUMMARY = 2;
const int VALUE_CATEGORY = 3;
const int VALUE_ICON = 4;
}

XapianIndex::XapianIndex(QObject* parent)
  : QObject(parent)
{
  m_databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/xapianindex");
}

XapianIndex::~XapianIndex() {}

QString XapianIndex::databasePath() const
{
  return m_databasePath;
}

bool XapianIndex::isAvailable() const
{
  return m_available;
}

QString XapianIndex::errorMessage() const
{
  return m_errorMessage;
}

bool XapianIndex::build(const QList<AppStream::Component>& components)
{
  try {
    QDir().mkpath(QFileInfo(m_databasePath).path());

    Xapian::WritableDatabase db(m_databasePath.toUtf8().constData(), Xapian::DB_CREATE_OR_OVERWRITE);

    int count = 0;
    Q_FOREACH (const AppStream::Component& component, components) {
      Xapian::Document doc;
      doc.set_data(component.id().toStdString());
      doc.add_boolean_term((QStringLiteral("XID") + sanitizeTerm(component.id())).toStdString());

      // One boolean term per package name; the query matches the exact term.
      Q_FOREACH (const QString& pkgName, component.packageNames()) {
        doc.add_boolean_term((QStringLiteral("XPACK") + sanitizeTerm(pkgName)).toStdString());
      }

      const QString name = component.name();
      const QString summary = component.summary();
      const QString description = component.description();
      doc.add_posting((QStringLiteral("S") + sanitizeTerm(name)).toStdString(), 1);
      doc.add_posting((QStringLiteral("XSUM") + sanitizeTerm(summary)).toStdString(), 1);
      doc.add_posting((QStringLiteral("XD") + sanitizeTerm(description)).toStdString(), 1);

      Q_FOREACH (const QString& cat, component.categories()) {
        doc.add_boolean_term((QStringLiteral("XCAT") + sanitizeTerm(cat)).toStdString());
      }

      doc.add_value(VALUE_ID, component.id().toStdString());
      doc.add_value(VALUE_NAME, name.toStdString());
      doc.add_value(VALUE_SUMMARY, summary.toStdString());
      doc.add_value(VALUE_CATEGORY, component.categories().join(QLatin1Char(';')).toStdString());
      // AppStream exposes icons(); use the first one if present.
      const AppStream::Icon icon = component.icons().isEmpty() ? AppStream::Icon() : component.icons().first();
      doc.add_value(VALUE_ICON, (!icon.isEmpty() ? icon.url().toString() : QString()).toStdString());

      db.add_document(doc);
      ++count;
    }

    // Flush to make term frequencies queryable.
    db.commit();
    m_available = true;
    m_errorMessage.clear();
    m_lastBuilt = QDateTime::currentDateTime();
    qDebug() << "XapianIndex: indexed" << count << "components at" << m_databasePath;
    Q_EMIT indexBuilt();

    return true;
  } catch (const Xapian::Error& e) {
    m_available = false;
    m_errorMessage = QString::fromUtf8(e.get_msg().c_str());
    qWarning() << "XapianIndex: build failed:" << m_errorMessage;
    Q_EMIT indexError(m_errorMessage);
    return false;
  }
}

QList<SearchPackageItem> XapianIndex::search(const QString& queryText, int offset, int limit)
{
  QList<SearchPackageItem> result;
  if (!m_available)
    return result;

  try {
    Xapian::Database db(m_databasePath.toUtf8().constData());
    Xapian::QueryParser parser;
    parser.set_stemmer(Xapian::Stem("en"));
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.add_prefix("name", "S");
    // package: matches the exact package-name term; free-text terms are
    // tokenized by QueryParser and need no sanitization.
    parser.add_boolean_prefix("package", "XPACK");
    parser.add_boolean_prefix("category", "XCAT");

    Xapian::Query query;
    // Empty query: match all documents in docid order. Do not feed empty
    // text or "*" to QueryParser.
    if (queryText.trimmed().isEmpty()) {
      query = Xapian::Query::MatchAll;
    } else {
      query = parser.parse_query(queryText.toUtf8().constData(), Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL);
    }
    if (query.empty()) {
      query = Xapian::Query::MatchAll;
    }

    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(offset, limit);
    m_totalMatches = int(mset.get_matches_estimated());

    for (Xapian::MSetIterator it = mset.begin(); it != mset.end(); ++it) {
      Xapian::Document doc = it.get_document();
      result.append(packageItemFromDocument(QByteArray::fromStdString(doc.get_data())));
    }
  } catch (const Xapian::Error& e) {
    qWarning() << "XapianIndex: search failed:" << QString::fromUtf8(e.get_msg().c_str());
  }
  return result;
}

QList<SearchPackageItem> XapianIndex::allInCategory(const QString& categoryId, int offset, int limit)
{
  QList<SearchPackageItem> result;
  if (!m_available)
    return result;

  try {
    Xapian::Database db(m_databasePath.toUtf8().constData());
    Xapian::Query query(("XCAT" + sanitizeTerm(categoryId)).toUtf8().constData());
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(offset, limit);
    m_totalMatches = int(mset.get_matches_estimated());
    for (Xapian::MSetIterator it = mset.begin(); it != mset.end(); ++it) {
      Xapian::Document doc = it.get_document();
      result.append(packageItemFromDocument(QByteArray::fromStdString(doc.get_data())));
    }
  } catch (const Xapian::Error& e) {
    qWarning() << "XapianIndex: category query failed:" << QString::fromUtf8(e.get_msg().c_str());
  }
  return result;
}

QList<CategoryItem> XapianIndex::categories() const
{
  QList<CategoryItem> result;
  if (!m_available)
    return result;

  try {
    Xapian::Database db(m_databasePath.toUtf8().constData());
    Xapian::TermIterator it = db.allterms_begin();
    while (it != db.allterms_end()) {
      const QString term = QString::fromUtf8((*it).c_str());
      if (term.startsWith(QLatin1String("XCAT"))) {
        const QString id = term.mid(4);
        CategoryItem item;
        item.id = id;
        item.name = id;
        item.count = int(db.get_termfreq(term.toUtf8().constData()));
        item.iconUrl = QUrl();
        result.append(item);
      }
      ++it;
    }
  } catch (const Xapian::Error& e) {
    qWarning() << "XapianIndex: categories failed:" << QString::fromUtf8(e.get_msg().c_str());
  }
  return result;
}

int XapianIndex::totalMatches() const
{
  return m_totalMatches;
}

QString XapianIndex::componentIdForPkgName(const QString& packageName) const
{
  if (!m_available)
    return QString();
  try {
    Xapian::Database db(m_databasePath.toUtf8().constData());
    Xapian::QueryParser parser;
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
    parser.add_boolean_prefix("package", "XPACK");
    Xapian::Query query = parser.parse_query(QStringLiteral("package:%1").arg(sanitizeTerm(packageName)).toUtf8().constData());
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 1);
    if (mset.size() == 0)
      return QString();
    return QString::fromUtf8(mset.begin().get_document().get_data().c_str());
  } catch (const Xapian::Error& e) {
    qWarning() << "XapianIndex: pkg lookup failed:" << QString::fromUtf8(e.get_msg().c_str());
  }
  return QString();
}

QString XapianIndex::sanitizeTerm(const QString& text)
{
  // Xapian terms allow only [a-z0-9_]; use the same lowercase
  // normalization on the write and query sides.
  return QString(text).toLower().replace(QRegularExpression(QStringLiteral("[^a-z0-9_]")), QStringLiteral("_"));
}

SearchPackageItem XapianIndex::packageItemFromDocument(const QByteArray& data)
{
  // The rows carry only appId/packageType/types; PackageKitSource
  // enriches them later for the Search and Discover tiles.
  const QString id = QString::fromUtf8(data);
  SearchPackageItem item;
  item.appId = id;
  item.packageType = QStringLiteral("packagekit");
  item.types = QStringList() << QStringLiteral("packagekit");
  return item;
}

QDateTime XapianIndex::lastBuilt() const
{
  return m_lastBuilt;
}

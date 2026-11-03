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

#include "apidomainsettings.h"
#include "apiconstants.h"

#include <QCoreApplication>
#include <QDebug>
#include <gio/gio.h>

namespace {
QString baseUrl;
} // namespace

void ApiDomainSettings::loadBaseUrl()
{
#ifdef BUNDLED_GSETTINGS
  /*
    To test this, set `export GSETTINGS_SCHEMA_DIR="/home/vagrant/openstore-app/build/deb/x86_64-linux-gnu/install/share/glib-2.0/schemas/"`
    and then run: `gsettings set io.open-store.lomiri-openstore-app api-base-url 'https://open-store.io/'`
  */

  QString bundledSchema = QCoreApplication::applicationDirPath() + QStringLiteral("/share/glib-2.0/schemas");
  GError* error = nullptr;
  GSettingsSchemaSource* source =
    g_settings_schema_source_new_from_directory(bundledSchema.toUtf8().constData(), g_settings_schema_source_get_default(), FALSE, &error);

  if (!source) {
    qWarning() << error->message;
    g_error_free(error);

    baseUrl = API_BASEURL;
    return;
  }
#else
  GSettingsSchemaSource* source = g_settings_schema_source_get_default();
  if (!source) {
    baseUrl = API_BASEURL;
    return;
  }
#endif

  GSettingsSchema* schema = g_settings_schema_source_lookup(source, GSETTINGS_SCHEMA_ID.toUtf8().constData(), TRUE);
  if (!schema) {
    qWarning() << "GSettings schema" << GSETTINGS_SCHEMA_ID << "is not installed";
    g_settings_schema_source_unref(source);
    baseUrl = API_BASEURL;
    return;
  }

  if (!g_settings_schema_has_key(schema, API_KEY_BASE_URL.toUtf8().constData())) {
    qWarning() << "GSettings schema" << GSETTINGS_SCHEMA_ID << "has no" << API_KEY_BASE_URL << "key";
    g_settings_schema_unref(schema);
    g_settings_schema_source_unref(source);
    baseUrl = API_BASEURL;
    return;
  }

  GSettings* settings = g_settings_new_full(schema, nullptr, nullptr);
  GVariant* value = g_settings_get_value(settings, API_KEY_BASE_URL.toUtf8().constData());
  QString url = QString::fromUtf8(g_variant_get_string(value, nullptr)).trimmed();

  g_variant_unref(value);
  g_object_unref(settings);
  g_settings_schema_unref(schema);
  g_settings_schema_source_unref(source);

  if (url.isEmpty()) {
    qWarning() << "GSettings url is empty";
    baseUrl = API_BASEURL;
    return;
  }

  const int schemeEnd = url.indexOf(QStringLiteral("://"));
  if (schemeEnd < 0) {
    url.prepend(QStringLiteral("https://"));
  } else {
    url.replace(0, schemeEnd, QStringLiteral("https"));
  }

  if (!url.endsWith('/')) {
    url.append('/');
  }

  baseUrl = url;
  qDebug() << "API base url: " << baseUrl;
}

QString ApiDomainSettings::getBaseUrl()
{
  if (baseUrl.isEmpty()) {
    loadBaseUrl();
  }

  return baseUrl;
}

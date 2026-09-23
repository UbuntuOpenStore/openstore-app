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

#include "indexbuilder.h"

#include "appstreampool.h"
#include "xapianindex.h"

IndexBuilder::IndexBuilder(QObject* parent)
  : QObject(parent)
{
}

void IndexBuilder::build(const QString& databasePath)
{
  const QList<AppStream::Component> components = AppStreamPool::loadFromDisk();
  if (components.isEmpty()) {
    Q_EMIT buildFailed(QStringLiteral("Failed to read AppStream metadata."));
    return;
  }

  QString errorMessage;
  const int count =
    XapianIndex::buildToFile(databasePath, components, &errorMessage, [this](int percent) { Q_EMIT progressChanged(percent); });
  if (count < 0) {
    Q_EMIT buildFailed(errorMessage);
    return;
  }

  Q_EMIT buildSucceeded(components);
}
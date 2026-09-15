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

#include "packageindex.h"

#include "indexstatus.h"
#include "xapianindex.h"

PackageIndex* PackageIndex::m_instance = 0;

PackageIndex::PackageIndex(QObject* parent)
  : QObject(parent)
{
  m_indexStatus = new IndexStatus(this);
  m_xapian = new XapianIndex(this);
}

PackageIndex::~PackageIndex() {}

void PackageIndex::initialize()
{
  m_indexStatus->initialize();
}

bool PackageIndex::build(const QList<AppStream::Component>& components)
{
  if (!m_xapian->build(components)) {
    m_indexStatus->setError(m_xapian->errorMessage(), true);
    return false;
  }
  m_indexStatus->setReady();
  return true;
}

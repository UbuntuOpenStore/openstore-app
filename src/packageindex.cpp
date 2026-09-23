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

#include "appstreampool.h"
#include "indexbuilder.h"
#include "indexstatus.h"
#include "xapianindex.h"

PackageIndex* PackageIndex::m_instance = 0;

PackageIndex::PackageIndex(QObject* parent)
  : QObject(parent)
{
  m_indexStatus = new IndexStatus(this);
  m_xapian = new XapianIndex(this);
  m_pool = new AppStreamPool(this);

  m_builder = new IndexBuilder();
  m_builder->moveToThread(&m_workerThread);
  connect(m_builder, &IndexBuilder::buildSucceeded, this, &PackageIndex::onBuildSucceeded);
  connect(m_builder, &IndexBuilder::buildFailed, this, &PackageIndex::onBuildFailed);
  connect(m_builder, &IndexBuilder::progressChanged, this, &PackageIndex::onProgress);
  m_workerThread.start();

  connect(m_indexStatus, &IndexStatus::stateChanged, this, [this]() {
    if (m_indexStatus->state() == QStringLiteral("initializing"))
      startBuild();
  });
}

PackageIndex::~PackageIndex()
{
  m_workerThread.quit();
  m_workerThread.wait();
  delete m_builder;
}

void PackageIndex::initialize()
{
  m_indexStatus->initialize();
}

void PackageIndex::startBuild()
{
  if (m_buildInFlight)
    return;
  m_buildInFlight = true;
  QMetaObject::invokeMethod(m_builder, "build", Qt::QueuedConnection, Q_ARG(QString, m_xapian->databasePath()));
}

void PackageIndex::onBuildSucceeded(const QList<AppStream::Component>& components)
{
  m_pool->setComponents(components);
  m_xapian->setAvailable();
  m_buildInFlight = false;
  m_indexStatus->setReady();
  Q_EMIT buildCompleted();
}

void PackageIndex::onBuildFailed(const QString& message)
{
  m_buildInFlight = false;
  m_indexStatus->setError(message, true);
}

void PackageIndex::onProgress(int percent)
{
  m_indexStatus->setProgress(percent);
}
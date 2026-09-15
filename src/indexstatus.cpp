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

#include "indexstatus.h"

IndexStatus::IndexStatus(QObject* parent)
  : QObject(parent)
{
}

void IndexStatus::initialize()
{
  if (m_state == QStringLiteral("initializing"))
    return;
  setState(QStringLiteral("initializing"));
}

void IndexStatus::retry()
{
  // Clear the error without emitting error(); the retry is a state change.
  m_errorMessage.clear();
  m_retryable = false;
  setState(QStringLiteral("uninitialized"));
  initialize();
}

void IndexStatus::setState(const QString& state)
{
  if (m_state == state)
    return;
  m_state = state;
  Q_EMIT stateChanged();
}

void IndexStatus::setError(const QString& message, bool retryable)
{
  m_errorMessage = message;
  m_retryable = retryable;
  setState(QStringLiteral("error"));
  Q_EMIT error(message, retryable);
}

void IndexStatus::setReady()
{
  m_lastUpdated = QDateTime::currentDateTime();
  setState(QStringLiteral("ready"));
  Q_EMIT ready();
}

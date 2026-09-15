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

#ifndef INDEXSTATUS_H
#define INDEXSTATUS_H

#include <QDateTime>
#include <QObject>

class IndexStatus : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString state READ state NOTIFY stateChanged)
  Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY stateChanged)
  Q_PROPERTY(bool retryable READ retryable NOTIFY stateChanged)
  Q_PROPERTY(QDateTime lastUpdated READ lastUpdated NOTIFY stateChanged)

public:
  explicit IndexStatus(QObject* parent = 0);

  QString state() const { return m_state; }
  QString errorMessage() const { return m_errorMessage; }
  bool retryable() const { return m_retryable; }
  QDateTime lastUpdated() const { return m_lastUpdated; }

  Q_INVOKABLE void initialize();
  Q_INVOKABLE void retry();

  void setState(const QString& state);
  void setError(const QString& message, bool retryable);
  void setReady();

Q_SIGNALS:
  void stateChanged();
  void ready();
  void error(const QString& message, bool retryable);

private:
  QString m_state = QStringLiteral("uninitialized");
  QString m_errorMessage;
  bool m_retryable = false;
  QDateTime m_lastUpdated;
};

#endif // INDEXSTATUS_H

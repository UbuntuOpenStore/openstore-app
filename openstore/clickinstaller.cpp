/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
 */

#include "clickinstaller.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QFileSystemWatcher>

ClickInstaller::ClickInstaller(QObject *parent) :
    QObject(parent),
    m_installerProcess(0),
    m_download(0)
{
    m_nam = new QNetworkAccessManager(this);

}

bool ClickInstaller::busy() const
{
    return m_installerProcess != 0 || m_download != 0;
}

int ClickInstaller::downloadProgress() const
{
    if (m_file.isOpen()) {
        return m_file.size();
    }
    return 0;
}

void ClickInstaller::installPackage(const QString &packageUrl)
{
//    qDebug() << "should install package" << packageUrl;
    if (busy()) {
        //qDebug() << "already busy. won't install" << packageUrl;
        return;
    }
    if (packageUrl.startsWith("http://") || packageUrl.startsWith("https://")) {
        fetchPackage(packageUrl);
        return;
    }
    QString localPath = packageUrl;
    localPath.remove(QRegExp("$file://"));
    installLocalPackage(localPath);
}

void ClickInstaller::fetchPackage(const QString &packageUrl)
{
    QUrl url(packageUrl);
//    qDebug() << "fetching package" << url.url();

    m_file.setFileName("/tmp/" + url.fileName());
    if (!m_file.open(QFile::WriteOnly | QFile::Truncate)) {
//        qDebug() << "Cannot open temp file" << m_file.fileName();
        return;
    }


    QNetworkRequest request;
    request.setUrl(url);

    m_download = m_nam->get(request);
    connect(m_download, &QNetworkReply::downloadProgress, this, &ClickInstaller::slotDownloadProgress);
    connect(m_download, &QNetworkReply::finished, this, &ClickInstaller::downloadFinished);

    Q_EMIT busyChanged();
    Q_EMIT downloadProgressChanged();
}

void ClickInstaller::installLocalPackage(const QString &packageFilePath)
{
    if (m_installerProcess) {
        return;
    }
    //qDebug() << "starting installer:" << packageFilePath;

    m_installerProcess = new QProcess(this);
    connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int,QProcess::ExitStatus)));
    connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
    m_installerProcess->start("pkcon", QStringList() << "install-local" << "--allow-untrusted" << packageFilePath);
    Q_EMIT busyChanged();
}

void ClickInstaller::installerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
//    qDebug() << "installing finished" << exitCode << exitStatus;
//    qDebug() << "stdout:" << m_installerProcess->readAll();
    m_installerProcess->deleteLater();
    m_installerProcess = 0;
    Q_EMIT busyChanged();
    Q_EMIT packageInstalled();
}

void ClickInstaller::processStatusChanged(QProcess::ProcessState state)
{
//    qDebug() << "process state changed:" << state;
}

void ClickInstaller::slotDownloadProgress()
{
    m_file.write(m_download->readAll());
//    qDebug() << "downloadProgress" << m_file.size();
    Q_EMIT downloadProgressChanged();
}

void ClickInstaller::downloadFinished()
{
//    qDebug() << "finished" << m_download->error() << m_download->errorString();
    m_file.write(m_download->readAll());
    m_file.close();

    m_download->deleteLater();
    m_download = 0;
    Q_EMIT downloadProgressChanged();
    Q_EMIT busyChanged();

    //QByteArray data = reply->readAll();

    installLocalPackage(m_file.fileName());
}

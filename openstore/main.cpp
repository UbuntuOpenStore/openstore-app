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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include "clickinstaller.h"
#include "appmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<ClickInstaller>("OpenStore", 1, 0, "ClickInstaller");
    qmlRegisterType<AppModel>("OpenStore", 1, 0, "AppModel");
    qmlRegisterUncreatableType<ApplicationItem>("OpenStore", 1, 0, "ApplicationItem", "Don't!");

    QQuickView view;

    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QGuiApplication::quit);

    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}


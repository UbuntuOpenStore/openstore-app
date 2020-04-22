/*
 * Copyright (C) 2017 Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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

import QtQuick 2.4
import Ubuntu.Components 1.3
import QtWebEngine 1.5
import OpenStore 1.0

Page {
    id: rootItem

    readonly property string baseUrl: OpenStoreNetworkManager.domain

    header: PageHeader {
        id: header
        title: i18n.tr("Sign in")
    }

    WebEngineView {
        id: webView
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        zoomFactor: units.gu(1) / 8
        url: baseUrl + "login"
        settings {
            localStorageEnabled: false
         }
         profile {
             persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies
         }

         property bool reloaded: false

        onUrlChanged: {
            if (url == baseUrl + "manage") {
                webView.opacity = 0
                function Timer() {
                    return Qt.createQmlObject("import QtQuick 2.0; Timer {}", root)
                }
                var timer = new Timer()
                timer.interval = 1000
                timer.repeat = false

                // TODO use this: https://gitlab.com/theopenstore/openstore-api/-/merge_requests/20
                timer.triggered.connect(function () {
                    runJavaScript('document.querySelector(".p-button--neutral").click(); setTimeout(function(){window.location += "/apikey#"+ document.querySelectorAll(".apikey span")[1].innerHTML;},100);');
                })
                timer.start()
            }
            else if (("" + url).startsWith(baseUrl + "manage/apikey")) {
                root.apiKey = ("" + url).replace(baseUrl + "manage/apikey#","")
                bottomEdgeStack.pop()
            }
        }
    }

    ActivityIndicator {
        running: visible
        visible: webView.opacity === 0
        anchors.centerIn: parent
    }
}

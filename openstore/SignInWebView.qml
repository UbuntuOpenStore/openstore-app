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

Page {
    id: rootItem

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
        url: "https://open-store.io/login"
        settings {
            localStorageEnabled: false
         }
         profile {
             persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies
         }

         property bool reloaded: false

        onUrlChanged: {
            if (url == "https://open-store.io/manage") {
                webView.opacity = 0
                function Timer() {
                    return Qt.createQmlObject("import QtQuick 2.0; Timer {}", root)
                }
                var timer = new Timer()
                timer.interval = 1000
                timer.repeat = false
                timer.triggered.connect(function () {
                    runJavaScript('document.querySelector(".p-button--neutral").click(); setTimeout(function(){window.location += "/apikey#"+ document.querySelectorAll(".apikey span")[1].innerHTML;},100);');
                })
                timer.start()
            }
            else if (("" + url).startsWith("https://open-store.io/manage/apikey")) {
                root.apiKey = ("" + url).replace("https://open-store.io/manage/apikey#","")
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
/*
 * Copyright (C) 2017 Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
 * Copyright (C) 2020 Brian Douglass
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
import Lomiri.Components 1.3
import OpenStore 1.0
import QtWebEngine 1.7

import "Components" as Components
Page {
    id: rootItem

    readonly property string baseUrl: OpenStoreNetworkManager.domain

    header: Components.HeaderBase {
        id: header
        title: i18n.tr("Sign in")
    }

    WebEngineView {
        id: webView
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        zoomFactor: units.gu(1) / 8

        url: baseUrl + "logged-in"

        settings {
            localStorageEnabled: false
        }
        profile {
            offTheRecord: true
            persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies
            httpUserAgent: 'OpenStore App'
        }

        //Remove website's header
        userScripts: [
            WebEngineScript {
                sourceUrl: "js/inject.js"
                injectionPoint: WebEngineScript.DocumentReady
                worldId: WebEngineScript.MainWorld
            }
        ]

        onUrlChanged: {
            var strUrl = url.toString();
            var checkUrl = baseUrl + 'logged-in?apiKey=';
            if (strUrl.indexOf(checkUrl) == 0) {
                root.apiKey = strUrl.replace(checkUrl, '');
                bottomEdgeStack.pop();
            }
        }
    }


    ActivityIndicator {
        running: visible
        visible: webView.opacity === 0
        anchors.centerIn: parent
    }
}

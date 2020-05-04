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
import OpenStore 1.0
import QtWebEngine 1.7

Page {
    id: rootItem

    readonly property string baseUrl: OpenStoreNetworkManager.domain

    header: PageHeader {
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

        url: baseUrl + "login"

        settings {
            localStorageEnabled: false
        }
        profile {
            offTheRecord: true
            httpUserAgent: 'OpenStore App'
        }

        onUrlChanged: {
            console.log(url);

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

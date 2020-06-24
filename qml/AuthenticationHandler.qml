/*
 * Copyright (C) 2017 Stefano Verzegnassi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License 3 as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import OpenStore.PamAuthentication 0.1

Item {
    id: handler

    property string serviceName

    readonly property bool isAlphanumeric: true     // TODO: Implement
    property bool requireAuthentication: true

    function authenticate(msg) {
        var passwordPopup = PopupUtils.open(Qt.resolvedUrl("PasswordDialog.qml"), QuickUtils.rootItem(handler), { isAlphanumeric: handler.isAlphanumeric, pamAuthentication: __pam, message: msg  })

        passwordPopup.succeeded.connect(function() {
            handler.authenticationSucceeded()
        })

        passwordPopup.aborted.connect(function() {
            handler.authenticationAborted()
        })

        passwordPopup.failed.connect(function(reason) {
            handler.authenticationFailed("password", reason)
        })

    }

    signal authenticationSucceeded()
    signal authenticationFailed(var type, var reason)
    signal authenticationAborted()

    Component.onCompleted: {
        handler.requireAuthentication = __pam.requireAuthentication()
    }

    PamAuthentication {
        id: __pam
        serviceName: handler.serviceName
    }
}

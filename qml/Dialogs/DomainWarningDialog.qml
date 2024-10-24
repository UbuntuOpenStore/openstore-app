/*
 * Copyright (C) 2020 - Brian Douglass
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
import Lomiri.Components.Popups 1.3

Component {
    id: domainWarningComponent

    Dialog {
        id: warningDialog
        title: i18n.tr("Warning")

        signal accepted();
        signal rejected();

        Label {
            anchors { left: parent.left; right: parent.right }
            wrapMode: Text.WordWrap
            maximumLineCount: Number.MAX_VALUE
            text: i18n.tr("You are currently using a non-standard domain for the OpenStore. This is a development feature. The domain you are using is:")
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            wrapMode: Text.WordWrap
            maximumLineCount: Number.MAX_VALUE
            text: OpenStoreNetworkManager.domain
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            wrapMode: Text.WordWrap
            maximumLineCount: Number.MAX_VALUE
            text: i18n.tr("Are you sure you want to continue?")
        }

        Button {
            text: i18n.tr("Yes, I know what I'm doing")
            color: theme.palette.normal.positive
            onClicked: {
                warningDialog.accepted();
            }
        }

        Button {
            text: i18n.tr("Get me out of here!")
            onClicked: {
                warningDialog.rejected();
            }
        }
    }
}

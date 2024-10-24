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
    id: unconfinedWarningPopup
    Dialog {
        id: unconfinedWarningDialog
        title: i18n.tr("Warning")
        text: i18n.tr("This app has access to restricted parts of the system and all of your data. It has the potential break your system. While the OpenStore maintainers have reviewed the code for this app for safety, they are not responsible for anything bad that might happen to your device or data from installing this app.")

        signal accepted()
        signal rejected()

        Button {
            text: i18n.tr("I understand the risks")
            color: theme.palette.normal.negative
            onClicked: {
                unconfinedWarningDialog.accepted()
                PopupUtils.close(unconfinedWarningDialog)
            }
        }

        Button {
            text: i18n.tr("Cancel")
            onClicked: {
                unconfinedWarningDialog.rejected();
                PopupUtils.close(unconfinedWarningDialog)
            }
        }
    }
}

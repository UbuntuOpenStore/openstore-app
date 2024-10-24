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
    id: donatingPopup
    Dialog {
        id: donatingDialog
        title: i18n.tr("Donating")
        text: i18n.tr("Would you like to support this app with a donation to the developer?")

        signal accepted()
        signal rejected()

        Button {
            text: i18n.tr("Donate now")
            color: theme.palette.normal.positive
            onClicked: {
                donatingDialog.accepted()
                PopupUtils.close(donatingDialog)
            }
        }
        Button {
            text: i18n.tr("Maybe later")
            onClicked: {
                donatingDialog.rejected();
                PopupUtils.close(donatingDialog)
            }
        }
    }
}

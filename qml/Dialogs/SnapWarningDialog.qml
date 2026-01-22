/*
 * Copyright (C) 2026 - Brian Douglass
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
import Qt.labs.settings 1.0

Component {
    id: snapWarningPopup
    Dialog {
        id: snapWarningDialog
        title: i18n.tr("Warning")
        text: i18n.tr("Not all snaps were designed with Ubuntu Touch in mind and might not work well. Additionally, the OpenStore is not aware of what permissions snap packages request.")

        signal accepted()
        signal rejected()

        Settings {
            id: dialogSettings
            property bool hideSnapWarning: false
        }

        CheckBox {
            id: dontShowAgainCheckbox
            text: i18n.tr("Don't show this warning again")
        }

        Button {
            text: i18n.tr("I understand")
            color: theme.palette.normal.negative
            onClicked: {
                if (dontShowAgainCheckbox.checked) {
                    dialogSettings.hideSnapWarning = true
                }
                snapWarningDialog.accepted()
                PopupUtils.close(snapWarningDialog)
            }
        }

        Button {
            text: i18n.tr("Cancel")
            onClicked: {
                snapWarningDialog.rejected();
                PopupUtils.close(snapWarningDialog)
            }
        }
    }
}

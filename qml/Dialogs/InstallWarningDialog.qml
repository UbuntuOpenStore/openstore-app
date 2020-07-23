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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0

Component {
    id: installQuestion

    Dialog {
        id: installQuestionDialog
        title: i18n.tr("Install unknown app?")
        text: i18n.tr("Do you want to install the unkown app %1?").arg(fileName)

        property string fileName
        signal accepted();
        signal rejected();

        ActivityIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: running
            running: PlatformIntegration.clickInstaller.busy
        }

        Button {
            text: i18n.tr("Install")
            color: theme.palette.normal.positive
            visible: !PlatformIntegration.clickInstaller.busy
            onClicked: {
                installQuestionDialog.accepted()
            }
        }
        Button {
            text: i18n.tr("Cancel")
            visible: !PlatformIntegration.clickInstaller.busy
            onClicked: {
                installQuestionDialog.rejected()
                PopupUtils.close(installQuestionDialog)
            }
        }

        Connections {
            target: PlatformIntegration.clickInstaller
            onPackageInstalled: PopupUtils.close(installQuestionDialog)
            onPackageInstallationFailed: PopupUtils.close(installQuestionDialog)
        }
    }
}

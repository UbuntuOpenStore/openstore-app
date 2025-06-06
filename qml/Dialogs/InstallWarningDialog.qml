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
import OpenStore 1.0

Component {
    id: installQuestion

    Dialog {
        id: installQuestionDialog
        title: i18n.tr("Install unknown app?")
        text: i18n.tr("Do you want to install the unknown app %1? Installing apps from outside the OpenStore is not recommended and can potentially harm your system. Only install this app if you got it from a trusted source.").arg(fileName)

        property bool wasAccepted: false
        property string fileName
        signal accepted();
        signal rejected();

        // We can't use PlatformIntegration.clickInstaller.busy
        // because app immediately freezes once the installation process starts
        // then we delay the installation to let UI changes take effect first
        Timer {
            id: delayedAccepted
            interval: 1
            onTriggered: accepted()
        }

        ActivityIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: running
            running: installQuestionDialog.wasAccepted
        }

        Button {
            text: i18n.tr("I understand the risks")
            color: theme.palette.normal.positive
            visible: !installQuestionDialog.wasAccepted
            onClicked: {
                installQuestionDialog.wasAccepted = true
                delayedAccepted.restart()
            }
        }

        Button {
            text: i18n.tr("Cancel")
            visible: !installQuestionDialog.wasAccepted
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

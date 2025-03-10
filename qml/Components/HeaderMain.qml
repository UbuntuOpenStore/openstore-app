/*
 * Copyright (C) 2020 - Joan Cibersheep
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

import QtQuick 2.9
import Lomiri.Components 1.3

HeaderBase {
    trailingActionBar {
        actions: [
            Action {
                iconName: "settings"
                shortcut: "Ctrl+s"
                text: i18n.tr("Settings")

                onTriggered: {
                    Qt.inputMethod.hide();
                    if (root.mainStackPage !== "discoverPage") {
                        pageStack.pop()
                    }
                    pageStack.push(Qt.resolvedUrl("../SettingsPage.qml"));
                }
            },
            Action {
                iconName: "find"
                shortcut: "Ctrl+f"
                text: i18n.tr("Search")

                onTriggered: {
                    Qt.inputMethod.hide();
                    if (root.mainStackPage !== "discoverPage") {
                        pageStack.pop()
                    }
                    pageStack.push(Qt.resolvedUrl("../SearchPage.qml"));
                }
            }
        ]
    }
}

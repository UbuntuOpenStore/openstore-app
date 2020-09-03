/*
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
 *
 */

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0

import "Components" as Components
import "Dialogs" as Dialogs

Page {
    id: searchPage
    objectName: "searchPage"

    property alias searchText: searchField.text
    property alias searchField: searchField
    property alias queryUrl: view.queryUrl

    header: Components.HeaderBase {
        title: i18n.tr("Search")
        contents: TextField {
            id: searchField
            anchors.centerIn: parent
            width: Math.min(parent.width, units.gu(36))

            // Disable predictive text
            inputMethodHints: Qt.ImhNoPredictiveText

            primaryItem: Icon {
                width: units.gu(2); height: width
                name: "find"
            }
            placeholderText: i18n.tr("Search in OpenStore...")
        }

        trailingActionBar {
            actions: [
                Action {
                    iconName: 'filters'
                    text: i18n.tr('Filters')

                    onTriggered: {
                        var popup = PopupUtils.open(filterDialog, searchPage, {
                            selectedSort: view.sortMode ? view.sortMode : 'relevance',
                            selectedType: view.filterType,
                        });
                        popup.accepted.connect(function(selectedSort, selectedType) {
                            PopupUtils.close(popup);
                            view.sortMode = selectedSort;
                            view.filterType = selectedType;
                        });
                        popup.rejected.connect(function(selectedSort, selectedType) {
                            PopupUtils.close(popup);
                        });
                    }
                }
            ]
        }
    }

    FilteredAppList {
        id: view
        anchors.fill: parent
        anchors.topMargin: searchPage.header.height

        onAppDetailsRequired: {
            PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady)
            PackagesCache.getPackageDetails(appId)
        }

        Timer {
            id: searchTimer
            interval: 500
            onTriggered: view.filterString = searchField.text
        }

        Connections {
            target: searchField
            onTextChanged: searchTimer.restart()
        }
    }

    Component.onCompleted: {
        if (searchField.text == "") {
            searchField.forceActiveFocus()
        } else {
            searchTimer.restart()
        }
    }

    Dialogs.FilterDialog {
        id: filterDialog
    }
}

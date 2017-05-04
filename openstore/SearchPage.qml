/*
 * Copyright (C) 2017 - Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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

Page {
    id: searchPage

    property alias model: view.model
    property alias query: searchField.text

    signal appDetailsRequired(var appId)

    header: PageHeader {
        title: i18n.tr("Search")
        automaticHeight: false
        leadingActionBar.actions: null
        trailingActionBar {
            anchors.rightMargin: 0
            delegate: TextualButtonStyle {}

            actions: Action {
                text: i18n.tr("Cancel")

                onTriggered: {
                    // Clear the search
                    searchField.text = ""
                    searchPage.pageStack.removePages(searchPage)
                }
            }
        }

        contents: TextField {
            id: searchField
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            primaryItem: Icon {
                height: units.gu(2); width: height
                name: "search"
            }

            placeholderText: i18n.tr("search in OpenStore...")
            onTextChanged: view.search(text)
            Component.onCompleted: view.search(text)

            // Disable predictive text
            inputMethodHints: Qt.ImhNoPredictiveText

            onVisibleChanged: forceActiveFocus()
        }
    }

    FilteredAppView {
        id: view

        // FIXME: TODO: Use "number of downloads" or "last updated" when they'll be available
        sortOrder: Qt.AscendingOrder
        sortProperty: "name"
        filterProperty: "searchHackishString"
        //filterPattern: new RegExp("$a") // A kind way to say SortFilterModel not to match anything until searchField is filled.

        function search(text) {
            //view.filterPattern = text ? new RegExp(text, 'i') : new RegExp("$a")
            view.filterPattern = new RegExp(text, 'i')
        }

        onAppDetailsRequired: searchPage.appDetailsRequired(appId)
    }
}

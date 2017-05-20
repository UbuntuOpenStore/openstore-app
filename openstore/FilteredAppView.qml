/*
 * Copyright (C) 2015 Michael Zanetti <michael.zanetti@ubuntu.com>
 * Copyright (C) 2017 Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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

ScrollView {
    id: rootItem
    anchors.fill: parent
    anchors.topMargin: parent.header ? parent.header.height : 0

    property var filterPattern: new RegExp()
    property string filterProperty

    property string sortProperty
    property int sortOrder

    property alias model: sortedFilteredAppModel.model
    property alias view: view

    property bool showTicks: true

    signal appDetailsRequired(var appId)

    ListView {
        id: view
        model: SortFilterModel {
            id: sortedFilteredAppModel

            filter.pattern: rootItem.filterPattern
            filter.property: rootItem.filterProperty

            sort.property: rootItem.sortProperty
            sort.order: rootItem.sortOrder
        }

        // TODO: Move it in Main.qml or elsewhere
        onCountChanged: {
            if (count > 0 && root.appIdToOpen != "") {
                var index = appModel.findApp(root.appIdToOpen)
                if (index >= 0) {
                    pageStack.addPageToNextColumn(mainPage, Qt.resolvedUrl("AppDetailsPage.qml"), {app: appModel.app(index)})
                    root.appIdToOpen = "";
                }
            }
        }

        delegate: ListItem {
            height: layout.height + divider.height

            ListItemLayout {
                id: layout
                title.text: model.name
                summary.text: model.tagline

                UbuntuShape {
                    SlotsLayout.position: SlotsLayout.Leading
                    aspect: UbuntuShape.Flat
                    image: Image {
                        source: model.icon
                        height: parent.height
                        width: parent.width
                    }
                }
                Icon {
                    SlotsLayout.position: SlotsLayout.Trailing
                    height: units.gu(2)
                    width: height
                    implicitHeight: height
                    implicitWidth: width
                    visible: model.installed && rootItem.showTicks
                    name: "tick"
                    color: model.updateAvailable ? UbuntuColors.orange : UbuntuColors.green
                }

                ProgressionSlot {}
            }
            onClicked: {
                rootItem.appDetailsRequired(model.appId)
            }
        }

        Loader {
            anchors.centerIn: parent
            active: view.count == 0
            sourceComponent: EmptyState {
                title: rootItem.filterProperty == "category" ? i18n.tr("Nothing here yet") : i18n.tr("No results found.").arg(rootItem.filterPattern)
                subTitle: rootItem.filterProperty == "category" ? i18n.tr("No app has been released in this department yet.") : i18n.tr("Try with a different search.")
                iconName: rootItem.filterProperty == "category" ? "ubuntu-store-symbolic" : "search"
                anchors.centerIn: parent
            }
        }
    }
}

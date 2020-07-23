/*
 * Copyright (C) 2015 Michael Zanetti <michael.zanetti@ubuntu.com>
 * Copyright (C) 2017 Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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

import QtQuick 2.4
import Ubuntu.Components 1.3
import OpenStore 1.0

import "Components" as Components

Item {
    id: rootItem

    property string filterString
    property string category
    property string sortMode
    property url queryUrl

    property alias view: viewLoader.item

    property bool showTicks: true

    signal appDetailsRequired(var appId)

    states: State {
        when: Boolean(true) // i.e. when this component is active
        PropertyChanges { target: searchModel; filterString: rootItem.filterString }
        PropertyChanges { target: searchModel; category: rootItem.category }
        PropertyChanges { target: searchModel; sortMode: rootItem.sortMode }
        PropertyChanges { target: searchModel; queryUrl: rootItem.queryUrl }
    }

    Loader {
        id: viewLoader
        anchors.fill: parent
        active: searchModel.count > 0
        sourceComponent: width > units.gu(80) ? gridViewComponent : listViewComponent
    }

    Component {
        id: listViewComponent

        ScrollView {
            anchors.fill: parent
            ListView {
                id: view

                // WORKAROUND: Fix for wrong grid unit size
                Component.onCompleted: root.flickable_responsive_scroll_fix(view)

                model: searchModel
                delegate: Components.PackageListItem {
                    appItem: model
                    onClicked: rootItem.appDetailsRequired(model.appId)
                    showTicks: rootItem.showTicks
                }
            }
        }
    }

    Component {
        id: gridViewComponent

        ScrollView {
            anchors.fill: parent

            GridView {
                id: view
                anchors.fill: parent
                anchors.margins: units.gu(2)

                // WORKAROUND: Fix for wrong grid unit size
                Component.onCompleted: root.flickable_responsive_scroll_fix(view)

                cellWidth: units.gu(16)
                cellHeight: units.gu(24)

                model: searchModel
                delegate: Components.PackageTile {
                    appItem: model
                    onClicked: rootItem.appDetailsRequired(model.appId)
                    width: view.cellWidth - units.gu(4)
                    height: view.cellHeight - units.gu(4)
                }
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        active: searchModel.count == 0
        sourceComponent: Components.EmptyState {
            title: rootItem.filterProperty == "category" ? i18n.tr("Nothing here yet") : i18n.tr("No results found.").arg(rootItem.filterPattern)
            subTitle: rootItem.filterProperty == "category" ? i18n.tr("No app has been released in this category yet.") : i18n.tr("Try with a different search.")
            iconName: rootItem.filterProperty == "category" ? "ubuntu-store-symbolic" : "search"
            anchors.centerIn: parent
        }
    }
}

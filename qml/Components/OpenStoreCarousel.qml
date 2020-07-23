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

Column {
    id: rootItem

    property string title
    property string subtitle
    property bool showProgression: true

    signal titleClicked()
    signal appTileClicked(var appItem)

    property alias viewModel: view.model
    function packageInfoGetter(i) { return null }

    property int tilesWidth: units.gu(10)

    spacing: units.gu(1)

    ListItem {
        divider.visible: false
        onClicked: rootItem.titleClicked()
        visible: rootItem.title || rootItem.subtitle

        ListItemLayout {
            anchors.centerIn: parent
            title.text: rootItem.title
            subtitle.text: rootItem.subtitle
            subtitle.wrapMode: Text.WordWrap

            ProgressionSlot {
                visible: rootItem.showProgression
            }
        }
    }

    ListView {
        id: view
        anchors { left: parent.left; right: parent.right }
        leftMargin: units.gu(2)
        rightMargin: units.gu(2)

        cacheBuffer: 0
        clip: false

        height: count > 0 ? rootItem.tilesWidth * 1.5 + units.gu(5) : 0
        visible: count > 0

        spacing: units.gu(1.5)

        orientation: ListView.Horizontal
        delegate: PackageTile {
            id: appDel

            appItem: rootItem.packageInfoGetter(modelData ? modelData : model.index)
            height: parent.height
            width: rootItem.tilesWidth

            onClicked: rootItem.appTileClicked(appDel.appItem)

            // WORKAROUND: Delay removal until image is loaded. That should
            // prevent potential race conditions.
            ListView.delayRemove: true
            onImageLoaded: removalTimer.restart()
            Timer {
               id: removalTimer
               interval: 3000
               onTriggered: ListView.delayRemove = false
            }
        }
    }
}

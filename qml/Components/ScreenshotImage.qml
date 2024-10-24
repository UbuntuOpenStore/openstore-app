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
import Lomiri.Components 1.3

Item {
    id: zI
    width: parent.width
    height: parent.height
    z: Number.MAX_VALUE

    Rectangle {
        id: zIbg
        parent: zI.parent
        anchors.fill: parent
        color: "black"
        opacity: 0
    }

    property real itemScale: 1
    property var model
    property int currentIndex

    transform: Scale {
        origin.x: 0
        origin.y: 0
        xScale: zI.itemScale
        yScale: zI.itemScale
    }

    ParallelAnimation {
        id: scaleInAnimation
        onStarted: {
            hideAnimation.initialScale = itemScale;
            hideAnimation.initialX = x;
            hideAnimation.initialY = y;
        }

        LomiriNumberAnimation { target: zI; property: "itemScale"; to: 1 }
        LomiriNumberAnimation { target: zI; properties: "x,y"; to: 0 }
        LomiriNumberAnimation { target: zIbg; property: "opacity"; to: 1 }
    }


    Component.onCompleted: {
        scaleInAnimation.start();
    }

    MouseArea {
        // Capture all mouse/touch events below
        anchors.fill: parent
        onWheel: wheel.accepted = true  // wheel events are not captured by default
    }

    ListView {
        anchors.fill: parent
        clip: true

        model: zI.model
        currentIndex: zI.currentIndex
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        delegate: Image {
            width: zI.width
            height: zI.height
            source: modelData
            fillMode: Image.PreserveAspectFit
       }
    }

    Rectangle {
        anchors.fill: closeBtn
        opacity: 0.5
        color: "black"
    }

    AbstractButton {
        id: closeBtn
        anchors.top: parent.top
        anchors.left: parent.left

        width: units.gu(8)
        height: width

        styleName: "IconButtonStyle"
        StyleHints {
            foregroundColor: "white"
            backgroundColor: closeBtn.pressed ? "#666666" : "transparent"
        }

        action: Action {
            iconName: "close"
            onTriggered: {
                hideAnimation.start()
            }
        }
    }

    ParallelAnimation {
        id: hideAnimation
        property real initialScale: 1
        property int initialX: 0
        property int initialY: 0


        LomiriNumberAnimation { target: zI; property: "itemScale"; to: hideAnimation.initialScale }
        LomiriNumberAnimation { target: zI; property: "x"; to: hideAnimation.initialX }
        LomiriNumberAnimation { target: zI; property: "y"; to: hideAnimation.initialY }
        LomiriNumberAnimation { target: zIbg; property: "opacity"; to: 0 }
        onStopped: {
            script: zI.destroy()
        }

    }
}

/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
 * Copyright (C) 2017 - Michael Zanetti <verzegnassi.stefano@gmail.com>
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

ListItem {
    height: units.gu(32)

    property alias model: screenshotsView.model

    visible: screenshotsView.count

    ListView {
        id: screenshotsView
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        leftMargin: units.gu(2)
        rightMargin: units.gu(2)
        clip: true
        height: count > 0 ? units.gu(24) : 0
        visible: count > 0
        spacing: units.gu(1)
        orientation: ListView.Horizontal
        delegate: UbuntuShape {
            height: parent.height
            // sh : lv.h = sw : x
            width: screenshot.sourceSize.width * height / screenshot.sourceSize.height
            aspect: UbuntuShape.Flat
            sourceFillMode: UbuntuShape.PreserveAspectFit
            source: Image {
                id: screenshot
                source: modelData
                smooth: true
                antialiasing: true
            }

            AbstractButton {
                id: screenShotButton
                anchors.fill: parent
                onClicked: {
                    print("opening at:", screenShotButton.mapToItem(root, 0, 0).x)
                    var zoomIn = Qt.createComponent(Qt.resolvedUrl("../Components/ScreenshotImage.qml"));

                    var props = {
                        x: screenShotButton.mapToItem(root, 0, 0).x,
                        y: screenShotButton.mapToItem(root, 0, 0).y,
                        itemScale: screenShotButton.height / root.height,
                        model: screenshotsView.model,
                        currentIndex: model.index
                    }

                    zoomIn.createObject(root, props);
                    //                            zoomIn.createObject(root, {x: 100, y: 100});
                }
            }
        }
    }
}

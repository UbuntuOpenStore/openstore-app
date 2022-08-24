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
import Ubuntu.Components 1.3

Item {
    property alias imageUrl: highlightedAppImg.source
    property alias appName: hAppName.text
    property alias appAuthor: hAppAut.text
    property alias appDesc: hAppDesc.text
    property alias appRatings: ratingsRow.appRatings

    property bool ready: false
    //appStatus 0 n/a, 1 Update available, 2 installed
    property int appStatus

    signal buttonClicked();

    width: parent.width
    height: highlightedAppImg.height + infoRect.height

    Image {
        id: highlightedAppImg
        width: parent.width
        height: units.gu(14)
        sourceSize.width: parent.width
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        verticalAlignment: Image.AlignVCenter

        onStatusChanged: {
            //TODO: Handle Image.Error
            ready = status === Image.Ready
        }
    }

    Rectangle {
        id: infoRect
        width: parent.width
        height: units.gu(20)
        visible: ready
        color: appColor
        anchors.top: highlightedAppImg.bottom

        Row {
            width: parent.width - units.gu(4)
            spacing: units.gu(2)

            anchors {
                top: parent.top
                topMargin: units.gu(2)
                horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: haBasic
                width: parent.width * .5 - units.gu(2)
                spacing: units.gu(2)

                LabelCentered {
                    id: hAppName
                    color: appColorText
                    textSize: Label.Large
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }

                LabelCentered {
                    id: hAppAut
                    color: appColorText
                    textSize: Label.Medium
                    maximumLineCount: 1
                    elide: Text.ElideRight
                }

                RatingsRow {
                    id: ratingsRow
                    visible: parent.width > units.gu(15)
                    spacing: (parent.width - units.gu(12.5)) / 6

                    anchors {
                        leftMargin: spacing
                        left: parent.left
                    }
                }
            }

            Column {
                id: haInfo
                spacing: units.gu(2)
                width: parent.width * .5

                Button {
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: { switch(appStatus) {
                          case 1:
                            return i18n.tr("Update");
                            break;
                          case 2:
                            return i18n.tr("Open");
                            break;
                          default:
                            return i18n.tr("Details");
                        }
                    }

                    onClicked: buttonClicked();
                }

                LabelCentered {
                    id: hAppDesc
                    horizontalAlignment: Text.AlignJustify
                    textSize: Label.Small
                    elide: Text.ElideRight
                    maximumLineCount: 6
                    color: appColorText
                }
            }
        }
    }
}


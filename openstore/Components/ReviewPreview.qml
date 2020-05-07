/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0


ListItem {
    id: reviewPreviewListItem
    height: reviewPreviewColumn.height
    property var reviews
    readonly property int count: reviews.reviewCount
    readonly property int maxLength: 512
    property var ownReview: null
    property bool ready: false

    Connections {
        target: reviews

        onOwnReviewResponse: {
            ownReview = 'body' in review ? review : null
            ready = true
        }

        onReviewPosted: PopupUtils.open(successPostDialog)

        onError: {
            errorText = text
            PopupUtils.open(errorDialog)
        }
    }

    function getRatingEmoji(rating) {
        switch(rating) {
            case 0:
              return "../Assets/thumbup.svg"
            case 1:
              return "../Assets/thumbdown.svg"
            case 3:
              return "../Assets/happy.svg"
            case 2:
              return "../Assets/neutral.svg"
            case 4:
              return "../Assets/buggy.svg"
        }

        console.log("DEBUGGING: getRatingEmoji returned unexpected rating", rating)
        return "../Assets/neutral.svg"
    }

    property string errorText: i18n.tr("Something went wrong...")

    Component {
         id: errorDialog
         Dialog {
             id: dialogue
             title: reviewPreviewListItem.errorText
             Rectangle {
                 height: units.gu(12)
                 color: "transparent"
                 Icon {
                     anchors.centerIn: parent
                     width: units.gu(8)
                     height: width
                     name: "edit-clear"
                 }
             }
             Button {
                 text: i18n.tr("Close")
                 onClicked: PopupUtils.close(dialogue)
             }
         }
    }

    Component {
         id: successPostDialog
         Dialog {
             id: dialogue
             title: i18n.tr("Your review has been posted")
             Rectangle {
                 height: units.gu(12)
                 color: "transparent"
                 Icon {
                     anchors.centerIn: parent
                     width: units.gu(8)
                     height: width
                     name: "tick"
                 }
             }
             Button {
                 text: i18n.tr("Close")
                 onClicked: PopupUtils.close(dialogue)
             }
         }
    }

    Component {
        id: composeDialog
        Dialog {
            id: dialogue
            readonly property var buttonWidth: (textArea.width - 4*units.gu(2)) / 5
            title: ready ? i18n.tr("Rate this app") : i18n.tr("Loading...")
            Component.onCompleted: reviews.getOwnReview(root.apiKey)

            function postReview(rating, body) {
                if (ownReview === null) {
                    app.review(body, rating, root.apiKey)
                }
                else {
                    app.editReview(body, rating, root.apiKey)
                }
            }

            TextArea {
                id: textArea
                readOnly: !ready
                text: ownReview !== null ? ownReview.body : ""
                placeholderText: i18n.tr("(Optional) Write a review")
            }

            Label {
                text: i18n.tr("%1/%2 characters").arg(textArea.text.length).arg(maxLength)
                textSize: Label.Small
                color: textArea.text.length > maxLength ? UbuntuColors.red : theme.palette.normal.base
            }

            Item {
                width: parent.width
                height: buttonWidth
                visible: !ready

                ActivityIndicator {
                    running: !ready
                    visible: !ready
                    anchors.centerIn: parent
                }
            }

            Row {
                spacing: units.gu(2)
                anchors.horizontalCenter: parent.horizontalCenter
                visible: ready

                ButtonEmoji {
                    emoji: "../Assets/thumbup.svg"
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(0, textArea.displayText)
                    }
                }

                ButtonEmoji {
                    emoji: "../Assets/thumbdown.svg"
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(1, textArea.displayText)
                    }
                }

                ButtonEmoji {
                    emoji: "../Assets/happy.svg"
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(3, textArea.displayText)
                    }
                }

                ButtonEmoji {
                    emoji: "../Assets/neutral.svg"
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(2, textArea.displayText)
                    }
                }

                ButtonEmoji {
                    emoji: "../Assets/buggy.svg"
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(4, textArea.displayText)
                    }
                }
            }

            Button {
                text: i18n.tr("Cancel")
                onClicked: PopupUtils.close(dialogue)
            }
        }
    }

    Column {
        id: reviewPreviewColumn

        width: reviewPreviewListItem.width - units.gu(4)
        height: count > 0 ? units.gu(42) : addReviewButton.height + units.gu(4)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: units.gu(2)

        Item {
            width: parent.width
            height: addReviewButton.height + units.gu(count > 0 ? 2 : 1)
            Label {
                text: i18n.tr("%1 reviews").arg(getNumberShortForm(count))
                textSize: Label.Large
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: units.gu(0.5)
            }
            Button {
                text: i18n.tr("Sign in to review this app")
                visible: root.apiKey === ""
                onClicked: bottomEdgeStack.push(Qt.resolvedUrl("../SignInWebView.qml"))
                anchors.right: parent.right
                anchors.top: parent.top
            }
            Button {
                id: addReviewButton
                text: app.installed ? i18n.tr("Review app") : i18n.tr("Install to review this app")
                visible: root.apiKey !== ""
                onClicked: PopupUtils.open(composeDialog)
                color: UbuntuColors.green
                anchors.right: parent.right
                anchors.top: parent.top
                enabled: app.installed
            }
        }

        ListView {
            id: reviewsListView
            width: parent.width
            visible: count > 0
            height: parent.height - addReviewButton.height - units.gu(2)
            orientation: ListView.Horizontal
            spacing: units.gu(2)
            onContentXChanged: if ( atXEnd ) reviews.loadMore()
            delegate: UbuntuShape {
                property var review: model
                height: reviewsListView.height - units.gu(4)
                width: height * 1.3
                aspect: UbuntuShape.DropShadow
                backgroundColor: theme.palette.normal.background
                ScrollView {
                    id: reviewScrollView
                    anchors.fill: parent
                    anchors.margins: units.gu(2)
                    contentItem: Column {
                        width: reviewScrollView.width
                        spacing: units.gu(1)
                        Row {
                            spacing: units.gu(1)
                            UbuntuShape {
                                width: units.gu(4)
                                height: units.gu(4)
                                //aspect: UbuntuShape.DropShadow

                                Icon {
                                    width: units.gu(3)
                                    height: width
                                    anchors.centerIn: parent
                                    asynchronous: true
                                    color: theme.palette.normal.baseText
                                    source: getRatingEmoji(review.rating)
                                }
                            }
                            Column {
                                Label {
                                    text: review.author
                                    font.bold: true
                                }
                                Label {
                                    text: (new Date(review.date)).toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                                    textSize: Label.Small
                                }
                            }
                        }
                        Label {
                            width: reviewScrollView.width
                            text: review.body
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
            model: reviews
        }
    }


}

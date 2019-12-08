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
    property var reviews: []
    readonly property int maxLength: 512

    function getRatingEmoji(rating) {
        switch(rating) {
            case ReviewItem.RatingThumbsUp: return "👍"
            case ReviewItem.RatingThumbsDown: return "👎"
            case ReviewItem.RatingHappy: return "🙂"
            case ReviewItem.RatingNeutral: return "😐"
            case ReviewItem.RatingBuggy: return "🐛"
        }
        return "😐"
    }

    function reviewsWithBody() {
        var reviewsWithBody = []
        for(var i = 0; i < reviews.length; i++) {
            if (reviews[i].body !== "") {
                reviewsWithBody[reviewsWithBody.length] = reviews[i]
            }
        }
        return reviewsWithBody
    }

    function postReview(rating, body) {
        app.review(body, rating, root.apiKey)
    }

    Component {
         id: dialog
         Dialog {
             id: dialogue
             readonly property var buttonWidth: (textArea.width - 4*units.gu(2)) / 5
             title: i18n.tr("Post a rating")
             TextArea {
                 id: textArea
                 placeholderText: i18n.tr("(Optional) Write a review.")
             }
             Label {
                 text: i18n.tr("%1/%2 characters").arg(textArea.text.length).arg(maxLength)
                 textSize: Label.Small
                 color: textArea.text.length > maxLength ? UbuntuColors.red : theme.palette.normal.base
             }
             Row {
                 spacing: units.gu(2)
                 anchors.horizontalCenter: parent.horizontalCenter
                 Button {
                    color: theme.palette.normal.background
                    text: "👍"
                    width: buttonWidth
                    height: width
                    enabled: textArea.text.length <= maxLength
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(ReviewItem.RatingThumbsUp, textArea.text)
                    }
                 }
                 Button {
                    color: theme.palette.normal.background
                    text: "👎"
                    width: buttonWidth
                    height: width
                    enabled: textArea.text.length <= maxLength
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(ReviewItem.RatingThumbsDown, textArea.text)
                    }
                 }
                 Button {
                    color: theme.palette.normal.background
                    text: "🙂"
                    width: buttonWidth
                    height: width
                    enabled: textArea.text.length <= maxLength
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(ReviewItem.RatingHappy, textArea.text)
                    }
                 }
                 Button {
                    color: theme.palette.normal.background
                    text: "😐"
                    width: buttonWidth
                    height: width
                    enabled: textArea.text.length <= maxLength
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(ReviewItem.RatingNeutral, textArea.text)
                    }
                 }
                 Button {
                    color: theme.palette.normal.background
                    text: "🐛"
                    width: buttonWidth
                    height: width
                    enabled: textArea.text.length <= maxLength
                    onClicked: {
                        PopupUtils.close(dialogue)
                        postReview(ReviewItem.RatingBuggy, textArea.text)
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
        height: units.gu(42)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: units.gu(2)

        Item {
            width: parent.width
            height: addReviewButton.height + units.gu(2)
            Label {
                text: i18n.tr("%1 reviews").arg(reviewsWithBody().length)
                textSize: Label.Large
                anchors.left: parent.left
                anchors.top: parent.top
            }
            Button {
                text: i18n.tr("Sign in to add review")
                visible: root.apiKey === ""
                onClicked: bottomEdgeStack.push(Qt.resolvedUrl("../SignInWebView.qml"))
                anchors.right: parent.right
                anchors.top: parent.top
            }
            Button {
                id: addReviewButton
                text: i18n.tr("Add review")
                visible: root.apiKey !== ""
                onClicked: PopupUtils.open(dialog)
                color: UbuntuColors.green
                anchors.right: parent.right
                anchors.top: parent.top
            }
        }

        ListView {
            id: reviewsListView
            width: parent.width
            height: parent.height - addReviewButton.height - units.gu(2)
            orientation: ListView.Horizontal
            spacing: units.gu(2)
            delegate: UbuntuShape {
                property var review: reviews[index]
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
                                aspect: UbuntuShape.DropShadow
                                Label {
                                    text: "👍"//getRatingEmoji(review.rating)
                                    textSize: Label.XLarge
                                    anchors.centerIn: parent
                                }
                            }
                            Column {
                                Label {
                                    text: review.author_displayname
                                    font.bold: true
                                }
                                Label {
                                    text: (new Date(review.date)).toLocaleString(Qt.locale(), Locale.ShortFormat)
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
            model: reviewsWithBody()
        }
    }


}
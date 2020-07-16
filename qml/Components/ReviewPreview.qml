/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0

ListItem {
    id: reviewPreviewListItem
    height: reviewPreviewColumn.height

    property var reviews
    readonly property int count: reviews.reviewCount
    readonly property int maxLength: 512
    property var ownReview: null
    property var ownRating: null
    property bool ready: false
    property bool first: true

    signal ratingUpdated(var rating)
    signal reviewPosted()

    Component.onCompleted: {
        if (root.apiKey) {
            reviews.getOwnReview(root.apiKey);
        }
    }

    Connections {
        target: reviews

        onOwnReviewResponse: {
            ownReview = 'body' in review ? review : null;
            ownRating = rating < 0 ? null : rating;

            if (!ready) {
                ratingUpdated(rating);

                ready = true;
            }
        }

        onReviewPosted: {
            PopupUtils.open(successPostDialog)
            reviewPreviewListItem.reviewPosted();
            ratingUpdated(ownRating);
        }

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
            title: i18n.tr("Error Posting Review")
            text: reviewPreviewListItem.errorText

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
            title: i18n.tr("Review Posted Correctly")
            text: i18n.tr("Your review has been posted successfully.")

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
            Component.onCompleted: reviews.getOwnReview(root.apiKey);

            function postReview(rating, body) {
                if (ownReview === null) {
                    app.review(body, rating, root.apiKey)
                }
                else {
                    app.editReview(body, rating, root.apiKey)
                }

                //Reset bool to update current rating
                ready = false
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
                color: textArea.text.length > maxLength ? palette.theme.normal.negative : theme.palette.normal.base
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
                    iconSelected: ownRating == 0
                    onClicked: ownRating = 0
                }

                ButtonEmoji {
                    emoji: "../Assets/happy.svg"
                    iconSelected: ownRating == 3
                    onClicked: ownRating = 3
                }

                ButtonEmoji {
                    emoji: "../Assets/neutral.svg"
                    iconSelected: ownRating == 2
                    onClicked: ownRating = 2
                }

                ButtonEmoji {
                    emoji: "../Assets/thumbdown.svg"
                    iconSelected: ownRating == 1
                    onClicked: ownRating = 1
                }

                ButtonEmoji {
                    emoji: "../Assets/buggy.svg"
                    iconSelected: ownRating == 4
                    onClicked: ownRating = 4
                }
            }

            Button {
                text: ownReview ? i18n.tr("Update") : i18n.tr('Submit')
                enabled: ownRating != null
                color: theme.palette.normal.positive

                onClicked: {
                    PopupUtils.close(dialogue)
                    postReview(ownRating, textArea.displayText)
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

        width: parent.width
        height: count > 0
            ? units.gu(32)
            : addReviewButton.height
        anchors.top: parent.top
        anchors.left: parent.left

        ListItem {
            id: addReviewButton
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            divider.visible: false

            ListItemLayout {
                property string reviewCountTxt: count > 0
                    ? i18n.tr("%1 review. ","%1 reviews. ", count).arg(getNumberShortForm(count))
                    : ""

                anchors.fill: parent
                title.text: {
                    if (app.installed) {
                        if (root.apiKey) {
                            if (ownReview) {
                                return reviewCountTxt + i18n.tr("Edit Review");
                            }

                            return reviewCountTxt + i18n.tr("Review app");
                        }

                        return reviewCountTxt + i18n.tr("Sign in to review this app");
                    }

                    return reviewCountTxt + i18n.tr("Install this app to review it");
                }

                title.color: theme.palette.normal.backgroundText
                ProgressionSlot {
                    visible: app.installed
                }
            }

            // TODO: Check if the app is of the owner before showing Preview Dialog
            onClicked: {
                if (app.installed) {
                    root.apiKey === ""
                        ? bottomEdgeStack.push(Qt.resolvedUrl("../SignInPage.qml"))
                        : PopupUtils.open(composeDialog)
                } else {
                    console.log("App is not installed")
                }
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

            model: reviews

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
        }
    }
}

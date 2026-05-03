/*
 * Copyright (C) 2026 Brian Douglass
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
import Lomiri.Components 1.3

Column {
    id: contentRating

    property var contentRatingData: null

    // Labels and descriptions from OARS: https://github.com/hughsie/oars/blob/master/docs/generate.html
    property var attributeLabels: {
        "violence-cartoon": i18n.tr("Cartoon Violence"),
        "violence-fantasy": i18n.tr("Fantasy Violence"),
        "violence-realistic": i18n.tr("Realistic Violence"),
        "violence-bloodshed": i18n.tr("Bloodshed"),
        "violence-sexual": i18n.tr("Sexual Violence"),
        "violence-desecration": i18n.tr("Desecration"),
        "violence-slavery": i18n.tr("Human Slavery"),
        "drugs-alcohol": i18n.tr("Alcohol"),
        "drugs-narcotics": i18n.tr("Narcotics"),
        "drugs-tobacco": i18n.tr("Tobacco"),
        "sex-nudity": i18n.tr("Nudity"),
        "sex-themes": i18n.tr("Sexual Themes"),
        "language-profanity": i18n.tr("Profanity"),
        "language-humor": i18n.tr("Humor"),
        "language-discrimination": i18n.tr("Discrimination"),
        "social-chat": i18n.tr("Online Text Messaging"),
        "social-info": i18n.tr("Information Sharing"),
        "social-audio": i18n.tr("Audio & Video Chat"),
        "social-location": i18n.tr("Location Sharing"),
        "social-contacts": i18n.tr("Contact Sharing"),
        "money-purchasing": i18n.tr("In-App Purchases"),
        "money-advertising": i18n.tr("Advertising"),
        "money-gambling": i18n.tr("Gambling")
    }

    property var attributeDescriptions: {
        "violence-cartoon": {
            "mild": i18n.tr("Cartoon characters in unsafe situations"),
            "moderate": i18n.tr("Cartoon characters in aggressive conflict"),
            "intense": i18n.tr("Cartoon characters showing graphic violence")
        },
        "violence-fantasy": {
            "mild": i18n.tr("Fantasy characters in unsafe situations"),
            "moderate": i18n.tr("Fantasy characters in aggressive conflict"),
            "intense": i18n.tr("Fantasy characters with graphic violence")
        },
        "violence-realistic": {
            "mild": i18n.tr("Realistic characters in unsafe situations"),
            "moderate": i18n.tr("Realistic characters in depictions of aggressive conflict"),
            "intense": i18n.tr("Realistic characters with graphic violence")
        },
        "violence-bloodshed": {
            "mild": i18n.tr("Unrealistic bloodshed"),
            "moderate": i18n.tr("Realistic bloodshed"),
            "intense": i18n.tr("Depictions of bloodshed and the mutilation of body parts")
        },
        "violence-sexual": {
            "intense": i18n.tr("Rape or other violent sexual behavior")
        },
        "violence-desecration": {
            "mild": i18n.tr("Visible dead human remains"),
            "moderate": i18n.tr("Dead human remains that are exposed to the elements"),
            "intense": i18n.tr("Graphic depictions of desecration of human bodies, for example being eaten by wild animals")
        },
        "violence-slavery": {
            "mild": i18n.tr("Depictions or references to historical slavery"),
            "moderate": i18n.tr("Depictions of modern-day slavery"),
            "intense": i18n.tr("Graphic depictions of modern-day slavery")
        },
        "drugs-alcohol": {
            "mild": i18n.tr("References to alcoholic beverages"),
            "moderate": i18n.tr("Use of alcoholic beverages")
        },
        "drugs-narcotics": {
            "mild": i18n.tr("References to illicit drugs"),
            "moderate": i18n.tr("Use of illicit drugs")
        },
        "drugs-tobacco": {
            "mild": i18n.tr("References to tobacco products"),
            "moderate": i18n.tr("Use of tobacco products")
        },
        "sex-nudity": {
            "mild": i18n.tr("Brief artistic nudity"),
            "moderate": i18n.tr("Prolonged nudity"),
            "intense": i18n.tr("Explicit nudity showing nipples or sexual organs")
        },
        "sex-themes": {
            "mild": i18n.tr("Provocative references or depictions"),
            "moderate": i18n.tr("Sexual references or depictions"),
            "intense": i18n.tr("Graphic sexual behavior")
        },
        "language-profanity": {
            "mild": i18n.tr("Mild or infrequent use of profanity"),
            "moderate": i18n.tr("Moderate use of profanity"),
            "intense": i18n.tr("Strong or frequent use of profanity")
        },
        "language-humor": {
            "mild": i18n.tr("Slapstick humor"),
            "moderate": i18n.tr("Vulgar or bathroom humor"),
            "intense": i18n.tr("Mature or sexual humor")
        },
        "language-discrimination": {
            "mild": i18n.tr("Negativity towards a specific group of people, e.g. ethnic jokes"),
            "moderate": i18n.tr("Discrimination designed to cause emotional harm, e.g. racism or homophobia"),
            "intense": i18n.tr("Explicit discrimination based on gender, sexuality, race or religion")
        },
        "social-chat": {
            "mild": i18n.tr("User-to-user interactions without chat functionality"),
            "moderate": i18n.tr("Moderated messaging between users"),
            "intense": i18n.tr("Uncontrolled chat functionality between users")
        },
        "social-info": {
            "mild": i18n.tr("Using any online API, e.g. a user-counter"),
            "moderate": i18n.tr("Sharing diagnostic data not identifiable to the user"),
            "intense": i18n.tr("Sharing information identifiable to the user, e.g. crash dumps")
        },
        "social-audio": {
            "moderate": i18n.tr("Moderated audio or video chat between users"),
            "intense": i18n.tr("Uncontrolled audio or video chat between users")
        },
        "social-location": {
            "intense": i18n.tr("Sharing physical location to other users")
        },
        "social-contacts": {
            "intense": i18n.tr("Sharing contact details with other users")
        },
        "money-purchasing": {
            "mild": i18n.tr("Users are encouraged to donate real money, e.g. using Patreon"),
            "intense": i18n.tr("Ability to spend real money in-app, e.g. buying new content or new levels")
        },
        "money-advertising": {
            "mild": i18n.tr("Product placement, e.g. billboards in a football game"),
            "moderate": i18n.tr("Explicit references to specific brands or trademarked products"),
            "intense": i18n.tr("Users are encouraged to purchase specific real-world items")
        },
        "money-gambling": {
            "mild": i18n.tr("Gambling on random events using tokens or credits"),
            "moderate": i18n.tr("Gambling using fictional money"),
            "intense": i18n.tr("Gambling using real money")
        }
    }

    property var severityLabels: {
        "mild": i18n.tr("Mild"),
        "moderate": i18n.tr("Moderate"),
        "intense": i18n.tr("Intense")
    }

    property var severityOrder: ["none", "mild", "moderate", "intense"]

    property bool isNotRated: !contentRatingData || !contentRatingData.type
    property var nonNoneAttributes: {
        if (!contentRatingData || !contentRatingData.content_attributes) {
            return [];
        }

        var attrs = contentRatingData.content_attributes;
        var filtered = [];
        for (var i = 0; i < attrs.length; i++) {
            if (attrs[i].value !== "none") {
                filtered.push(attrs[i]);
            }
        }
        return filtered;
    }
    property bool isAllAges: !isNotRated && nonNoneAttributes.length === 0

    property int maxSeverityIndex: {
        var max = 0;
        for (var i = 0; i < nonNoneAttributes.length; i++) {
            var idx = severityOrder.indexOf(nonNoneAttributes[i].value);
            if (idx > max) {
                max = idx;
            }
        }
        return max;
    }

    property var sortedAttributes: {
        var attrs = nonNoneAttributes.slice();
        attrs.sort(function(a, b) {
            return severityOrder.indexOf(b.value) - severityOrder.indexOf(a.value);
        });
        return attrs;
    }

    spacing: units.gu(1)
    width: parent.width

    ListItem {
        height: titleLayout.height
        divider.visible: false

        ListItemLayout {
            id: titleLayout
            anchors.centerIn: parent
            title.text: i18n.tr("Content Rating")
        }
    }

    ListItem {
        height: contentLayout.height + units.gu(2)
        divider.visible: false

        Column {
            id: contentLayout
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: units.gu(2)
            }
            spacing: units.gu(2)

            Item {
                width: parent.width
                height: Math.max(statusIcon.height, statusColumn.height)

                Icon {
                    id: statusIcon
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                    }
                    width: units.gu(5)
                    height: width
                    name: {
                        if (isNotRated) return "info";
                        if (isAllAges) return "security";
                        return "security-alert";
                    }
                    color: {
                        if (isNotRated) return theme.palette.normal.base;
                        if (isAllAges) return theme.palette.normal.positive;
                        if (maxSeverityIndex >= 3) return theme.palette.normal.negative;
                        if (maxSeverityIndex >= 2) return LomiriColors.orange;
                        return LomiriColors.warmGrey;
                    }
                }

                Column {
                    id: statusColumn
                    anchors {
                        left: statusIcon.right
                        right: parent.right
                        leftMargin: units.gu(2)
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: units.gu(0.5)

                    Label {
                        width: parent.width
                        text: {
                            if (isNotRated) return i18n.tr("This app has not been rated");
                            if (isAllAges) return i18n.tr("All Ages");
                            return "";
                        }
                        visible: isNotRated || isAllAges
                        font.weight: isAllAges ? Font.DemiBold : Font.Normal
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: parent.width
                        text: i18n.tr("No concerning content")
                        visible: isAllAges
                        textSize: Label.Small
                        color: theme.palette.normal.backgroundSecondaryText
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Column {
                width: parent.width
                spacing: units.gu(1.5)
                visible: !isNotRated && !isAllAges

                Repeater {
                    model: sortedAttributes

                    delegate: Column {
                        width: parent.width
                        spacing: units.gu(0.5)

                        property var attribute: sortedAttributes[index]

                        Row {
                            width: parent.width
                            spacing: units.gu(2)

                            Label {
                                text: attributeLabels[attribute.id] || attribute.id
                                width: parent.width - severityLabel.width - parent.spacing
                                wrapMode: Text.WordWrap
                            }

                            Label {
                                id: severityLabel
                                text: severityLabels[attribute.value] || attribute.value
                                font.weight: Font.DemiBold
                                color: {
                                    if (attribute.value === "intense") return theme.palette.normal.negative;
                                    if (attribute.value === "moderate") return LomiriColors.orange;
                                    if (attribute.value === "mild") return LomiriColors.warmGrey;
                                    return theme.palette.normal.foregroundText;
                                }
                            }
                        }

                        Label {
                            width: parent.width
                            text: {
                                if (attributeDescriptions[attribute.id] &&
                                    attributeDescriptions[attribute.id][attribute.value]) {
                                    return attributeDescriptions[attribute.id][attribute.value];
                                }
                                return "";
                            }
                            visible: text !== ""
                            textSize: Label.Small
                            wrapMode: Text.WordWrap
                            color: {
                                if (attribute.value === "intense") return theme.palette.normal.negative;
                                if (attribute.value === "moderate") return LomiriColors.orange;
                                if (attribute.value === "mild") return LomiriColors.warmGrey;
                                return theme.palette.normal.backgroundSecondaryText;
                            }
                        }
                    }
                }
            }

            Label {
                width: parent.width
                text: i18n.tr("Self reported rating using <a href=\"https://hughsie.github.io/oars/\">Open Age Rating Service</a>")
                textSize: Label.XSmall
                color: theme.palette.normal.backgroundSecondaryText
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
}

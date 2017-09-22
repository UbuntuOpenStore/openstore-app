/*
 * Copyright (C) 2017 - Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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

AdaptivePageLayout {
    id: rootItem

    signal categoryClicked(var name, var id)

    property bool __isThereSecondPage

    onCategoryClicked: {
        var pageProps = {
            title: name,
            category: id
        }

        var incubator = rootItem.addPageToNextColumn(rootItem.primaryPage, filteredAppPageComponent, pageProps)
        if (incubator && incubator.status == Component.Loading) {
            incubator.onStatusChanged = function(status) {
                if (status == Component.Ready) {
                    incubator.object.Component.destruction.connect(function() {
                        __isThereSecondPage = false
                    });
                }
            }
        }

        __isThereSecondPage = true
    }

    states: [
        State {
            when: __isThereSecondPage && rootItem.columns == 1
            PropertyChanges {
                target: tabView
                barVisible: false
            }
        }
    ]

    layouts: [
        PageColumnsLayout {
            when: rootItem.width >= units.gu(90)
            PageColumn {
                maximumWidth: units.gu(50)
                minimumWidth: units.gu(40)
                preferredWidth: units.gu(40)
            }
            PageColumn {
                fillWidth: true
            }
        },
        PageColumnsLayout {
            when: true
            PageColumn {
                fillWidth: true
            }
        }
    ]

    primaryPage: Page {
        id: categoryListPage

        header: PageHeader {
            title: i18n.tr("Categories")
        }

        ScrollView {
            anchors.fill: parent
            anchors.topMargin: categoryListPage.header.height

            UbuntuListView {
                id: categoryView
                anchors.fill: parent

                property int __currentTmpIndex
                currentIndex: rootItem.columns > 1 ? __currentTmpIndex : -1

                // WORKAROUND: Fix for wrong grid unit size
                Component.onCompleted: root.flickable_responsive_scroll_fix(categoryView)

                model: categoriesModel
                delegate: ListItem {
                    divider.anchors.leftMargin: units.gu(6.5)
                    onClicked: {
                        categoryView.__currentTmpIndex = model.index
                        rootItem.categoryClicked(model.name, model.id)
                    }
                    ListItemLayout {
                        anchors.centerIn: parent

                        Image {
                            SlotsLayout.position: SlotsLayout.Leading
                            source: model.iconUrl
                            width: units.gu(2.5); height: width
                            sourceSize: Qt.size(width, height)
                        }

                        title.text: "%1 (%2)".arg(model.name).arg(model.count)
                        ProgressionSlot {}
                    }
                }
            }
        }
    }
}





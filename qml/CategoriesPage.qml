/*
 * Copyright (C) 2017 - Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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
import Lomiri.Components 1.3

import "Components" as Components
Page {
    id: categoriesPage
    anchors.fill: parent

    signal categoryClicked(var name, var id)

    onCategoryClicked: {
        var pageProps = {
            title: name,
            category: id
        }

        //categoryView.__currentTmpIndex = id;
        pageStack.push(filteredAppPageComponent, pageProps);
    }

    header: Components.HeaderMain {
        title: i18n.tr("Categories")
        flickable: categoryView
    }

    ScrollView {
        anchors.fill: parent

        LomiriListView {
            id: categoryView
            anchors.fill: parent

            property int __currentTmpIndex: 0
            currentIndex: __currentTmpIndex

            // WORKAROUND: Fix for wrong grid unit size
            Component.onCompleted: root.flickable_responsive_scroll_fix(categoryView);

            model: categoriesModel
            delegate: ListItem {
                divider.anchors.leftMargin: units.gu(6.5)
                onClicked: {
                    categoryView.__currentTmpIndex = model.index;
                    //bottomEdgeStack.clear();
                    categoryClicked(name, id)
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

    Label {
        visible: categoriesModel.ready && categoriesModel.rowCount() === 0
        anchors.fill: parent
        text: i18n.tr("Nothing to see here yet!")
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
    }
}

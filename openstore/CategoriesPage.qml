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

Page {
    id: categoryPage

    header: PageHeader {
        title: i18n.tr("Categories")
        leadingActionBar.actions: Action {
            iconName: "close"
            onTriggered: categoryPage.pageStack.removePages(categoryPage)
        }
    }

    signal categoryClicked(var name, var code)

    onCategoryClicked: {
        var pageProps = {
            title: name,
            filterPattern: new RegExp(code.toString()),
            filterProperty: "category"
        }
        categoryPage.pageStack.addPageToNextColumn(categoryPage, filteredAppPageComponent, pageProps)
    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: categoryPage.header.height

        ListView {
            id: categoryView
            anchors.fill: parent
            model: categories.list
            delegate: ListItem {
                onClicked: categoryPage.categoryClicked(modelData, modelData)
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: modelData
                    ProgressionSlot {}
                }
            }
            /*section.property: "section"
            section.delegate: Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: units.gu(1)
                }

                height: units.gu(4)
                verticalAlignment: Text.AlignBottom
                textSize: Label.Small
                text: section == "mainCategories" ? i18n.tr("General") : i18n.tr("Categories")
            }*/
        }
    }

}

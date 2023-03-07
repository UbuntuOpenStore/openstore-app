/*
 * Copyright (C) 2020 - Brian Douglass
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

Component {
    id: filterComponent

    Dialog {
        id: filterDialog
        title: i18n.tr('Filters and Sorting')

        property string selectedSort: 'relevance'
        property string selectedType: ''

        signal accepted(string selectedSort, string selectedType)
        signal rejected()

        Label {
            anchors { left: parent.left; right: parent.right }
            wrapMode: Text.WordWrap
            maximumLineCount: Number.MAX_VALUE
            text: i18n.tr('Sort By')
        }

        ListModel {
            id: sortModel

            Component.onCompleted: {
                sortModel.append({ label: i18n.tr('Relevance'), value: 'relevance' });
                sortModel.append({ label: i18n.tr('Most Popular'), value: '-calculated_rating' });
                sortModel.append({ label: i18n.tr('Least Popular'), value: 'calculated_rating' });
                sortModel.append({ label: i18n.tr('Title (A-Z)'), value: 'name' });
                sortModel.append({ label: i18n.tr('Title (Z-A)'), value: '-name' });
                sortModel.append({ label: i18n.tr('Newest'), value: '-published_date' });
                sortModel.append({ label: i18n.tr('Oldest'), value: 'published_date' });
                sortModel.append({ label: i18n.tr('Latest Update'), value: '-updated_date' });
                sortModel.append({ label: i18n.tr('Oldest Update'), value: 'updated_date' });

                for (var i = 0; i < sortModel.count; i++) {
                    if (sortModel.get(i).value == selectedSort) {
                        sortBy.text = sortModel.get(i).label;
                    }
                }
            }
        }

        ComboButton {
            id: sortBy
            text: i18n.tr('Relevance')

            onClicked: sortBy.expanded = !sortBy.expanded

            ListView {
                model: sortModel

                delegate: ListItem {
                    height: layout.height

                    ListItemLayout {
                        id: layout
                        title.text: label
                    }

                    onClicked: {
                        sortBy.text = label;
                        selectedSort = value;
                        sortBy.expanded = false;
                    }
                }
            }
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            wrapMode: Text.WordWrap
            maximumLineCount: Number.MAX_VALUE
            text: i18n.tr('Type')
        }

        ListModel {
            id: typeModel

            Component.onCompleted: {
                typeModel.append({ label: i18n.tr('All Types'), value: '' });
                typeModel.append({ label: i18n.tr('Apps'), value: 'app' });
                typeModel.append({ label: i18n.tr('Bookmarks'), value: 'webapp' });
                typeModel.append({ label: i18n.tr('Web Apps'), value: 'webapp+' });

                for (var i = 0; i < typeModel.count; i++) {
                    if (typeModel.get(i).value == selectedType) {
                        type.text = typeModel.get(i).label;
                    }
                }
            }
        }

        ComboButton {
            id: type
            text: i18n.tr('All Types')

            onClicked: type.expanded = !type.expanded

            ListView {
                model: typeModel

                delegate: ListItem {
                    height: layout.height

                    ListItemLayout {
                        id: layout
                        title.text: label
                    }

                    onClicked: {
                        type.text = label;
                        selectedType = value;
                        type.expanded = false;
                    }
                }
            }
        }

        Button {
            text: i18n.tr('Apply')
            color: theme.palette.normal.positive
            onClicked: {
                filterDialog.accepted(selectedSort, selectedType);
            }
        }

        Button {
            text: i18n.tr('Cancel')
            onClicked: {
                filterDialog.rejected();
            }
        }
    }
}

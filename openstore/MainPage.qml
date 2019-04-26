import QtQuick 2.4
import Ubuntu.Components 1.3

import "Components" as Components

PageTreeNode {
    id: mainPage
    anchors.fill: parent

    function showSearch(text) {
        tabView.selectedIndex = 2   // search tab
        view.item.searchText = text || ""
        view.item.searchField.forceActiveFocus()
    }

    function showSearchQuery(url) {
        tabView.selectedIndex = 2   // search tab
        view.item.queryUrl = url || ""
    }

    function showCategory(name, id) {
        tabView.selectedIndex = 1   // category
        view.item.categoryClicked(name, id)
    }

    function openMyApps() {
        tabView.selectedIndex = 3; // My Apps
    }

    Components.ConvergentTabView {
        id: tabView
        anchors.fill: parent

        model: [
            { "name": i18n.tr("Discover"), "iconName": "ubuntu-store-symbolic", "sourceUrl": Qt.resolvedUrl("DiscoverTab.qml") },
            { "name": i18n.tr("Categories"), "iconName": "view-list-symbolic", "sourceUrl": Qt.resolvedUrl("CategoriesTab.qml") },
            { "name": i18n.tr("Search"), "iconName": "find", "sourceUrl": Qt.resolvedUrl("SearchTab.qml") },
            { "name": i18n.tr("My Apps"), "iconName": "document-save", "sourceUrl": Qt.resolvedUrl("InstalledAppsTab.qml"), "count": appModel.updatesAvailableCount }
        ]

        Loader {
            id: view
            anchors.fill: parent
            clip: true

            source: tabView.model[tabView.selectedIndex].sourceUrl

            onItemChanged: {
                if (item) {
                    item.parent = tabView.mainContent
                    item.anchors.fill = tabView.mainContent
                }
            }
        }
    }
}

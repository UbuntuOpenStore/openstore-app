import QtQuick 2.4
import Ubuntu.Components 1.3
import OpenStore 1.0

import "Components" as Components

Page {
    id: searchPage
    objectName: "searchPage"

    property alias searchText: searchField.text
    property alias searchField: searchField
    property alias queryUrl: view.queryUrl

    header: Components.HeaderBase {
        title: i18n.tr("Search")
        contents: TextField {
            id: searchField
            anchors.centerIn: parent
            width: Math.min(parent.width, units.gu(36))

            // Disable predictive text
            inputMethodHints: Qt.ImhNoPredictiveText

            primaryItem: Icon {
                width: units.gu(2); height: width
                name: "find"
            }
            placeholderText: i18n.tr("Search in OpenStore...")
        }
    }

    FilteredAppView {
        id: view
        anchors.fill: parent
        anchors.topMargin: searchPage.header.height

        onAppDetailsRequired: {
            PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady)
            PackagesCache.getPackageDetails(appId)
        }

        Timer {
            id: searchTimer
            interval: 500
            onTriggered: view.filterString = searchField.text
        }

        Connections {
            target: searchField
            onTextChanged: searchTimer.restart()
        }
    }

    Component.onCompleted: {
        if (searchField.text == "") {
            searchField.forceActiveFocus()
        } else {
            searchTimer.restart()
        }
    }
}

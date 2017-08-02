import QtQuick 2.4
import Ubuntu.Components 1.3
import OpenStore 1.0

import "Components" as Components

Page {
    id: rootItem

    property alias searchText: searchField.text
    property alias searchField: searchField
    property alias queryUrl: view.queryUrl

    header: PageHeader {
        title: i18n.tr("Search")
        contents: TextField {
            id: searchField
            anchors.centerIn: parent
            width: Math.min(parent.width, units.gu(36))

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
        anchors.topMargin: rootItem.header.height

        onAppDetailsRequired: {
            var pageProps = { app: view.getPackage(index) }
            bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
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
}

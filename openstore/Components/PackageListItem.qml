import QtQuick 2.4
import Ubuntu.Components 1.3

ListItem {
    id: rootItem
    property var appItem
    property bool showTicks: true

    height: layout.height

    RatingsRow {
        appItem: appItem
        width: units.gu(12)
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: units.gu(12)
        anchors.bottomMargin: units.gu(1.5)
    }

    ListItemLayout {
        id: layout

        title.text: rootItem.appItem.name
        summary.text: rootItem.appItem.tagline
        summary.wrapMode: Text.WrapAtWordBoundaryOrAnywhere

        UbuntuShape {
            id: appIconShape
            SlotsLayout.position: SlotsLayout.Leading
            aspect: UbuntuShape.Flat
            image: Image {
                source: rootItem.appItem.icon
                sourceSize.width: parent.width
				sourceSize.height: parent.height
                height: parent.height
                width: parent.width
            }
        }
        Icon {
            SlotsLayout.position: SlotsLayout.Trailing
            height: units.gu(2)
            width: height
            implicitHeight: height
            implicitWidth: width
            visible: rootItem.appItem.installed && rootItem.showTicks
            name: "tick"
            color: rootItem.appItem.updateAvailable ? UbuntuColors.orange : UbuntuColors.green
        }

        ProgressionSlot {}
    }
}

import QtQuick 2.4
import Ubuntu.Components 1.3

ListItem {
    id: rootItem
    property var appItem
    property bool showTicks: true

    height: layout.height

    ListItemLayout {
        id: layout

        title.text: rootItem.appItem.name
        summary.text: rootItem.appItem.tagline
        summary.wrapMode: Text.WrapAtWordBoundaryOrAnywhere

        Column {
            width: appIconShape.width
            height: appIconShape.height + units.gu(1)
            SlotsLayout.position: SlotsLayout.Leading
            spacing: units.gu(0.6)

            UbuntuShape {
                id: appIconShape
                aspect: UbuntuShape.Flat
                image: Image {
                    source: rootItem.appItem.icon
                    sourceSize.width: parent.width
                    sourceSize.height: parent.height
                    height: parent.height
                    width: parent.width
                }
            }

            RatingsRow {
                width: appIconShape.width
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

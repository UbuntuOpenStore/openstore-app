import QtQuick 2.4
import Ubuntu.Components 1.3

AbstractButton {
    id: rootItem
    property var appItem

    Column {
        anchors.fill: parent

        UbuntuShape {
            width: parent.width
            height: width
            aspect: UbuntuShape.Flat
            sourceFillMode: UbuntuShape.PreserveAspectFit
            source: Image {
                source: rootItem.appItem.icon
                sourceSize.width: parent.width
				sourceSize.height: parent.height
            }
        }

        ListItemLayout {
            anchors {
                left: parent.left; leftMargin: units.gu(-2)
                right: parent.right; rightMargin: units.gu(-2)
            }

            height: units.gu(4)
            title {
                text: rootItem.appItem.name
                textSize: Label.Small
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 2
            }

            subtitle {
                text: rootItem.appItem.author
                textSize: Label.XSmall
            }

            summary.text: rootItem.appItem.installed ? rootItem.appItem.updateAvailable ? i18n.tr("Update available").toUpperCase() : i18n.tr("✓ Installed").toUpperCase() : ""
            summary.textSize: Label.XSmall
        }
    }
}

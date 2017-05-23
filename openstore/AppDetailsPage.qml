/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import QtQuick.Layouts 1.1
import OpenStore 1.0


Page {
    id: appDetailsPage
    header: PageHeader {
        title: app ? app.name : i18n.tr("App details")
        automaticHeight: false
    }

    property var app: null


    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.topMargin: parent.header.height

        // WORKAROUND: Fix for wrong grid unit size
        Component.onCompleted: root.flickable_responsive_scroll_fix(scrollView.flickableItem)

        Column {
            id: mainColumn
            width: scrollView.width
            //spacing: units.gu(1)

            ListItem {
                height: units.gu(16)

                ListItemLayout {
                    anchors.fill: parent
                    title.text: app.name
                    subtitle.text: app.author
                    summary.text: printSize(app.fileSize)

                    UbuntuShape {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(12); height: width
                        aspect: UbuntuShape.Flat

                        image: Image {
                            height: parent.height
                            width: parent.width
                            source: app ? app.icon : ""
                        }
                    }
                }
            }

            ListItem {
                height: units.gu(8)

                RowLayout {
                    id: buttonsRow
                    anchors.fill: parent
                    anchors.margins: units.gu(2)
                    spacing: units.gu(2)
                    visible: !appModel.installer.busy

                    Button {
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: app.installed ? i18n.tr("Upgrade") : i18n.tr("Install")
                        visible: !app.installed || (app.installed && app.installedVersion < app.version)
                        color: UbuntuColors.green
                        onClicked: {
                            appModel.installer.installPackage(app.packageUrl)
                        }
                    }

                    Button {
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: i18n.tr("Remove")
                        visible: app.installed
                        color: UbuntuColors.red
                        onClicked: {
                            var popup = PopupUtils.open(removeQuestion, root, {pkgName: app.name || "<i>" + i18n.tr("unknown") + "</i>" });
                            popup.accepted.connect(function() {
                                appModel.installer.removePackage(app.appId, app.installedVersion)
                            })
                        }
                    }
                }

                ProgressBar {
                    id: installerProgressBar
                    anchors {
                        left: parent.left; leftMargin: units.gu(2)
                        right: parent.right; rightMargin: units.gu(2)
                        verticalCenter: parent.verticalCenter
                    }
                    maximumValue: app ? app.fileSize : 0
                    value: appModel.installer.downloadProgress
                    visible: appModel.installer.busy
                    indeterminate: appModel.installer.downloadProgress == 0
                }
            }

            ListItem {
                visible: {
                    for (var i=0; i<app.hooksCount; ++i) {
                        if (includesUnconfinedLocations(app.readPaths(i)))
                            return true
                        if (includesUnconfinedLocations(app.writePaths(i)))
                            return true
                        if (app.apparmorTemplate(i).indexOf("unconfined") >= 0)
                            return true
                    }
                    return false
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    subtitle.text: i18n.tr("This app has access to restricted system data, see below for details.")
                    subtitle.color: UbuntuColors.red
                    subtitle.maximumLineCount: 2
                    subtitle.wrapMode: Text.WordWrap

                    Icon {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        name: "security-alert"
                    }
                }
            }

            ListItem {
                height: units.gu(32)
                visible: screenshotsView.count

                ListView {
                    id: screenshotsView
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    leftMargin: units.gu(2)
                    rightMargin: units.gu(2)
                    clip: true
                    height: count > 0 ? units.gu(24) : 0
                    visible: count > 0
                    spacing: units.gu(1)
                    orientation: ListView.Horizontal
                    model: app.screenshots
                    delegate: UbuntuShape {
                        height: parent.height
                        // sh : lv.h = sw : x
                        width: screenshot.sourceSize.width * height / screenshot.sourceSize.height
                        aspect: UbuntuShape.Flat
                        sourceFillMode: UbuntuShape.PreserveAspectFit
                        source: Image {
                            id: screenshot
                            source: modelData
                            smooth: true
                            antialiasing: true
                        }

                        AbstractButton {
                            id: screenShotButton
                            anchors.fill: parent
                            onClicked: {
                                print("opening at:", screenShotButton.mapToItem(root, 0, 0).x)
                                zoomIn.createObject(root, {x: screenShotButton.mapToItem(root, 0, 0).x, y: screenShotButton.mapToItem(root, 0, 0).y, itemScale: screenShotButton.height / root.height, imageSource: modelData});
                                //                            zoomIn.createObject(root, {x: 100, y: 100});
                            }
                        }

                        Component {
                            id: zoomIn
                            Rectangle {
                                id: zI
                                width: parent.width
                                height: parent.height
                                color: "black"

                                property real itemScale: 1
                                property string imageSource
                                transform: Scale {
                                    origin.x: 0
                                    origin.y: 0
                                    xScale: zI.itemScale
                                    yScale: zI.itemScale
                                }

                                ParallelAnimation {
                                    id: scaleInAnimation
                                    onStarted: {
                                        hideAnimation.initialScale = itemScale;
                                        hideAnimation.initialX = x;
                                        hideAnimation.initialY = y;
                                    }

                                    UbuntuNumberAnimation { target: zI; property: "itemScale"; to: 1 }
                                    UbuntuNumberAnimation { target: zI; properties: "x,y"; to: 0 }
                                }


                                Component.onCompleted: {
                                    scaleInAnimation.start();
                                }

                                Image {
                                    anchors.fill: parent
                                    source: zI.imageSource
                                    fillMode: Image.PreserveAspectFit
                                }

                                AbstractButton {
                                    anchors.fill: parent
                                    onClicked: {
                                        hideAnimation.start()
                                    }
                                }

                                ParallelAnimation {
                                    id: hideAnimation
                                    property real initialScale: 1
                                    property int initialX: 0
                                    property int initialY: 0


                                    UbuntuNumberAnimation { target: zI; property: "itemScale"; to: hideAnimation.initialScale }
                                    UbuntuNumberAnimation { target: zI; property: "x"; to: hideAnimation.initialX }
                                    UbuntuNumberAnimation { target: zI; property: "y"; to: hideAnimation.initialY }
                                    onStopped: {
                                        script: zI.destroy()
                                    }

                                }
                            }
                        }
                    }
                }
            }

            ListItem {
                height: descLayout.height
                onClicked: descLayout.showAll = !descLayout.showAll
                ListItemLayout {
                    id: descLayout
                    property bool showAll: false
                    title.text: app.tagline || i18n.tr("Description")
                    subtitle.text: app.description
                    subtitle.textSize: Label.Small
                    subtitle.wrapMode: Text.WordWrap
                    subtitle.maximumLineCount: showAll ? Number.MAX_VALUE : 5

                    Icon {
                        width: units.gu(2); height: width
                        SlotsLayout.position: SlotsLayout.Last
                        name: descLayout.showAll ? "go-up" : "go-down"
                    }
                }
            }

            ListItem {
                height: changelogLayout.height
                visible: app.changelog
                onClicked: changelogLayout.showAll = !changelogLayout.showAll
                ListItemLayout {
                    id: changelogLayout
                    property bool showAll: false
                    // TRANSLATORS: Title of the changelog section
                    title.text: i18n.tr("What's New")
                    subtitle.text: app.changelog
                    subtitle.textSize: Label.Small
                    subtitle.wrapMode: Text.WordWrap
                    subtitle.maximumLineCount: showAll ? Number.MAX_VALUE : 5
                    
                    Icon {
                        width: units.gu(2); height: width
                        SlotsLayout.position: SlotsLayout.Last
                        name: changelogLayout.showAll ? "go-up" : "go-down"
                    }
                }
            }
            
            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Packager/Publisher")
                    subtitle.text: app.maintainer || i18n.tr("OpenStore team")
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Installed version")
                    subtitle.text: app.installedVersion || "None"
                }
            }
            
            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent

                    title.text: i18n.tr("Latest available version")
                    subtitle.text: app.version
                }
            }
            
            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("License") || "<i>" + i18n.tr("N/A") + "</i>"
                    subtitle.text: app.license
                }
            }

            ListItem {
                onClicked: Qt.openUrlExternally(app.source)
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Source Code") || "<i>" + i18n.tr("N/A") + "</i>"
                    subtitle.text: app.source
                    ProgressionSlot { visible: app.source }
                }
            }

            ListItem {
                onClicked: {
                    // FIXME: I don't like this heuristic, but there's no other way to get a reference
                    // to the page that pushed 'appDetailsPage' into the stack.
                    // The parent node is actually the PageWrapper that created this page, 'parentPage' is one of its properties.
                    //var realParentPage = appDetailsPage.parentNode.parentPage

                    var pageProps = {
                        title: app.author,
                        filterPattern: new RegExp(app.author),
                        filterProperty: "author"
                    }

                    appDetailsPage.pageStack.addPageToCurrentColumn(/*realParentPage*/ appDetailsPage, filteredAppPageComponent, pageProps)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // TRANSLATORS: This is the button that shows a list of all the packages from the same author. %1 is the name of the author.
                    title.text: i18n.tr("More from %1").arg(app.author)
                    ProgressionSlot {}
                }
            }

            ListItem {
                onClicked: {
                    // FIXME: I don't like this heuristic, but there's no other way to get a reference
                    // to the page that pushed 'appDetailsPage' into the stack.
                    // The parent node is actually the PageWrapper that created this page, 'parentPage' is one of its properties.
                    //var realParentPage = appDetailsPage.parentNode.parentPage

                    var pageProps = {
                        title: app.category,
                        filterPattern: new RegExp(app.category),
                        filterProperty: "category"
                    }

                    appDetailsPage.pageStack.addPageToCurrentColumn(/*realParentPage*/ appDetailsPage, filteredAppPageComponent, pageProps)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // FIXME: app.category is not localized.
                    // TRANSLATORS: This is the button that shows a list of all the other packages in the same category. %1 is the name of the category.
                    title.text: i18n.tr("Other apps in %1").arg(app.category)
                    ProgressionSlot {}
                }
            }

            SectionDivider {
                text: i18n.tr("Package contents")
            }

            Repeater {
                model: app.hooksCount

                delegate: ListItem {
                    height: hookDelLayout.height + units.gu(3)

                    property var hooks: app.hooks(index)
                    property string permissions: app.permissions(index)
                    property string readpaths: app.readPaths(index)
                    property string writepaths: app.writePaths(index)
                    property string hookName: app.hookName(index)
                    property string apparmorTemplate: app.apparmorTemplate(index)

                    Column {
                        id: hookDelLayout
                        anchors { left: parent.left; right: parent.right; margins: units.gu(2) }
                        y: units.gu(1)
                        spacing: units.gu(1)

                        RowLayout {
                            width: parent.width
                            height: units.gu(4)

                            Label {
                                text: hookName
                                Layout.fillWidth: true
                            }

                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "stock_application"
                                visible: (hooks & ApplicationItem.HookDesktop)
                            }
                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "search"
                                visible: (hooks & ApplicationItem.HookScope)
                            }
                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "stock_website"
                                visible: (hooks & ApplicationItem.HookUrls)
                            }
                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "share"
                                visible: (hooks & ApplicationItem.HookContentHub)
                            }
                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "notification"
                                visible: (hooks & ApplicationItem.HookPushHelper)
                            }
                            HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "contact-group"
                                visible: (hooks & ApplicationItem.HookAccountService)
                            }
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            height: units.gu(6)
                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: apparmorTemplate.indexOf("unconfined") >= 0
                            }

                            title.text: i18n.tr("AppArmor profile")
                            subtitle.text: apparmorTemplate || "Ubuntu confined app"
                            subtitle.color: apparmorTemplate.indexOf("unconfined") >= 0 ? UbuntuColors.red : theme.palette.normal.backgroundSecondaryText
                            subtitle.maximumLineCount: Number.MAX_VALUE
                        }


                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            height: units.gu(6)
                            visible: permissions.length > 0

                            Icon {
                                property var restrictedPerms: ["bluetooth", "calendar", "contacts", "debug", "history", "music_files", "picture_files", "video_files"]
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: {
                                    var length = restrictedPerms.length;
                                    while(length--) {
                                       if (permissions.indexOf(restrictedPerms[length]) > -1)
                                           return true
                                    }
                                    return false
                                }
                            }

                            title.text: i18n.tr("Permissions")
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WordWrap
                            subtitle.text: {
                                if (permissions) {
                                    return permissions.replace("bluetooth", "<font color=\"#ED3146\">bluetooth</font>")
                                                      .replace("calendar", "<font color=\"#ED3146\">calendar</font>")
                                                      .replace("contacts", "<font color=\"#ED3146\">contacts</font>")
                                                      .replace("debug", "<font color=\"#ED3146\">debug</font>")
                                                      .replace("history", "<font color=\"#ED3146\">history</font>")
                                                      .replace("music_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("picture_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("video_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("music_files", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("picture_files", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("video_files", "<font color=\"#ED3146\">music_files_read</font>")
                                }

                                return "<i>" + i18n.tr("none required") + "</i>"
                            }
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            height: units.gu(6)
                            visible: readpaths.length > 0

                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: includesUnconfinedLocations(readpaths)
                            }

                            title.text: i18n.tr("Read paths")
                            subtitle.text: readpaths || "<i>" + i18n.tr("none") + "</i>"
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            height: units.gu(6)
                            visible: writepaths.length > 0
                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: includesUnconfinedLocations(writepaths)
                            }

                            title.text: i18n.tr("Write paths")
                            subtitle.text: writepaths || "<i>" + i18n.tr("none") + "</i>"
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        Button {
                            anchors { right: parent.right }
                            text: "Open"
                            color: UbuntuColors.green
                            visible: app.installed &&  (hooks & ApplicationItem.HookDesktop)
                            onClicked: Qt.openUrlExternally("appid://" + app.appId + "/" + hookName + "/" + app.installedVersion)
                        }
                    }
                }
            }
        }
    }


    Component {
        id: removeQuestion
        Dialog {
            id: removeQuestionDialog
            title: i18n.tr("Remove package")
            text: i18n.tr("Do you want to remove %1?").arg(pkgName)

            property string pkgName
            signal accepted();
            signal rejected();

            Button {
                text: i18n.tr("Cancel")
                onClicked: {
                    removeQuestionDialog.rejected();
                    PopupUtils.close(removeQuestionDialog)
                }

            }
            Button {
                text: i18n.tr("Remove")
                color: UbuntuColors.red
                onClicked: {
                    removeQuestionDialog.accepted();
                    PopupUtils.close(removeQuestionDialog)
                }
            }
        }
    }


    function printSize(size) {
        var s

        s = 1024 * 1024 * 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in GB
            return i18n.tr("%1 GB").arg((size / s).toFixed(2));

        s = 1024 * 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in MB
            return i18n.tr("%1 MB").arg((size / s).toFixed(2));

        s = 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in kB
            return i18n.tr("%1 kB").arg(parseInt(size / s));

        // TRANSLATORS: %1 is the size of a file, expressed in bytes
        return i18n.tr("%1 bytes").arg(size);
    }

    function includesUnconfinedLocations(paths) {
        var p = paths.split(",")
        var j = 0

        for (var i=0; i < p.length; ++i) {
            var x = p[i]
            if (x.match(/[^\w\s]/)) {
                if (x.indexOf("/home/phablet/.cache/" + app.appId) == -1 && x.indexOf("/home/phablet/.config/" + app.appId) == -1) {
                    ++j
                }
            }
        }

        return (j > 0)
    }
}

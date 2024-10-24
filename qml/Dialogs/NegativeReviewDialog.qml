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
import Lomiri.Components 1.3
import Lomiri.Components.Popups 1.3

Component {
    id: negativeReivewQuestion

    Dialog {
        id: negativeReivewQuestionDialog
        title: i18n.tr("Review confirmation")
        text: i18n.tr("Would you like to contact the publisher or file a bug instead? By opening a discussion with the developer(s) you could be helping to improve this app.")

        property int rating
        property string review
        property var app
        property bool existing
        signal accepted();
        signal rejected();

        Button {
            text: i18n.tr("Get support")
            color: theme.palette.normal.positive
            onClicked: {
                Qt.openUrlExternally(app.supportUrl);

                negativeReivewQuestionDialog.accepted()
                PopupUtils.close(negativeReivewQuestionDialog)
            }
        }
        Button {
            text: i18n.tr("Post my review")
            onClicked: {
                if (existing) {
                    app.editReview(review, rating, root.apiKey)
                }
                else {
                    app.review(review, rating, root.apiKey)
                }

                negativeReivewQuestionDialog.rejected()
                PopupUtils.close(negativeReivewQuestionDialog)
            }
        }
    }
}

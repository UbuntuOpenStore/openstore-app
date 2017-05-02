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

ListModel {
    Component.onCompleted: {
        // main categories
//        append({ code: 'All', name: i18n.tr("All"), section: "mainCategories" })
//        append({ code: 'Installed', name: i18n.tr("Installed"), section: "mainCategories" })

        // sub categories
        // code definitions from: https://github.com/UbuntuOpenStore/openappstore-web/blob/6291d13e8059793e9ee95f5f9986d786d029f6c7/www/app/services/info.js
        append({ code: 'Books & Comics', name: i18n.tr('Books & Comics'), section: "subCategories" })
        append({ code: 'Business', name: i18n.tr('Business'), section: "subCategories"  })
        append({ code: 'Communication', name: i18n.tr("Communication"), section: "subCategories"  })
        append({ code: 'Developer Tools', name: i18n.tr("Developer Tools"), section: "subCategories" })
        append({ code: 'Education', name: i18n.tr("Education"), section: "subCategories" })
        append({ code: 'Entertainment', name: i18n.tr("Entertainment"), section: "subCategories" })
        append({ code: 'Finance', name: i18n.tr("Finance"), section: "subCategories" })
        append({ code: 'Food & Drink', name: i18n.tr("Food & Drink"), section: "subCategories" })
        append({ code: 'Games', name: i18n.tr("Games"), section: "subCategories" })
        append({ code: 'Graphics', name: i18n.tr("Graphics"), section: "subCategories" })
        append({ code: 'Health & Fitness', name: i18n.tr("Health & Fitness"), section: "subCategories" })
        append({ code: 'Lifestyle', name: i18n.tr("Lifestyle"), section: "subCategories" })
        append({ code: 'Media & Video', name: i18n.tr("Media & Video"), section: "subCategories" })
        append({ code: 'Medical', name: i18n.tr("Medical"), section: "subCategories" })
        append({ code: 'Music & Audio', name: i18n.tr("Music & Audio"), section: "subCategories" })
        append({ code: 'News & Magazines', name: i18n.tr("News & Magazines"), section: "subCategories" })
        append({ code: 'Personalisation', name: i18n.tr("Personalisation"), section: "subCategories" })
        append({ code: 'Productivity', name: i18n.tr("Productivity"), section: "subCategories" })
        append({ code: 'Reference', name: i18n.tr("Reference"), section: "subCategories" })
        append({ code: 'Science & Engineering', name: i18n.tr("Science & Engineering"), section: "subCategories" })
        append({ code: 'Shopping', name: i18n.tr("Shopping"), section: "subCategories" })
        append({ code: 'Social Networking', name: i18n.tr("Social Networking"), section: "subCategories" })
        append({ code: 'Sports', name: i18n.tr("Sports"), section: "subCategories" })
        append({ code: 'Travel & Local', name: i18n.tr("Travel & Local"), section: "subCategories" })
        append({ code: 'Universal Accesss/Accessibility', name: i18n.tr("Universal Access/Accessibility"), section: "subCategories" })
        append({ code: 'Utilities', name: i18n.tr("Utilities"), section: "subCategories" })
        append({ code: 'Weather', name: i18n.tr("Weather"), section: "subCategories" })
    }
}

/*
 * Copyright (C) 2020 Brian Douglass
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
 *
 */

//Let's remove the header from the website when we Sign In. Also fixes the styles being broken in old qt webengine versions.
var styles = `
    #navigation, header, footer {
        display: none !important
    }

    .btn {
        background-color: #e95420;
        color: white;
        font-weight: bold;
        border-radius: 1.5rem;
        display: block;
        margin-bottom: 16px;
        width: 100%;
        padding-top: 12px;
        padding-bottom: 12px;
        text-decoration: none;
        text-align: center;
        border: 0;
        font-size: 16px;
        font-family: "Ubuntu", sans-serif;
    }

    .btn svg {
        height: 1.5rem;
        width: 1.5rem;
        padding-right: 8px;
    }
`;

var styleSheet = document.createElement("style");
styleSheet.type = "text/css";
styleSheet.innerText = styles;
document.head.appendChild(styleSheet);

# OpenStore App

Ubuntu Touch app for the [OpenStore](https://open-store.io/).

[![OpenStore](https://open-store.io/badges/en_US.png)](https://open-store.io/app/openstore.openstore-team)

## Reporting Bugs

Please report any bugs/features/requests in our [bug tracker](https://gitlab.com/theopenstore/openstore-meta/issues).

## Translations

If you would like to help out with translations head over to the OpenStore
project on the [Weblate](https://hosted.weblate.org/projects/open-store-io/openstore-app/).

## Development

### Code Style

This project uses clang-format for C++ code formatting based on Mozilla style. Code is automatically formatted when you commit changes. See [CODING_STYLE.md](CODING_STYLE.md) for details.

To manually format all code:

```bash
./scripts/format-code.sh
```

## Donations

We rely on donations from generous individuals like you to continue our mission
of supporting the Ubuntu Touch community. Your contribution can help us improve
the quality of the OpenStore website & app, add new features, and make them more
accessible to users around the world. We appreciate any donation, big or small,
and thank you for your support in helping us to build a better future for mobile
technology! Donate on [Liberapay](https://liberapay.com/OpenStore/).

## Development

* Setup [clickable](https://github.com/bhdouglass/clickable)
* Initialize submodules, run `git submodule update --init --recursive`
* From the root directory of this repo, run `clickable`
* The app will be compiled and launched on your phone
* Profit!

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3, as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranties of MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

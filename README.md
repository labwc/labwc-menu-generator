# labwc-menu-generator

## Features

- Independent of Desktop Environments and associated menu-packages.  This is
  achieved by categorising system .desktop files against a built-in
  directory-schema rather than parsing .menu and .directory files (see
  [Menu Specification]).

- [Desktop Entry Specification] compliant parsing of `Desktop Entry` .desktop
  files.

- Openbox 3.6 formatted output.

- Localized values for .desktop file keys such as `Name[sr_YU]`

[Desktop Entry Specification]: https://specifications.freedesktop.org/desktop-entry/1.1/
[Menu Specification]: https://specifications.freedesktop.org/menu/1.0/

## 2. Build

To build, simply run:

    meson setup build/
    meson compile -C build/

Run-time dependencies include: `glib-2.0`

Build dependencies include: `meson`, `ninja`, `gcc`/`clang`

## Repology

[![Packaging status](https://repology.org/badge/vertical-allrepos/labwc-menu-generator.svg)](https://repology.org/project/labwc-menu-generator/versions)

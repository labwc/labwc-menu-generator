#!/bin/sh
#
# This is an example wrapper script for `labwc-menu-generator` which aims to
# demonstrate how a user can customize the menu with entries before/after the
# directories and to ignore certain applications.
#
# `labwc-menu-generator` does the hard work of parsing system application
# .desktop files and categorizing them into directories. Technically it would
# of course be possible to add options for further customisation such as what
# is achieved by this script, but that would mostly likely result in a couple
# of undesireable outcomes:
#
# 1. Significantly extend project scope to cater for unusual and obscure
#    requirements (including translations), thus making it harder to maintain
#    and less likely to survive in the long-run.
#
# 2. Make the user-interface disproportionately more complicated compared with
#    writing a simple wrapper script.
#

ignore_files=$(mktemp)
trap "rm -f $ignore_files" EXIT
cat <<'EOF' >${ignore_files}
gammastep-indicator.desktop
libreoffice-startcenter.desktop
cmake-gui.desktop
electron27.desktop
org.gtk.IconBrowser4.desktop
org.gtk.gtk4.NodeEditor.desktop
org.gtk.PrintEditor4.desktop
org.gtk.WidgetFactory4.desktop
bssh.desktop
bvnc.desktop
qv4l2.desktop
qvidcap.desktop
pcmanfm-qt-desktop-pref.desktop
pcmanfm-desktop-pref.desktop
libfm-pref-apps.desktop
org.xfce.mousepad-settings.desktop
avahi-discover.desktop
org.codeberg.dnkl.footclient.desktop
org.codeberg.dnkl.foot-server.desktop
lstopo.desktop
gtk-lshw.desktop
urxvtc.desktop
urxvt-tabbed.desktop
xterm.desktop
rofi.desktop
rofi-theme-selector.desktop
EOF

printf '%b\n' '<?xml version="1.0" encoding="UTF-8"?>
<openbox_menu>
<menu id="root-menu" label="root-menu">
  <item label="Web Browser">
    <action name="Execute"><command><![CDATA[firefox labwc.github.io]]></command></action>
  </item>
  <item label="Terminal">
    <action name="Execute" command="sakura -e zsh" />
  </item>
  <item label="Tweaks">
    <action name="Execute" command="labwc-tweaks"/>
  </item>
  <separator />'

# -b|--bare    Disables header and footer
# -i|--ignore  Excludes all .desktop files listed at the top of this file
labwc-menu-generator -b -i "${ignore_files}"

printf '%b\n' '<separator />
  <item label="Reconfigure">
    <action name="Reconfigure" />
  </item>
  <item label="Exit">
    <action name="Exit" />
  </item>
</menu> <!-- root-menu -->
</openbox_menu>'

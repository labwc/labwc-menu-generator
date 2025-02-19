#!/bin/sh
#
# Example usage (~/.config/labwc/menu.xml):
#
#     <?xml version="1.0" encoding="utf-8"?>
#     <openbox_menu>
#       <menu id="root-menu" label="" execute="example-dynamic-root-menu.sh"/>
#     </openbox_menu>

terminal="foot"

printf '%b\n' '<openbox_pipe_menu>
  <item label="Web Browser">
    <action name="Execute"><command>firefox</command></action>
  </item>
  <item label="Terminal">
    <action name="Execute" command="'"$terminal"'"/>
  </item>
  <item label="Tweaks">
    <action name="Execute" command="labwc-tweaks"/>
  </item>
  <separator />'

labwc-menu-generator -b -t "${terminal}"

printf '%b\n' '<separator />
  <item label="Reconfigure">
    <action name="Reconfigure" />
  </item>
  <item label="Exit">
    <action name="Exit" />
  </item>
</openbox_pipe_menu>'

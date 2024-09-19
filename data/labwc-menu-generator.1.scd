labwc-menu-generator(1)

# NAME

labwc-menu-generator - a menu generator for labwc

# SYNOPSIS

*labwc-menu-generator* [options...]

# DESCRIPTION

Labwc-menu-generator is independent of Desktop Environments and
associated menu-packages.

This is achieved by categorising system .desktop files against a built-
in directory-schema rather than parsing .menu and .directory files.

# OPTIONS

*-b, --bare*
	Show no header or footer

*-d, --desktop*
	Add .desktop filename as a comment in the XML output

*-h, --help*
	Show help message and quit

*-i, --ignore <file>*
	Specify file listing .desktop files to ignore

*-I, --icons*
	Add icon="" attribute

*-n, --no-duplicates*
	Limit desktop entries to one directory only

*-p, --pipemenu*
	Output in pipemenu format

*-t, --terminal-prefix <command>*
	Specify prefix for Terminal=true entries, for example 'foot' or
	'xterm -e'

# AUTHORS

The Labwc Team - https://github.com/labwc/labwc-menu-generator

# REPORT BUGS

Please report any bugs at:

https://github.com/labwc/labwc-menu-generator/issues/

# SEE ALSO

labwc(1), labwc-menu(5), labwc-actions(5)

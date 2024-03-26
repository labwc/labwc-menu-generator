#!/bin/sh

: ${BUILDDIR=../../build}

test_description='parse .desktop files to generate openbox menu'
. ./sharness.sh

test_menu() {
	export XDG_DATA_HOME=${PWD}
	export XDG_DATA_DIRS="bad-location"
	export LABWC_MENU_GENERATOR_DEBUG_FIRST_DIR_ONLY=1
	export LANG="$2"
	cat ../t1000/"$1" >expect
	cp -a ../t1000/applications .
	${BUILDDIR}/labwc-menu-generator >actual
	test_cmp expect actual
}

test_expect_success 'parse .desktop files with LANG=C' '
test_menu "menu.xml" "C"
'

test_done

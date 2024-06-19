#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tap.h"
#include "test-lib.h"

int main(void)
{
	char actual[] = "/tmp/t1001-actual";
	char expect[] = "../t/t1001/menu-sv.xml";

	plan(1);

	/* test 1 */
	diag("t1001.t - simple run based on a sample of .desktop files with i18n");
	setenv("XDG_DATA_HOME", "../t/t1000", 1);
	setenv("XDG_DATA_DIRS", "bad-location", 1);
	setenv("LABWC_MENU_GENERATOR_DEBUG_FIRST_DIR_ONLY", "1", 1);
	setenv("LANG", "sv_SE.utf8", 1);
	char command[1000];
	snprintf(command, sizeof(command), "./labwc-menu-generator >%s", actual);
	system(command);
	bool pass = test_cmp_files(actual, expect);
	if (pass) {
		unlink(actual);
	}
	return exit_status();
}


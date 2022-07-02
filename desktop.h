/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdbool.h>

struct app {
	char *name;
	char *name_localized;
	char *generic_name;
	char *generic_name_localized;
	char *exec;
	char *tryexec;
	char *working_dir;
	bool tryexec_not_in_path;
	char *icon;
	char *categories;
	bool nodisplay;
	char *filename;
	bool terminal;
	bool has_been_mapped;
};

/* desktop_entries_create - parse system .desktop files */
GList *desktop_entries_create(void);
void desktop_entries_destroy(GList *apps);

/* return "Name[$ll]" and "Name[$ll_CC] */
char *name_ll_get(void);
char *name_llcc_get(void);

#endif /* DESKTOP_H */

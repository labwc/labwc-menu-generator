// SPDX-License-Identifier: GPL-2.0-only
/*
 * labwc-menu-generator
 * Copyright (C) Johan Malm 2022
 */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <dirent.h>
#include <getopt.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "desktop.h"
#include "ignore.h"
#include "schema.h"

static bool no_duplicates;
static bool no_footer;
static bool no_header;
static bool pipemenu;
static bool show_desktop_filename;
static bool show_icons;
static char *terminal_prefix;

static const struct option long_options[] = {
	{"bare", no_argument, NULL, 'b'},
	{"desktop", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"ignore", required_argument, NULL, 'i'},
	{"icons", no_argument, NULL, 'I'},
	{"no-duplicates", no_argument, NULL, 'n'},
	{"pipemenu", no_argument, NULL, 'p'},
	{"terminal-prefix", required_argument, NULL, 't'},
	{0, 0, 0, 0}
};

static const char labwc_menu_generator_usage[] =
"Usage: labwc-menu-generator [options...]\n"
"  -b, --bare               Show no header or footer\n"
"  -d, --desktop            Add .desktop filename as a comment in the XML output\n"
"  -h, --help               Show help message and quit\n"
"  -i, --ignore <file>      Specify file listing .desktop files to ignore\n"
"  -I, --icons              Add icon=\"\" attribute\n"
"  -n, --no-duplicates      Limit desktop entries to one directory only\n"
"  -p, --pipemenu           Output in pipemenu format\n"
"  -t, --terminal-prefix    Specify prefix for Terminal=true entries\n";

static void
usage(void)
{
	printf("%s", labwc_menu_generator_usage);
	exit(0);
}

static bool
ismatch(gchar **dir_categories, const char *app_categories)
{
	for (char **p = dir_categories; *p; p++) {
		if (!p || !*p || !**p) {
			continue;
		}
		if (strstr(app_categories, *p)) {
			return true;
		}
	}
	return false;
}

static void
print_app_to_buffer(struct app *app, GString *submenu)
{
	if (show_desktop_filename) {
		g_string_append_printf(submenu, "    <!-- %s -->\n", app->filename);
	}

	/*
	 * For Terminal=true entries we prefix the command if the user has
	 * specified a --terminal-prefix value. Typical values would be 'foot',
	 * 'alacritty -e' or 'xterm -e'. Many terminals use the -e option, but
	 * not all.
	 */
	gchar *command = app->terminal && terminal_prefix ?
		g_strdup_printf("%s '%s'", terminal_prefix, app->exec) :
		g_strdup_printf("%s", app->exec);
	if (!command) {
		fprintf(stderr, "fatal: cannot allocate");
		exit(EXIT_FAILURE);
	}

	g_string_append_printf(submenu, "    <item label=\"%s\"",
		app->name_localized ? app->name_localized : app->name);
	if (show_icons && app->icon) {
		g_string_append_printf(submenu, " icon=\"%s\"", app->icon);
	}
	g_string_append_printf(submenu, ">\n");


	g_string_append_printf(submenu,
		"      <action name=\"Execute\"><command>%s</command></action>\n",
		command);
	g_string_append_printf(submenu, "    </item>\n");

	g_free(command);
}

static bool
should_not_display(struct app *app)
{
	return app->nodisplay || app->tryexec_not_in_path;
}

#if ! GLIB_CHECK_VERSION(2, 68, 0)
static void
compat_replace(GString *s, const char *before, const char *after, int limit)
{
	char **split = g_strsplit(s->str, before, limit - 1);
	g_string_assign(s, g_strjoinv(after, split));
	g_strfreev(split);
}
#define g_string_replace compat_replace
#endif

static void
print_apps_in_other_directory(GList *apps, GString *submenu)
{
	GList *iter;
	for (iter = apps; iter; iter = iter->next) {
		struct app *app = (struct app *)iter->data;
		if (should_not_display(app) || app->has_been_mapped) {
			continue;
		}
		print_app_to_buffer(app, submenu);
	}
	g_string_replace(submenu, "&", "&amp;", 0);
}

struct dir {
	char *name;
	char *name_localized;
	char *icon;
	char *categories;
};

static void
print_apps_for_one_directory(GList *apps, struct dir *dir, GString *submenu)
{
	gchar **categories = g_strsplit(dir->categories, ";", -1);
	GList *iter;
	for (iter = apps; iter; iter = iter->next) {
		struct app *app = (struct app *)iter->data;
		if (should_not_display(app)) {
			continue;
		}

		/*
		 * dir->categories often contains a semi-colon at the end,
		 * giving an empty field which we ignore
		 */
		if (!app->categories || app->categories[0] == '\0') {
			continue;
		}

		/* Only include apps with the right categories */
		if (!ismatch(categories, app->categories)) {
			continue;
		}

		if (no_duplicates && app->has_been_mapped) {
			continue;
		}
		app->has_been_mapped = true;
		print_app_to_buffer(app, submenu);
	}
	g_string_replace(submenu, "&", "&amp;", 0);
	g_strfreev(categories);
}

static void
print_menu(GList *dirs, GList *apps)
{
	GString *submenu = g_string_new(NULL);

	if (!no_header) {
		if (pipemenu) {
			printf("<openbox_pipe_menu>\n");
		} else {
			printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			printf("<openbox_menu>\n");
			printf("<menu id=\"root-menu\" label=\"root-menu\">\n");
		}
	}

	/* Handle all directories except 'Other' */
	GList *iter;
	for (iter = dirs; iter; iter = iter->next) {
		struct dir *dir = (struct dir *)iter->data;
		if (!dir->categories) {
			/* the 'Other' directory */
			continue;
		}
		g_string_erase(submenu, 0, -1);
		print_apps_for_one_directory(apps, dir, submenu);
		if (!submenu->len) {
			continue;
		}

		printf("  <menu id=\"%s\" label=\"%s\"", dir->name, dir->name_localized ? : dir->name);
		if (show_icons && dir->icon) {
			printf(" icon=\"%s\"", dir->icon);
		}
		printf(">\n");

		printf("%s", submenu->str);
		printf("  </menu> <!-- %s -->\n", dir->name);
	}

	/* Put any left over applications in 'Other' */
	for (iter = dirs; iter; iter = iter->next) {
		struct dir *dir = (struct dir *)iter->data;
		if (dir->categories) {
			continue;
		}
		g_string_erase(submenu, 0, -1);
		print_apps_in_other_directory(apps, submenu);
		if (!submenu->len) {
			continue;
		}
		printf("  <menu id=\"%s\" label=\"%s\">\n",
			dir->name, dir->name_localized ? : dir->name);
		printf("%s", submenu->str);
		printf("  </menu> <!-- %s -->\n", dir->name);
	}

	if (!no_footer) {
		if (pipemenu) {
			printf("</openbox_pipe_menu>\n");
		} else {
			printf("</menu> <!-- root-menu -->\n");
			printf("</openbox_menu>\n");
		}
	}

	g_string_free(submenu, TRUE);
}

static int
compare_dir_name(const void *a, const void *b)
{
	const struct dir *aa = (struct dir *)a;
	const struct dir *bb = (struct dir *)b;
	const char *aa_name, *bb_name;

	/*
	 * We use g_utf8_casefold+strcmp instead of merely strcasecmp to
	 * correctly sort languages other than English.
	 */
	aa_name = aa->name_localized ? aa->name_localized : aa->name;
	bb_name = bb->name_localized ? bb->name_localized : bb->name;
	aa_name = g_utf8_casefold(aa_name, -1);
	bb_name = g_utf8_casefold(bb_name, -1);
	int ret = strcmp(aa_name, bb_name);
	g_free((void *)aa_name);
	g_free((void *)bb_name);
	return ret;
}

GList *directory_entries_create(void)
{
	GList *dirs = NULL;

	/* Schema defined in schema.h */
	for (int i = 0; schema[i].key; i++) {
		static struct dir *dir;
		char *key = schema[i].key;
		char *value = schema[i].value;

		/* The keyword 'Name' starts a new directory section */
		if (!strcmp("Name", key)) {
			dir = calloc(1, sizeof(struct dir));
			dir->name = strdup(value);
			dirs = g_list_append(dirs, dir);
		}
		assert(dir);

		if (!strcmp("Categories", key)) {
			dir->categories = strdup(value);
		} else if (!strcmp("Icon", key)) {
			dir->icon = strdup(value);
		}

		if (!strcmp(key, name_llcc_get())) {
			dir->name_localized = strdup(value);
		}
		if (dir->name_localized) {
			continue;
		}
		if (!strcmp(key, name_ll_get())) {
			dir->name_localized = strdup(value);
		}
	}
	dirs = g_list_sort(dirs, (GCompareFunc)compare_dir_name);
	return dirs;
}

void
directory_entries_destroy(GList *dirs)
{
	GList *iter;
	for (iter = dirs; iter; iter = iter->next) {
		struct dir *dir = (struct dir *)iter->data;
		g_free(dir->name);
		g_free(dir->name_localized);
		g_free(dir->categories);
		g_free(dir->icon);
		g_free(dir);
	}
	g_list_free(dirs);
}

int
main(int argc, char **argv)
{
	int c;
	while (1) {
		int index = 0;
		c = getopt_long(argc, argv, "bdhi:Inpt:", long_options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'b':
			no_footer = true;
			no_header = true;
			break;
		case 'd':
			show_desktop_filename = true;
			break;
		case 'i':
			ignore_init(optarg);
			break;
		case 'I':
			show_icons = true;
			break;
		case 'n':
			no_duplicates = true;
			break;
		case 'p':
			pipemenu = true;
			break;
		case 't':
			terminal_prefix = optarg;
			break;
		case 'h':
		default:
			usage();
		}
	}
	if (optind < argc) {
		usage();
	}

	GList *apps = desktop_entries_create();
	GList *dirs = directory_entries_create();

	print_menu(dirs, apps);

	desktop_entries_destroy(apps);
	directory_entries_destroy(dirs);
	ignore_finish();

	return 0;
}

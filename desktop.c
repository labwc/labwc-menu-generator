// SPDX-License-Identifier: GPL-2.0-only
/*
 * Parse .desktop files in $XDG_DATA_{HOME,DIRS}/applications/
 * Copyright (C) Johan Malm 2022
 */
#define _POSIX_C_SOURCE 200809L

/* Allow extended dirent entries */
#if defined(__linux__)
	#define _DEFAULT_SOURCE
#else
	#define __BSD_VISIBLE 1
#endif

#include <ctype.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <stdbool.h>
#include "desktop.h"

static GList *apps;

static char ll[24] = { 0 };
static char llcc[24] = { 0 };
static char name_ll[64] = { 0 };
static char name_llcc[64] = { 0 };
static char generic_name_ll[64] = { 0 };
static char generic_name_llcc[64] = { 0 };

static void
i18n_init(void)
{
	static bool has_been_initialized;

	if (has_been_initialized) {
		return;
	}
	has_been_initialized = true;

	/*
	 * Read $LANG and parse ll_CC.UTF8 format where
	 *  - ‘ll’ is an ISO 639 two-letter language code (lowercase)
	 *  - ‘CC’ is an ISO 3166 two-letter country code (uppercase)
	 */
	char *p = getenv("LANG");
	if (!p) {
		fprintf(stderr, "$LANG not set");
		return;
	}

	/* ll_CC */
	strncpy(llcc, p, sizeof(llcc));
	p = strrchr(llcc, '.');
	if (p) {
		*p = '\0';
	}

	/* ll */
	strncpy(ll, llcc, sizeof(ll));
	p = strrchr(ll, '_');
	if (p) {
		*p = '\0';
	}

	snprintf(name_ll, sizeof(name_ll), "Name[%s]", ll);
	snprintf(name_llcc, sizeof(name_llcc), "Name[%s]", llcc);
	snprintf(generic_name_ll, sizeof(generic_name_ll),
		"GenericName[%s]", ll);
	snprintf(generic_name_llcc, sizeof(generic_name_llcc),
		"GenericName[%s]", llcc);
}

char *name_ll_get(void) { return name_ll; }
char *name_llcc_get(void) { return name_llcc; }

static void
parse_line(char *line, struct app *app, int *is_desktop_entry)
{
	/* We only read the [Desktop Entry] section of a .desktop file */
	if (line[0] == '[') {
		if (!strncmp(line, "[Desktop Entry]", 15)) {
			*is_desktop_entry = 1;
		} else {
			*is_desktop_entry = 0;
		}
	}
	if (!*is_desktop_entry) {
		return;
	}

	char *key, *value;
	gchar **argv = g_strsplit(line, "=", 2);
	if (g_strv_length(argv) != 2) {
		g_strfreev(argv);
		return;
	}
	key = g_strstrip(argv[0]);
	value = g_strstrip(argv[1]);

	if (!strcmp("Name", key)) {
		app->name = strdup(value);
	} else if (!strcmp("GenericName", key)) {
		app->generic_name = strdup(value);
	} else if (!strcmp("Exec", key)) {
		app->exec = strdup(value);
	} else if (!strcmp("TryExec", key)) {
		app->tryexec = strdup(value);
	} else if (!strcmp("Path", key)) {
		app->working_dir = strdup(value);
	} else if (!strcmp("Icon", key)) {
		app->icon = strdup(value);
	} else if (!strcmp("Categories", key)) {
		app->categories = strdup(value);
	} else if (!strcmp("NoDisplay", key)) {
		if (!strcasecmp(value, "true"))
			app->nodisplay = true;
	} else if (!strcmp("Terminal", key)) {
		if (!strcasecmp(value, "true"))
			app->terminal = true;
	}

	/* localized name */
	if (!strcmp(key, name_llcc)) {
		app->name_localized = strdup(value);
	}
	if (!app->name_localized && !strcmp(key, name_ll)) {
		app->name_localized = strdup(value);
	}

	/* localized generic name */
	if (!strcmp(key, generic_name_llcc)) {
		app->generic_name_localized = strdup(value);
	}
	if (!app->generic_name_localized && !strcmp(key, generic_name_ll)) {
		app->generic_name_localized = strdup(value);
	}
	g_strfreev(argv);
}

static bool
is_duplicate_desktop_file(char *filename)
{
	if (!filename) {
		return false;
	}
	GList *iter;
	for (iter = apps; iter; iter = iter->next) {
		struct app *app = (struct app *)iter->data;
		if (!app->filename) {
			continue;
		}
		if (!strcmp(app->filename, filename)) {
			return true;
		}
	}
	return false;
}

static void
delchar(char *p)
{
	size_t len = strlen(p);
	memmove(p, p + 1, len);
	*(p + len) = '\0';
}

static void
rtrim(char **s)
{
	size_t len;
	char *end;

	len = strlen(*s);
	if (!len)
		return;
	end = *s + len - 1;
	while (end >= *s && isspace(*end))
		end--;
	*(end + 1) = '\0';
}

/*
 * Remove all %? fields from .desktop Exec= field
 * Note:
 *  (a) %% becomes %
 *  (b) backslash escaped characters are resolved
 */
static void
strip_exec_field_codes(char **exec)
{
	if (!**exec || !*exec) {
		return;
	}
	for (char *p = *exec; *p; p++) {
		if (*p == '\\') {
			delchar(p);
			continue;
		}
		if (*p == '%') {
			delchar(p);
			if (*p == '\0') {
				break;
			}
			if (*p != '%') {
				delchar(p);
			}
		}
	}
	rtrim(exec);
}

static bool
isprog(const char *prog)
{
	char *s = g_find_program_in_path(prog);
	if (!s) {
		return false;
	}
	g_free(s);
	return true;
}

static struct app *
add_app(FILE *fp, char *filename)
{
	char line[4096], *p;
	int is_desktop_entry;

	struct app *app = calloc(1, sizeof(struct app));
	is_desktop_entry = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '\0') {
			continue;
		}
		p = strrchr(line, '\n');
		if (!p) {
			continue;
		}
		*p = '\0';

		/*
		 * .desktop files should be utf-8 compatible, but there are bad
		 * applications which don't comply so we need to handle
		 * exceptions.
		 */
		if (!g_utf8_validate(line, p - &line[0], NULL)) {
			fprintf(stderr, "warn: file '%s' not utf-8 compatible",
				filename);
			/* free current app */
			return NULL;
		}
		parse_line(line, app, &is_desktop_entry);
	}
	app->filename = strdup(filename);

	/* post-processing */
	if (app->exec) {
		strip_exec_field_codes(&app->exec);
	}
	if (app->tryexec && !isprog(app->tryexec)) {
		app->tryexec_not_in_path = true;
	}

	return app;
}

static void
process_file(char *filename, const char *path)
{
	char fullname[4096];

	if (!strstr(filename, ".desktop")) {
		return;
	}
	size_t len = strlen(path);
	strncpy(fullname, path, sizeof(fullname));
	strncpy(fullname + len, filename, sizeof(fullname) - len);
	FILE *fp = fopen(fullname, "r");
	if (!fp) {
		fprintf(stderr, "warn: could not open file %s", filename);
		return;
	}
	if (is_duplicate_desktop_file(filename)) {
		goto out;
	}
	struct app *app = add_app(fp, filename);
	if (app) {
		apps = g_list_append(apps, app);
	}
out:
	fclose(fp);
}

static void
traverse_directory(const char *path)
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(path);
	if (!dp) {
		return;
	}
	while ((entry = readdir(dp))) {
		if (entry->d_type == DT_DIR) {
			if (entry->d_name[0] != '.') {
				char new_path[PATH_MAX];

				snprintf(new_path, PATH_MAX, "%s%s/", path,
					 entry->d_name);
				traverse_directory(new_path);
			}
		} else if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
			process_file(entry->d_name, path);
		}
	}
	closedir(dp);
}

static int
compare_app_name(const void *a, const void *b)
{
	const struct app *aa = (struct app *)a;
	const struct app *bb = (struct app *)b;
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

static struct  {
	const char *prefix;
	const char *path;
} xdg_data_dirs[] = {
	{ "XDG_DATA_HOME", "" },
	{ "HOME", "/.local/share" },
	{ "XDG_DATA_DIRS", "" },
	{ NULL, "/usr/share" },
	{ NULL, "/usr/local/share" },
	{ NULL, "/opt/share" },
	{ NULL, NULL }
};

GList *
desktop_entries_create(void)
{
	GString *s = g_string_new(NULL);

	i18n_init();

	for (int i = 0; xdg_data_dirs[i].path; ++i) {
		if (xdg_data_dirs[i].prefix) {
			char *env = getenv(xdg_data_dirs[i].prefix);
			if (!env) {
				continue;
			}

			/*
			 * We need to respect that $XDG_DATA_DIRS might contain
			 * a number of directories separated by a colon
			 */
			gchar **prefixes = g_strsplit(env, ":", -1);
			for (gchar **p = prefixes; *p; p++) {
				g_string_printf(s, "%s%s/applications/", *p,
						xdg_data_dirs[i].path);
				traverse_directory(s->str);
			}
			g_strfreev(prefixes);
		} else {
			g_string_printf(s, "%s/applications/",
					xdg_data_dirs[i].path);
			traverse_directory(s->str);
		}
		if (getenv("LABWC_MENU_GENERATOR_DEBUG_FIRST_DIR_ONLY")) {
			break;
		}
	}
	g_string_free(s, TRUE);
	apps = g_list_sort(apps, (GCompareFunc)compare_app_name);

	return apps;
}

void
desktop_entries_destroy(GList *apps)
{
	GList *iter;
	for (iter = apps; iter; iter = iter->next) {
		struct app *app = (struct app *)iter->data;
		g_free(app->name);
		g_free(app->name_localized);
		g_free(app->generic_name);
		g_free(app->generic_name_localized);
		g_free(app->exec);
		g_free(app->tryexec);
		g_free(app->working_dir);
		g_free(app->icon);
		g_free(app->categories);
		g_free(app->filename);
		g_free(app);
	}
	g_list_free(apps);
}

// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (C) Johan Malm 2024 */
#define _POSIX_C_SOURCE 200809L
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "ignore.h"

static GList *ignore_files;

void
ignore_init(const char *filename)
{
	if (!filename || !*filename) {
		return;
	}
	FILE *stream = fopen(filename, "r");
	if (!stream) {
		return;
	}
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stream) != -1) {
		char *p = strrchr(line, '\n');
		if (p) {
			*p = '\0';
		}
		ignore_files = g_list_append(ignore_files,
			g_strdup(g_strstrip(line)));
	}
	free(line);
	fclose(stream);
}

void
ignore_finish(void)
{
	g_list_free_full(ignore_files, g_free);
}

bool
should_ignore(const char *filename)
{
	GList *iter;
	for (iter = ignore_files; iter; iter = iter->next) {
		if (!g_strcmp0((char *)iter->data, filename)) {
			return true;
		}
	}
	return false;
}

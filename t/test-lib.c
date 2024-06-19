#define _POSIX_C_SOURCE 200809L
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include "tap.h"

static void
show_diff(const char *filename, const char *buf, size_t size)
{
	char cmd[1000];
	snprintf(cmd, sizeof(cmd), "diff -u - %s >&2", filename);
	FILE *f = popen(cmd, "w");
	fwrite(buf, size, 1, f);
	pclose(f);
}

struct buf {
	gchar *data;
	gsize len;
};

bool
test_cmp_files(const char *filename_actual, const char *filename_expect)
{
	struct buf actual, expect;
	g_file_get_contents(filename_actual, &actual.data, &actual.len, NULL);
	g_file_get_contents(filename_expect, &expect.data, &expect.len, NULL);

	bool is_equal = !g_strcmp0(actual.data, expect.data);
	ok1(is_equal);
	if (!is_equal) {
		show_diff(filename_actual, expect.data, expect.len);
	}

	g_free(actual.data);
	g_free(expect.data);

	return is_equal;
}


/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef IGNORE_H
#define IGNORE_H
#include <stdbool.h>

void ignore_init(const char *filename);
void ignore_finish(void);
bool should_ignore(const char *filename);

#endif /* IGNORE_H */

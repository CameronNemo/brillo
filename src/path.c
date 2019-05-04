/* SPDX-License-Identifier: 0BSD */

#include <stdarg.h>
#include <limits.h>
#include <string.h>

#include "common.h"
#include "vlog.h"

/**
 * path_component:
 * @c:	component to verify
 *
 * Returns: true if c is a valid path component, otherwise false
 **/
bool path_component(const char *c) {
	if (!c || strnlen(c, NAME_MAX + 1) > NAME_MAX || strchr(c, '/') != NULL)
		return false;
	return true;
}

/**
 * path_new:
 *
 * Returns: pointer to initialized path string, or NULL on failure
 **/
char *path_new()
{
	char *p;

	if (!(p = malloc(PATH_MAX))) {
		vlog_err("malloc: %m");
		return NULL;
	}

	*p = '\0';

	return p;
}

/**
 * path_append:
 * @str:	string to append to
 * @fmt:	format string to append
 * @args:	variadic arguments to pass to printf
 *
 * Appends the format and arguments to the
 * supplied string, freeing str on error.
 *
 * Returns: str, or NULL on error
 **/
char *path_append(char * const str, const char *fmt, ...)
{
	int r;
	va_list ap;

	va_start(ap, fmt);

	r = vsnprintf(str + strlen(str), PATH_MAX - strlen(str), fmt, ap);

	if (r < 0 || (size_t)r >= (PATH_MAX - strlen(str))) {
		vlog_err("snprintf");
		free(str);
		va_end(ap);
		return NULL;
	}

	va_end(ap);
	return str;
}

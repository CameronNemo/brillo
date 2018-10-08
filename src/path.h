#ifndef PATH_H
#define PATH_H

#include <stdlib.h>
#include <stdarg.h>
#if 0
#include "helpers.h"
#endif

bool path_component(const char *c);
char *path_append(char * const str, const char *fmt, ...);
char *path_new(void);

#endif /* PATH_H */

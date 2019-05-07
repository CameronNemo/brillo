/* SPDX-License-Identifier: 0BSD */

#ifndef BURN_H
#define BURN_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

static inline void burn__o(void *o)
{
	free(*(void **)o);
}

static inline void burn__fd(int *fd)
{
	close(*fd);
}

static inline void burn__file(FILE **file)
{
	if (*file)
		fclose(*file);
}

static inline void burn__dir(DIR **dir)
{
	if (*dir)
		closedir(*dir);
}

#define burn_o __attribute__((cleanup(burn__o)))
#define burn_fd __attribute__((cleanup(burn__fd))) int
#define burn_dir __attribute__((cleanup(burn__dir))) DIR *
#define burn_file __attribute__((cleanup(burn__file))) FILE *

#endif /* BURN_H */

/* SPDX-License-Identifier: MIT */

#ifndef __BURNO_H
#define __BURNO_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

static inline void __burno__(void *o)
{
	free(*(void **)o);
}

static inline void __burnfd__(int *fd)
{
	close(*fd);
}

static inline void __burnfile__(FILE **file)
{
	if (*file)
		fclose(*file);
}

static inline void __burndir__(DIR **dir)
{
	if (*dir)
		closedir(*dir);
}

#define __burno 	__attribute__((cleanup(__burno__)))
#define __burnfd	__attribute__((cleanup(__burnfd__)))
#define __burnfile	__attribute__((cleanup(__burnfile__)))
#define __burndir	__attribute__((cleanup(__burndir__)))

#endif /* __BURNO_H */

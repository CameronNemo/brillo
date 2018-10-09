#ifndef LIGHT_HELPERS_H
#define LIGHT_HELPERS_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	LIGHT_ERROR_LEVEL = 1,
	LIGHT_WARN_LEVEL,
	LIGHT_NOTE_LEVEL
} light_loglevel_t;

light_loglevel_t light_loglevel;

#define LIGHT_LOG(lvl, fp, prefix, ...)			\
	if (light_loglevel >= lvl)			\
		do {					\
			fprintf(fp, prefix);		\
			fprintf(fp, "%s:%d\t%s():\t", __FILE__, __LINE__, __func__); \
			fprintf(fp, __VA_ARGS__);	\
			fprintf(fp, "\n");		\
		} while (0)

#define LIGHT_NOTE(...)  LIGHT_LOG(LIGHT_NOTE_LEVEL,  stderr, "NOTE: ", __VA_ARGS__)
#define LIGHT_WARN(...)  LIGHT_LOG(LIGHT_WARN_LEVEL,  stderr, "WARN: ", __VA_ARGS__)
#define LIGHT_ERR(...)   LIGHT_LOG(LIGHT_ERROR_LEVEL, stderr, "!ERR: ", __VA_ARGS__)

#endif				/* LIGHT_HELPERS_H */

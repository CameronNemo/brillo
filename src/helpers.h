#ifndef LIGHT_HELPERS_H
#define LIGHT_HELPERS_H

#include <stdbool.h>

#define LIGHT_CLAMP(val, min, max)					\
	(val < min							\
	 ? light_log_clamp_min(min)					\
	 : (val > max							\
	    ? light_log_clamp_max(max)					\
	    : val ))

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
			fprintf(fp, "%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
			fprintf(fp, __VA_ARGS__);	\
			fprintf(fp, "\n");		\
		} while (0)

#define LIGHT_NOTE(...) LIGHT_LOG(LIGHT_NOTE_LEVEL,  stderr, "NOTE: ", __VA_ARGS__)
#define LIGHT_WARN(...) LIGHT_LOG(LIGHT_WARN_LEVEL,  stderr, "WARN: ", __VA_ARGS__)
#define LIGHT_ERR(...)  LIGHT_LOG(LIGHT_ERROR_LEVEL, stderr, "!ERR: ", __VA_ARGS__)
#define LIGHT_MEMERR()  LIGHT_ERR("memory error");
#define LIGHT_PERMLOG(act, log)						\
	do {								\
		log("could not open '%s' for " act, filename);		\
		log("Verify it exists with the right permissions");	\
	} while (0)
#define LIGHT_PERMERR(x)         LIGHT_PERMLOG(x, LIGHT_ERR)
#define LIGHT_PERMWARN(x)        LIGHT_PERMLOG(x, LIGHT_WARN)

bool light_write_val(char const *filename, unsigned long v);
bool light_read_val(char const *filename, unsigned long *v);
bool light_test_w(char const *filename);
bool light_test_r(char const *filename);

/* Clamps the `percent` value between 0% and 100% */
double light_clamp_pct(double percent);

/* Prints a notice about a value which was below `x` and was adjusted to it */
unsigned long light_log_clamp_min(unsigned long x);

/* Prints a notice about a value which was above `x` and was adjusted to it */
unsigned long light_log_clamp_max(unsigned long x);

#endif				/* LIGHT_HELPERS_H */

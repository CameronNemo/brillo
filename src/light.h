/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef LIGHT_H
#define LIGHT_H

#include <stdlib.h>
#include <stdint.h>

typedef enum LIGHT_FIELD {
	LIGHT_FIELD_UNSET = 0,
	LIGHT_BRIGHTNESS,
	LIGHT_MAX_BRIGHTNESS,
	LIGHT_MIN_CAP,
	LIGHT_SAVERESTORE
} LIGHT_FIELD;

typedef enum LIGHT_TARGET {
	LIGHT_TARGET_UNSET = 0,
	LIGHT_BACKLIGHT,
	LIGHT_KEYBOARD
} LIGHT_TARGET;

typedef enum LIGHT_CTRL_MODE {
	LIGHT_CTRL_UNSET = 0,
	LIGHT_CTRL_AUTO,
	LIGHT_CTRL_ALL,
	LIGHT_CTRL_SPECIFY
} LIGHT_CTRL_MODE;

typedef enum LIGHT_OP_MODE {
	LIGHT_OP_UNSET = 0,
	LIGHT_GET,
	LIGHT_SET,
	LIGHT_ADD,
	LIGHT_SUB,
	LIGHT_PRINT_HELP,	/* Prints help and exits  */
	LIGHT_PRINT_VERSION,	/* Prints version info and exits */
	LIGHT_LIST_CTRL,
	LIGHT_RESTORE,
	LIGHT_SAVE
} LIGHT_OP_MODE;

typedef enum LIGHT_VAL_MODE {
	LIGHT_VAL_UNSET = 0,
	LIGHT_RAW,
	LIGHT_PERCENT,
	LIGHT_PERCENT_EXPONENTIAL
} LIGHT_VAL_MODE;

struct light_conf {
	char *sys_prefix;
	char *cache_prefix;
	char *ctrl;
	LIGHT_CTRL_MODE ctrl_mode;
	LIGHT_OP_MODE op_mode;
	LIGHT_VAL_MODE val_mode;
	LIGHT_TARGET target;
	LIGHT_FIELD field;
	int64_t value;
	int64_t usec;
	int64_t cached_max;
};

static inline void light_free(struct light_conf **conf)
{
	if (!(*conf))
		return;
	free((*conf)->ctrl);
	free((*conf)->sys_prefix);
	free((*conf)->cache_prefix);
	free(*conf);
}

#define light_t __attribute__((cleanup(light_free))) struct light_conf *

struct light_conf *light_new(void);
void light_defaults(struct light_conf *conf);

#endif				/* LIGHT_H */

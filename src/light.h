#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#define LIGHT_PROG "brillo"
#define LIGHT_VER "1.4.2"

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

typedef struct {
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
} light_conf_t;

light_conf_t *light_new(void);
void light_defaults(light_conf_t *conf);
void light_free(light_conf_t *conf);

#endif				/* LIGHT_H */

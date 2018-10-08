#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#define LIGHT_PROG "brillo"
#define LIGHT_VER "1.3.0"

typedef enum LIGHT_FIELD {
	LIGHT_BRIGHTNESS = 0,
	LIGHT_MAX_BRIGHTNESS,
	LIGHT_MIN_CAP,
	LIGHT_SAVERESTORE
} LIGHT_FIELD;

typedef enum LIGHT_TARGET {
	LIGHT_BACKLIGHT = 0,
	LIGHT_KEYBOARD
} LIGHT_TARGET;

typedef enum LIGHT_CTRL_MODE {
	LIGHT_AUTO = 0,
	LIGHT_SPECIFY
} LIGHT_CTRL_MODE;

typedef enum LIGHT_OP_MODE {
	LIGHT_GET = 0,
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
	LIGHT_RAW = 0,
	LIGHT_PERCENT,
	LIGHT_PERCENT_EXPONENTIAL
} LIGHT_VAL_MODE;

typedef struct {
	LIGHT_CTRL_MODE ctrl_mode;
	char *sys_prefix;
	char *cache_prefix;
	char *ctrl;
	LIGHT_OP_MODE op_mode;
	LIGHT_VAL_MODE val_mode;
	LIGHT_TARGET target;
	LIGHT_FIELD field;
	int64_t value;
	int64_t usec;
	int64_t cached_max;
} light_conf_t;

light_conf_t *light_new(void);
void light_free(light_conf_t *conf);

/* Global variable holding the settings for the current run */
//light_conf_t light_conf;

#endif				/* LIGHT_H */

#ifndef LIGHT_H
#define LIGHT_H

#include "helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>

#define LIGHT_PROG "brillo"
#define LIGHT_VER_MAJOR 1
#define LIGHT_VER_MINOR 2
#define LIGHT_VER_TYPE "rc1"
#define LIGHT_VENDOR_YEAR 2018
#define LIGHT_VENDOR "Cameron Nemo"
#define LIGHT_YEAR 2014
#define LIGHT_AUTHOR "Fredrik Haikarainen"

#define ASSERT_SET(t,v) \
  if(v)\
  {\
    fprintf(stderr, t" arguments can not be used in conjunction.\n");\
    return false;\
  }\
  v = true;

#define ASSERT_OPSET() ASSERT_SET("Operation", opSet)
#define ASSERT_TARGETSET() ASSERT_SET("Target", targetSet)
#define ASSERT_FIELDSET() ASSERT_SET("Field", fieldSet)
#define ASSERT_CTRLSET() ASSERT_SET("Controller", ctrlSet)
#define ASSERT_VALSET() ASSERT_SET("Value", valSet)

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
	LIGHT_PERCENT
} LIGHT_VAL_MODE;

typedef struct {
	LIGHT_CTRL_MODE ctrl_mode;
	char *sys_prefix;
	char *cache_prefix;
	char *ctrl;
	LIGHT_OP_MODE op_mode;
	LIGHT_VAL_MODE val_mode;
	unsigned long val_raw;
	double val_pct;
	LIGHT_TARGET target;
	LIGHT_FIELD field;
	unsigned long cached_max;
} light_conf_t;

/* Global variable holding the settings for the current run */
light_conf_t light_conf;

/* Initialization */

void light_defaults();
bool light_parse_args(int argc, char **argv);
void light_print_version(void);
void light_print_help(void);
bool light_list(void);
bool light_info(void);
bool light_initialize(void);
void light_free(void);

/* Execution */

bool light_execute(void);
char *light_path_new(const char *controller, LIGHT_FIELD type);
bool light_fetch(char const *controller, LIGHT_FIELD field, unsigned long *v);
bool light_set(char const *controller, LIGHT_FIELD field, unsigned long v);
bool light_ctrl_check(char const *controller);
bool light_fetch_mincap(char const *controller, unsigned long *mincap);
bool light_restore(char const *controller);
DIR *light_ctrl_iter_new();
char *light_ctrl_iter_next(DIR * dir);
char *light_ctrl_auto();

#endif				/* LIGHT_H */

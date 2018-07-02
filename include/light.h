#ifndef LIGHT_H
#define LIGHT_H

#include "helpers.h"

#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>

#define LIGHT_VER_MAJOR 0
#define LIGHT_VER_MINOR 10
#define LIGHT_VER_TYPE "beta"
#define LIGHT_YEAR 2014
#define LIGHT_AUTHOR "Fredrik Haikarainen"

#define ASSERT_SET(t,v) \
  if(v)\
  {\
    fprintf(stderr, t" arguments can not be used in conjunction.\n");\
    return FALSE;\
  }\
  v = TRUE;

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
  LIGHT_PRINT_HELP,   /* Prints help and exits  */
  LIGHT_PRINT_VERSION, /* Prints version info and exits */
  LIGHT_LIST_CTRL,
  LIGHT_RESTORE,
  LIGHT_SAVE

} LIGHT_OP_MODE;

typedef enum LIGHT_VAL_MODE {
  LIGHT_RAW = 0,
  LIGHT_PERCENT
} LIGHT_VAL_MODE;

typedef struct light_runtimeArguments_s {
  /* Which controller to use */
  LIGHT_CTRL_MODE controllerMode;
  char           *specifiedController;

  /* What to do with the controller */
  LIGHT_OP_MODE   operationMode;
  LIGHT_VAL_MODE  valueMode;
  unsigned long   specifiedValueRaw; /* The specified value in raw mode */
  double          specifiedValuePercent; /* The specified value in percent */

  LIGHT_TARGET   target;
  LIGHT_FIELD    field;

  /* Cache data */
  unsigned long   cachedMaxBrightness;

} light_runtimeArguments, *light_runtimeArguments_p;

/* Global variable holding the settings for the current run */
light_runtimeArguments light_Configuration;

/* Initialization */

void light_defaultConfig();
LIGHT_BOOL light_parseArguments(int argc, char** argv);
void light_printVersion(void);
void light_printHelp(void);
LIGHT_BOOL light_listControllers(void);
LIGHT_BOOL light_handleInfo(void);
LIGHT_BOOL light_initialize(void);

/* Execution */

LIGHT_BOOL light_execute(void);
char      *light_genPath(const char *controller, LIGHT_FIELD type);
LIGHT_BOOL light_getBrightness(char const *controller, unsigned long *v);
LIGHT_BOOL light_getMaxBrightness(char const *controller, unsigned long *v);
LIGHT_BOOL light_setBrightness(char const *controller, unsigned long v);
LIGHT_BOOL light_controllerAccessible(char const *controller);
/* WARNING: `controller` HAS to be at most NAME_MAX, otherwise fails */
LIGHT_BOOL light_getMinCap(char const *controller, LIGHT_BOOL *hasMinCap, unsigned long *minCap);
LIGHT_BOOL light_setMinCap(char const *controller, unsigned long v);
LIGHT_BOOL light_saveBrightness(char const *controller, unsigned long v);
LIGHT_BOOL light_restoreBrightness(char const *controller);
DIR  *light_genCtrlIterator();
char *light_nextCtrl(DIR* dir);
char *light_getBestCtrl();

#endif /* LIGHT_H */

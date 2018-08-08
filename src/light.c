#include "light.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * light_initExecution:
 *
 * Initializes values needed to execute the requested operation.
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_initExecution(unsigned long *rawCurr, unsigned long *rawMax, unsigned long *minCap)
{
  LIGHT_BOOL hasMinCap;

  if(light_Configuration.cachedMaxBrightness != 0)
  {
    *rawMax = light_Configuration.cachedMaxBrightness;
  }
  else if(!light_getMaxBrightness(light_Configuration.specifiedController, rawMax))
  {
    LIGHT_ERR("could not get max brightness");
    return FALSE;
  }

  /* No need to go further if targetting mincap */
  if(light_Configuration.field == LIGHT_MIN_CAP ||
     light_Configuration.field == LIGHT_MAX_BRIGHTNESS)
  {
    /* Init other values to 0 */
    *rawCurr = *minCap = 0;
    return TRUE;
  }

  if(!light_getBrightness(light_Configuration.specifiedController, rawCurr))
  {
    LIGHT_ERR("could not get brightness");
    return FALSE;
  }

  if(!light_getMinCap(light_Configuration.specifiedController, &hasMinCap, minCap))
  {
    LIGHT_ERR("could not get min brightness");
    return FALSE;
  }

  if(hasMinCap && *minCap > *rawMax )
  {
    LIGHT_ERR_FMT("invalid minimum cap (raw) value of '%lu' for controller", *minCap);
    LIGHT_ERR_FMT("minimum cap must be inferior to '%lu'", *rawMax);
    return FALSE;
  }
  return TRUE;
}

/**
 * light_executeGet:
 * @rawCurr:	current raw value
 * @rawMax:	maximum raw value
 * @minCap:	minimum raw value
 *
 * Executes the get operation, printing the appropriate field to standard out.
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_executeGet(unsigned long rawCurr, unsigned long rawMax, unsigned long minCap)
{
  unsigned long raw;
  double	pct;

  if (rawMax == 0)
    return FALSE;

  switch (light_Configuration.field) {
    case LIGHT_BRIGHTNESS:
      raw = rawCurr;
      pct = light_clampPercent(((double)rawCurr) / ((double)rawMax) * 100);
      break;
    case LIGHT_MAX_BRIGHTNESS:
      raw = rawMax;
      pct = 100.00;
      break;
    case LIGHT_MIN_CAP:
      raw = minCap;
      pct = light_clampPercent(((double)minCap)  / ((double)rawMax) * 100);
      break;
    case LIGHT_SAVERESTORE:
      return TRUE;
    default:
      return FALSE;
  }

  if (light_Configuration.valueMode == LIGHT_RAW)
    printf("%lu\n", raw);
  else
    printf("%.2f\n", pct);

  return TRUE;
}

/**
 * light_executeSet:
 * @rawCurr:    current raw value
 * @rawMax:     maximum raw value
 * @minCap:     minimum raw value
 *
 * Sets the minimum cap or brightness value.
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_executeSet(unsigned long rawCurr, unsigned long rawMax, unsigned long minCap)
{
  unsigned long val;

  if (light_Configuration.valueMode == LIGHT_RAW)
    val = light_Configuration.specifiedValueRaw;
  else
    val = (unsigned long) ( (light_Configuration.specifiedValuePercent * ((double)rawMax)) / 100.0);

  /* set the minimum cap */
  if (light_Configuration.field == LIGHT_MIN_CAP)
  {
    if (light_setMinCap(light_Configuration.specifiedController, LIGHT_CLAMP(val, 0, rawMax)))
      return TRUE;
    LIGHT_ERR("could not set minimum cap");
    return FALSE;
  }

  /* set the brightness */

  if (light_Configuration.field != LIGHT_BRIGHTNESS)
    return FALSE;

  switch (light_Configuration.operationMode)
  {
    case LIGHT_SUB:
      /* val is unsigned so we need to get back to >= 0 */
      if(val > rawCurr)
        val = -rawCurr;
      else
        val = -val;
    case LIGHT_ADD:
      val += rawCurr;
    case LIGHT_SET:
      break;
    default:
      return FALSE;
  }

  if(light_setBrightness(light_Configuration.specifiedController, LIGHT_CLAMP(val, minCap, rawMax)))
    return TRUE;

  LIGHT_ERR("could not set brightness");
  return FALSE;
}

/**
 * light_execute:
 * Executes the requested operation.
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_execute()
{
  unsigned long rawCurr; /* The current brightness, in raw units */
  unsigned long  rawMax; /* The max brightness, in raw units */
  unsigned long  minCap; /* The minimum cap, in raw units */

  if(!light_initExecution(&rawCurr, &rawMax, &minCap))
    return FALSE;

  LIGHT_NOTE_FMT("executing light on '%s' controller", light_Configuration.specifiedController);

  switch (light_Configuration.operationMode) {
  case LIGHT_GET:
    return light_executeGet(rawCurr, rawMax, minCap);
  case LIGHT_SAVE:
    if(light_saveBrightness(light_Configuration.specifiedController, rawCurr))
      return TRUE;
    LIGHT_ERR("could not save brightness");
    return FALSE;
  case LIGHT_RESTORE:
    if(light_restoreBrightness(light_Configuration.specifiedController))
      return TRUE;
    LIGHT_ERR("could not restore brightness");
    return FALSE;
  case LIGHT_SET:
  case LIGHT_SUB:
  case LIGHT_ADD:
    return light_executeSet(rawCurr, rawMax, minCap);
  /* Should not be reached */
  default:
    fprintf(stderr, "Controller: %s\nValueRaw: %lu\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nField: %u\n\n",
                     light_Configuration.specifiedController, light_Configuration.specifiedValueRaw,
                     light_Configuration.specifiedValuePercent, light_Configuration.operationMode,
                     light_Configuration.valueMode, light_Configuration.field);
    fprintf(stderr, "Invalid combination of commandline arguments.\n");
    light_printHelp();
    return FALSE;
  }
}

/**
 * light_genPath:
 * @controller:	name of controller device
 * @type:	field being accessed
 *
 * Generates a path in /sys or /etc for a given operation and
 * stores it in the string pointed to by buffer.
 *
 * WARNING: this function allocates memory, but does not free it.
 *          free the data pointed to by the return value after use.
 *
 * Returns: the generated path, or NULL on failure
 **/
char *light_genPath(const char *controller, LIGHT_FIELD type)
{
  char *path_new, *path_fmt, *path_prefix, *subsystem;
  int   r;

  if(!controller || NAME_MAX < strnlen(controller, NAME_MAX + 1))
  {
    LIGHT_ERR_FMT("invalid controller '%s', couldn't generate path", controller);
    return NULL;
  }

  if((path_new = malloc(PATH_MAX)) == NULL)
  {
    LIGHT_MEMERR();
    return NULL;
  }

  if (type == LIGHT_BRIGHTNESS || type == LIGHT_MAX_BRIGHTNESS)
    path_prefix = "/sys/class";
  else if (type == LIGHT_MIN_CAP || type == LIGHT_SAVERESTORE)
    path_prefix = "/var/cache/" LIGHT_PROG;
  else
    return NULL;

  if(light_Configuration.target == LIGHT_BACKLIGHT)
    subsystem = "backlight";
  else
    subsystem = "leds";

  switch(type)
  {
    case LIGHT_BRIGHTNESS:
      path_fmt = "%s/%s/%s/brightness";
      break;
    case LIGHT_MAX_BRIGHTNESS:
      path_fmt = "%s/%s/%s/max_brightness";
      break;
    case LIGHT_MIN_CAP:
      path_fmt = "%s/%s.%s.mincap";
      break;
    case LIGHT_SAVERESTORE:
      path_fmt = "%s/%s.%s.brightness";
      break;
    default:
      return NULL;
  }

  if((path_new = malloc(PATH_MAX)) == NULL)
  {
    LIGHT_MEMERR();
    return NULL;
  }

  r = snprintf(path_new, PATH_MAX, path_fmt, path_prefix, subsystem, controller);

  if(r < 0 || r >= PATH_MAX || path_new == NULL)
  {
    LIGHT_ERR("failed to copy generated path into buffer");
    if (path_new)
      free(path_new);
    return NULL;
  }

  return path_new;
}

/**
 * light_getBrightness:
 * @controller:	name of controller device
 * @v:		pointer to value being read
 *
 * Reads brightness value from the appropriate path.
 *
 * Returns: TRUE if value is successfully read, otherwise FALSE
 **/
LIGHT_BOOL light_getBrightness(char const *controller, unsigned long *v)
{
  char       *path;
  LIGHT_BOOL  r;

  if ((path = light_genPath(controller, LIGHT_BRIGHTNESS)) == NULL)
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }

  r = light_readULong(path, v);
  free(path);

  if(!r)
  {
    LIGHT_ERR("could not read value from brightness file");
    return FALSE;
  }

  return TRUE;
}

/**
 * light_getMaxBrightness:
 * @controller: name of controller device
 * @v:          pointer to value being read
 *
 * Reads max brightness value from the appropriate path.
 *
 * Returns: TRUE if value is successfully read, otherwise FALSE
 **/
LIGHT_BOOL light_getMaxBrightness(char const *controller, unsigned long *v)
{
  char       *path;
  LIGHT_BOOL  r;

  if ((path = light_genPath(controller, LIGHT_MAX_BRIGHTNESS)) == NULL)
  {
    LIGHT_ERR("could not generate path to maximum brightness file");
    return FALSE;
  }

  r = light_readULong(path , v);
  free(path);

  if(!r)
  {
    LIGHT_ERR("could not read value from max brightness file");
    return FALSE;
  }

  if(*v == 0)
  {
    LIGHT_ERR("max brightness is 0, so controller is not valid");
    return FALSE;
  }

  return TRUE;
}

/**
 * light_setBrightness:
 * @controller: name of controller device
 * @v:		new brightness value
 *
 * Sets the brightness for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_setBrightness(char const *controller, unsigned long v)
{
  char       *path;
  LIGHT_BOOL  r;

  if((path = light_genPath(controller, light_Configuration.field)) == NULL)
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("setting brightness %lu (raw) to controller", v);
  r = light_writeULong(path, v);

  if(!r)
  {
    LIGHT_ERR("could not write value to brightness file");
  }

  free(path);
  return r;
}

/**
 * light_controllerAccessible:
 * @controller:	name of controller to check
 *
 * Returns: TRUE if controller is accessible, otherwise FALSE
 **/
LIGHT_BOOL light_controllerAccessible(char const *controller)
{
  char *brightnessPath = NULL;

  /* On auto mode, we need to check if we can read the max brightness value
     of the controller for later computation */
  if(light_Configuration.controllerMode == LIGHT_AUTO ||
     light_Configuration.field == LIGHT_MAX_BRIGHTNESS)
  {
    if((brightnessPath = light_genPath(controller, LIGHT_MAX_BRIGHTNESS)) == NULL)
    {
      return FALSE;
    }
    if(!light_isReadable(brightnessPath))
    {
      LIGHT_WARN("could not open controller max brightness file for reading, so controller is not accessible");
      free(brightnessPath);
      return FALSE;
    }
    free(brightnessPath);
  }

  if((brightnessPath = light_genPath(controller, LIGHT_BRIGHTNESS)) == NULL)
    return FALSE;

  if(light_Configuration.operationMode != LIGHT_GET &&
     light_Configuration.field != LIGHT_MIN_CAP &&
     !light_isWritable(brightnessPath))
  {
    LIGHT_WARN("could not open controller brightness file for writing, so controller is not accessible");
    free(brightnessPath);
    return FALSE;
  }
  else if (!light_isReadable(brightnessPath))
  {
    LIGHT_WARN("could not open controller brightness file for reading, so controller is not accessible");
    free(brightnessPath);
    return FALSE;
  }

  free(brightnessPath);
  return TRUE;
}

/**
 * light_genCtrlIterator:
 *
 * Opens the appropriate directory for a target.
 *
 * WARNING: returns an opened directory, which
 *          should be closed after use
 *
 * Returns: directory, or NULL on failure
 **/
DIR *light_genCtrlIterator()
{
  DIR *dir;

  if(light_Configuration.target == LIGHT_KEYBOARD)
    dir = opendir("/sys/class/leds");
  else
    dir = opendir("/sys/class/backlight");

  if(dir == NULL)
    LIGHT_ERR("could not open backlight or leds directory in /sys/class");

  return dir;
}

/**
 * light_nextCtrl:
 * @dir:	opened directory to iterate over
 *
 * Iterates over the directory given by dir.
 *
 * WARNING: will allocate a string and return it,
 *          this string should be freed after use
 *
 * Returns: name of the next controller, NULL on end of dir or failure
 **/
char *light_nextCtrl(DIR *dir)
{
  struct dirent *file;

  if(!dir)
  {
    LIGHT_ERR("directory uninitialized");
    return NULL;
  }

  while((file = readdir(dir)) != NULL)
  {
    if(!(file->d_name) || file->d_name[0] == '.')
      continue;

    if(NAME_MAX < strnlen(file->d_name, NAME_MAX + 1))
    {
      LIGHT_WARN("invalid controller found, continuing...");
      continue;
    }

    return strndup(file->d_name, NAME_MAX);
  }

  return NULL;
}

/**
 * light_getBestCtrl:
 *
 * Iterates over the appropriate directory and finds the
 * controller with the highest max brightness.
 *
 * WARNING: will return an allocated string, which
 *          should be freed after use
 *
 * Returns: best controller, or NULL if no suitable controller is found
 **/
char *light_getBestCtrl()
{
  DIR *dir;
  char *best, *next;

  best = NULL;

  LIGHT_NOTE("finding best controller...");

  if((dir = light_genCtrlIterator()) == NULL)
    return NULL;

  while((next = light_nextCtrl(dir)) != NULL)
  {
    unsigned long max = 0;

    if(light_controllerAccessible(next) && light_getMaxBrightness(next, &max))
    {
        if(max > light_Configuration.cachedMaxBrightness)
        {
          light_Configuration.cachedMaxBrightness = max;
	  if (best)
            free(best);
          best = next;
          LIGHT_NOTE_FMT("using controller '%s', it is an improvement", best);
	  continue;
        }
        LIGHT_NOTE_FMT("ignoring controller '%s', it is not an improvement", next);
    }else{
      LIGHT_WARN_FMT("ignoring controller '%s', not accessible", next);
    }

    free(next);
  }

  closedir(dir);

  if(best == NULL)
    LIGHT_ERR("could not find an accessible controller");

  return best;
}

/**
 * light_getMinCap:
 * @controller:	name of controller device
 * @hasMinCap:	will be set to TRUE if the controller has a min cap stored
 * @minCap:	pointer to store the minimum cap value
 *
 * Returns: FALSE if could not determine minimum cap, otherwise true
 **/
LIGHT_BOOL light_getMinCap(char const * controller, LIGHT_BOOL * hasMinCap, unsigned long * minCap)
{
  char *path;

  if((path = light_genPath(controller, LIGHT_MIN_CAP)) == NULL)
  {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
  }

  if(!light_isReadable(path)){
    *hasMinCap = FALSE;
    *minCap = 0;
    free(path);
    LIGHT_NOTE("cap file doesn't exist or can't read from it, so assuming a minimum brightness of 0");
    return TRUE;
  }

  if(!light_readULong(path, minCap))
  {
    LIGHT_ERR("could not read minimum cap from file");
    free(path);
    return FALSE;
  }

  *hasMinCap = TRUE;

  free(path);
  return TRUE;
}

/**
 * light_setMinCap:
 * @controller: name of controller device
 * @v:          new minimum cap value
 *
 * Sets the minimum cap for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_setMinCap(char const * controller, unsigned long v)
{
  char *path;

  if((path = light_genPath(controller, LIGHT_MIN_CAP)) == NULL)
  {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("setting minimum cap to %lu (raw)", v);
  if(!light_writeULong(path, v))
  {
    LIGHT_ERR("could not write to minimum cap file");
    free(path);
    return FALSE;
  }

  free(path);
  return TRUE;
}

/**
 * light_saveBrightness:
 * @controller: name of controller device
 * @v:          brightness value to save
 *
 * Saves the brightness value for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_saveBrightness(char const *controller, unsigned long v){
  char *path;

  if((path = light_genPath(controller, LIGHT_SAVERESTORE)) == NULL)
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("saving brightness %lu (raw) to save file\n", v);
  if(!light_writeULong(path, v))
  {
    LIGHT_ERR("could not write to save/restore file");
    free(path);
    return FALSE;
  }

  free(path);
  return TRUE;
}

/**
 * light_restoreBrightness:
 * @controller: name of controller device
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_restoreBrightness(char const *controller){
  char *path;
  unsigned long v = 0;

  if((path = light_genPath(controller, LIGHT_SAVERESTORE)) == NULL)
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

  LIGHT_NOTE("restoring brightness from saved file");
  if(!light_readULong(path, &v))
  {
    LIGHT_ERR("could not read saved value");
    free(path);
    return FALSE;
  }

  if(!light_setBrightness(controller, v))
  {
    LIGHT_ERR("could not set restored brightness");
    free(path);
    return FALSE;
  }

  free(path);
  return TRUE;
}

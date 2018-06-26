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

  if(light_Configuration.hasCachedMaxBrightness)
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
 *
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
 *
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
 *
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
 * light_validControllerName:
 *
 * @controller:	name of controller to validate
 *
 * Returns: FALSE if controller is too long or NULL, otherwise TRUE
 **/
LIGHT_BOOL light_validControllerName(char const *controller)
{
  if(!controller)
  {
    return FALSE;
  }

  if(strlen(controller) > NAME_MAX)
  {
    LIGHT_WARN_FMT("controller \"%s\"'s name is too long", controller);
    return FALSE;
  }
  return TRUE;
}

/**
 * light_genPath:
 *
 * @controller:	name of controller device
 * @target:	device class being targeted
 * @type:	field being accessed
 * @buffer:	pointer to where the generated path will be stored
 *
 * Generates a path in /sys or /etc for a given operation and
 * stores it in the string pointed to by buffer.
 *
 * WARNING: this function allocates memory, but does not free it.
 *          free the value pointed to by buffer after use.
 *
 * Returns: TRUE if a path is successfully generated, otherwise FALSE
 **/
LIGHT_BOOL light_genPath(char const *controller, LIGHT_TARGET target, LIGHT_FIELD type, char **buffer)
{
  char* returner;
  int spfVal = -1;

  if(!light_validControllerName(controller))
  {
    LIGHT_ERR("invalid controller, couldn't generate path");
    return FALSE;
  }

  if(!buffer)
  {
    LIGHT_ERR("a valid buffer is required");
    return FALSE;
  }
  *buffer = NULL;

  /* PATH_MAX define includes the '\0' character, so no + 1 here*/
  if((returner = malloc(PATH_MAX)) == NULL)
  {
    LIGHT_MEMERR();
    return FALSE;
  }

  if(target == LIGHT_BACKLIGHT)
  {
    switch(type)
    {
      case LIGHT_BRIGHTNESS:
        spfVal = snprintf(returner, PATH_MAX, "/sys/class/backlight/%s/brightness", controller);
        break;
      case LIGHT_MAX_BRIGHTNESS:
        spfVal = snprintf(returner, PATH_MAX, "/sys/class/backlight/%s/max_brightness", controller);
        break;
      case LIGHT_MIN_CAP:
        spfVal = snprintf(returner, PATH_MAX, "/etc/light/mincap/%s", controller);
        break;
      case LIGHT_SAVERESTORE:
        spfVal = snprintf(returner, PATH_MAX, "/etc/light/save/%s", controller);
        break;
    }
  }else{
    switch(type)
    {
      case LIGHT_BRIGHTNESS:
        spfVal = snprintf(returner, PATH_MAX, "/sys/class/leds/%s/brightness", controller);
        break;
      case LIGHT_MAX_BRIGHTNESS:
        spfVal = snprintf(returner, PATH_MAX, "/sys/class/leds/%s/max_brightness", controller);
        break;
      case LIGHT_MIN_CAP:
        spfVal = snprintf(returner, PATH_MAX, "/etc/light/mincap/kbd/%s", controller);
        break;
      case LIGHT_SAVERESTORE:
        spfVal = snprintf(returner, PATH_MAX, "/etc/light/save/kbd/%s", controller);
        break;
    }
  }

  if(spfVal < 0)
  {
    LIGHT_ERR("snprintf failed");
    free(returner);
    return FALSE;
  }

  /* PATH_MAX define includes the '\0' character, so - 1 here*/
  if(spfVal > PATH_MAX - 1)
  {
    LIGHT_ERR("generated path is too long to be handled");
    return FALSE;
  }

  *buffer = returner;
  return TRUE;
}

/**
 * light_getBrightnessPath:
 *
 * @controller:	name of controller device
 * @path:	pointer to where the generated path will be stored
 *
 * Returns: TRUE if a path is successfully generated, otherwise FALSE
 **/
LIGHT_BOOL light_getBrightnessPath(char const *controller, char **path)
{
  if(!light_genPath(controller, light_Configuration.target, LIGHT_BRIGHTNESS, path))
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }
  return TRUE;
}

/**
 * light_getBrightness:
 *
 * @controller:	name of controller device
 * @v:		pointer to value being read
 *
 * Reads brightness value from the appropriate path.
 *
 * Returns: TRUE if value is successfully read, otherwise FALSE
 **/
LIGHT_BOOL light_getBrightness(char const *controller, unsigned long *v)
{
  char *brightnessPath = NULL;
  LIGHT_BOOL readVal = FALSE;

  if(!light_getBrightnessPath(controller, &brightnessPath))
  {
    return FALSE;
  }

  readVal = light_readULong( brightnessPath , v);
  free(brightnessPath);

  if(!readVal)
  {
    LIGHT_ERR("could not read value from brightness file");
    return FALSE;
  }
  return TRUE;
}

/**
 * light_getMaxBrightnessPath:
 *
 * @controller: name of controller device
 * @path:       pointer to where the generated path will be stored
 *
 * Returns: TRUE if a path is successfully generated, otherwise FALSE
 **/
LIGHT_BOOL light_getMaxBrightnessPath(char const *controller, char **path)
{
  if(!light_genPath(controller, light_Configuration.target, LIGHT_MAX_BRIGHTNESS, path))
  {
    LIGHT_ERR("could not generate path to maximum brightness file");
    return FALSE;
  }
  return TRUE;
}

/**
 * light_getMaxBrightness:
 *
 * @controller: name of controller device
 * @v:          pointer to value being read
 *
 * Reads max brightness value from the appropriate path.
 *
 * Returns: TRUE if value is successfully read, otherwise FALSE
 **/
LIGHT_BOOL light_getMaxBrightness(char const *controller, unsigned long *v)
{
  char *maxPath = NULL;
  LIGHT_BOOL readVal = FALSE;

  if (!light_getMaxBrightnessPath(controller, &maxPath))
  {
    return FALSE;
  }
  readVal = light_readULong(maxPath , v);
  free(maxPath);

  if(!readVal)
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
 *
 * @controller: name of controller device
 * @v:		new brightness value
 *
 * Sets the brightness for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_setBrightness(char const *controller, unsigned long v)
{
  char *brightnessPath = NULL;
  LIGHT_BOOL writeVal = FALSE;

  if(!light_genPath(controller, light_Configuration.target, light_Configuration.field, &brightnessPath))
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("setting brightness %lu (raw) to controller", v);
  writeVal = light_writeULong(brightnessPath, v);

  if(!writeVal)
  {
    LIGHT_ERR("could not write value to brightness file");
  }

  free(brightnessPath);
  return writeVal;
}

/**
 * light_controllerAccessible:
 *
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
    if(!light_getMaxBrightnessPath(controller, &brightnessPath))
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

  if(!light_getBrightnessPath(controller, &brightnessPath))
  {
    return FALSE;
  }

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
 * light_prepareControllerIteration:
 *
 * @dir:	pointer to store the opened directory at
 *
 * Opens the appropriate directory for a target,
 * and saves the directory in dir.
 *
 * Returns: TRUE if directory is successfully opened, otherwise false
 **/
LIGHT_BOOL light_prepareControllerIteration(DIR **dir)
{
  if(!dir)
  {
    LIGHT_ERR("specified dir was NULL");
    return FALSE;
  }

  if(light_Configuration.target == LIGHT_KEYBOARD)
  {
    *dir = opendir("/sys/class/leds");
  }
  else
  {
    *dir = opendir("/sys/class/backlight");
  }
  if(dir == NULL)
  {
    LIGHT_ERR("could not open backlight or leds directory in /sys/class");
    return FALSE;
  }
  return TRUE;
}

/**
 * light_iterateControllers:
 *
 * @dir:	opened directory to iterate over
 * @currCtrl:	string to store controller in,
 * 		with a size no less than NAME_MAX + 1
 *
 * Iterates over the directory given by dir,
 * stores the name of the next valid controller
 * in the string given by currCtrl.
 *
 * Returns: TRUE if a valid controller is found, otherwise FALSE
 **/
LIGHT_BOOL light_iterateControllers(DIR *dir, char *currCtrl)
{
  struct dirent *file;
  LIGHT_BOOL controllerFound = FALSE;

  if(!dir || !currCtrl)
  {
    LIGHT_ERR("one of the arguments was NULL");
    return FALSE;
  }

  while(!controllerFound)
  {
    file = readdir(dir);

    if(file == NULL)
    {
      return FALSE;
    }
    else if(file->d_name[0] == '.')
    {
      continue;
    }
    else if(!light_validControllerName(file->d_name))
    {
      LIGHT_WARN_FMT("invalid controller '%s' found, continuing...", file->d_name);
      continue;
    }

    controllerFound = TRUE;
  }

  snprintf(currCtrl, NAME_MAX + 1, "%s", file->d_name);
  return TRUE;
}

/**
 * light_getBestController:
 *
 * @controller:	string to store the name of the best controller
 *
 * Iterates over the appropriate directory and finds the
 * controller with the highest max brightness.
 *
 * Returns: TRUE if a suitable controller is found, otherwise FALSE
 **/
LIGHT_BOOL light_getBestController(char *controller)
{
  DIR *dir;
  unsigned long bestValYet = 0;
  LIGHT_BOOL foundOkController = FALSE;
  char bestYet[NAME_MAX + 1];
  char currentController[NAME_MAX + 1];

  if(!controller)
  {
    LIGHT_ERR("controller buffer was NULL");
    return FALSE;
  }

  if(!light_prepareControllerIteration(&dir))
  {
    LIGHT_ERR("can't list controllers");
    return FALSE;
  }

  while(light_iterateControllers(dir, currentController))
  {
    unsigned long currVal = 0;

    LIGHT_NOTE_FMT("found '%s' controller", currentController);
    if(light_controllerAccessible(currentController))
    {

      if(light_getMaxBrightness(currentController, &currVal))
      {
        if(currVal > bestValYet)
        {
          foundOkController = TRUE;
          bestValYet = currVal;
          snprintf(bestYet, NAME_MAX + 1, "%s", currentController);
          light_Configuration.hasCachedMaxBrightness = TRUE;
          light_Configuration.cachedMaxBrightness = currVal;
        }else{
          LIGHT_NOTE("ignoring controller as better one already found");
        }
      }else{
        LIGHT_WARN("could not read max brightness from file");
      }
    }else{
      LIGHT_WARN("controller not accessible");
    }
  }

  closedir(dir);

  if(!foundOkController)
  {
    LIGHT_ERR("could not find an accessible controller");
    return FALSE;
  }

  if(bestValYet == 0)
  {
    LIGHT_ERR("found accessible controller but it's useless/corrupt");
    return FALSE;
  }

  snprintf(controller, NAME_MAX + 1, "%s", bestYet);
  return TRUE;
}

/**
 * light_getMinCap:
 *
 * @controller:	name of controller device
 * @hasMinCap:	will be set to TRUE if the controller has a min cap stored
 * @minCap:	pointer to store the minimum cap value
 *
 * Returns: FALSE if could not determine minimum cap, otherwise true
 **/
LIGHT_BOOL light_getMinCap(char const * controller, LIGHT_BOOL * hasMinCap, unsigned long * minCap)
{
 char * mincapPath = NULL;

 if(!light_genPath(controller, light_Configuration.target, LIGHT_MIN_CAP, &mincapPath))
 {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
 }

  if(!light_isReadable(mincapPath)){
    *hasMinCap = FALSE;
    *minCap = 0;
    free(mincapPath);
    LIGHT_NOTE("cap file doesn't exist or can't read from it, so assuming a minimum brightness of 0");
    return TRUE;
  }

  if(!light_readULong(mincapPath, minCap))
  {
    LIGHT_ERR("could not read minimum cap from file");
    free(mincapPath);
    return FALSE;
  }

  *hasMinCap = TRUE;

  free(mincapPath);
  return TRUE;
}

/**
 * light_setMinCap:
 *
 * @controller: name of controller device
 * @v:          new minimum cap value
 *
 * Sets the minimum cap for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_setMinCap(char const * controller, unsigned long v)
{
  char * mincapPath = NULL;
  if(!light_genPath(controller, light_Configuration.target, LIGHT_MIN_CAP, &mincapPath))
  {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("setting minimum cap to %lu (raw)", v);
  if(!light_writeULong(mincapPath, v))
  {
    LIGHT_ERR("could not write to minimum cap file");
    free(mincapPath);
    return FALSE;
  }

  free(mincapPath);
  return TRUE;
}

/**
 * light_saveBrightness:
 *
 * @controller: name of controller device
 * @v:          brightness value to save
 *
 * Saves the brightness value for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_saveBrightness(char const *controller, unsigned long v){
  char *savePath = NULL;

  if(!light_genPath(controller, light_Configuration.target, LIGHT_SAVERESTORE, &savePath))
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

  LIGHT_NOTE_FMT("saving brightness %lu (raw) to save file\n", v);
  if(!light_writeULong(savePath, v))
  {
    LIGHT_ERR("could not write to save/restore file");
    free(savePath);
    return FALSE;
  }

  free(savePath);
  return TRUE;
}

/**
 * light_restoreBrightness:
 *
 * @controller: name of controller device
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: TRUE if write was successful, otherwise FALSE
 **/
LIGHT_BOOL light_restoreBrightness(char const *controller){
  char *restorePath = NULL;
  unsigned long v = 0;

  if(!light_genPath(controller, light_Configuration.target, LIGHT_SAVERESTORE, &restorePath))
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

  LIGHT_NOTE("restoring brightness from saved file");
  if(!light_readULong(restorePath, &v))
  {
    LIGHT_ERR("could not read saved value");
    free(restorePath);
    return FALSE;
  }

  if(!light_setBrightness(controller, v))
  {
    LIGHT_ERR("could not set restored brightness");
    free(restorePath);
    return FALSE;
  }

  free(restorePath);
  return TRUE;
}

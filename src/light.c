#include "light.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

void light_defaultConfig()
{
  light_Configuration.controllerMode         = LIGHT_AUTO;
  memset(&light_Configuration.specifiedController, '\0', NAME_MAX + 1);
  light_Configuration.operationMode          = LIGHT_GET;
  light_Configuration.valueMode              = LIGHT_PERCENT;
  light_Configuration.specifiedValueRaw      = 0;
  light_Configuration.specifiedValuePercent  = 0.0;
  light_Configuration.target                 = LIGHT_BACKLIGHT;
  light_Configuration.field                  = LIGHT_BRIGHTNESS;
  light_Configuration.hasCachedMaxBrightness = FALSE;
  light_Configuration.cachedMaxBrightness    = 0;
  light_verbosity                            = 0;
}

LIGHT_BOOL light_checkOperations()
{
  LIGHT_BOOL valid = TRUE;
  LIGHT_OP_MODE op = light_Configuration.operationMode;

  /* Nothing to check if we just print info */
  if(op == LIGHT_PRINT_HELP || op == LIGHT_PRINT_VERSION || op == LIGHT_LIST_CTRL)
  {
    return TRUE;
  }

  switch (light_Configuration.field) {
  case LIGHT_BRIGHTNESS:
    if(op != LIGHT_GET && op != LIGHT_SET &&
       op != LIGHT_ADD && op != LIGHT_SUB)
    {
      valid = FALSE;
      fprintf(stderr, "Wrong operation specified for brightness. You can use only -G -S -A or -U\n\n");
    }
    break;
  case LIGHT_MAX_BRIGHTNESS:
    if(op != LIGHT_GET)
    {
      valid = FALSE;
      fprintf(stderr, "Wrong operation specified for max brightness. You can only use -G\n\n");
    }
    break;
  case LIGHT_MIN_CAP:
    if(op != LIGHT_GET && op != LIGHT_SET)
    {
      valid = FALSE;
      fprintf(stderr, "Wrong operation specified for min cap. You can only use -G or -S\n\n");
    }
  default:
    break;
  }
  return valid;
}


LIGHT_BOOL light_parseArguments(int argc, char** argv)
{
  int currFlag;
  int verbosity;

  LIGHT_BOOL opSet = FALSE;
  LIGHT_BOOL targetSet = FALSE;
  LIGHT_BOOL fieldSet = FALSE;
  LIGHT_BOOL ctrlSet = FALSE;
  LIGHT_BOOL valSet = FALSE;

  while((currFlag = getopt(argc, argv, "HhVGSAULIObmclkas:prv:")) != -1)
  {
    switch(currFlag)
    {
      /* -- Operations -- */
      case 'H':
      case 'h':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_PRINT_HELP;
        break;
      case 'V':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_PRINT_VERSION;
        break;
      case 'G':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_GET;
        break;
      case 'S':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_SET;
        break;
      case 'A':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_ADD;
        break;
      case 'U':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_SUB;
        break;
      case 'L':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_LIST_CTRL;
        break;
      case 'I':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_RESTORE;
        break;
      case 'O':
        ASSERT_OPSET();
        light_Configuration.operationMode = LIGHT_SAVE;
        break;

      /* -- Targets -- */
      case 'l':
        ASSERT_TARGETSET();
        light_Configuration.target = LIGHT_BACKLIGHT;
        break;
      case 'k':
        ASSERT_TARGETSET();
        light_Configuration.target = LIGHT_KEYBOARD;
        break;

      /* -- Fields -- */
      case 'b':
        ASSERT_FIELDSET();
        light_Configuration.field = LIGHT_BRIGHTNESS;
        break;
      case 'm':
        ASSERT_FIELDSET();
        light_Configuration.field = LIGHT_MAX_BRIGHTNESS;
        break;
      case 'c':
        ASSERT_FIELDSET();
        light_Configuration.field = LIGHT_MIN_CAP;
        break;

      /* -- Controller selection -- */
      case 'a':
        ASSERT_CTRLSET();
        light_Configuration.controllerMode = LIGHT_AUTO;
        break;;
      case 's':
        ASSERT_CTRLSET();
        light_Configuration.controllerMode = LIGHT_SPECIFY;
        if(optarg == NULL)
        {
          fprintf(stderr, "-s NEEDS an argument.\n\n");
          light_printHelp();
        }

        if(!light_validControllerName(optarg))
        {
          fprintf(stderr, "can't handle controller '%s'\n", optarg);
          return FALSE;
        }
        strncpy(light_Configuration.specifiedController, optarg, NAME_MAX);
        light_Configuration.specifiedController[NAME_MAX] = '\0';
        break;
      /* -- Value modes -- */
      case 'p':
        ASSERT_VALSET();
        light_Configuration.valueMode = LIGHT_PERCENT;
        break;
      case 'r':
        ASSERT_VALSET();
        light_Configuration.valueMode = LIGHT_RAW;
        break;

      /* -- Other -- */
      case 'v':
        if(optarg == NULL)
        {
          fprintf(stderr, "-v NEEDS an argument.\n\n");
          light_printHelp();
          return FALSE;
        }
        if(sscanf(optarg, "%i", &verbosity) != 1)
        {
          fprintf(stderr, "-v Verbosity is not specified in a recognizable format.\n\n");
          light_printHelp();
          return FALSE;
        }
        if(verbosity < 0 || verbosity > 3)
        {
          fprintf(stderr, "-v Verbosity has to be between 0 and 3.\n\n");
          light_printHelp();
          return FALSE;
        }
        light_verbosity = (LIGHT_LOG_LEVEL)verbosity;
        break;
    }
  }

  if(!light_checkOperations())
  {
    light_printHelp();
    return FALSE;
  }

  /* If we need a <value> (for writing), make sure we have it! */
  if(light_Configuration.operationMode == LIGHT_SET ||
     light_Configuration.operationMode == LIGHT_ADD ||
     light_Configuration.operationMode == LIGHT_SUB)
  {
    if(argc - optind != 1)
    {
      fprintf(stderr, "Light needs an argument for <value>.\n\n");
      light_printHelp();
      return FALSE;
    }

    if(light_Configuration.valueMode == LIGHT_PERCENT)
    {
      if(sscanf(argv[optind], "%lf", &light_Configuration.specifiedValuePercent) != 1){
        fprintf(stderr, "<value> is not specified in a recognizable format.\n\n");
        light_printHelp();
        return FALSE;
      }
      light_Configuration.specifiedValuePercent = light_clampPercent(light_Configuration.specifiedValuePercent);
    }else{
      if(sscanf(argv[optind], "%lu", &light_Configuration.specifiedValueRaw) != 1){
        fprintf(stderr, "<value> is not specified in a recognizable format.\n\n");
        light_printHelp();
        return FALSE;
      }
    }

  }

  return TRUE;
}

void light_printVersion(){
  printf("Light %u.%u (%s)\n", LIGHT_VER_MAJOR, LIGHT_VER_MINOR, LIGHT_VER_TYPE);
  printf("Copyright (C) %u %s\n", LIGHT_YEAR, LIGHT_AUTHOR);
  printf("This is free software, see the source for copying conditions.  There is NO\n");
  printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n\n");
}

void light_printHelp(){
  printf("Usage: light <options> <value>\n");
  printf("<value> has to be either integral(raw mode) or decimal(percent mode) depending on the specified value mode.\n");
  printf("<options> can be any of the following:\n\n");

  printf("Operations (can not be used in conjunction):\n");
  printf("  -H -h:\tPrints this help and exits\n");
  printf("  -V:\t\tPrints version info and exits\n");
  printf("  -G:\t\tGet value (default)\n");
  printf("  -S:\t\tSet value\n");
  printf("  -A:\t\tAdd value\n");
  printf("  -U:\t\tSubtract value\n");
  printf("  -L:\t\tList controllers\n");
  printf("  -I:\t\tRestore brightness\n");
  printf("  -O:\t\tSave brightness\n\n");

  printf("Targets (can not be used in conjunction):\n");
  printf("  -l:\t\tAct on screen backlight (default)\n");
  printf("  -k:\t\tAct on keyboard backlight\n\n");

  printf("Fields (can not be used in conjunction):\n");
  printf("  -b:\t\tBrightness (default)\n  \t\tUsed with [GSAU]\n\n");
  printf("  -m:\t\tMaximum brightness\n  \t\tUsed with [G]\n\n");
  printf("  -c:\t\tMinimum cap\n  \t\tUsed with [GS]\n");
  printf("  \t\tG returns null if no minimum cap is set.\n\n");

  printf("Controller selection (can not be used in conjunction):\n");
  printf("  -a:\t\tSelects controller automatically (default).\n");
  printf("  -s:\t\tSpecify controller to use. (needs argument)\n\n");

  printf("Value modes (can not be used in conjunction):\n");
  printf("  -p:\t\tInterpret <value> as, and output values in, percent. (default)\n");
  printf("  -r:\t\tInterpret <value> as, and output values in, raw mode.\n\n");

  printf("Other:\n");
  printf("  -v:\t\tSets the verbosity level, (needs argument).\n  \t\t0: Only outputs read values.\n  \t\t1: Read values, Errors.\n  \t\t2: Read values, Errors, Warnings.\n  \t\t3: Read values, Errors, Warnings, Notices.\n\n");
}

LIGHT_BOOL light_initialize(int argc, char** argv)
{
  int mkdirVal;
  LIGHT_OP_MODE mode;

  light_defaultConfig();
  if(!light_parseArguments(argc, argv))
  {
    LIGHT_ERR("could not parse arguments");
    return FALSE;
  }
  mode = light_Configuration.operationMode;

  /* Just return true for operation modes that do not need initialization */
  if(mode == LIGHT_PRINT_HELP ||
     mode == LIGHT_PRINT_VERSION ||
     mode == LIGHT_LIST_CTRL)
  {
      return TRUE;
  }

  if(mode == LIGHT_SAVE ||
     (mode == LIGHT_SET && light_Configuration.field == LIGHT_MIN_CAP))
  {
    /* Make sure we have a valid /etc/light directory, as well as mincap and save */
    char const * const dirs[5] = {"/etc/light", "/etc/light/mincap", "/etc/light/save", "/etc/light/mincap/kbd", "/etc/light/save/kbd"};
    char const * const *dir = dirs;
    char const * const direrr = "'%s' does not exist and could not be created, make sure this application is run as root.";

    while (dir < dirs + 5)
    {
      mkdirVal = mkdir(*dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if(mkdirVal != 0 && errno != EEXIST)
      {
        LIGHT_ERR_FMT(direrr, *dir);
        return FALSE;
      }
      ++dir;
    }
  }

  /* Make sure we have a valid controller before we proceed */
  if(light_Configuration.controllerMode == LIGHT_AUTO)
  {
    LIGHT_NOTE("Automatic mode -- finding best controller");
    if(!light_getBestController(light_Configuration.specifiedController))
    {
      LIGHT_ERR("could not find suitable controller");
      return FALSE;
    }
  }
  else if(!light_controllerAccessible(light_Configuration.specifiedController))
  {
    LIGHT_ERR_FMT("selected controller '%s' is not valid",
                  light_Configuration.specifiedController);
    return FALSE;
  }

  return TRUE;
}

/* Print help and version info */
LIGHT_BOOL light_handleInfo()
{
  if(light_Configuration.operationMode == LIGHT_PRINT_HELP)
  {
    light_printHelp();
    return TRUE;
  }

  if(light_Configuration.operationMode == LIGHT_PRINT_VERSION)
  {
    light_printVersion();
    return TRUE;
  }

  if(light_Configuration.operationMode == LIGHT_LIST_CTRL)
  {
    /* listControllers() can return FALSE, but only if it does not find any controllers. That is not enough for an unsuccessfull run. */
    light_listControllers();
    return TRUE;
  }
  return FALSE;
}

LIGHT_BOOL light_initExecution(unsigned long *rawCurr, unsigned long *rawMax, LIGHT_BOOL *hasMinCap, unsigned long *minCap)
{
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
    *hasMinCap = FALSE;
    return TRUE;
  }

  if(!light_getBrightness(light_Configuration.specifiedController, rawCurr))
  {
    LIGHT_ERR("could not get brightness");
    return FALSE;
  }

  if(!light_getMinCap(light_Configuration.specifiedController, hasMinCap, minCap))
  {
    LIGHT_ERR("could not get min brightness");
    return FALSE;
  }

  if( *hasMinCap && *minCap > *rawMax )
  {
    LIGHT_WARN_FMT("invalid minimum cap (raw) value of '%lu' for controller, ignoring and using 0", *minCap);
    LIGHT_WARN_FMT("minimum cap must be inferior to '%lu'", *rawMax);
    minCap = 0;
  }
  return TRUE;
}

LIGHT_BOOL light_execute()
{
  unsigned long rawCurr; /* The current brightness, in raw units */
  double    percentCurr; /* The current brightness, in percent  */
  unsigned long  rawMax; /* The max brightness, in percent      */

  unsigned long   minCap; /* The minimum cap, in raw units */
  double   percentMinCap; /* The minimum cap, in percent */
  LIGHT_BOOL   hasMinCap; /* If we have a minimum cap     */

  LIGHT_VAL_MODE valueMode;

  if(light_handleInfo())
  {
    return TRUE;
  }

  if(!light_initExecution(&rawCurr, &rawMax, &hasMinCap, &minCap))
  {
    return FALSE;
  }

  valueMode = light_Configuration.valueMode;
  percentCurr =   light_clampPercent(((double)rawCurr) / ((double)rawMax) * 100);
  percentMinCap = light_clampPercent(((double)minCap)  / ((double)rawMax) * 100);

  LIGHT_NOTE_FMT("executing light on '%s' controller", light_Configuration.specifiedController);

  /* Handle get operations */
  if(light_Configuration.operationMode == LIGHT_GET)
  {
    switch(light_Configuration.field){
      case LIGHT_BRIGHTNESS:
        (valueMode == LIGHT_RAW) ? printf("%lu\n", rawCurr) : printf("%.2f\n", percentCurr);
        break;
      case LIGHT_MAX_BRIGHTNESS:
        (valueMode == LIGHT_RAW) ? printf("%lu\n", rawMax) : printf("100.00\n"); /* <- I know how stupid it is but it might just make someones life easier */
        break;
      case LIGHT_MIN_CAP:
        (valueMode == LIGHT_RAW) ? printf("%lu\n", minCap) : printf("%.2f\n", percentMinCap);
        break;
      case LIGHT_SAVERESTORE:
        break;
    }
    return TRUE;
  }

  /* Handle saves and restores*/
  if(light_Configuration.operationMode == LIGHT_SAVE){
    if(!light_saveBrightness(light_Configuration.specifiedController, rawCurr))
    {
      LIGHT_ERR("could not save brightness");
      return FALSE;
    }

    return TRUE;
  }

  if(light_Configuration.operationMode == LIGHT_RESTORE){
    if(!light_restoreBrightness(light_Configuration.specifiedController)){
      LIGHT_ERR("could not restore brightness");
      return FALSE;
    }

    return TRUE;
  }

  /* Handle set/add/sub operations */
  if(light_Configuration.operationMode == LIGHT_SET ||
     light_Configuration.operationMode == LIGHT_ADD ||
     light_Configuration.operationMode == LIGHT_SUB)
  {
    unsigned long specValueRaw = valueMode == LIGHT_RAW ?
      light_Configuration.specifiedValueRaw :
      (unsigned long) ( (light_Configuration.specifiedValuePercent * ((double)rawMax)) / 100.0);

    if(light_Configuration.field == LIGHT_MIN_CAP)
    {
      /* Handle minimum cap files */
      if(!light_setMinCap(light_Configuration.specifiedController, LIGHT_CLAMP(specValueRaw, 0, rawMax)))
      {
        LIGHT_ERR("could not set minimum cap");
        return FALSE;
      }

      /* All good? Return true. */
      return TRUE;

    }else if(light_Configuration.field == LIGHT_BRIGHTNESS){
      /* Handle brightness writing */
      unsigned long writeVal;

      switch(light_Configuration.operationMode)
      {
        case LIGHT_SET:
          writeVal = LIGHT_CLAMP(specValueRaw, minCap, rawMax);
          break;
        case LIGHT_ADD:
          writeVal = LIGHT_CLAMP(rawCurr + specValueRaw, minCap, rawMax);
          break;
        case LIGHT_SUB:
          /* check if we're going below 0, which wouldn't work with unsigned values */
          if(rawCurr < specValueRaw)
          {
            light_logInfClamp(minCap);
            writeVal = minCap;
            break;
          }
          writeVal = LIGHT_CLAMP(rawCurr - specValueRaw, minCap, rawMax);
         break;
        /* we have already taken other possibilities, so we shouldn't get here */
        default:
          return FALSE;
      }

      /* Attempt to write */
      if(!light_setBrightness(light_Configuration.specifiedController, writeVal))
      {
        LIGHT_ERR("could not set brightness");
        return FALSE;
      }

      /* All good? return true. */
      return TRUE;
    }
  }

  /* Handle saves and restores*/
  if(light_Configuration.operationMode == LIGHT_SAVE){
    if(!light_saveBrightness(light_Configuration.specifiedController, rawCurr))
    {
      LIGHT_ERR("could not save brightness");
      return FALSE;
    }

    return TRUE;
  }

  if(light_Configuration.operationMode == LIGHT_RESTORE){
    if(!light_restoreBrightness(light_Configuration.specifiedController)){
      LIGHT_ERR("could not restore brightness");
      return FALSE;
    }

    return TRUE;
  }

  fprintf(stderr, "Controller: %s\nValueRaw: %lu\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nField: %u\n\n", light_Configuration.specifiedController, light_Configuration.specifiedValueRaw, light_Configuration.specifiedValuePercent, light_Configuration.operationMode, valueMode, light_Configuration.field);

  fprintf(stderr, "You did not specify a valid combination of commandline arguments. Have some help: \n");
  light_printHelp();
  return FALSE;
}

void light_free()
{

}

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

LIGHT_BOOL light_getBrightnessPath(char const *controller, char **path)
{
  if(!light_genPath(controller, light_Configuration.target, LIGHT_BRIGHTNESS, path))
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }
  return TRUE;
}

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

LIGHT_BOOL light_getMaxBrightnessPath(char const *controller, char **path)
{
  if(!light_genPath(controller, light_Configuration.target, LIGHT_MAX_BRIGHTNESS, path))
  {
    LIGHT_ERR("could not generate path to maximum brightness file");
    return FALSE;
  }
  return TRUE;
}

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

LIGHT_BOOL light_iterateControllers(DIR *dir, char *currentController)
{
  struct dirent *file;
  LIGHT_BOOL controllerFound = FALSE;

  if(!dir || !currentController)
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

    if(file->d_name[0] != '.')
    {
      if(!light_validControllerName(file->d_name))
      {
        LIGHT_WARN_FMT("invalid controller '%s' found, continuing...", file->d_name);
        continue;
      }
      controllerFound = TRUE;
    }
  }

  strncpy(currentController, file->d_name, NAME_MAX);
  currentController[NAME_MAX] = '\0';
  return TRUE;
}

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
          strncpy(bestYet, currentController, NAME_MAX);
          bestYet[NAME_MAX] = '\0';
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

  strncpy(controller, bestYet, NAME_MAX);
  controller[NAME_MAX] = '\0';
  return TRUE;
}

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

LIGHT_BOOL light_listControllers()
{
  DIR *dir;
  char controller[NAME_MAX + 1];
  LIGHT_BOOL foundController = FALSE;

  if(!light_prepareControllerIteration(&dir))
  {
    LIGHT_ERR("can't list controllers");
    return FALSE;
  }

  while(light_iterateControllers(dir, controller))
  {
    printf("%s\n", controller);
    foundController = TRUE;
  }

  if(!foundController)
  {
    LIGHT_WARN("no controllers found, either check your system or your permissions");
    return FALSE;
  }

  return TRUE;
}

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

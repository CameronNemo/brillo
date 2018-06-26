#include "light.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * light_defaultConfig:
 *
 * Initialize the default configuration values.
 **/
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

/**
 * light_checkOperations:
 *
 * Ensure that the operationMode is valid for the configuration's field.
 *
 * Returns: FALSE if an invalid operation mode is used, otherwise TRUE.
 **/
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
       op != LIGHT_ADD && op != LIGHT_SUB && 
       op != LIGHT_SAVE && op != LIGHT_RESTORE)
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


/**
 * light_parseArguments:
 *
 * @argc	argument count
 * @argv	argument array
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_parseArguments(int argc, char** argv)
{
  int currFlag;
  int verbosity;

  LIGHT_BOOL opSet = FALSE;
  LIGHT_BOOL targetSet = FALSE;
  LIGHT_BOOL fieldSet = FALSE;
  LIGHT_BOOL ctrlSet = FALSE;
  LIGHT_BOOL valSet = FALSE;

  light_defaultConfig();

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
          return FALSE;
        }

        if(!light_validControllerName(optarg))
        {
          fprintf(stderr, "can't handle controller '%s'\n", optarg);
          return FALSE;
        }
        snprintf(light_Configuration.specifiedController, NAME_MAX + 1, "%s", optarg);
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

/**
 * light_printVersion:
 *
 * Prints version and copyright information to standard output.
 **/
void light_printVersion(){
  printf("Light %u.%u (%s)\n", LIGHT_VER_MAJOR, LIGHT_VER_MINOR, LIGHT_VER_TYPE);
  printf("Copyright (C) %u %s\n", LIGHT_YEAR, LIGHT_AUTHOR);
  printf("This is free software, see the source for copying conditions.  There is NO\n");
  printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n\n");
}

/**
 * light_printHelp:
 *
 * Prints help dialog to standard output.
 **/
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

/**
 * light_initialize:
 *
 * Initializes the configuration for the operation being requested.
 * Ensures the stored configuration directories exist, and that a
 * valid controller exists.
 *
 * Returns: TRUE on success, FALSE on failure
 **/
LIGHT_BOOL light_initialize()
{
  int mkdirVal;
  LIGHT_OP_MODE mode;

  mode = light_Configuration.operationMode;

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

/**
 * light_handleInfo:
 *
 * Print help and version info or list controllers,
 * according to the operationMode.
 *
 * Returns: TRUE if info was shown, otherwise FALSE
 **/
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

/**
 * light_listControllers:
 *
 * Prints controller names for the appropriate target.
 *
 * Returns: FALSE if could not list controllers or no
 * 		controllers found, otherwise TRUE
 **/
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

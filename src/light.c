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
  memset(&light_Configuration.specifiedController, '\0', 256);
  light_Configuration.operationMode          = LIGHT_GET;
  light_Configuration.valueMode              = LIGHT_PERCENT;
  light_Configuration.specifiedValueRaw      = 0;
  light_Configuration.specifiedValuePercent  = 0.0;
  light_Configuration.target                 = LIGHT_BRIGHTNESS;
  light_verbosity                            = 0;
}

LIGHT_BOOL light_parseArguments(int argc, char** argv)
{
  int currFlag;

  LIGHT_BOOL opSet = FALSE;
  LIGHT_BOOL targetSet = FALSE;
  LIGHT_BOOL ctrlSet = FALSE;
  LIGHT_BOOL valSet = FALSE;

  unsigned long specLen = 0;

  while((currFlag = getopt(argc, argv, "HhVGSAULIObmcas:prv:")) != -1)
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
      case 'b':
        ASSERT_TARGETSET();
        light_Configuration.target = LIGHT_BRIGHTNESS;
        break;
      case 'm':
        ASSERT_TARGETSET();
        light_Configuration.target = LIGHT_MAX_BRIGHTNESS;
        break;
      case 'c':
        ASSERT_TARGETSET();
        light_Configuration.target = LIGHT_MIN_CAP;
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
          printf("-s NEEDS an argument.\n\n");
          light_printHelp();
        }

        specLen = strlen(optarg);
        if(specLen > 255)
        {
          specLen = 255;
        }

        strncpy(light_Configuration.specifiedController, optarg, specLen);

        light_Configuration.specifiedController[255] = '\0';
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
          printf("-v NEEDS an argument.\n\n");
          light_printHelp();
          return FALSE;
        }
        if(sscanf(optarg, "%i", &light_verbosity) != 1)
        {
          printf("-v Verbosity is not specified in a recognizable format.\n\n");
          light_printHelp();
          return FALSE;
        }
        if(light_verbosity < 0 || light_verbosity > 3)
        {
          printf("-v Verbosity has to be between 0 and 3.\n\n");
          light_printHelp();
          return FALSE;
        }
        break;
    }
  }

  /* If we need a <value> (for writing), make sure we have it! */
  if(light_Configuration.operationMode == LIGHT_SET ||
     light_Configuration.operationMode == LIGHT_ADD ||
     light_Configuration.operationMode == LIGHT_SUB)
  {
    if(argc - optind != 1)
    {
      printf("Light needs an argument for <value>.\n\n");
      light_printHelp();
      return FALSE;
    }

    if(light_Configuration.valueMode == LIGHT_PERCENT)
    {
      if(sscanf(argv[optind], "%lf", &light_Configuration.specifiedValuePercent) != 1){
        printf("<value> is not specified in a recognizable format.\n\n");
        light_printHelp();
        return FALSE;
      }
      light_Configuration.specifiedValuePercent = LIGHT_CLAMP(light_Configuration.specifiedValuePercent, 0.00, 100.00);
    }else{
      if(sscanf(argv[optind], "%lu", &light_Configuration.specifiedValueRaw) != 1){
        printf("<value> is not specified in a recognizable format.\n\n");
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

  light_defaultConfig();
  if(!light_parseArguments(argc, argv))
  {
    LIGHT_ERR("could not parse arguments");
    return FALSE;
  }

  /* Just return true for operation modes that do not need initialization */
  if(light_Configuration.operationMode == LIGHT_PRINT_HELP || light_Configuration.operationMode == LIGHT_PRINT_VERSION || light_Configuration.operationMode == LIGHT_LIST_CTRL)
  {
      return TRUE;
  }

  /* Make sure we have a valid /etc/light directory, as well as mincap and save */
  mkdirVal = mkdir("/etc/light", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if(mkdirVal != 0 && errno != EEXIST)
  {
    LIGHT_ERR("/etc/light does not exist and could not be created");
    return FALSE;
  }

  mkdirVal = mkdir("/etc/light/mincap", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if(mkdirVal != 0 && errno != EEXIST)
  {
    LIGHT_ERR("/etc/light/mincap does not exist and could not be created");
    return FALSE;
  }

  mkdirVal = mkdir("/etc/light/save", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if(mkdirVal != 0 && errno != EEXIST)
  {
    LIGHT_ERR("/etc/light/save does not exist and could not be created");
    return FALSE;
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

  if(!light_controllerAccessible(light_Configuration.specifiedController))
  {
    LIGHT_ERR("selected controller is not valid, make sure this application is run as root.");
    return FALSE;
  }
  

  return TRUE;
}

LIGHT_BOOL light_execute()
{
  unsigned long rawCurr; /* The current brightness, in raw mode */
  double    percentCurr; /* The current brightness, in percent  */
  unsigned long  rawMax; /* The max brightness, in percent      */
  
  unsigned long  rawSetP; /* The final value to be set, in raw mode, when setting with percent */
  unsigned long  rawAddP; /* The final value to be set, in raw mode, when adding with percent */
  unsigned long  rawSubP; /* The final value to be set, in raw mode, when subtracting with percent */

  unsigned long  rawSetR; /* The final value to be set, in raw mode, when setting with raw values */
  unsigned long  rawAddR; /* The final value to be set, in raw mode, when adding with raw values */
  unsigned long  rawSubR; /* The final value to be set, in raw mode, when subtracting with raw values */

  unsigned long   minCap; /* The minimum cap, in raw mode */
  double   percentMinCap; /* The minimum cap, in percent */
  LIGHT_BOOL   hasMinCap; /* If we have a minimum cap     */
  unsigned long  minCapP; /* The final value to be set, in raw mode, when setting with percent (This is uncapped and used for minimum cap setting) */
  unsigned long  minCapR; /* The final value to be set, in raw mode, when setting with raw values (This is uncapped and used for minimum cap setting) */

  unsigned long  *writeVal; /* The actual value used for writing */

  /* Print help and version info */
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

  /* Prepare variables */

  /* -- First, get the current, min and max values directly from controller/configuration (raw values) */
  if(!light_getBrightness(light_Configuration.specifiedController, &rawCurr))
  {
    LIGHT_ERR("could not get brightness");
    return FALSE;
  }

  if(!light_getMaxBrightness(light_Configuration.specifiedController, &rawMax))
  {
    LIGHT_ERR("could not get max brightness");
    return FALSE;
  }

  if(!light_getMinCap(light_Configuration.specifiedController, &hasMinCap, &minCap))
  {
    LIGHT_ERR("could not get min brightness");
    return FALSE;
  }

  if( hasMinCap && ( minCap < 0 || minCap > rawMax ) )
  {
    LIGHT_WARN("invalid minimum cap for controller, ignoring and using 0");
    minCap = 0;
  }

  /* -- Secondly, calculate the rest of the values (Clamp them here as well!) */
  percentCurr = LIGHT_CLAMP( ((double)rawCurr) /  ((double)rawMax) * 100 , 0.00, 100.00 );
  percentMinCap = LIGHT_CLAMP( ((double)minCap) / ((double)rawMax) * 100 , 0.00, 100.00 );  

  rawSetP     = LIGHT_CLAMP( ((unsigned long) (light_Configuration.specifiedValuePercent * ((double)rawMax) ) / 100) , minCap, rawMax );
  rawAddP     = LIGHT_CLAMP( ((unsigned long) ( (percentCurr + light_Configuration.specifiedValuePercent) * ((double)rawMax)) / 100) , minCap, rawMax );

  if( light_Configuration.specifiedValuePercent > percentCurr)
  {
    rawSubP   = LIGHT_CLAMP(0, minCap, rawMax);
  }else{
    rawSubP     = LIGHT_CLAMP( ((unsigned long) ( (percentCurr - light_Configuration.specifiedValuePercent) * ((double)rawMax)) / 100) , minCap, rawMax );
  }

  rawSetR     = LIGHT_CLAMP( light_Configuration.specifiedValueRaw , minCap, rawMax );
  rawAddR     = LIGHT_CLAMP( rawCurr + light_Configuration.specifiedValueRaw , minCap, rawMax );
  
  if(light_Configuration.specifiedValueRaw > rawCurr){
    rawSubR     = LIGHT_CLAMP(0, minCap, rawMax)
  }else{
    rawSubR     = LIGHT_CLAMP( rawCurr - light_Configuration.specifiedValueRaw , minCap, rawMax );
  }

  minCapP     = LIGHT_CLAMP(((unsigned long) (light_Configuration.specifiedValuePercent * ((double)rawMax) ) / 100), 0, rawMax);
  minCapR     = LIGHT_CLAMP( light_Configuration.specifiedValueRaw, 0, rawMax );
  
  /* Handle get operations */
  if(light_Configuration.operationMode == LIGHT_GET)
  {
    switch(light_Configuration.target){
      case LIGHT_BRIGHTNESS:
        (light_Configuration.valueMode == LIGHT_RAW) ? printf("%lu\n", rawCurr) : printf("%.2f\n", percentCurr);
        break;
      case LIGHT_MAX_BRIGHTNESS:
        (light_Configuration.valueMode == LIGHT_RAW) ? printf("%lu\n", rawMax) : printf("100.00\n"); /* <- I know how stupid it is but it might just make someones life easier */
        break;
      case LIGHT_MIN_CAP:
        (light_Configuration.valueMode == LIGHT_RAW) ? printf("%lu\n", minCap) : printf("%.2f\n", percentMinCap);
        break;
      case LIGHT_SAVERESTORE:
        break;
    }

    return TRUE;
  }

  /* Handle set/add/sub operations */
  if(light_Configuration.operationMode == LIGHT_SET ||
     light_Configuration.operationMode == LIGHT_ADD || 
     light_Configuration.operationMode == LIGHT_SUB)
  {

    /* Set the pointer we will use for write to a fallback (that shouldn't change anything) */
    writeVal = &rawCurr;

    if(light_Configuration.target == LIGHT_MIN_CAP)
    {
      /* Handle minimum cap files */

      /* If we are not attempting to set, fail! */
      if(light_Configuration.operationMode != LIGHT_SET)
      {
        printf("Minimum cap can only be used with get/set operations.\n");
        return FALSE;
      }

      /* Point our writevalue and attempt to write*/
      writeVal = (light_Configuration.valueMode == LIGHT_RAW) ? &minCapR : &minCapP;
      if(!light_setMinCap(light_Configuration.specifiedController, *writeVal))
      {
        LIGHT_ERR("could not set minimum cap");
        return FALSE;
      }

      /* All good? Return true. */
      return TRUE;

    }else if(light_Configuration.target == LIGHT_BRIGHTNESS){
      /* Handle brightness writing */

      /* Point our writevalue according to configuration */
      switch(light_Configuration.operationMode)
      {
        case LIGHT_SET:
          writeVal = (light_Configuration.valueMode == LIGHT_RAW) ? &rawSetR : &rawSetP;
          break;
        case LIGHT_ADD:
          if(rawCurr == rawAddP && rawAddP != rawMax)
          {
            rawAddP+=1;
          }
          writeVal = (light_Configuration.valueMode == LIGHT_RAW) ? &rawAddR : &rawAddP;
          break;
        case LIGHT_SUB:
          writeVal = (light_Configuration.valueMode == LIGHT_RAW) ? &rawSubR : &rawSubP;
          break;
        case LIGHT_GET:
        case LIGHT_PRINT_HELP:
        case LIGHT_PRINT_VERSION:
        case LIGHT_LIST_CTRL:
        case LIGHT_SAVE:
        case LIGHT_RESTORE:
          break;
      }

      /* Attempt to write */
      if(!light_setBrightness(light_Configuration.specifiedController, *writeVal))
      {
        LIGHT_ERR("could not set brightness");
        return FALSE;
      }

      /* All good? return true. */
      return TRUE;

    }else{
      /* If we didn't provide a valid target for write operations, fail. */
      printf("set/add/subtract operations are only available for brightness and minimum cap files.\n");
      return FALSE;
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

  printf("Controller: %s\nValueRaw: %lu\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nTarget: %u\n\n", light_Configuration.specifiedController, light_Configuration.specifiedValueRaw, light_Configuration.specifiedValuePercent, light_Configuration.operationMode, light_Configuration.valueMode, light_Configuration.target);

  printf("You did not specify a valid combination of commandline arguments. Have some help: \n");
  light_printHelp();
  return FALSE;
}

void light_free()
{

}

LIGHT_BOOL light_genPath(char const *controller, LIGHT_TARGET type, char **buffer)
{
  char* returner = malloc(256);
  int spfVal = -1;

  if(returner == NULL)
  {
    LIGHT_MEMERR();
    buffer = NULL;
    return FALSE;
  }

  memset(returner, '\0', 256);
  
  switch(type)
  {
    case LIGHT_BRIGHTNESS:
      spfVal = sprintf(returner, "/sys/class/backlight/%s/brightness", controller);
      break;
    case LIGHT_MAX_BRIGHTNESS:
      spfVal = sprintf(returner, "/sys/class/backlight/%s/max_brightness", controller);
      break;
    case LIGHT_MIN_CAP:
      spfVal = sprintf(returner, "/etc/light/mincap/%s", controller);
      break;
    case LIGHT_SAVERESTORE:
      spfVal = sprintf(returner, "/etc/light/save/%s", controller);
      break;
  }

  if(spfVal < 0)
  {
    LIGHT_ERR("sprintf failed");
    free(returner);
    buffer = NULL;
    return FALSE;
  }

  *buffer = returner;

  return TRUE;
}

LIGHT_BOOL light_getBrightness(char const *controller, unsigned long *v)
{
  char *brightnessPath = NULL;
  LIGHT_BOOL readVal = FALSE;

  if(!light_genPath(controller, LIGHT_BRIGHTNESS, &brightnessPath))
  {
    LIGHT_ERR("could not generate path to brightness file");
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

LIGHT_BOOL light_getMaxBrightness(char const *controller, unsigned long *v)
{
  char *maxPath;
  LIGHT_BOOL readVal = FALSE;

  if(!light_genPath(controller, LIGHT_MAX_BRIGHTNESS, &maxPath))
  {
    LIGHT_ERR("could not generate path to maximum brightness file");
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

  if(!light_genPath(controller, LIGHT_BRIGHTNESS, &brightnessPath))
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }

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
  unsigned long dummy;

  if(!light_getBrightness(controller, &dummy))
  {
    LIGHT_WARN("could not read controllers brightness file, so controller is not accessible")
    return FALSE;
  }

  if(!light_getMaxBrightness(controller, &dummy))
  {
    LIGHT_WARN("could not read controller max brightness file, so controller is not accessible")
    return FALSE;
  }

  if(!light_genPath(controller, LIGHT_BRIGHTNESS, &brightnessPath))
  {
    LIGHT_ERR("could not generate path to brightness file");
    return FALSE;
  }

  if(!light_isWritable(brightnessPath))
  {
    LIGHT_WARN("could not open controller brightness file for writing, so controller is not accessible");
    free(brightnessPath);
    return FALSE;
  }

  free(brightnessPath);
  return TRUE;
}

LIGHT_BOOL light_iterateControllers()
{
  LIGHT_BOOL dotsKilled = FALSE;

  if(light_iteratorDir == NULL)
  {
    light_iteratorDir = opendir("/sys/class/backlight");
    if(light_iteratorDir == NULL)
    {
      LIGHT_ERR("could not open /sys/class/backlight directory");
      return FALSE;
    }
  }

  while(!dotsKilled)
  {
    light_iterator = readdir(light_iteratorDir);
    if(light_iterator == NULL)
    {
      if(light_iteratorDir != NULL)
      {
        closedir(light_iteratorDir);
        light_iteratorDir = NULL;
      }
      return FALSE;
    }

    if(light_iterator->d_name[0] != '.')
    {
      dotsKilled = TRUE;
    }
  }

  strcpy(light_currentController, light_iterator->d_name);

  return TRUE;
}

LIGHT_BOOL light_getBestController(char *controller)
{
  char bestYet[256];
  unsigned long bestValYet = 0;
  LIGHT_BOOL foundOkController = FALSE;

  memset(bestYet, '\0', 256);

  while(light_iterateControllers())
  {
    unsigned long currVal = 0;

    if(light_getMaxBrightness(light_currentController, &currVal))
    {
      
      if(light_controllerAccessible(light_currentController))
      {
        if(currVal > bestValYet)
        {
        foundOkController = TRUE;
        bestValYet = currVal;
        memset(bestYet, '\0', 256);
        strcpy(bestYet, light_currentController);
        }else{
          LIGHT_NOTE("ignoring controller as better one already found");
        }
      }else{
        LIGHT_WARN("controller not accessible");
      }
    }else{
      LIGHT_WARN("could not read max brightness from file");
    }
  }

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

  memset(controller, '\0', 256);
  strcpy(controller, bestYet);

  return TRUE;
}

LIGHT_BOOL light_getMinCap(char const * controller, LIGHT_BOOL * hasMinCap, unsigned long * minCap)
{
 char * mincapPath = NULL;
 
 if(!light_genPath(controller, LIGHT_MIN_CAP, &mincapPath))
 {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
 }

  if(!light_isReadable(mincapPath)){
    *hasMinCap = FALSE;
    *minCap = 0;
    free(mincapPath);
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
  if(!light_genPath(controller, LIGHT_MIN_CAP, &mincapPath))
  {
    LIGHT_ERR("could not generate path to minimum cap file");
    return FALSE;
  }

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
  LIGHT_BOOL foundController = FALSE;

  while(light_iterateControllers())
  {
    if(!foundController)
    {
      foundController = TRUE;
    }

    printf("%s\n", light_currentController);
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
  if(!light_genPath(controller, LIGHT_SAVERESTORE, &savePath))
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

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

  if(!light_genPath(controller, LIGHT_SAVERESTORE, &restorePath))
  {
    LIGHT_ERR("could not generate path to save/restore file");
    return FALSE;
  }

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

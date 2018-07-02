#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

LIGHT_BOOL light_readUInt(char const * filename, unsigned int *i)
{
  FILE* fileHandle;
  unsigned int iCopy;

  fileHandle = fopen(filename, "r");

  if(!fileHandle)
  {
    LIGHT_PERMERR("reading");
    return FALSE;
  }

  if(fscanf(fileHandle, "%u", &iCopy) != 1)
  {
    LIGHT_ERR_FMT("Couldn't parse a positive integer number from '%s'", filename);
    fclose(fileHandle);
    return FALSE;
  }
  
  *i = iCopy;

  fclose(fileHandle);
  return TRUE;
}

LIGHT_BOOL light_writeUInt(char const * filename, unsigned int i)
{
  FILE* fileHandle;

  fileHandle = fopen(filename, "w");

  if(!fileHandle)
  {
    LIGHT_PERMERR("writing");
    return FALSE;
  }

  if(fprintf(fileHandle, "%u", i) < 0)
  {
    LIGHT_ERR("fprintf failed");
    fclose(fileHandle);
    return FALSE;
  }

  fclose(fileHandle);
  return TRUE;
}


LIGHT_BOOL light_readULong(char const * filename, unsigned long *i)
{
  FILE* fileHandle;
  unsigned long iCopy;;

  fileHandle = fopen(filename, "r");

  if(!fileHandle)
  {
    LIGHT_PERMERR("reading");
    return FALSE;
  }

  if(fscanf(fileHandle, "%lu", &iCopy) != 1)
  {
    LIGHT_ERR_FMT("Couldn't parse a positive integer number from '%s'", filename);
    fclose(fileHandle);
    return FALSE;
  }
  
  *i = iCopy;

  fclose(fileHandle);
  return TRUE;
}

LIGHT_BOOL light_writeULong(char const * filename, unsigned long i)
{
  FILE* fileHandle;

  fileHandle = fopen(filename, "w");

  if(!fileHandle)
  {
    LIGHT_PERMERR("writing");
    return FALSE;
  }

  if(fprintf(fileHandle, "%lu", i) < 0)
  {
    LIGHT_ERR("fprintf failed");
    fclose(fileHandle);
    return FALSE;
  }

  fclose(fileHandle);
  return TRUE;
}

LIGHT_BOOL light_isWritable(char const * filename)
{
  FILE* fileHandle = fopen(filename, "r+");

  if(!fileHandle)
  {
    LIGHT_PERMWARN("writing");
    return FALSE;
  }

  fclose(fileHandle);
  return TRUE;
}

LIGHT_BOOL light_isReadable(char const * filename)
{
  FILE* fileHandle = fopen(filename, "r");

  if(!fileHandle)
  {
    LIGHT_PERMWARN("reading");
    return FALSE;
  }

  fclose(fileHandle);
  return TRUE;
}

unsigned long light_logInfClamp(unsigned long x)
{
  LIGHT_NOTE_FMT("specified value is inferior to %lu (raw), so adjusting it to this mininum value", x);
  return x;
}

unsigned long light_logSupClamp(unsigned long x)
{
  LIGHT_NOTE_FMT("specified value is superior to %lu (raw), so adjusting it to this maximum value", x);
  return x;
}

double light_clampPercent(double p)
{
  if(p < 0.0)
  {
    LIGHT_WARN_FMT("specified value %g%% is not valid, adjusting it to 0%%", p);
    return 0.0;
  }else if(p > 100.0){
    LIGHT_WARN_FMT("specified value %g%% is not valid, adjusting it to 100%%", p);
    return 100.0;
  }
  return p;
}

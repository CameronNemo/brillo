#include "light.h"

#include <stdio.h>

#define LIGHT_RETURNVAL_INITFAIL  2
#define LIGHT_RETURNVAL_EXECFAIL  1
#define LIGHT_RETURNVAL_SUCCESS   0

int main(int argc, char** argv)
{
  if(!light_parseArguments(argc, argv))
  {
    LIGHT_ERR("Arguments parsing failed");
    return LIGHT_RETURNVAL_INITFAIL;
  }

  if(light_handleInfo())
  {
    return LIGHT_RETURNVAL_SUCCESS;
  }

  if(!light_initialize())
  {
    LIGHT_ERR("Initialization failed");
    return LIGHT_RETURNVAL_INITFAIL;
  }

  if(!light_execute())
  {
    LIGHT_ERR("Execution failed");
    return LIGHT_RETURNVAL_EXECFAIL;
  }

  return LIGHT_RETURNVAL_SUCCESS;
}

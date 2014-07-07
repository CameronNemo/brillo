#include "light.h"

#include <stdio.h>

#define LIGHT_RETURNVAL_INITFAIL  2
#define LIGHT_RETURNVAL_EXECFAIL  1
#define LIGHT_RETURNVAL_SUCCESS   0

int main(int argc, char** argv)
{
  if(!light_initialize(argc, argv))
  {
    LIGHT_ERR("Initialization failed");
    return LIGHT_RETURNVAL_INITFAIL;
  }

  if(!light_execute())
  {
    LIGHT_ERR("Execution failed");
    light_free();
    return LIGHT_RETURNVAL_EXECFAIL;
  }

  light_free();
  return LIGHT_RETURNVAL_SUCCESS;
}

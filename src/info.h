#ifndef INFO_H
#define INFO_H

#include "light.h"

void info_print_version(void);
void info_print_help(void);
bool info_print(LIGHT_OP_MODE op, char *prefix, bool exec);

#endif /* INFO_H */

#ifndef EXEC_H
#define EXEC_H

#include "light.h"

bool light_execute(void);
char *light_path_new(const char *controller, LIGHT_FIELD type);
bool light_fetch(char const *controller, LIGHT_FIELD field, uint64_t *v);

#endif /* EXEC_H */

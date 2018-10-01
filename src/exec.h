#ifndef EXEC_H
#define EXEC_H

#include "light.h"

bool light_execute(light_conf_t *conf);
char *light_path_new(light_conf_t *conf, LIGHT_FIELD type);
bool light_fetch(light_conf_t *conf, LIGHT_FIELD field, uint64_t *v);

#endif /* EXEC_H */

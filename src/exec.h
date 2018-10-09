#ifndef EXEC_H
#define EXEC_H

#include "light.h"

bool exec_op(light_conf_t *conf);
char *light_path_new(light_conf_t *conf, LIGHT_FIELD type);
int64_t light_fetch(light_conf_t *conf, LIGHT_FIELD field);

#endif /* EXEC_H */

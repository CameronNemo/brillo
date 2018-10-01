#ifndef CTRL_H
#define CTRL_H

#include <dirent.h>

#include "light.h"

//bool ctrl_check(char const *controller);
char *ctrl_iter_next(DIR * dir);
bool ctrl_auto(light_conf_t *conf);

#endif /* CTRL_H */

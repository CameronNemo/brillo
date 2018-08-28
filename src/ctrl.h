#ifndef CTRL_H
#define CTRL_H

#include <dirent.h>

bool light_ctrl_check(char const *controller);
char *light_ctrl_iter_next(DIR * dir);
char *light_ctrl_auto();

#endif /* CTRL_H */

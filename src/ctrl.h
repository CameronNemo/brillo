/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef CTRL_H
#define CTRL_H

#include <dirent.h>

#include "light.h"

char *ctrl_iter_next(DIR * dir)
	__attribute__ ((warn_unused_result));
bool ctrl_auto(struct light_conf *conf)
	__attribute__ ((warn_unused_result));

#endif /* CTRL_H */

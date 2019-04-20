/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>

#include "light.h"

bool exec_op(struct light_conf *conf);
char *light_path_new(struct light_conf *conf, LIGHT_FIELD type)
	__attribute__ ((warn_unused_result));
int64_t light_fetch(struct light_conf *conf, LIGHT_FIELD field);

#endif /* EXEC_H */

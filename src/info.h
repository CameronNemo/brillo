/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef INFO_H
#define INFO_H

#include "light.h"

bool info_help(void);
bool info_print(LIGHT_OP_MODE op, char *prefix, bool exec);

#endif /* INFO_H */

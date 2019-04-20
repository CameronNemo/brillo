/* SPDX-License-Identifier: 0BSD */

#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

#include "light.h"

#define VALUE_PCT_MAX 10000

int64_t value_clamp(int64_t val, int64_t min, int64_t max);
int64_t value_from_raw(LIGHT_VAL_MODE mode, int64_t raw, int64_t max);
int64_t value_to_raw(LIGHT_VAL_MODE mode, int64_t val, int64_t max);
int64_t value_from_string(LIGHT_VAL_MODE mode, const char *str);

#define VALUE_CLAMP_PCT(val) value_clamp(val, 0, VALUE_PCT_MAX)

#endif /* VALUE_H */

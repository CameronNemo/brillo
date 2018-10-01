#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

#include "light.h"

#define VALUE_PCT_MAX 10000

uint64_t value_clamp(uint64_t val, uint64_t min, uint64_t max);
uint64_t value_from_raw(LIGHT_VAL_MODE mode, uint64_t raw, uint64_t max);
uint64_t value_to_raw(LIGHT_VAL_MODE mode, uint64_t val, uint64_t max);

#define VALUE_CLAMP_PCT(val) value_clamp(val, 0, VALUE_PCT_MAX)

#endif /* VALUE_H */

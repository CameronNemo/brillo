/* SPDX-License-Identifier: 0BSD */

#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include "value.h"
#include "vlog.h"

/**
 * value_clamp:
 * @val:	value to clamp
 * @min:	minimum value to use
 * @max:	maximum value to use
 *
 * Returns: clamped value
 **/
int64_t value_clamp(int64_t val, int64_t min, int64_t max)
{
	if (val < min) {
		vlog_notice("Raising value '%" PRId64 "' to '%" PRId64 "'", val, min);
		return min;
	} else if (val > max) {
		vlog_notice("Lowering value '%" PRId64 "' to '%" PRId64 "'", val, max);
		return max;
	} else {
		return val;
	}
}

/**
 * value_log_pct:
 * @raw:	raw value to convert
 * @max:	raw maximum value
 *
 * Converts a raw value to a logarithmic percentage.
 *
 * Returns: the percentage.
 **/
static int64_t value_log_pct(int64_t raw, int64_t max)
{
	return (int64_t) ((log((double) raw) / log((double) max)) * VALUE_PCT_MAX);
}

/**
 * value_from_raw:
 * @mode:	mode used to calculate value
 * @raw:	raw value to use in calculation
 * @max:	raw maximum value to use
 *
 * Calculates a linear or exponential percentage.
 *
 * Returns: the percentage, or a negative value on error
 **/
int64_t value_from_raw(LIGHT_VAL_MODE mode, int64_t raw, int64_t max)
{
	if (mode == LIGHT_RAW) {
		return raw;
	} else if (mode == LIGHT_PERCENT) {
		return VALUE_CLAMP_PCT((raw * VALUE_PCT_MAX) / max);
	} else if (mode == LIGHT_PERCENT_EXPONENTIAL) {
		return value_log_pct(raw, max);
	} else {
		return -1;
	}
}

/**
 * value_exp_raw:
 * @val:	logarithmic percentage to convert
 * @max:	raw maximum value
 *
 * Returns: a raw value from a logarithmic percentage.
 **/
static int64_t value_exp_raw(int64_t val, int64_t max)
{
	return (int64_t) (exp((double) val * log((double) max) / VALUE_PCT_MAX));
}

/**
 * value_to_raw:
 * @mode:	value mode used to calculate raw value
 * @val:	value to convert to raw value
 * @max:	raw maximum value to use
 *
 * Calculates a raw value based on the value mode.
 *
 * Returns: the raw value
 **/
int64_t value_to_raw(LIGHT_VAL_MODE mode, int64_t val, int64_t max)
{
	if (mode == LIGHT_RAW) {
		return val;
	} else if (mode == LIGHT_PERCENT) {
		return ((val * max) / VALUE_PCT_MAX);
	} else if (mode == LIGHT_PERCENT_EXPONENTIAL) {
		int64_t value = value_exp_raw(val, max);
		/* Protect against getting stuck in the sunken place --
		 * if the brightness is too low, future increments may not
		 * be enough to increase the percentage above zero. */
		if (value_from_raw(mode, value, max) == 0)
			return value_to_raw(mode, val + (VALUE_PCT_MAX / 200), max);
		else
			return value;
	} else {
		return -1;
	}
}

/**
 * value_from_string:
 * @mode:	mode to use to convert value
 * @str:	string that the value is stored in
 *
 * Returns: the value, or -1 on failure
 **/
int64_t value_from_string(LIGHT_VAL_MODE mode, const char* str)
{
	int ret;
	int64_t value = -1;
	double value_pct = -1;

	if (mode == LIGHT_RAW)
		ret = sscanf(str, "%" SCNd64, &value);
	else
		ret = sscanf(str, "%lf", &value_pct);

	if (ret != 1)
		return -1;

	if (mode == LIGHT_RAW)
		return value;
	else
		return VALUE_CLAMP_PCT((int64_t) (value_pct * (VALUE_PCT_MAX / 100)));
}

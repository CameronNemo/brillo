/* SPDX-License-Identifier: GPL-3.0-only */

#include <errno.h>

#include "common.h"

#include "burno.h"
#include "vlog.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "light.h"
#include "value.h"
#include "file.h"
#include "exec.h"

static int64_t exec_get_min(struct light_conf *conf);
static bool exec_write(struct light_conf *conf, LIGHT_FIELD field, int64_t val_old, int64_t val_new);
static bool exec_restore(struct light_conf *conf);

/**
 * exec_get_max:
 *
 * Determines max value from cache, or by fetching it.
 *
 * Returns: negative value on failure, raw max value on success
 **/
static int64_t exec_get_max(struct light_conf *conf)
{
	if (conf->cached_max != 0)
		return conf->cached_max;
	return light_fetch(conf, LIGHT_MAX_BRIGHTNESS);
}

/**
 * exec_open:
 * @conf:	configuration object
 * @field:	field to access
 * @flags:	flags to pass to open
 *
 * Opens a given field with the flags specified.
 *
 * Returns: an fd on success, negative value on failure
 **/
static int exec_open(struct light_conf *conf, LIGHT_FIELD field, int flags)
{
	burn_o char *path = light_path_new(conf, field);
	return path ? file_open(path, flags) : -1;
}

/**
 * exec_get:
 * @conf:	configuration object
 * @max:	maximum raw value
 *
 * Executes the get operation, printing the appropriate field to standard out.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_get(struct light_conf *conf)
{
	int64_t raw_val, val, max;

	if ((max = exec_get_max(conf)) < 0)
		return false;

	switch (conf->field) {
	case LIGHT_BRIGHTNESS:
		raw_val = light_fetch(conf, LIGHT_BRIGHTNESS);
		break;
	case LIGHT_MAX_BRIGHTNESS:
		raw_val = max;
		break;
	case LIGHT_MIN_CAP:
		raw_val = exec_get_min(conf);
		break;
	case LIGHT_SAVERESTORE:
		return true;
	default:
		return false;
	}

	if (raw_val < 0)
		return false;

	val = value_from_raw(conf->val_mode, raw_val, max);

	if (conf->val_mode == LIGHT_RAW)
		printf("%" PRId64 "\n", val);
	else
		printf("%.2f\n", ((double) val / 100.00));

	return true;
}

/**
 * exec_set:
 * @conf:	configuration object to operate on
 *
 * Sets the minimum cap or brightness value.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_set(struct light_conf *conf)
{
	int64_t new_value, curr_value, new_raw, max, curr_raw = -1, mincap = 0;
	burn_fd fd = exec_open(conf, conf->field, O_WRONLY);

	if (fd < 0)
		return false;

	if (conf->field == LIGHT_MIN_CAP)
		curr_raw = exec_get_min(conf);
	else
		mincap = exec_get_min(conf);

	if (conf->field != LIGHT_MIN_CAP)
		curr_raw = light_fetch(conf, conf->field);

	if (curr_raw < 0)
		return false;

	if ((max = exec_get_max(conf)) < 0)
		return false;

	new_value = conf->value;
	curr_value = value_from_raw(conf->val_mode, curr_raw, max);
	vlog_notice("specified value: %" PRId64, new_value);
	vlog_notice("current value: %" PRId64, curr_value);

	if (conf->field == LIGHT_BRIGHTNESS) {
		switch (conf->op_mode) {
		case LIGHT_SUB:
			/* val is unsigned so we need to get back to >= 0 */
			if (new_value > curr_value)
				new_value = -curr_value;
			else
				new_value = -new_value;
			/* FALLTHRU */
		case LIGHT_ADD:
			new_value += curr_value;
			break;
		case LIGHT_SET:
			break;
		default:
			return false;
		}
	} else if (conf->field != LIGHT_MIN_CAP) {
		return false;
	}

	new_raw = value_to_raw(conf->val_mode, new_value, max);

	/* Force any increment to result in some change, however small */
	if (conf->op_mode == LIGHT_ADD && new_raw <= curr_raw)
		new_raw += 1;

	new_raw = value_clamp(new_raw, mincap, max);

	return file_write(fd, curr_raw, new_raw, conf->usec);
}

/**
 * exec_all:
 * @conf:	configuration object to operate on
 *
 * Iterates through available controllers and executes
 * the requested operation for each one.
 *
 * Returns: true on success, false on failure
 **/
bool exec_all(struct light_conf *conf)
{
	bool ret = true;
	burn_dir dir = opendir(conf->sys_prefix);

	if (!dir) {
		vlog_err("opendir: %m");
		return false;
	}

	/* Change the controller mode so exec_op() does its thing */
	conf->ctrl_mode = LIGHT_CTRL_SPECIFY;

	while ((conf->ctrl = ctrl_iter_next(dir))) {
		if (conf->op_mode == LIGHT_GET)
			fprintf(stdout, "%s\t", conf->ctrl);
		if (!exec_op(conf))
			ret = false;
		free(conf->ctrl);
	}

	return ret;
}

/**
 * exec_save:
 * @conf:	configuration object
 *
 * Saves current value to the cache file.
 **/
static bool exec_save(struct light_conf *conf)
{
	int64_t curr = light_fetch(conf, LIGHT_BRIGHTNESS);
	if (curr < 0)
		return false;
	return exec_write(conf, LIGHT_SAVERESTORE, curr, curr);
}

/**
 * exec_op:
 * @conf:	configuration object to operate on
 *
 * Executes the requested operation.
 *
 * Returns: true on success, false on failure
 **/
bool exec_op(struct light_conf *conf)
{
	if (info_print(conf->op_mode, conf->sys_prefix, false))
		return info_print(conf->op_mode, conf->sys_prefix, true);

	if (conf->ctrl_mode == LIGHT_CTRL_ALL)
		return exec_all(conf);

	vlog_notice("executing light on '%s' controller", conf->ctrl);

	switch (conf->op_mode) {
	case LIGHT_SAVE:
		return exec_save(conf);
	case LIGHT_GET:
		return exec_get(conf);
	case LIGHT_RESTORE:
		return exec_restore(conf);
	case LIGHT_SET:
	case LIGHT_SUB:
	case LIGHT_ADD:
		return exec_set(conf);
	default:
		/* Should not be reached */
		fprintf(stderr,
			"Controller: %s\nValue: %" PRId64 "\nOpMode: %u\nValMode: %u\nField: %u\n\n",
			conf->ctrl, conf->value, conf->op_mode,
			conf->val_mode, conf->field);
		fprintf(stderr,
			"Invalid combination of commandline arguments.\n");
		return info_help();
	}
}

/**
 * light_path_new:
 * @conf:	configuration object to generate path from
 * @type:	field being accessed
 *
 * Generates a path in /sys or /etc for a given operation and
 * stores it in the string pointed to by buffer.
 *
 * WARNING: this function allocates memory, but does not free it.
 *          free the data pointed to by the return value after use.
 *
 * Returns: the generated path, or NULL on failure
 **/
char *light_path_new(struct light_conf *conf, LIGHT_FIELD type)
{
	char *p;
	const char *fmt, *prefix;

	if (!path_component(conf->ctrl))
		return NULL;

	if (type == LIGHT_BRIGHTNESS || type == LIGHT_MAX_BRIGHTNESS)
		prefix = conf->sys_prefix;
	else if (type == LIGHT_MIN_CAP || type == LIGHT_SAVERESTORE)
		prefix = conf->cache_prefix;
	else
		return NULL;

	switch (type) {
	case LIGHT_BRIGHTNESS:
		fmt = "%s/%s/brightness";
		break;
	case LIGHT_MAX_BRIGHTNESS:
		fmt = "%s/%s/max_brightness";
		break;
	case LIGHT_MIN_CAP:
		fmt = "%s.%s.mincap";
		break;
	case LIGHT_SAVERESTORE:
		fmt = "%s.%s.brightness";
		break;
	default:
		return NULL;
	}

	if (!(p = path_new()))
		return NULL;

	return path_append(p, fmt, prefix, conf->ctrl);
}

/**
 * light_fetch:
 * @conf:	configuration object to fetch from
 * @field:	field to fetch value from
 *
 * Fetches value from the appropriate path.
 *
 * Returns: value on success, -errno on failure
 **/
int64_t light_fetch(struct light_conf *conf, LIGHT_FIELD field)
{
	burn_o char *path = light_path_new(conf, field);
	return path ? file_read(path) : -ENOMEM;
}

/**
 * exec_write:
 * @conf:	configuration object to operate on
 * @field:	field to write value into
 * @val_old:	old value
 * @val_new:	new value
 *
 * Writes a value for a given controller and field.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_write(struct light_conf *conf, LIGHT_FIELD field,
		int64_t val_old, int64_t val_new)
{
	burn_fd fd = exec_open(conf, field, O_WRONLY);
	return fd > 0 ? file_write(fd, val_old, val_new, conf->usec) : false;
}

/**
 * exec_get_min:
 * @conf:	configuration object to operate on
 *
 * Returns: the mincap if it is available, otherwise 0
 **/
static int64_t exec_get_min(struct light_conf *conf)
{
	int64_t mincap = light_fetch(conf, LIGHT_MIN_CAP);
	if (mincap == -ENOENT)
		mincap = 1;
	if (mincap >= 0)
		return mincap;
	vlog_err("fetching mincap value: %m");
	return 0;
}

/**
 * exec_restore:
 * @conf:	configuration object to operate on
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_restore(struct light_conf *conf)
{
	int64_t val = light_fetch(conf, LIGHT_SAVERESTORE);

	conf->value = val;
	conf->val_mode = LIGHT_RAW;
	conf->op_mode = LIGHT_SET;

	return val >= 0 ? exec_set(conf) : false;
}

#include "common.h"
#include "path.h"
#include "info.h"
#include "light.h"
#include "exec.h"

static bool exec_fetch_mincap(light_conf_t *conf, uint64_t *mincap);
static bool exec_set_field(light_conf_t *conf, LIGHT_FIELD field, uint64_t v);
//static bool exec_set(uint64_t curr, uint64_t max, uint64_t mincap);
static bool exec_restore(light_conf_t *conf);
//static bool exec_init(uint64_t * curr, uint64_t * max, uint64_t * mincap);

/**
 * exec_init:
 *
 * Initializes values needed to execute the requested operation.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_init(light_conf_t *conf,
		uint64_t * curr, uint64_t * max, uint64_t * mincap)
{
	if (conf->cached_max != 0) {
		*max = conf->cached_max;
	} else if (!light_fetch(conf, LIGHT_MAX_BRIGHTNESS, max)) {
		LIGHT_ERR("could not get max brightness");
		return false;
	}

	/* No need to go further if targetting mincap */
	if ((conf->field == LIGHT_MIN_CAP &&
	     conf->op_mode == LIGHT_SET) ||
	    conf->field == LIGHT_MAX_BRIGHTNESS) {
		/* Init other values to 0 */
		*curr = *mincap = 0;
		return true;
	}

	if (!light_fetch(conf, LIGHT_BRIGHTNESS, curr)) {
		LIGHT_ERR("could not get brightness");
		return false;
	}

	if (!exec_fetch_mincap(conf, mincap)) {
		LIGHT_ERR("could not get mincap");
		return false;
	}

	if (*mincap > *max) {
		LIGHT_ERR("invalid mincap value of '%" SCNu64 "'", *mincap);
		LIGHT_ERR("mincap must be inferior to '%" SCNu64 "'", *max);
		return false;
	}

	return true;
}

/**
 * exec_get:
 * @field:	field to operate on
 * @mode:	value mode to use
 * @curr:	current raw value
 * @max:	maximum raw value
 * @mincap:	minimum raw value
 *
 * Executes the get operation, printing the appropriate field to standard out.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_get(LIGHT_FIELD field, LIGHT_VAL_MODE mode,
		uint64_t curr, uint64_t max, uint64_t mincap)
{
	uint64_t raw;
	double pct;

	if (max == 0)
		return false;

	switch (field) {
	case LIGHT_BRIGHTNESS:
		raw = curr;
		pct = light_clamp_pct(((double)curr) / ((double)max) * 100);
		break;
	case LIGHT_MAX_BRIGHTNESS:
		raw = max;
		pct = 100.00;
		break;
	case LIGHT_MIN_CAP:
		raw = mincap;
		pct = light_clamp_pct(((double)mincap) / ((double)max) * 100);
		break;
	case LIGHT_SAVERESTORE:
		return true;
	default:
		return false;
	}

	if (mode == LIGHT_RAW)
		printf("%" SCNu64 "\n", raw);
	else
		printf("%.2f\n", pct);

	return true;
}

/**
 * exec_set:
 * @conf:	configuration object to operate on
 * @curr:	current raw value
 * @max:	maximum raw value
 * @mincap:	minimum raw value
 *
 * Sets the minimum cap or brightness value.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_set(light_conf_t *conf,
		uint64_t curr, uint64_t max, uint64_t mincap)
{
	uint64_t val;

	if (conf->val_mode == LIGHT_RAW)
		val = conf->val_raw;
	else
		val = (uint64_t)((conf->val_pct * ((double)max)) / 100.0);

	if (conf->field == LIGHT_BRIGHTNESS) {
		switch (conf->op_mode) {
		case LIGHT_SUB:
			/* val is unsigned so we need to get back to >= 0 */
			if (val > curr)
				val = -curr;
			else
				val = -val;
			/* FALLTHRU */
		case LIGHT_ADD:
			val += curr;
			break;
		case LIGHT_SET:
			break;
		default:
			return false;
		}
	} else if (conf->field != LIGHT_MIN_CAP) {
		return false;
	}

	val = LIGHT_CLAMP(val, mincap, max);
	return exec_set_field (conf, conf->field, val);
}

/**
 * light_execute:
 * @conf:	configuration object to operate on
 *
 * Executes the requested operation.
 *
 * Returns: true on success, false on failure
 **/
bool light_execute(light_conf_t *conf)
{
	uint64_t curr;	/* The current brightness, in raw units */
	uint64_t max;	/* The max brightness, in raw units */
	uint64_t mincap;	/* The minimum cap, in raw units */

	if (info_print(conf->op_mode, conf->sys_prefix, false))
		return info_print(conf->op_mode, conf->sys_prefix, true);

	if (!exec_init(conf, &curr, &max, &mincap))
		return false;

	LIGHT_NOTE("executing light on '%s' controller", conf->ctrl);

	switch (conf->op_mode) {
	case LIGHT_GET:
		return exec_get(conf->field, conf->val_mode, curr, max, mincap);
	case LIGHT_SAVE:
		return exec_set_field(conf, LIGHT_SAVERESTORE, curr);
	case LIGHT_RESTORE:
		return exec_restore(conf);
	case LIGHT_SET:
	case LIGHT_SUB:
	case LIGHT_ADD:
		return exec_set(conf, curr, max, mincap);
	default:
		/* Should not be reached */
		fprintf(stderr,
			"Controller: %s\nValueRaw: %" SCNu64 "\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nField: %u\n\n",
			conf->ctrl, conf->val_raw, conf->val_pct,
			conf->op_mode, conf->val_mode, conf->field);
		fprintf(stderr,
			"Invalid combination of commandline arguments.\n");
		info_print_help();
		return false;
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
char *light_path_new(light_conf_t *conf, LIGHT_FIELD type)
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
 * @v:		pointer to store value in
 *
 * Fetches value from the appropriate path.
 *
 * Returns: true if value is successfully read, otherwise false
 **/
bool light_fetch(light_conf_t *conf, LIGHT_FIELD field, uint64_t *v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(conf, field)))
		return false;

	LIGHT_NOTE("fetching value from '%s'", path);

	r = light_read_val(path, v);
	free(path);
	return r;
}

/**
 * exec_set_field:
 * @conf:	configuration object to operate on
 * @field:	field to write value into
 * @v:		new value
 *
 * Sets a value for a given controller and field.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_set_field(light_conf_t *conf, LIGHT_FIELD field, uint64_t v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(conf, field)))
		return false;

	LIGHT_NOTE("writing value %" SCNu64 " (raw) to '%s'", v, path);

	r = light_write_val(path, v);
	free(path);

	if (!r) {
		LIGHT_ERR("error writing value to file");
	}

	return r;
}

/**
 * exec_fetch_mincap:
 * @conf:	configuration object to operate on
 * @mincap:	pointer to store the minimum cap value
 *
 * Returns: false if could not determine minimum cap, otherwise true
 **/
static bool exec_fetch_mincap(light_conf_t *conf, uint64_t *mincap)
{
	if (light_fetch(conf, LIGHT_MIN_CAP, mincap))
		return true;
	LIGHT_NOTE("can't access mincap file, assuming no min");
	*mincap = 0;
	return true;
}

/**
 * exec_restore:
 * @conf:	configuration object to operate on
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_restore(light_conf_t *conf)
{
	uint64_t v = 0;

	LIGHT_NOTE("restoring brightness from saved file");

	if (!light_fetch(conf, LIGHT_SAVERESTORE, &v)) {
		LIGHT_ERR("could not read saved value");
		return false;
	}

	if (!exec_set_field(conf, LIGHT_BRIGHTNESS, v)) {
		LIGHT_ERR("could not set restored brightness");
		return false;
	}

	return true;
}

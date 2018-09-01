#include "common.h"
#include "path.h"
#include "info.h"
#include "light.h"
#include "exec.h"

static bool exec_fetch_mincap(char const *controller, uint64_t *mincap);
static bool exec_set_field(char const *controller, LIGHT_FIELD field, uint64_t v);
static bool exec_set(uint64_t curr, uint64_t max, uint64_t mincap);
static bool exec_restore(char const *controller);
static bool exec_init(uint64_t * curr, uint64_t * max, uint64_t * mincap);

/**
 * exec_init:
 *
 * Initializes values needed to execute the requested operation.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_init(uint64_t * curr, uint64_t * max, uint64_t * mincap)
{
	if (light_conf.cached_max != 0) {
		*max = light_conf.cached_max;
	} else if (!light_fetch(light_conf.ctrl, LIGHT_MAX_BRIGHTNESS, max)) {
		LIGHT_ERR("could not get max brightness");
		return false;
	}

	/* No need to go further if targetting mincap */
	if ((light_conf.field == LIGHT_MIN_CAP &&
	     light_conf.op_mode == LIGHT_SET) ||
	    light_conf.field == LIGHT_MAX_BRIGHTNESS) {
		/* Init other values to 0 */
		*curr = *mincap = 0;
		return true;
	}

	if (!light_fetch(light_conf.ctrl, LIGHT_BRIGHTNESS, curr)) {
		LIGHT_ERR("could not get brightness");
		return false;
	}

	if (!exec_fetch_mincap(light_conf.ctrl, mincap)) {
		LIGHT_ERR("could not get mincap");
		return false;
	}

	if (*mincap > *max) {
		LIGHT_ERR("invalid mincap value of '%lu'", *mincap);
		LIGHT_ERR("mincap must be inferior to '%lu'", *max);
		return false;
	}

	return true;
}

/**
 * exec_get:
 * @curr:	current raw value
 * @max:	maximum raw value
 * @mincap:	minimum raw value
 *
 * Executes the get operation, printing the appropriate field to standard out.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_get(uint64_t curr, uint64_t max, uint64_t mincap)
{
	uint64_t raw;
	double pct;

	if (max == 0)
		return false;

	switch (light_conf.field) {
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

	if (light_conf.val_mode == LIGHT_RAW)
		printf("%lu\n", raw);
	else
		printf("%.2f\n", pct);

	return true;
}

/**
 * exec_set:
 * @curr:	current raw value
 * @max:	maximum raw value
 * @mincap:	minimum raw value
 *
 * Sets the minimum cap or brightness value.
 *
 * Returns: true on success, false on failure
 **/
static bool exec_set(uint64_t curr, uint64_t max, uint64_t mincap)
{
	uint64_t val;

	if (light_conf.val_mode == LIGHT_RAW)
		val = light_conf.val_raw;
	else
		val = (uint64_t)((light_conf.val_pct * ((double)max)) / 100.0);

	if (light_conf.field == LIGHT_BRIGHTNESS) {
		switch (light_conf.op_mode) {
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
	} else if (light_conf.field != LIGHT_MIN_CAP) {
		return false;
	}

	val = LIGHT_CLAMP(val, mincap, max);
	return exec_set_field (light_conf.ctrl, light_conf.field, val);
}

/**
 * light_execute:
 * Executes the requested operation.
 *
 * Returns: true on success, false on failure
 **/
bool light_execute()
{
	uint64_t curr;	/* The current brightness, in raw units */
	uint64_t max;	/* The max brightness, in raw units */
	uint64_t mincap;	/* The minimum cap, in raw units */

	if (info_print(false))
		return info_print(true);

	if (!exec_init(&curr, &max, &mincap))
		return false;

	LIGHT_NOTE("executing light on '%s' controller", light_conf.ctrl);

	switch (light_conf.op_mode) {
	case LIGHT_GET:
		return exec_get(curr, max, mincap);
	case LIGHT_SAVE:
		return exec_set_field(light_conf.ctrl, LIGHT_SAVERESTORE, curr);
	case LIGHT_RESTORE:
		return exec_restore(light_conf.ctrl);
	case LIGHT_SET:
	case LIGHT_SUB:
	case LIGHT_ADD:
		return exec_set(curr, max, mincap);
	default:
		/* Should not be reached */
		fprintf(stderr,
			"Controller: %s\nValueRaw: %lu\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nField: %u\n\n",
			light_conf.ctrl, light_conf.val_raw, light_conf.val_pct,
			light_conf.op_mode, light_conf.val_mode,
			light_conf.field);
		fprintf(stderr,
			"Invalid combination of commandline arguments.\n");
		info_print_help();
		return false;
	}
}

/**
 * light_path_new:
 * @controller:	name of controller device
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
char *light_path_new(const char *controller, LIGHT_FIELD type)
{
	char *p;
	const char *fmt, *prefix;

	if (!path_component(controller))
		return NULL;

	if (type == LIGHT_BRIGHTNESS || type == LIGHT_MAX_BRIGHTNESS)
		prefix = light_conf.sys_prefix;
	else if (type == LIGHT_MIN_CAP || type == LIGHT_SAVERESTORE)
		prefix = light_conf.cache_prefix;
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

	return path_append(p, fmt, prefix, controller);
}

/**
 * light_fetch:
 * @controller:	name of controller device
 * @field:	field to fetch value from
 * @v:		pointer to store value in
 *
 * Fetches value from the appropriate path.
 *
 * Returns: true if value is successfully read, otherwise false
 **/
bool light_fetch(char const *controller, LIGHT_FIELD field, uint64_t *v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(controller, field)))
		return false;

	LIGHT_NOTE("fetching value from '%s'", path);

	r = light_read_val(path, v);
	free(path);
	return r;
}

/**
 * exec_set_field:
 * @controller: name of controller device
 * @field:	field to write value into
 * @v:		new value
 *
 * Sets a value for a given controller and field.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_set_field(char const *controller, LIGHT_FIELD field, uint64_t v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(controller, field)))
		return false;

	LIGHT_NOTE("writing value %lu (raw) to '%s'", v, path);

	r = light_write_val(path, v);
	free(path);

	if (!r) {
		LIGHT_ERR("error writing value to file");
	}

	return r;
}

/**
 * exec_fetch_mincap:
 * @controller:	name of controller device
 * @mincap:	pointer to store the minimum cap value
 *
 * Returns: false if could not determine minimum cap, otherwise true
 **/
static bool exec_fetch_mincap(char const *controller, uint64_t *mincap)
{
	if (light_fetch(controller, LIGHT_MIN_CAP, mincap))
		return true;
	LIGHT_NOTE("can't access mincap file, assuming no min");
	*mincap = 0;
	return true;
}

/**
 * exec_restore:
 * @controller: name of controller device
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
static bool exec_restore(char const *controller)
{
	uint64_t v = 0;

	LIGHT_NOTE("restoring brightness from saved file");

	if (!light_fetch(controller, LIGHT_SAVERESTORE, &v)) {
		LIGHT_ERR("could not read saved value");
		return false;
	}

	if (!exec_set_field(controller, LIGHT_BRIGHTNESS, v)) {
		LIGHT_ERR("could not set restored brightness");
		return false;
	}

	return true;
}

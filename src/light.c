#include "light.h"

#include <string.h>
#include <errno.h>

/**
 * light_initExecution:
 *
 * Initializes values needed to execute the requested operation.
 *
 * Returns: true on success, false on failure
 **/
bool light_initExecution(unsigned long *rawCurr, unsigned long *rawMax,
			       unsigned long *minCap)
{
	if (light_conf.cached_max != 0) {
		*rawMax = light_conf.cached_max;
	} else if (!light_fetch(light_conf.ctrl, LIGHT_MAX_BRIGHTNESS, rawMax)) {
		LIGHT_ERR("could not get max brightness");
		return false;
	}

	/* No need to go further if targetting mincap */
	if (light_conf.field == LIGHT_MIN_CAP ||
	    light_conf.field == LIGHT_MAX_BRIGHTNESS) {
		/* Init other values to 0 */
		*rawCurr = *minCap = 0;
		return true;
	}

	if (!light_fetch(light_conf.ctrl, LIGHT_BRIGHTNESS, rawCurr)) {
		LIGHT_ERR("could not get brightness");
		return false;
	}

	if (!light_fetch_mincap(light_conf.ctrl, minCap)) {
		LIGHT_ERR("could not get mincap");
		return false;
	}

	if (*minCap > *rawMax) {
		LIGHT_ERR("invalid mincap value of '%lu'", *minCap);
		LIGHT_ERR("mincap must be inferior to '%lu'", *rawMax);
		return false;
	}

	return true;
}

/**
 * light_executeGet:
 * @rawCurr:	current raw value
 * @rawMax:	maximum raw value
 * @minCap:	minimum raw value
 *
 * Executes the get operation, printing the appropriate field to standard out.
 *
 * Returns: true on success, false on failure
 **/
bool light_executeGet(unsigned long rawCurr, unsigned long rawMax,
			    unsigned long minCap)
{
	unsigned long raw;
	double pct;

	if (rawMax == 0)
		return false;

	switch (light_conf.field) {
	case LIGHT_BRIGHTNESS:
		raw = rawCurr;
		pct =
		    light_clampPercent(((double)rawCurr) / ((double)rawMax) *
				       100);
		break;
	case LIGHT_MAX_BRIGHTNESS:
		raw = rawMax;
		pct = 100.00;
		break;
	case LIGHT_MIN_CAP:
		raw = minCap;
		pct =
		    light_clampPercent(((double)minCap) / ((double)rawMax) *
				       100);
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
 * light_executeSet:
 * @rawCurr:    current raw value
 * @rawMax:     maximum raw value
 * @minCap:     minimum raw value
 *
 * Sets the minimum cap or brightness value.
 *
 * Returns: true on success, false on failure
 **/
bool light_executeSet(unsigned long rawCurr, unsigned long rawMax,
			    unsigned long minCap)
{
	unsigned long val;

	if (light_conf.val_mode == LIGHT_RAW)
		val = light_conf.val_raw;
	else
		val =
		    (unsigned long)((light_conf.val_pct * ((double)rawMax)) /
				    100.0);

	/* set the minimum cap */
	if (light_conf.field == LIGHT_MIN_CAP) {
		if (light_setMinCap
		    (light_conf.ctrl, LIGHT_CLAMP(val, 0, rawMax)))
			return true;
		LIGHT_ERR("could not set minimum cap");
		return false;
	}

	/* set the brightness */

	if (light_conf.field != LIGHT_BRIGHTNESS)
		return false;

	switch (light_conf.op_mode) {
	case LIGHT_SUB:
		/* val is unsigned so we need to get back to >= 0 */
		if (val > rawCurr)
			val = -rawCurr;
		else
			val = -val;
	case LIGHT_ADD:
		val += rawCurr;
	case LIGHT_SET:
		break;
	default:
		return false;
	}

	if (light_setBrightness
	    (light_conf.ctrl, LIGHT_CLAMP(val, minCap, rawMax)))
		return true;

	LIGHT_ERR("could not set brightness");
	return false;
}

/**
 * light_execute:
 * Executes the requested operation.
 *
 * Returns: true on success, false on failure
 **/
bool light_execute()
{
	unsigned long rawCurr;	/* The current brightness, in raw units */
	unsigned long rawMax;	/* The max brightness, in raw units */
	unsigned long minCap;	/* The minimum cap, in raw units */

	if (!light_initExecution(&rawCurr, &rawMax, &minCap))
		return false;

	LIGHT_NOTE("executing light on '%s' controller", light_conf.ctrl);

	switch (light_conf.op_mode) {
	case LIGHT_GET:
		return light_executeGet(rawCurr, rawMax, minCap);
	case LIGHT_SAVE:
		if (light_saveBrightness(light_conf.ctrl, rawCurr))
			return true;
		LIGHT_ERR("could not save brightness");
		return false;
	case LIGHT_RESTORE:
		if (light_restoreBrightness(light_conf.ctrl))
			return true;
		LIGHT_ERR("could not restore brightness");
		return false;
	case LIGHT_SET:
	case LIGHT_SUB:
	case LIGHT_ADD:
		return light_executeSet(rawCurr, rawMax, minCap);
		/* Should not be reached */
	default:
		fprintf(stderr,
			"Controller: %s\nValueRaw: %lu\nValuePercent: %.2f\nOpMode: %u\nValMode: %u\nField: %u\n\n",
			light_conf.ctrl, light_conf.val_raw, light_conf.val_pct,
			light_conf.op_mode, light_conf.val_mode,
			light_conf.field);
		fprintf(stderr,
			"Invalid combination of commandline arguments.\n");
		light_print_help();
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
	char *path_new, *path_fmt, *path_prefix, *subsystem;
	int r;

	if (!controller || NAME_MAX < strnlen(controller, NAME_MAX + 1)) {
		LIGHT_ERR("invalid controller '%s', couldn't generate path",
			  controller);
		return NULL;
	}

	if (type == LIGHT_BRIGHTNESS || type == LIGHT_MAX_BRIGHTNESS)
		path_prefix = "/sys/class";
	else if (type == LIGHT_MIN_CAP || type == LIGHT_SAVERESTORE)
		// TODO: per-user storage dir
		path_prefix = "/var/cache/" LIGHT_PROG;
	else
		return NULL;

	if (light_conf.target == LIGHT_BACKLIGHT)
		subsystem = "backlight";
	else
		subsystem = "leds";

	switch (type) {
	case LIGHT_BRIGHTNESS:
		path_fmt = "%s/%s/%s/brightness";
		break;
	case LIGHT_MAX_BRIGHTNESS:
		path_fmt = "%s/%s/%s/max_brightness";
		break;
	case LIGHT_MIN_CAP:
		path_fmt = "%s/%s.%s.mincap";
		break;
	case LIGHT_SAVERESTORE:
		path_fmt = "%s/%s.%s.brightness";
		break;
	default:
		return NULL;
	}

	if (!(path_new = malloc(PATH_MAX))) {
		LIGHT_MEMERR();
		return NULL;
	}

	r = snprintf(path_new, PATH_MAX, path_fmt, path_prefix, subsystem,
		     controller);

	if (r < 0 || r >= PATH_MAX || path_new == NULL) {
		LIGHT_ERR("failed to copy generated path into buffer");
		if (path_new)
			free(path_new);
		return NULL;
	}

	return path_new;
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
bool light_fetch(char const *controller, LIGHT_FIELD f, unsigned long *v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(controller, f)))
		return false;

	LIGHT_NOTE("fetching value from '%s'", path);

	r = light_read_val(path, v);
	free(path);

	if (!r)
		return false;
	else
		return true;
}

/**
 * light_setBrightness:
 * @controller: name of controller device
 * @v:		new brightness value
 *
 * Sets the brightness for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
bool light_setBrightness(char const *controller, unsigned long v)
{
	char *path;
	bool r;

	if (!(path = light_path_new(controller, light_conf.field))) {
		LIGHT_ERR("could not generate path to brightness file");
		return false;
	}

	LIGHT_NOTE("setting brightness %lu (raw) to controller", v);
	r = light_write_val(path, v);

	if (!r) {
		LIGHT_ERR("could not write value to brightness file");
	}

	free(path);
	return r;
}

/**
 * light_ctrl_check:
 * @controller:	name of controller to check
 *
 * Returns: true if controller is accessible, otherwise false
 **/
bool light_ctrl_check(char const *controller)
{
	char *path = NULL;

	/* On auto mode, we need to check if we can read the max brightness value
	   of the controller for later computation */
	if (light_conf.ctrl_mode == LIGHT_AUTO ||
	    light_conf.field == LIGHT_MAX_BRIGHTNESS) {
		if (!(path = light_path_new(controller, LIGHT_MAX_BRIGHTNESS))) {
			return false;
		}
		if (!light_test_r(path)) {
			LIGHT_WARN ("controller not accessible: max_brightness not readable");
			free(path);
			return false;
		}
		free(path);
	}

	if (!(path = light_path_new(controller, LIGHT_BRIGHTNESS)))
		return false;

	if (light_conf.op_mode != LIGHT_GET &&
	    light_conf.op_mode != LIGHT_SAVE &&
	    light_conf.field != LIGHT_MIN_CAP &&
	    !light_test_w(path)) {
		LIGHT_WARN ("controller not accessible: brightness not writeable");
		free(path);
		return false;
	} else if (!light_test_r(path)) {
		LIGHT_WARN ("controller not accessible: brightness not readable");
		free(path);
		return false;
	}

	free(path);
	return true;
}

/**
 * light_ctrl_iter_new:
 *
 * Opens the appropriate directory for a target.
 *
 * WARNING: returns an opened directory, which
 *          should be closed after use
 *
 * Returns: directory, or NULL on failure
 **/
DIR *light_ctrl_iter_new()
{
	DIR *dir;

	if (light_conf.target == LIGHT_KEYBOARD)
		dir = opendir("/sys/class/leds");
	else
		dir = opendir("/sys/class/backlight");

	if (!dir)
		LIGHT_ERR ("could not open directory in /sys");

	return dir;
}

/**
 * light_ctrl_iter_next:
 * @dir:	opened directory to iterate over
 *
 * Iterates over the directory given by dir.
 *
 * WARNING: will allocate a string and return it,
 *          this string should be freed after use
 *
 * Returns: name of the next controller, NULL on end of dir or failure
 **/
char *light_ctrl_iter_next(DIR * dir)
{
	struct dirent *file;

	if (!dir) {
		LIGHT_ERR("directory uninitialized");
		return NULL;
	}

	while ((file = readdir(dir))) {
		if (!(file->d_name) || file->d_name[0] == '.')
			continue;

		if (NAME_MAX < strnlen(file->d_name, NAME_MAX + 1)) {
			LIGHT_WARN("invalid controller found, continuing...");
			continue;
		}

		return strndup(file->d_name, NAME_MAX);
	}

	return NULL;
}

/**
 * light_ctrl_auto:
 *
 * Iterates over the appropriate directory and finds the
 * controller with the highest max brightness.
 *
 * WARNING: will return an allocated string, which
 *          should be freed after use
 *
 * Returns: best controller, or NULL if no suitable controller is found
 **/
char *light_ctrl_auto()
{
	DIR *dir;
	char *best, *next;

	best = NULL;

	LIGHT_NOTE("finding best controller...");

	if (!(dir = light_ctrl_iter_new()))
		return NULL;

	while ((next = light_ctrl_iter_next(dir))) {
		unsigned long max = 0;

		if (light_ctrl_check(next)
		    && light_fetch(next, LIGHT_MAX_BRIGHTNESS, &max)) {
			if (max > light_conf.cached_max) {
				light_conf.cached_max = max;
				if (best)
					free(best);
				best = next;
				LIGHT_NOTE
				    ("using controller '%s', it is an improvement",
				     best);
				continue;
			}
			LIGHT_NOTE
			    ("ignoring controller '%s', it is not an improvement",
			     next);
		} else {
			LIGHT_WARN("ignoring controller '%s', not accessible",
				   next);
		}

		free(next);
	}

	closedir(dir);

	if (!best)
		LIGHT_ERR("could not find an accessible controller");

	return best;
}

/**
 * light_fetch_mincap:
 * @controller:	name of controller device
 * @mincap:	pointer to store the minimum cap value
 *
 * Returns: false if could not determine minimum cap, otherwise true
 **/
bool light_fetch_mincap(char const *controller, unsigned long *mincap)
{
	errno = 0;
	if (light_fetch(controller, LIGHT_MIN_CAP, mincap)) {
		return true;
	} else if (errno == EACCES) {
		LIGHT_NOTE("can't access mincap file, assuming no min");
		*mincap = 0;
		return true;
	} else {
		LIGHT_ERR("error reading from mincap file");
		return false;
	}
}

/**
 * light_setMinCap:
 * @controller: name of controller device
 * @v:          new minimum cap value
 *
 * Sets the minimum cap for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
bool light_setMinCap(char const *controller, unsigned long v)
{
	char *path;

	if (!(path = light_path_new(controller, LIGHT_MIN_CAP))) {
		LIGHT_ERR("could not generate path to minimum cap file");
		return false;
	}

	LIGHT_NOTE("setting minimum cap to %lu (raw)", v);
	if (!light_write_val(path, v)) {
		LIGHT_ERR("could not write to minimum cap file");
		free(path);
		return false;
	}

	free(path);
	return true;
}

/**
 * light_saveBrightness:
 * @controller: name of controller device
 * @v:          brightness value to save
 *
 * Saves the brightness value for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
bool light_saveBrightness(char const *controller, unsigned long v)
{
	char *path;

	if (!(path = light_path_new(controller, LIGHT_SAVERESTORE))) {
		LIGHT_ERR("could not generate path to save/restore file");
		return false;
	}

	LIGHT_NOTE("saving brightness %lu (raw) to save file", v);
	if (!light_write_val(path, v)) {
		LIGHT_ERR("could not write to save/restore file");
		free(path);
		return false;
	}

	free(path);
	return true;
}

/**
 * light_restoreBrightness:
 * @controller: name of controller device
 *
 * Restores the brightness value for a given controller.
 *
 * Returns: true if write was successful, otherwise false
 **/
bool light_restoreBrightness(char const *controller)
{
	unsigned long v = 0;

	LIGHT_NOTE("restoring brightness from saved file");

	if (!light_fetch(controller, LIGHT_SAVERESTORE, &v)) {
		LIGHT_ERR("could not read saved value");
		return false;
	}

	if (!light_setBrightness(controller, v)) {
		LIGHT_ERR("could not set restored brightness");
		return false;
	}

	return true;
}

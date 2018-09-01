#include <getopt.h>

#include "common.h"
#include "helpers.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "light.h"

#define ASSERT_SET(t,v) \
  if(v)\
  {\
    fprintf(stderr, t" arguments can not be used in conjunction.\n");\
    return false;\
  }\
  v = true;

#define ASSERT_OPSET() ASSERT_SET("Operation", opSet)
#define ASSERT_TARGETSET() ASSERT_SET("Target", targetSet)
#define ASSERT_FIELDSET() ASSERT_SET("Field", fieldSet)
#define ASSERT_CTRLSET() ASSERT_SET("Controller", ctrlSet)
#define ASSERT_VALSET() ASSERT_SET("Value", valSet)

/**
 * light_defaults:
 *
 * Initialize the default configuration values.
 **/
static void light_defaults()
{
	light_conf.ctrl_mode = LIGHT_AUTO;
	light_conf.ctrl = NULL;
	light_conf.op_mode = LIGHT_GET;
	light_conf.val_mode = LIGHT_PERCENT;
	light_conf.val_raw = 0;
	light_conf.val_pct = 0.0;
	light_conf.target = LIGHT_BACKLIGHT;
	light_conf.field = LIGHT_BRIGHTNESS;
	light_conf.cached_max = 0;
	light_loglevel = 0;
}

/**
 * light_check_ops:
 *
 * Ensure that the op_mode is valid for the configuration's field.
 *
 * Returns: false if an invalid operation mode is used, otherwise true.
 **/
static bool light_check_ops()
{
	LIGHT_OP_MODE op = light_conf.op_mode;

	/* Nothing to check if we just print info */
	if (op == LIGHT_PRINT_HELP ||
	    op == LIGHT_PRINT_VERSION ||
	    op == LIGHT_LIST_CTRL)
		return true;

	switch (light_conf.field) {
	case LIGHT_MAX_BRIGHTNESS:
		if (op == LIGHT_GET)
			return true;
		fprintf(stderr, "You can only use -G with the brightness field.\n\n");
		return false;
	case LIGHT_MIN_CAP:
		if (op == LIGHT_GET || op == LIGHT_SET)
			return true;
		fprintf(stderr, "You can only use -G or -S with the min cap field.\n\n");
		return false;
	default:
		return true;
	}
}

/**
 * light_parse_args:
 * @argc	argument count
 * @argv	argument array
 *
 * WARNING: may allocate a string in light_conf.ctrl,
 *          but will not free it
 *
 * Returns: true on success, false on failure
 **/
bool light_parse_args(int argc, char **argv)
{
	int opt;
	int verbosity;

	char *value = NULL;

	bool opSet = false;
	bool targetSet = false;
	bool fieldSet = false;
	bool ctrlSet = false;
	bool valSet = false;

	light_defaults();

	while ((opt = getopt(argc, argv, "HhVGS:A:U:LIObmclkas:prv:")) != -1) {
		switch (opt) {
			/* -- Operations -- */
		case 'H':
		case 'h':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_PRINT_HELP;
			break;
		case 'V':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_PRINT_VERSION;
			break;
		case 'G':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_GET;
			break;
		case 'S':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_SET;
			value = optarg;
			break;
		case 'A':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_ADD;
			value = optarg;
			break;
		case 'U':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_SUB;
			value = optarg;
			break;
		case 'L':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_LIST_CTRL;
			break;
		case 'I':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_RESTORE;
			break;
		case 'O':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_SAVE;
			break;

			/* -- Targets -- */
		case 'l':
			ASSERT_TARGETSET();
			light_conf.target = LIGHT_BACKLIGHT;
			break;
		case 'k':
			ASSERT_TARGETSET();
			light_conf.target = LIGHT_KEYBOARD;
			break;

			/* -- Fields -- */
		case 'b':
			ASSERT_FIELDSET();
			light_conf.field = LIGHT_BRIGHTNESS;
			break;
		case 'm':
			ASSERT_FIELDSET();
			light_conf.field = LIGHT_MAX_BRIGHTNESS;
			break;
		case 'c':
			ASSERT_FIELDSET();
			light_conf.field = LIGHT_MIN_CAP;
			break;

			/* -- Controller selection -- */
		case 'a':
			ASSERT_CTRLSET();
			light_conf.ctrl_mode = LIGHT_AUTO;
			break;;
		case 's':
			ASSERT_CTRLSET();
			light_conf.ctrl_mode = LIGHT_SPECIFY;

			if (!path_component(optarg)) {
				fprintf(stderr,
					"can't handle controller '%s'\n",
					optarg);
				return false;
			}
			light_conf.ctrl = strdup(optarg);
			break;
			/* -- Value modes -- */
		case 'p':
			ASSERT_VALSET();
			light_conf.val_mode = LIGHT_PERCENT;
			break;
		case 'r':
			ASSERT_VALSET();
			light_conf.val_mode = LIGHT_RAW;
			break;

			/* -- Other -- */
		case 'v':
			if (sscanf(optarg, "%i", &verbosity) != 1) {
				fprintf(stderr,
					"Verbosity not specified in a recognizable format.\n\n");
				info_print_help();
				return false;
			}
			if (verbosity < 0 || verbosity > 3) {
				fprintf(stderr,
					"Verbosity has to be between 0 and 3.\n\n");
				info_print_help();
				return false;
			}
			light_loglevel = (light_loglevel_t) verbosity;
			break;
		default:
			info_print_help();
			return false;
		}
	}

	if (!light_check_ops()) {
		info_print_help();
		return false;
	}

	/* Parse <value> (for set/add/subtract operations) */
	if (value) {
		int r;
		if (light_conf.val_mode == LIGHT_PERCENT)
			r = sscanf(value, "%lf", &light_conf.val_pct);
		else
			r = sscanf(value, "%lu", &light_conf.val_raw);

		if (r != 1) {
			LIGHT_ERR("<value> not specified in a recognizable format");
			info_print_help();
			return false;
		}

		if (light_conf.val_mode == LIGHT_PERCENT)
			light_conf.val_pct = light_clamp_pct(light_conf.val_pct);
	}

	return true;
}

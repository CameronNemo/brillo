#include <getopt.h>

#include "common.h"
#include "helpers.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "value.h"
#include "light.h"

#define ASSERT_SET(t,v) \
  if(v)\
  {\
    fprintf(stderr, t" arguments can not be used in conjunction.\n");\
    goto error;\
  }\
  v = true;

#define ASSERT_OPSET() ASSERT_SET("Operation", opSet)
#define ASSERT_TARGETSET() ASSERT_SET("Target", targetSet)
#define ASSERT_FIELDSET() ASSERT_SET("Field", fieldSet)
#define ASSERT_CTRLSET() ASSERT_SET("Controller", ctrlSet)
#define ASSERT_VALSET() ASSERT_SET("Value", valSet)

/**
 * parse_check:
 * @op:		operation being done
 * @field:	field being accessed
 *
 * Ensure that the operation is valid for the field.
 *
 * Returns: false if an invalid operation mode is used, otherwise true.
 **/
static bool parse_check(LIGHT_OP_MODE op, LIGHT_FIELD field)
{
	/* Nothing to check if we just print info */
	if (op == LIGHT_PRINT_HELP ||
	    op == LIGHT_PRINT_VERSION ||
	    op == LIGHT_LIST_CTRL)
		return true;

	switch (field) {
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
 * parse_args:
 * @argc	argument count
 * @argv	argument array
 *
 * WARNING: may allocate a string in light_conf->ctrl,
 *          but will not free it
 *
 * Returns: a valid conf object on success, NULL on failure
 **/
light_conf_t *parse_args(int argc, char **argv)
{
	int opt;
	int verbosity = 0;
	light_conf_t *light_conf = NULL;
	char *value = NULL;

	if (!(light_conf = light_new()))
		return NULL;

	bool opSet = false;
	bool targetSet = false;
	bool fieldSet = false;
	bool ctrlSet = false;
	bool valSet = false;

	while ((opt = getopt(argc, argv, "HhVGS:A:U:LIObmclkas:pqrv:")) != -1) {
		switch (opt) {
			/* -- Operations -- */
		case 'H':
		case 'h':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_PRINT_HELP;
			break;
		case 'V':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_PRINT_VERSION;
			break;
		case 'G':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_GET;
			break;
		case 'S':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_SET;
			value = optarg;
			break;
		case 'A':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_ADD;
			value = optarg;
			break;
		case 'U':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_SUB;
			value = optarg;
			break;
		case 'L':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_LIST_CTRL;
			break;
		case 'I':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_RESTORE;
			break;
		case 'O':
			ASSERT_OPSET();
			light_conf->op_mode = LIGHT_SAVE;
			break;

			/* -- Targets -- */
		case 'l':
			ASSERT_TARGETSET();
			light_conf->target = LIGHT_BACKLIGHT;
			break;
		case 'k':
			ASSERT_TARGETSET();
			light_conf->target = LIGHT_KEYBOARD;
			break;

			/* -- Fields -- */
		case 'b':
			ASSERT_FIELDSET();
			light_conf->field = LIGHT_BRIGHTNESS;
			break;
		case 'm':
			ASSERT_FIELDSET();
			light_conf->field = LIGHT_MAX_BRIGHTNESS;
			break;
		case 'c':
			ASSERT_FIELDSET();
			light_conf->field = LIGHT_MIN_CAP;
			break;

			/* -- Controller selection -- */
		case 'a':
			ASSERT_CTRLSET();
			break;
		case 's':
			ASSERT_CTRLSET();
			if (!path_component(optarg)) {
				fprintf(stderr,
					"can't handle controller '%s'\n",
					optarg);
				light_free(light_conf);
				return NULL;
			}
			light_conf->ctrl = strdup(optarg);
			break;
			/* -- Value modes -- */
		case 'p':
			ASSERT_VALSET();
			light_conf->val_mode = LIGHT_PERCENT;
			break;
		case 'q':
			ASSERT_VALSET();
			light_conf->val_mode = LIGHT_PERCENT_EXPONENTIAL;
			break;
		case 'r':
			ASSERT_VALSET();
			light_conf->val_mode = LIGHT_RAW;
			break;

			/* -- Other -- */
		case 'v':
			if (sscanf(optarg, "%i", &verbosity) != 1) {
				fprintf(stderr,
					"Verbosity not specified in a recognizable format.\n\n");
				goto error;
			}
			if (verbosity < 0 || verbosity > 3) {
				fprintf(stderr,
					"Verbosity has to be between 0 and 3.\n\n");
				goto error;
			}
			break;
		default:
			goto error;
		}
	}

	if (!parse_check(light_conf->op_mode, light_conf->field))
		goto error;

	light_loglevel = (light_loglevel_t) verbosity;

	/* Parse <value> (for set/add/subtract operations) */
	if (value) {
		int r;
		double pct = 0.0;

		if (light_conf->val_mode != LIGHT_RAW)
			r = sscanf(value, "%lf", &pct);
		else
			r = sscanf(value, "%" SCNu64, &light_conf->value);

		if (r != 1) {
			LIGHT_ERR("<value> not specified in a recognizable format");
			goto error;
		}

		if (light_conf->val_mode != LIGHT_RAW)
			light_conf->value = VALUE_CLAMP_PCT((uint64_t) (pct * (VALUE_PCT_MAX / 100)));
	}

	return light_conf;
error:
	info_print_help();
	light_free(light_conf);
	return NULL;
}

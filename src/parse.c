#include <getopt.h>

#include "common.h"
#include "log.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "value.h"
#include "light.h"

#define PARSE_SET(str, box, item) \
	if (box != 0) { \
		fprintf(stderr, str" arguments can not be used in conjunction.\n"); \
		goto error; \
	} else { \
		box = item; \
	}

#define PARSE_SET_OP(new)	PARSE_SET("Operation", light_conf->op_mode, new)
#define PARSE_SET_TARGET(new)	PARSE_SET("Target", light_conf->target, new)
#define PARSE_SET_FIELD(new)	PARSE_SET("Field", light_conf->field, new)
#define PARSE_SET_CTRL(new)	PARSE_SET("Controller", light_conf->ctrl_mode, new)
#define PARSE_SET_VAL(new)	PARSE_SET("Value", light_conf->val_mode, new)

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
	int opt, level;
	light_conf_t *light_conf = NULL;
	char *value = NULL;

	level = 0;

	if (!(light_conf = light_new()))
		return NULL;

	while ((opt = getopt(argc, argv, "HhVGS:A:U:LIObmclkaes:pqrv:u:")) != -1) {
		switch (opt) {
			/* -- Operations -- */
		case 'H':
		case 'h':
			PARSE_SET_OP(LIGHT_PRINT_HELP);
			break;
		case 'V':
			PARSE_SET_OP(LIGHT_PRINT_VERSION);
			break;
		case 'G':
			PARSE_SET_OP(LIGHT_GET);
			break;
		case 'S':
			PARSE_SET_OP(LIGHT_SET);
			value = optarg;
			break;
		case 'A':
			PARSE_SET_OP(LIGHT_ADD);
			value = optarg;
			break;
		case 'U':
			PARSE_SET_OP(LIGHT_SUB);
			value = optarg;
			break;
		case 'L':
			PARSE_SET_OP(LIGHT_LIST_CTRL);
			break;
		case 'I':
			PARSE_SET_OP(LIGHT_RESTORE);
			break;
		case 'O':
			PARSE_SET_OP(LIGHT_SAVE);
			break;

			/* -- Targets -- */
		case 'l':
			PARSE_SET_TARGET(LIGHT_BACKLIGHT);
			break;
		case 'k':
			PARSE_SET_TARGET(LIGHT_KEYBOARD);
			break;

			/* -- Fields -- */
		case 'b':
			PARSE_SET_FIELD(LIGHT_BRIGHTNESS);
			break;
		case 'm':
			PARSE_SET_FIELD(LIGHT_MAX_BRIGHTNESS);
			break;
		case 'c':
			PARSE_SET_FIELD(LIGHT_MIN_CAP);
			break;

			/* -- Controller selection -- */
		case 'a':
			PARSE_SET_CTRL(LIGHT_CTRL_AUTO);
			break;
		case 'e':
			PARSE_SET_CTRL(LIGHT_CTRL_ALL);
			break;
		case 's':
			PARSE_SET_CTRL(LIGHT_CTRL_SPECIFY);
			if (path_component(optarg)) {
				light_conf->ctrl = strdup(optarg);
				break;
			}
			fprintf(stderr,	"can't handle controller: '%s'\n", optarg);
			light_free(light_conf);
			return NULL;
			/* -- Value modes -- */
		case 'p':
			PARSE_SET_VAL(LIGHT_PERCENT);
			break;
		case 'q':
			PARSE_SET_VAL(LIGHT_PERCENT_EXPONENTIAL);
			break;
		case 'r':
			PARSE_SET_VAL(LIGHT_RAW);
			break;

			/* -- Other -- */
		case 'v':
			if (sscanf(optarg, "%i", &level) != 1) {
				fprintf(stderr, "verbosity not recognizable.\n");
				goto error;
			}
			if (level < 0 || level > 3) {
				fprintf(stderr, "verbosity must be in range 0-3.\n");
				goto error;
			}
			break;
		case 'u':
			if (sscanf(optarg, "%" SCNd64, &light_conf->usec) != 1) {
				fprintf(stderr,	"usecs not recognizable.\n");
				goto error;
			}
			break;
		default:
			goto error;
		}
	}

	light_loglevel = (light_loglevel_t) level;

	light_defaults(light_conf);

	if (!parse_check(light_conf->op_mode, light_conf->field))
		goto error;

	if (light_conf->field != LIGHT_BRIGHTNESS && light_conf->usec != 0) {
		LIGHT_WARN("Resetting time to zero for non-brightness field");
		light_conf->usec = 0;
	}

	if (value &&
	    (light_conf->value = value_from_string(light_conf->val_mode, value)) < 0) {
		fprintf(stderr, "value not recognizable.\n");
		goto error;
	}

	return light_conf;
error:
	info_print_help();
	light_free(light_conf);
	return NULL;
}

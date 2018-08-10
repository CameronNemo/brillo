#include "light.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * light_defaults:
 *
 * Initialize the default configuration values.
 **/
void light_defaults()
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
bool light_check_ops()
{
	bool valid = true;
	LIGHT_OP_MODE op = light_conf.op_mode;

	/* Nothing to check if we just print info */
	if (op == LIGHT_PRINT_HELP || op == LIGHT_PRINT_VERSION
	    || op == LIGHT_LIST_CTRL) {
		return true;
	}

	switch (light_conf.field) {
	case LIGHT_BRIGHTNESS:
		if (op != LIGHT_GET && op != LIGHT_SET &&
		    op != LIGHT_ADD && op != LIGHT_SUB &&
		    op != LIGHT_SAVE && op != LIGHT_RESTORE) {
			valid = false;
			fprintf(stderr,
				"Wrong operation specified for brightness. You can use only -G -S -A or -U\n\n");
		}
		break;
	case LIGHT_MAX_BRIGHTNESS:
		if (op != LIGHT_GET) {
			valid = false;
			fprintf(stderr,
				"Wrong operation specified for max brightness. You can only use -G\n\n");
		}
		break;
	case LIGHT_MIN_CAP:
		if (op != LIGHT_GET && op != LIGHT_SET) {
			valid = false;
			fprintf(stderr,
				"Wrong operation specified for min cap. You can only use -G or -S\n\n");
		}
	default:
		break;
	}
	return valid;
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
	int currFlag;
	int verbosity;

	bool opSet = false;
	bool targetSet = false;
	bool fieldSet = false;
	bool ctrlSet = false;
	bool valSet = false;

	light_defaults();

	while ((currFlag = getopt(argc, argv, "HhVGSAULIObmclkas:prv:")) != -1) {
		switch (currFlag) {
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
			break;
		case 'A':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_ADD;
			break;
		case 'U':
			ASSERT_OPSET();
			light_conf.op_mode = LIGHT_SUB;
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
			if (optarg == NULL) {
				fprintf(stderr, "-s NEEDS an argument.\n\n");
				light_print_help();
				return false;
			}

			if (!optarg || NAME_MAX < strnlen(optarg, NAME_MAX + 1)) {
				fprintf(stderr,
					"can't handle controller '%s'\n",
					optarg);
				return false;
			}
			light_conf.ctrl = strndup(optarg, NAME_MAX);
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
			if (optarg == NULL) {
				fprintf(stderr, "-v NEEDS an argument.\n\n");
				light_print_help();
				return false;
			}
			if (sscanf(optarg, "%i", &verbosity) != 1) {
				fprintf(stderr,
					"-v Verbosity is not specified in a recognizable format.\n\n");
				light_print_help();
				return false;
			}
			if (verbosity < 0 || verbosity > 3) {
				fprintf(stderr,
					"-v Verbosity has to be between 0 and 3.\n\n");
				light_print_help();
				return false;
			}
			light_loglevel = (light_loglevel_t) verbosity;
			break;
		}
	}

	if (!light_check_ops()) {
		light_print_help();
		return false;
	}

	/* If we need a <value> (for writing), make sure we have it! */
	if (light_conf.op_mode == LIGHT_SET ||
	    light_conf.op_mode == LIGHT_ADD ||
	    light_conf.op_mode == LIGHT_SUB) {
		if (argc - optind != 1) {
			fprintf(stderr,
				"Light needs an argument for <value>.\n\n");
			light_print_help();
			return false;
		}

		if (light_conf.val_mode == LIGHT_PERCENT) {
			if (sscanf(argv[optind], "%lf", &light_conf.val_pct) !=
			    1) {
				fprintf(stderr,
					"<value> is not specified in a recognizable format.\n\n");
				light_print_help();
				return false;
			}
			light_conf.val_pct =
			    light_clampPercent(light_conf.val_pct);
		} else {
			if (sscanf(argv[optind], "%lu", &light_conf.val_raw) !=
			    1) {
				fprintf(stderr,
					"<value> is not specified in a recognizable format.\n\n");
				light_print_help();
				return false;
			}
		}

	}

	return true;
}

/**
 * light_print_version:
 *
 * Prints version and copyright information to standard output.
 **/
void light_print_version()
{
	printf("%s %u.%u (%s)\n", LIGHT_PROG, LIGHT_VER_MAJOR, LIGHT_VER_MINOR,
	       LIGHT_VER_TYPE);
	printf("Copyright (C) %u %s, ", LIGHT_VENDOR_YEAR, LIGHT_VENDOR);
	printf("%u %s\n", LIGHT_YEAR, LIGHT_AUTHOR);
	printf
	    ("This is free software, see the source for copying conditions.  There is NO\n");
	printf
	    ("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n");
}

/**
 * light_print_help:
 *
 * Prints help dialog to standard output.
 **/
void light_print_help()
{
	printf("Usage: %s <options> <value>\n", LIGHT_PROG);
	printf
	    ("<value> has to be either integral(raw mode) or decimal(percent mode) depending on the specified value mode.\n");
	printf("<options> can be any of the following:\n\n");

	printf("Operations (can not be used in conjunction):\n");
	printf("  -H -h:\tPrints this help and exits\n");
	printf("  -V:\t\tPrints version info and exits\n");
	printf("  -G:\t\tGet value (default)\n");
	printf("  -S:\t\tSet value\n");
	printf("  -A:\t\tAdd value\n");
	printf("  -U:\t\tSubtract value\n");
	printf("  -L:\t\tList controllers\n");
	printf("  -I:\t\tRestore brightness\n");
	printf("  -O:\t\tSave brightness\n\n");

	printf("Targets (can not be used in conjunction):\n");
	printf("  -l:\t\tAct on screen backlight (default)\n");
	printf("  -k:\t\tAct on keyboard backlight\n\n");

	printf("Fields (can not be used in conjunction):\n");
	printf("  -b:\t\tBrightness (default)\n  \t\tUsed with [GSAU]\n\n");
	printf("  -m:\t\tMaximum brightness\n  \t\tUsed with [G]\n\n");
	printf("  -c:\t\tMinimum cap\n  \t\tUsed with [GS]\n");
	printf("  \t\tG returns null if no minimum cap is set.\n\n");

	printf("Controller selection (can not be used in conjunction):\n");
	printf("  -a:\t\tSelects controller automatically (default).\n");
	printf("  -s:\t\tSpecify controller to use. (needs argument)\n\n");

	printf("Value modes (can not be used in conjunction):\n");
	printf
	    ("  -p:\t\tInterpret <value> as, and output values in, percent. (default)\n");
	printf
	    ("  -r:\t\tInterpret <value> as, and output values in, raw mode.\n\n");

	printf("Other:\n");
	printf
	    ("  -v:\t\tSets the verbosity level, (needs argument).\n  \t\t0: Only outputs read values.\n  \t\t1: Read values, Errors.\n  \t\t2: Read values, Errors, Warnings.\n  \t\t3: Read values, Errors, Warnings, Notices.\n\n");
}

/**
 * light_initialize:
 *
 * Initializes the configuration for the operation being requested.
 * Ensures that a valid controller exists.
 *
 * WARNING: may allocate a string in light_conf.ctrl,
 *          but will not free it
 *
 * Returns: true on success, false on failure
 **/
bool light_initialize()
{
	/* Make sure we have a valid controller before we proceed */
	if ((light_conf.ctrl_mode == LIGHT_AUTO &&
	     !(light_conf.ctrl = light_ctrl_auto())) ||
	    !light_ctrl_check(light_conf.ctrl))
		return false;

	return true;
}

/**
 * light_info:
 *
 * Print help and version info or list controllers,
 * according to the op_mode.
 *
 * Returns: true if info was shown, otherwise false
 **/
bool light_info()
{
	if (light_conf.op_mode == LIGHT_PRINT_HELP) {
		light_print_help();
		return true;
	}

	if (light_conf.op_mode == LIGHT_PRINT_VERSION) {
		light_print_version();
		return true;
	}

	if (light_conf.op_mode == LIGHT_LIST_CTRL) {
		/* listControllers() can return false, but only if it does not find any controllers. That is not enough for an unsuccessfull run. */
		light_list();
		return true;
	}
	return false;
}

/**
 * light_list:
 *
 * Prints controller names for the appropriate target.
 *
 * Returns: false if could not list controllers or no
 * 		controllers found, otherwise true
 **/
bool light_list()
{
	DIR *dir;
	char *controller;

	if ((dir = light_ctrl_iter_new()) == NULL)
		return false;

	while ((controller = light_ctrl_iter_next(dir)) != NULL) {
		printf("%s\n", controller);
		free(controller);
	}

	closedir(dir);

	return true;
}

/**
 * light_free:
 *
 * Free the controller pointer.
 **/
void light_free()
{
	char *c = light_conf.ctrl;
	if (c)
		free(c);
}

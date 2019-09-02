/* SPDX-License-Identifier: GPL-3.0-only */

#include <getopt.h>
#include <string.h>

#include "common.h"
#include "vlog.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "value.h"
#include "light.h"

#define PARSE_SET(str, box, item) \
	if (box != 0) { \
		fprintf(stderr, "%s arguments can not be used in conjunction.\n", str); \
		return info_help(); \
	} else { \
		box = item; \
	}

#define PARSE_SET_OP(new)	PARSE_SET("Operation", ctx->op_mode, new)
#define PARSE_SET_TARGET(new)	PARSE_SET("Target", ctx->target, new)
#define PARSE_SET_FIELD(new)	PARSE_SET("Field", ctx->field, new)
#define PARSE_SET_CTRL(new)	PARSE_SET("Controller", ctx->ctrl_mode, new)
#define PARSE_SET_VAL(new)	PARSE_SET("Value", ctx->val_mode, new)

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
		vlog_err("only use -G with the max brightness field");
		return false;
	case LIGHT_MIN_CAP:
		if (op == LIGHT_GET || op == LIGHT_SET)
			return true;
		vlog_err("only use -G or -S with the min cap field");
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
 * WARNING: may allocate a string in ctx->ctrl,
 *          but will not free it
 *
 * Returns: a valid conf object on success, NULL on failure
 **/
bool parse_args(int argc, char **argv, struct light_conf *ctx)
{
	int opt, level;
	char *value = NULL, *ctrl = NULL;

	level = -1;

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
			ctrl = optarg;
			break;
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
				vlog_err("verbosity not recognizable");
				return info_help();
			}
			if (level < 0 || level > 8) {
				vlog_err("verbosity must be in range 0-8");
				return info_help();
			}
			break;
		case 'u':
			if (sscanf(optarg, "%" SCNd64, &ctx->usec) != 1) {
				vlog_err("usecs not recognizable");
				return info_help();
			}
			break;
		default:
			return info_help();
		}
	}

	if (level >= 0)
		vlog_lvl_set((vlog_lvl_t) level);

	light_defaults(ctx);

	if (!parse_check(ctx->op_mode, ctx->field))
		return info_help();

	if (ctx->field != LIGHT_BRIGHTNESS && ctx->usec != 0) {
		vlog_warning("Resetting time to zero for non-brightness field");
		ctx->usec = 0;
	}

	if (value &&
	    (ctx->value = value_from_string(ctx->val_mode, value)) < 0) {
		vlog_err("value not recognizable");
		return info_help();
	}

	if (ctrl && (!path_component(ctrl) || !(ctx->ctrl = strdup(ctrl)))) {
		vlog_err("can't handle controller: '%s'", ctrl);
		return info_help();
	}

	return true;
}

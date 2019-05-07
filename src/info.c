/* SPDX-License-Identifier: GPL-3.0-only */

#include "common.h"

#include "burno.h"
#include "vlog.h"
#include "ctrl.h"
#include "light.h"
#include "info.h"

/**
 * info_list:
 * @prefix:	sysfs prefix to walk through
 *
 * Prints controller names in the specified prefix.
 *
 * Returns: false if could not list controllers or no
 *	      controllers found, otherwise true
 **/
bool info_list(char *prefix)
{
	burn_dir dir = opendir(prefix);

	if (!dir) {
		vlog_err("opendir: %m");
		return false;
	}

	for (char *c; (c = ctrl_iter_next(dir)); free(c))
		printf("%s\n", c);

	return true;
}

/**
 * info_help:
 *
 * Prints help dialog to standard output.
 *
 * Returns: false
 **/
bool info_help()
{
	printf("Usage: %s [options] [operation [value]]\n", PROG);
	return false;
}

/**
 * info_print:
 * @op:		operation mode to use
 * @prefix:	sysfs prefix to hand to list controllers
 * @exec:	whether or not to take action
 *
 * If exec is true, prints information
 * according to the operation mode.
 *
 * Returns: true if op_mode is an info mode, otherwise false
 **/
bool info_print(LIGHT_OP_MODE op, char *prefix, bool exec)
{
	switch (op) {
		case LIGHT_PRINT_HELP:
			if (exec)
				info_help();
			break;
		case LIGHT_PRINT_VERSION:
			if (exec)
				printf("%s %s\n", PROG, VERSION);;
			break;
		case LIGHT_LIST_CTRL:
			if (exec)
				info_list(prefix);
			break;
		default:
			return false;
	}

	return true;
}

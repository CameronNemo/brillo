/* SPDX-License-Identifier: GPL-3.0-only */

#include <stdlib.h>

#include "light.h"
#include "log.h"
#include "parse.h"
#include "init.h"
#include "exec.h"

int main(int argc, char **argv)
{
	__lightburn struct light_conf *ctx = NULL;

	if (!(ctx = parse_args(argc, argv))) {
		LIGHT_ERR("Arguments parsing failed");
		return 2;
	}

	if (!(init_strings(ctx))) {
		LIGHT_ERR("Initialization failed");
		return EXIT_FAILURE;
	}

	if (!exec_op(ctx)) {
		LIGHT_ERR("Execution failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

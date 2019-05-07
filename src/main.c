/* SPDX-License-Identifier: GPL-3.0-only */

#include <stdlib.h>
#include <stdbool.h>

#include "light.h"
#include "vlog.h"
#include "parse.h"
#include "init.h"
#include "exec.h"

int main(int argc, char **argv)
{
	light_t ctx = light_new();

	if (!ctx)
		return EXIT_FAILURE;

	if (!(parse_args(argc, argv, ctx))) {
		vlog_err("arguments parsing failed");
		return 2;
	}

	if (!(init_strings(ctx))) {
		vlog_err("initialization failed");
		return EXIT_FAILURE;
	}

	if (!exec_op(ctx)) {
		vlog_err("execution failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

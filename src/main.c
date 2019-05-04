/* SPDX-License-Identifier: GPL-3.0-only */

#include <stdlib.h>

#include "light.h"
#include "vlog.h"
#include "parse.h"
#include "init.h"
#include "exec.h"

int main(int argc, char **argv)
{
	__lightburn struct light_conf *ctx = NULL;

	if (!(ctx = parse_args(argc, argv))) {
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

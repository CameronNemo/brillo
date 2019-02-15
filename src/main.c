#include <stdlib.h>

#include "light.h"
#include "log.h"
#include "parse.h"
#include "init.h"
#include "exec.h"

int main(int argc, char **argv)
{
	light_conf_t *light_conf = NULL;

	if (!(light_conf = parse_args(argc, argv))) {
		LIGHT_ERR("Arguments parsing failed");
		return 2;
	}

	if (!(init_strings(light_conf))) {
		LIGHT_ERR("Initialization failed");
		return EXIT_FAILURE;
	}

	if (!exec_op(light_conf)) {
		light_free(light_conf);
		LIGHT_ERR("Execution failed");
		return EXIT_FAILURE;
	}

	light_free(light_conf);
	return EXIT_SUCCESS;
}

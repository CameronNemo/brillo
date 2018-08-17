#include "light.h"

#define LIGHT_RETURNVAL_INITFAIL  2

int main(int argc, char **argv)
{
	if (!light_parse_args(argc, argv)) {
		LIGHT_ERR("Arguments parsing failed");
		return LIGHT_RETURNVAL_INITFAIL;
	}

	if (!light_initialize()) {
		light_free();
		LIGHT_ERR("Initialization failed");
		return LIGHT_RETURNVAL_INITFAIL;
	}

	if (!light_execute()) {
		light_free();
		LIGHT_ERR("Execution failed");
		return EXIT_FAILURE;
	}

	light_free();
	return EXIT_SUCCESS;
}

#include "common.h"
#include "helpers.h"

bool light_write_val(char const *filename, uint64_t i)
{
	FILE *fileHandle;

	fileHandle = fopen(filename, "w");

	if (!fileHandle) {
		LIGHT_PERMERR("writing");
		return false;
	}

	if (fprintf(fileHandle, "%lu", i) < 0) {
		LIGHT_ERR("fprintf failed");
		fclose(fileHandle);
		return false;
	}

	fclose(fileHandle);
	return true;
}

bool light_read_val(char const *filename, uint64_t *i)
{
	FILE *fileHandle;
	uint64_t iCopy;

	fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMERR("reading");
		return false;
	}

	if (fscanf(fileHandle, "%lu", &iCopy) != 1) {
		LIGHT_ERR("Couldn't parse a positive integer number from '%s'",
			  filename);
		fclose(fileHandle);
		return false;
	}

	*i = iCopy;

	fclose(fileHandle);
	return true;
}

uint64_t light_log_clamp_min(uint64_t x)
{
	LIGHT_NOTE("raising specified value to min: %lu (raw)", x);
	return x;
}

uint64_t light_log_clamp_max(uint64_t x)
{
	LIGHT_NOTE("lowering specified value to max: %lu (raw)", x);
	return x;
}

double light_clamp_pct(double p)
{
	if (p < 0.0) {
		LIGHT_WARN("raising specified value %g%% to 0%%", p);
		return 0.0;
	} else if (p > 100.0) {
		LIGHT_WARN("lowering specified value %g%% to 100%%", p);
		return 100.0;
	}
	return p;
}

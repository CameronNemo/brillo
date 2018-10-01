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

	if (fprintf(fileHandle, "%" SCNu64, i) < 0) {
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

	if (fscanf(fileHandle, "%" SCNu64, &iCopy) != 1) {
		LIGHT_ERR("Couldn't parse a positive integer number from '%s'",
			  filename);
		fclose(fileHandle);
		return false;
	}

	*i = iCopy;

	fclose(fileHandle);
	return true;
}

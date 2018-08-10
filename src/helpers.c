#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

bool light_write_val(char const *filename, unsigned long i)
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

bool light_read_val(char const *filename, unsigned long *i)
{
	FILE *fileHandle;
	unsigned long iCopy;

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

bool light_test_w(char const *filename)
{
	FILE *fileHandle = fopen(filename, "r+");

	if (!fileHandle) {
		LIGHT_PERMWARN("writing");
		return false;
	}

	fclose(fileHandle);
	return true;
}

bool light_test_r(char const *filename)
{
	FILE *fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMWARN("reading");
		return false;
	}

	fclose(fileHandle);
	return true;
}

unsigned long light_log_clamp_min(unsigned long x)
{
	LIGHT_NOTE
	    ("specified value is inferior to %lu (raw), so adjusting it to this mininum value",
	     x);
	return x;
}

unsigned long light_log_clamp_max(unsigned long x)
{
	LIGHT_NOTE
	    ("specified value is superior to %lu (raw), so adjusting it to this maximum value",
	     x);
	return x;
}

double light_clamp_pct(double p)
{
	if (p < 0.0) {
		LIGHT_WARN
		    ("specified value %g%% is not valid, adjusting it to 0%%",
		     p);
		return 0.0;
	} else if (p > 100.0) {
		LIGHT_WARN
		    ("specified value %g%% is not valid, adjusting it to 100%%",
		     p);
		return 100.0;
	}
	return p;
}

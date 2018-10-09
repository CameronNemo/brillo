#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "log.h"
#include "file.h"

#define FILE_MODE_DEFAULT (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/**
 * file_write_smooth:
 * @fd:		file descriptor to write to
 * @start:	starting value
 * @end:	value to eventually write
 * @usec:	time used to smooth the write
 *
 * Writes to the file pointed to by fd, optionally smoothing
 * the operation over usec microseconds.
 *
 * Returns: true on success, false on failure.
 **/
static bool file_write_smooth(int fd, int64_t start, int64_t end, int64_t usec)
{
	struct timespec tspec;

	tspec.tv_sec = 0;
	tspec.tv_nsec = 1000;

	for (int64_t i = 0; i <= usec; i++) {
		int64_t next_value;

		if (usec == 0)
			next_value = end;
		else
			next_value = ((start * usec) + ((end - start) * i)) / usec;

		if (next_value < 0)
			next_value = 0;

		errno = 0;
		if (ftruncate(fd, 0) < 0) {
			LIGHT_ERR("ftruncate: %s", strerror(errno));
			errno = 0;
			close(fd);
			return false;
		}

		if (dprintf(fd, "%" PRId64, next_value) < 0) {
			LIGHT_ERR("dprintf: %" PRId64, next_value);
			close(fd);
			return false;
		}

		fflush(NULL);

		if (usec == 0)
			continue;

		errno = 0;
		if (nanosleep(&tspec, NULL) < 0) {
			LIGHT_ERR("nanosleep: %s", strerror(errno));
			errno = 0;
			close(fd);
			return false;
		}
	}

	return true;
}

/**
 * file_write:
 * @path:	path to write value to
 * @start:	current base value
 * @end:	value to eventually write
 * @usec:	time in microseconds to take to write the value
 *
 * Opens a given path, obtains a lock for the file, then writes into the path.
 * Uses a smooth scale if start and end are not equal and usec is non-zero.
 *
 * Returns: true on success, false on failure
 **/
bool file_write(char const *path, int64_t start, int64_t end, int64_t usec)
{
	int fd;

	errno = 0;
	if ((fd = open(path, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC, FILE_MODE_DEFAULT)) < 0) {
		LIGHT_ERR("open: %s: '%s'", strerror(errno), path);
		errno = 0;
		return false;
	}

	errno = 0;
	if (lockf(fd, F_LOCK, 0) < 0) {
		LIGHT_ERR("lockf: %s: '%s'", strerror(errno), path);
		errno = 0;
		close(fd);
		return false;
	}

	if (!file_write_smooth(fd, start, end, usec))
		return false;

	close(fd);
	return true;
}

/**
 * file_read:
 * @path:	path to read value from
 *
 * Returns: value, or -errno on error
 */
int64_t file_read(char const *path)
{
	FILE *file;
	int64_t value;

	errno = 0;
	if (!(file = fopen(path, "r"))) {
		LIGHT_ERR("fopen: %s: '%s'", strerror(errno), path);
		return -errno;
	}

	errno = 0;
	if (fscanf(file, "%" SCNd64, &value) != 1) {
		LIGHT_ERR("fscanf: %s: '%s'", strerror(errno), path);
		errno = 0;
		fclose(file);
		return -errno;
	}

	fclose(file);
	return value;
}

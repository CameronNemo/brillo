/* SPDX-License-Identifier: 0BSD */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>

#include "burno.h"
#include "vlog.h"
#include "file.h"

#define FILE_MODE_DEFAULT (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define SMOOTH_WRITES_PER_SECOND 50
#define SMOOTH_ITER_DURATION 1e9 / SMOOTH_WRITES_PER_SECOND

/**
 * file_write_sleep:
 * @nsec:	nanoseconds to sleep for
 * @t0:		initial time spec to compare to
 *
 * Sleeps for nsec nanoseconds, subtracted by how much time
 * has passed since the time stored in t0.
 *
 * Returns: true on success, false on failure
 **/
static bool file_write_sleep(long nsec, struct timespec t0)
{
	struct timespec t_sleep, t1;

	t_sleep.tv_sec = 0;
	t_sleep.tv_nsec = nsec;

	/* get current time so that we can compare to t0 */
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &t1) < 0) {
		vlog_err("clock_gettime: %m");
		return false;
	}

	if ((t1.tv_sec - t0.tv_sec) > 1) {
		vlog_warning("time diff greater than 1 second, skipping sleep");
		return true;
	}

	if ((t1.tv_nsec - t0.tv_nsec) < 0)
		t_sleep.tv_nsec -= (t1.tv_nsec - t0.tv_nsec + 1e9);
	else
		t_sleep.tv_nsec -= (t1.tv_nsec - t0.tv_nsec);

	if (t_sleep.tv_nsec <= 0)
		return true;

	/* don't overload nanosleep() */
	if (t_sleep.tv_nsec >= 1e9)
		t_sleep.tv_nsec = 1e9 - 1;

	if (nanosleep(&t_sleep, NULL) < 0) {
		vlog_err("nanosleep: %m");
		return false;
	}

	return true;
}

/**
 * file_rewrite:
 * @fd:		file descriptor to write to
 * @val:	value to write into file
 *
 * Truncates the file described by fd and prints val into it.
 *
 * Returns: true on success, false on failure
 **/
static bool file_rewrite(int fd, int64_t val)
{
	if (val < 0)
		val = 0;

	if (ftruncate(fd, 0) < 0) {
		vlog_err("ftruncate: %m");
		return false;
	}

	if (dprintf(fd, "%" PRId64, val) < 0) {
		vlog_err("dprintf: %" PRId64, val);
		return false;
	}

	/* flush all data to disk so the change takes effect */
	if (fsync(fd) != 0) {
		vlog_err("fsync: %m");
		return false;
	}

	return true;
}

/**
 * file_write:
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
bool file_write(int fd, int64_t start, int64_t end, int64_t usec)
{
	struct timespec t0;

	vlog_notice("Writing (raw) value: %" PRId64, end);

	int64_t num_writes = usec * SMOOTH_WRITES_PER_SECOND / 1e6;

	for (int64_t i = 0; i <= num_writes; i++) {
		int64_t next_value;

		/* save current time to account for the time
		 * taken to perform the write operation */
		clock_gettime(CLOCK_MONOTONIC_RAW, &t0);

		if (usec == 0 || i == num_writes)
			next_value = end;
		else
			next_value = ((start * num_writes) + ((end - start) * i)) / num_writes;

		if (!file_rewrite(fd, next_value))
			return false;

		if (!file_write_sleep(SMOOTH_ITER_DURATION, t0))
			return false;
	}

	return true;
}

/**
 * file_open:
 * @path:	path to open
 * @mode:	access mode to pass to open()
 *
 * Opens a given path and obtains a lock for the file.
 *
 * Returns: an fd for the path on success, -1 on failure
 **/
int file_open(const char *const path, int mode)
{
	int fd;

	if ((fd = open(path, mode | O_TRUNC | O_CREAT | O_SYNC, FILE_MODE_DEFAULT)) < 0) {
		vlog_err("open '%s': %m", path);
		return -1;
	}

	if (lockf(fd, F_LOCK, 0) < 0) {
		vlog_err("lockf '%s': %m", path);
		close(fd);
		return -1;
	}

	return fd;
}

/**
 * file_read:
 * @path:	path to read value from
 *
 * Returns: value, or -errno on error
 */
int64_t file_read(const char *const path)
{
	int64_t value;

	burn_file file = fopen(path, "r");

	if (!file)
		return -errno;

	errno = 0;
	if (fscanf(file, "%" SCNd64, &value) != 1) {
		if (errno == 0)
			/* cppcheck-suppress resourceLeak */
			return -EINVAL;
		/* cppcheck-suppress resourceLeak */
		return -errno;
	}

	/* cppcheck-suppress resourceLeak */
	return value;
}

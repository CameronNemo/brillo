/* SPDX-License-Identifier: 0BSD */

#ifndef FILE_H
#define FILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool file_write(int fd, int64_t start, int64_t end, int64_t usec);
int file_open(char const *path, int mode);
int64_t file_read(char const *path);

#endif /* FILE_H */

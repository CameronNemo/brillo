#ifndef FILE_H
#define FILE_H

bool file_write(char const *path, int64_t start, int64_t end, int64_t usec);
int64_t file_read(char const *path);

#endif /* FILE_H */

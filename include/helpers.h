#ifndef LIGHT_HELPERS_H
#define LIGHT_HELPERS_H

/* Clamps x(value) between y(min) and z(max) in a nested ternary operation. 
 * if(x < y)
 * {
 *  y;
 * }else{
 *  if(x>z)
 *  {
 *    z;
 *  }else{
 *    x;
 *  }
 * }*/
#define LIGHT_CLAMP(x, y, z) ((x<y) ? y : ((x>z) ? z : x ));

#define LIGHT_NOTE(x) if(light_verbosity > 2){printf("%s.\n", x);}

#define LIGHT_WARN(x) if(light_verbosity > 1){printf("warning: \"%s\", in \"%s\" on line %u.\n", x, __FILE__, __LINE__);}

#define LIGHT_ERR(x) if(light_verbosity > 0){printf("error: \"%s\", in \"%s\" on line %u.\n", x, __FILE__, __LINE__);}

#define LIGHT_MEMERR() LIGHT_ERR("memory error");

/* Verbosity levels: 
 * 0 - No output
 * 1 - Errors
 * 2 - Errors, warnings 
 * 3 - Errors, warnings, notices */
int    light_verbosity;

/* Typedef for boolean values */
typedef enum LIGHT_BOOL {
  FALSE = 0,
  TRUE
} LIGHT_BOOL;

/* Reads an unsigned integer from a file into `i` if able, otherwise returns FALSE and leaves `i` untouched */
LIGHT_BOOL light_readUInt(char const *filename, unsigned int *v);

/* Writes an unsigned integer `i` into file `filename` if able, otherwise returns FALSE */
LIGHT_BOOL light_writeUInt(char const *filename, unsigned int v);

LIGHT_BOOL light_writeULong(char const *filename, unsigned long v);
LIGHT_BOOL light_readULong(char const *filename, unsigned long *v);

/* Reads a file into null-terminated `buffer` if able, otherwise returns FALSE
 * If `size` isn't NULL, it will be set to the read size.
 *
 * WARNING: `buffer` HAS to be freed by the user, also make sure it is NULL before passed */
LIGHT_BOOL light_readString(char const * filename, char * buffer, long * size);

/* Returns TRUE if `path` is a valid directory, FALSE otherwise */
LIGHT_BOOL light_isDir(char const * path);

/* Returns TRUE if file is writable, FALSE otherwise */
LIGHT_BOOL light_isWritable(char const * filename);

/* Returns TRUE if file is readable, FALSE otherwise */
LIGHT_BOOL light_isReadable(char const * filename);

#endif /* LIGHT_HELPERS_H */

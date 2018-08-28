#include <sys/stat.h>

#include "common.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "light.h"

/**
 * light_init_sys:
 *
 * Initializes the sys prefix string.
 *
 * Returns: pointer to allocated prefix, or NULL on failure
 **/
char *light_init_sys(const char *tgt)
{
	char *s;

	if (!(s = path_new()))
		return NULL;

	return path_append(s, "/sys/class/%s", tgt);
}

/**
 * light_init_cache:
 *
 * Initializes the cache prefix string,
 * attempts to create the directory.
 *
 * Returns: pointer to allocated prefix, or NULL on failure
 **/
char *light_init_cache(const char * const tgt)
{
	char *s;
	const char *env, *dirfmt;
	int r;

	if ((geteuid() == 0 && (env = "/var/cache")) || 
	    (env = getenv("XDG_CACHE_HOME")))
		dirfmt = "%s/" LIGHT_PROG;
	else if ((env = getenv("HOME")))
		dirfmt = "%s/.cache/" LIGHT_PROG;

	if (!env) {
		LIGHT_ERR("XDG/HOME env vars not set, failed to init cache");
		return NULL;
	}

	if (!(s = path_new()))
		return NULL;

	if (!(s = path_append(s, dirfmt, env)))
		return NULL;

	/* make the cache dir */
	r = mkdir(s, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	/* warn now, the exec will fail later if necessary */
	if (r != 0 && errno != EEXIST)
		LIGHT_WARN("mkdir: %s", strerror(errno));

	return path_append(s, "/%s", tgt);
}

/**
 * light_initialize:
 *
 * Initializes sys/cache prefixes and controller string.
 *
 * WARNING: may allocate strings in light_conf struct,
 *          call light_free() to free them
 *
 * Returns: true on success, false on failure
 **/
bool light_initialize()
{
	const char *tgt;

	if (light_conf.target == LIGHT_BACKLIGHT)
		tgt = "backlight";
	else if (light_conf.target == LIGHT_KEYBOARD)
		tgt = "leds";
	else
		return false;

	if (!(light_conf.sys_prefix = light_init_sys(tgt)))
		return false;

	/* info mode needs no more initialization */
	if (light_info(false))
		return true;

	if (!(light_conf.cache_prefix = light_init_cache(tgt)))
		return false;

	/* Make sure we have a valid controller before we proceed */
	if ((light_conf.ctrl_mode == LIGHT_AUTO &&
	     !(light_conf.ctrl = light_ctrl_auto())) ||
	    !light_ctrl_check(light_conf.ctrl))
		return false;

	return true;
}

/**
 * light_free:
 *
 * Free the string pointers in the light_conf struct.
 **/
void light_free()
{
	char *c;

	c = light_conf.ctrl;
	if (c)
		free(c);

	c = light_conf.sys_prefix;
	if (c)
		free(c);

	c = light_conf.cache_prefix;
	if (c)
		free(c);
}

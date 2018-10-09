#include <sys/stat.h>

#include "common.h"
#include "log.h"
#include "path.h"
#include "info.h"
#include "ctrl.h"
#include "light.h"

/**
 * init_sys:
 * @tgt:	either "leds" or "backlight"
 *
 * Initializes the sysfs prefix string.
 *
 * Returns: pointer to allocated prefix, or NULL on failure
 **/
static char *init_sys(const char *tgt)
{
	char *s;

	if (!(s = path_new()))
		return NULL;

	return path_append(s, "/sys/class/%s", tgt);
}

/**
 * init_cache:
 * @tgt:	either "leds" or "backlight"
 *
 * Initializes the cache prefix string,
 * attempts to create the directory.
 *
 * Returns: pointer to allocated prefix, or NULL on failure
 **/
static char *init_cache(const char * const tgt)
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
 * init_strings:
 * @conf:	light configuration object to initialize
 *
 * Initializes sys/cache prefixes and controller string.
 *
 * WARNING: may allocate strings in light_conf struct,
 *          call light_free() to free them
 *
 * Returns: true on success, false on failure
 **/
bool init_strings(light_conf_t *conf)
{
	const char *tgt;

	if (conf->target == LIGHT_BACKLIGHT)
		tgt = "backlight";
	else if (conf->target == LIGHT_KEYBOARD)
		tgt = "leds";
	else
		goto error;

	if (!(conf->sys_prefix = init_sys(tgt)))
		goto error;

	/* info mode needs no more initialization */
	if (info_print(conf->op_mode, conf->sys_prefix, false))
		return true;

	if (!(conf->cache_prefix = init_cache(tgt)))
		goto error;

	/* Make sure we have a valid controller before we proceed */
	if ((conf->ctrl_mode == LIGHT_CTRL_ALL) || conf->ctrl || ctrl_auto(conf))
		return true;

error:
	light_free(conf);
	return false;
}

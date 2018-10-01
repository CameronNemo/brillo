#include "common.h"
#include "helpers.h"
#include "light.h"
#include "exec.h"
#include "ctrl.h"

/**
 * ctrl_iter_next:
 * @dir:	opened directory to iterate over
 *
 * Iterates over the directory given by dir.
 *
 * WARNING: will allocate a string and return it,
 *          this string should be freed after use
 *
 * Returns: name of the next controller, NULL on end of dir or failure
 **/
char *ctrl_iter_next(DIR * dir)
{
	struct dirent *file;

	if (!dir) {
		LIGHT_ERR("directory uninitialized");
		return NULL;
	}

	while ((file = readdir(dir))) {
		if (file->d_name[0] != '.')
			return strdup(file->d_name);
	}

	return NULL;
}

/**
 * ctrl_auto:
 * @conf:	configuration object to work on
 *
 * Iterates over the appropriate directory and finds the
 * controller with the highest max brightness. Stores the
 * name of the controller and the max brightness value in
 * the configuration object
 *
 * WARNING: will return an allocated string, which
 *          should be freed after use
 *
 * Returns: best controller, or NULL if no suitable controller is found
 **/
bool ctrl_auto(light_conf_t *conf)
{
	DIR *dir;
	char *next, *prev;

	LIGHT_NOTE("finding best controller...");

	if (!(dir = opendir(conf->sys_prefix))) {
		LIGHT_ERR("opendir: %s", strerror(errno));
		return false;
	}

	while ((next = ctrl_iter_next(dir))) {
		uint64_t max = 0;
		prev = conf->ctrl;
		conf->ctrl = next;

		if (light_fetch(conf, LIGHT_MAX_BRIGHTNESS, &max)) {
			if (max > conf->cached_max) {
				LIGHT_NOTE("found (better) controller '%s'", next);
				conf->cached_max = max;
				if (prev)
					free(prev);
				continue;
			} else {
				LIGHT_NOTE("found worse controller '%s'", next);
				conf->ctrl = prev;
			}
		} else {
			LIGHT_WARN("found inaccessible controller '%s'", next);
			conf->ctrl = prev;
		}

		free(next);
	}

	closedir(dir);

	if (!conf->ctrl) {
		LIGHT_ERR("could not find an accessible controller");
		return false;
	}

	return true;
}

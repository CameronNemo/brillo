/* SPDX-License-Identifier: GPL-3.0-only */

#include <string.h>

#include "common.h"

#include "burno.h"
#include "vlog.h"
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
		vlog_err("directory uninitialized");
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
bool ctrl_auto(struct light_conf *conf)
{
	char *next, *prev;
	burn_dir dir = opendir(conf->sys_prefix);

	if (!dir) {
		vlog_err("opendir: %m");
		return false;
	}

	while ((next = ctrl_iter_next(dir))) {
		int64_t max = 0;
		prev = conf->ctrl;
		conf->ctrl = next;

		if ((max = light_fetch(conf, LIGHT_MAX_BRIGHTNESS)) > 0) {
			if (max > conf->cached_max) {
				vlog_debug("found (better) controller '%s'", next);
				conf->cached_max = max;
				if (prev)
					free(prev);
				continue;
			} else {
				vlog_notice("found worse controller '%s'", next);
				conf->ctrl = prev;
			}
		} else {
			vlog_warning("found inaccessible controller '%s'", next);
			conf->ctrl = prev;
		}

		free(next);
	}

	if (conf->ctrl) {
		vlog_notice("automatically chose controller: '%s'", conf->ctrl);
		return true;
	}

	vlog_err("could not find an accessible controller");
	return false;
}

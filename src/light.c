/* SPDX-License-Identifier: GPL-3.0-only */

#include <stdlib.h>
#include <stdio.h>

#include "light.h"
#include "vlog.h"

/**
 * light_new:
 *
 * Creates a new light configuration object and initializes its values.
 *
 * Returns: light configuration object, or NULL on memory error
 **/
struct light_conf *light_new()
{
	struct light_conf *conf = NULL;

	if (!(conf = malloc(sizeof(struct light_conf)))) {
		vlog_err("malloc: %m");
		return NULL;
	}

	conf->ctrl = NULL;
	conf->sys_prefix = NULL;
	conf->cache_prefix = NULL;
	conf->ctrl_mode = LIGHT_CTRL_UNSET;
	conf->op_mode = LIGHT_OP_UNSET;
	conf->val_mode = LIGHT_VAL_UNSET;
	conf->target = LIGHT_TARGET_UNSET;
	conf->field = LIGHT_FIELD_UNSET;
	conf->value = 0;
	conf->usec = 0;
	conf->cached_max = 0;

	return conf;
}

/**
 * light_defaults:
 * @conf:	configuration object to populate
 *
 * Populate a light configuration object with default values
 * for all struct members that have not been set.
 *
 * Returns: void
 **/
void light_defaults(struct light_conf *conf)
{
	if (conf->ctrl_mode == 0)
		conf->ctrl_mode = LIGHT_CTRL_AUTO;

	if (conf->op_mode == 0)
		conf->op_mode = LIGHT_GET;

	if (conf->val_mode == 0)
		conf->val_mode = LIGHT_PERCENT;

	if (conf->target == 0)
		conf->target = LIGHT_BACKLIGHT;

	if (conf->field == 0)
		conf->field = LIGHT_BRIGHTNESS;
}

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "light.h"
#include "log.h"

/**
 * light_new:
 *
 * Creates a new light configuration object and initializes its values.
 *
 * Returns: light configuration object, or NULL on memory error
 **/
light_conf_t *light_new()
{
	light_conf_t *conf = NULL;

	errno = 0;
	if (!(conf = malloc(sizeof(light_conf_t)))) {
		LIGHT_ERR("malloc: %s", strerror(errno));
		errno = 0;
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
void light_defaults(light_conf_t *conf)
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

/**
 * light_free:
 * @conf:	configuration object to free
 *
 * Free the conf object and its string pointers.
 **/
void light_free(light_conf_t *conf)
{
	char *c;

	if (!conf)
		return;

	c = conf->ctrl;
	if (c)
		free(c);

	c = conf->sys_prefix;
	if (c)
		free(c);

	c = conf->cache_prefix;
	if (c)
		free(c);

	free(conf);
}

#include <stdlib.h>

#include "light.h"
#include "helpers.h"

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

	if (!(conf = malloc(sizeof(light_conf_t)))) {
		LIGHT_MEMERR();
		return NULL;
	}

	conf->ctrl = NULL;
	conf->sys_prefix = NULL;
	conf->cache_prefix = NULL;
	conf->op_mode = LIGHT_GET;
	conf->val_mode = LIGHT_PERCENT;
	conf->val_raw = 0;
	conf->val_pct = 0.0;
	conf->target = LIGHT_BACKLIGHT;
	conf->field = LIGHT_BRIGHTNESS;
	conf->cached_max = 0;

	return conf;
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

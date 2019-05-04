/* SPDX-License-Identifier: 0BSD */

#include <stdio.h>
#include <stdarg.h>

#include "vlog.h"

static vlog_lvl_t vlog_lvl = VLOG_LVL_DEFAULT;
static const char *vlog_lvl_to_string(vlog_lvl_t lvl);

void vlog(vlog_lvl_t lvl, const char *fmt, ...)
{
	if (lvl > vlog_lvl)
		return;
	fprintf(stderr, "%s: ", vlog_lvl_to_string(lvl));
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

vlog_lvl_t vlog_lvl_set(vlog_lvl_t lvl)
{
	return (vlog_lvl = lvl);
}

vlog_lvl_t vlog_lvl_get(void)
{
	return vlog_lvl;
}

static const char *vlog_lvl_to_string(vlog_lvl_t lvl)
{
	switch (lvl) {
		case VLOG_LVL_EMERGENCY: return "Emergency";
		case VLOG_LVL_ALERT: return "Alert";
		case VLOG_LVL_CRITICAL: return "Critical";
		case VLOG_LVL_ERROR: return "Error";
		case VLOG_LVL_WARNING: return "Warning";
		case VLOG_LVL_NOTICE: return "Notice";
		case VLOG_LVL_INFORMATIONAL: return "Informational";
		case VLOG_LVL_DEBUG: return "Debug";
		default: return "Unknown";
	}
}

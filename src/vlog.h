/* SPDX-License-Identifier: 0BSD */

#ifndef VLOG_H
#define VLOG_H

typedef enum {
	VLOG_LVL_EMERGENCY = 0,
	VLOG_LVL_ALERT,
	VLOG_LVL_CRITICAL,
	VLOG_LVL_ERROR,
	VLOG_LVL_WARNING,
	VLOG_LVL_NOTICE,
	VLOG_LVL_INFORMATIONAL,
	VLOG_LVL_DEBUG
} vlog_lvl_t;

#ifndef VLOG_LVL_DEFAULT
#define VLOG_LVL_DEFAULT VLOG_LVL_WARNING
#endif

void vlog(vlog_lvl_t lvl, const char* fmt, ...);

#define vlog_emerg(...) vlog(VLOG_LVL_EMERGENCY, __VA_ARGS__)
#define vlog_alert(...) vlog(VLOG_LVL_ALERT, __VA_ARGS__)
#define vlog_crit(...) vlog(VLOG_LVL_CRITICAL, __VA_ARGS__)
#define vlog_err(...) vlog(VLOG_LVL_ERROR, __VA_ARGS__)
#define vlog_warning(...) vlog(VLOG_LVL_WARNING, __VA_ARGS__)
#define vlog_notice(...) vlog(VLOG_LVL_NOTICE, __VA_ARGS__)
#define vlog_info(...) vlog(VLOG_LVL_INFORMATIONAL, __VA_ARGS__)
#define vlog_debug(...) vlog(VLOG_LVL_DEBUG, __VA_ARGS__)

vlog_lvl_t vlog_lvl_set(vlog_lvl_t lvl);
vlog_lvl_t vlog_lvl_get(void);

#endif /* VLOG_H */

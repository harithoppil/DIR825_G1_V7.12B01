#ifndef __TBS_WATCHDOG_H__
#define __TBS_WATCHDOG_H__

#define WDT_PROC_ENTRY                   "watchdog"

enum {
	WDT_MODE_DISABLE = 0,
	WDT_MODE_KERNEL  = 1,
	WDT_MODE_USER    = 2,
};

extern void watchdog_open(void);
extern void watchdog_kick(void);
extern void watchdog_close(void);

#endif

/*
 * LED Class Core
 *
 * Copyright 2005-2006 Openedhand Ltd.
 *
 * Author: Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/leds.h>
#include <led.h>

struct tbs_ledcore *ledcore = NULL;
static unsigned long boot_fault_flags = 0;

void tbs_led_data_blinking(led_name led)
{
	if((ledcore) && (0 == ledcore->bdata[led].blink_pending)) {/* check if this led is working already ? */
		if(ledcore->led_blinking_handler) {/* Blinking handler is registered */
			ledcore->led_blinking_handler(led);
		}
	}
}

void tbs_led_trigger_set(led_name led, trig_name trigger)
{
	if((ledcore) && (ledcore->trigger_set))	{/* Trigger set is registered */
		ledcore->trigger_set(led, trigger);
	}
}

void tbs_led_system_fault_blinking(led_name led)
{
	if(led < led_end) {
		boot_fault_flags |= (1 << led);
	}
	if(boot_fault_flags) {
		tbs_led_trigger_set(led_power_green, led_off_trig);
		tbs_led_trigger_set(led_power_red, led_flash_2hz_trig);
	}
}

EXPORT_SYMBOL_GPL(ledcore);
EXPORT_SYMBOL(tbs_led_data_blinking);
EXPORT_SYMBOL_GPL(tbs_led_system_fault_blinking);
EXPORT_SYMBOL(tbs_led_trigger_set);

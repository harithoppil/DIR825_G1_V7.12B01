
/*
 * LED tbscore  TBS平台LED核心驱动
 */
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <tbs_ioctl.h>
#include <led.h>

/* 周期从零开始，偶数周期灭灯，奇数周期亮灯 */
static unsigned int flash_off_seq[16] = {500, 10, 0};/* 亮灭时序:长灭 */
static unsigned int flash_on_seq[16] = {10, 500, 0};/* 亮灭时序:灭0.01s，再长亮 */
static unsigned int flash_write_seq[16] = {100, 200, 100, 200, 400, 0};/* 亮灭时序:灭0.1s，再亮0.2s，灭0.1s，再亮0.2s，间隔0.5s再循环 */
static unsigned int flash_hz_seq[16] = {500, 500, 0};/* 亮灭时序:灭0.5s，亮0.5s，再循环 */
static unsigned int flash_2hz_seq[16] = {250, 250, 0};/* 亮灭时序:灭0.25s，亮0.25s，再循环 */
static unsigned int flash_4hz_seq[16] = {125, 125, 0};/* 亮灭时序:灭0.125s，亮0.125s，再循环 */
static unsigned int dataflash_seq[16] = {50, 50, 0};/* 亮灭时序:灭0.05s，亮0.05s，再循环 */
static unsigned int wps_inprogress_seq[16] = {200, 100, 0};/* 亮灭时序:灭0.2s，亮0.1s，重复120s后长灭 */
static unsigned int wps_error_seq[16] = {100, 100, 0};/* 亮灭时序:灭0.1s，再亮0.1s，重复120s后常灭 */
static unsigned int wps_overlap_seq[16] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 400, 0};/* 亮灭时序:灭0.1s，亮0.1s，重复5次，延时0.5秒再循环 */
static unsigned int wps_success_seq[16] = {10, 300000, 0};/* 亮灭时序:灭0.01s，亮300s，长亮 */
static unsigned int wlan_beacon_seq[16] = {450, 50, 0};/* 亮灭时序:灭0.45s，亮0.05s，再循环 */
static unsigned int blinking_seq[16] = {70, 30, 0};/* 亮灭时序:灭0.05s，亮0.02s，高频率调用，不循环 */

/***************************************************************************************/
/* 有一定时长的trigger，按周期的整数倍计算时间，使达到指定的时长数值之后，             */
/* 加一定的余量，可让trigger停留在偶数周期以保持常灭状态，停留在奇数周期以保持常亮状态 */
/***************************************************************************************/
static struct tbs_led_trigger led_triggers[] = {
	{led_off_trig, 500, flash_off_seq},                                 //endless
	{led_on_trig, 510, flash_on_seq},                                   //endless
	{led_flash_trig, LED_TRIG_TTL_MAX, flash_write_seq},                //endless
	{led_flash_hz_trig, LED_TRIG_TTL_MAX, flash_hz_seq},                //endless	
	{led_flash_2hz_trig, LED_TRIG_TTL_MAX, flash_2hz_seq},              //endless
	{led_flash_4hz_trig, LED_TRIG_TTL_MAX, flash_4hz_seq},              //endless
	{led_dataflash_trig, LED_TRIG_TTL_MAX, dataflash_seq},              //endless
	{led_wps_inprogress_trig, 120200, wps_inprogress_seq},              //2min in ms
	{led_wps_error_trig, 120100, wps_error_seq},                        //2min in ms
	{led_wps_overlap_trig, 120400, wps_overlap_seq},                    //2min in ms
	{led_wps_success_trig, 300020, wps_success_seq},                    //5min in ms
	{led_wlan_beacon_trig, LED_TRIG_TTL_MAX, wlan_beacon_seq},          //endless
	{led_blinking_trig, 100, blinking_seq},                             //100ms
	{led_end_trig, LED_TRIG_TTL_MAX, NULL}
};


/***************************************************
     亮灭交替完成整个时序，再使用定时器循环
 ***************************************************/
static void tbs_led_timer_handler(unsigned long data)
{
	struct tbs_led_trigger_data *td = (struct tbs_led_trigger_data *)data;
	unsigned int t = jiffies;
	struct tbs_led_trigger *trig;

	if(!td)	{
		return;
	} else if((!td->dev) || (!td->dev->cur_trig) || (td->phase > 12)) {
		goto out;
	} else {
		trig = td->dev->cur_trig;
		if(trig->flash_seq[td->phase] < 1) {
			td->phase = 0;
		}		
		if(td->ttl < LED_TRIG_TTL_MAX) {
			if((td->ttl > 0) && (td->ttl >= trig->flash_seq[td->phase])) {
				td->ttl -= trig->flash_seq[td->phase];
			} else {/* Run out of cycle time */
				leddebug("%s: led %d OUT!\n", __func__, td->dev->name);
				goto out;
			}
		}
	}
	if(td->phase & 1) {/* 奇数周期亮灯 */
		if(led_off_trig != trig->name) {
			ledcore->handler->led_on(td->dev);
		}
	} else {/* 偶数周期灭灯 */
		ledcore->handler->led_off(td->dev);
	}
	t += msecs_to_jiffies(trig->flash_seq[td->phase]);
	td->phase++;
	mod_timer(&td->timer, t);
	return;
out:
	td->phase = 0;	
	td->blink_pending = 0;
}


/***************************************************
 ***************************************************/
static void tbs_led_flash_trig_off(struct led_dev *dev)
{
	struct tbs_led_trigger_data *td = &ledcore->bdata[dev->name];

	del_timer_sync(&td->timer);
	leddebug("%s: led=%d\n", __func__, dev->name);
	td->ttl = 0;
	td->blink_pending = 0;
	td->phase = 0;
}

static void tbs_led_flash_trig_on(struct led_dev *dev)
{
	struct tbs_led_trigger_data *td = &ledcore->bdata[dev->name];

	td->phase = 0;
	td->ttl = dev->cur_trig->cycle_time;/* 设定生存时间 */
	tbs_led_timer_handler((unsigned long)td);
}

static void blinking_handler(led_name led)
{
	struct tbs_led_trigger_data *td = NULL;

	td = &ledcore->bdata[led];
	if((td->dev) && (td->dev->cur_trig) &&
		((led_blinking_trig == td->dev->cur_trig->name) || (led_on_trig == td->dev->cur_trig->name))) {
		td->blink_pending = 1;
		tbs_led_flash_trig_on(td->dev);
	}
}

/**************************************************
    设置led_dev所用的trigger
***************************************************/
static int led_dev_trigger_set(led_name led, trig_name trigger)
{
	struct led_dev *dev = NULL;
	int ret_val = -1;
	led_name start, end;

	if(trigger >= led_end_trig) {
		leddebug(KERN_ERR "%s, Trigger name %d larger than TRIG_NAME_MAX\n", __func__, trigger);
		goto err;
	} else if(led >= led_end) {
        leddebug(KERN_ERR "%s, Led name:%d is larger than LED_NAME_MAX\n", __func__, led);
        goto err;
	} else {			
		if(led_all == led) {/* 当使用led_all时，操作全部的LED */
			end = led_end;
		} else {
			end = led + 1;
		}
		for(start = led; start < end; start++) {
			if(ledcore) {
				dev = ledcore->bdata[start].dev;
			} else {
				dev = NULL;
			}
			if(NULL == dev) {
				leddebug(KERN_ERR "%s, Led %d is not registered yet.\n", __func__, start);
				continue;
			}
			if(NULL != dev->cur_trig) {/* 先让原来设置的trigger失效 */
			    tbs_led_flash_trig_off(dev);
			}
			dev->cur_trig = &led_triggers[trigger];/* 设置新的trigger */
		    tbs_led_flash_trig_on(dev);/* 启用新的trigger */
		}
		ret_val = 0;
	}
err:
    return ret_val;
}

/***************************************************
    获取led_dev当前所用的trigger: 正常情况返回LED已设置trigger, 
    如果led_name异常，或此LED没有设置trigger，返回led_end_trig;
***************************************************/
static trig_name led_dev_trigger_get(led_name led)
{
	trig_name trig = led_end_trig;
	
    if(led >= led_end) {
        leddebug(KERN_ERR "LED:Led name:%d is larger than LED_NAME_MAX\n", led);
        goto err;
	}
	if(NULL == ledcore->bdata[led].dev) {
		leddebug(KERN_ERR "LED:can not get trigger, led %d is not registered yet.\n", led);
		goto err;
	}
    if(NULL == ledcore->bdata[led].dev->cur_trig) {
        leddebug(KERN_ERR "LED: No trigger on this led.\n");
        goto err;
   	} else {
		trig = ledcore->bdata[led].dev->cur_trig->name;
	}
err:	
    return trig;
}



/**************************************************
    注册hw_handler到core
***************************************************/
int led_hw_handle_register(struct led_hw_handler *handler)
{
	int ret_val = -1;
    if((!ledcore) || (!handler)) {
    	goto err;
   	} else if(ledcore->handler) {
        leddebug(KERN_ERR "LED:Global_led_hw_handler is registered.\n");
        goto err;
   	} else {
		ledcore->handler = handler;
		ret_val = 0;
	}
err:
	return ret_val;
}

int led_hw_handle_unregister(struct led_hw_handler *handler)
{
	if((!ledcore) || (!handler)) {
		goto err;
	} else if(!ledcore->handler) {
		leddebug(KERN_WARNING "LED:led_hw_handle led_off is not registered.\n");
	} else {
		ledcore->handler = NULL;
	}
err:	
	return 0;
}


/***************************************************
     注册led_dev到core
 ***************************************************/
int led_dev_register(struct led_dev *led)
{
    int ret_val = -1;

    if((!ledcore) || (!led) || (!ledcore->handler)) {
        leddebug(KERN_ERR "LED:NULL pointer in %s.\n", __func__);
        goto err;
	}
	if(ledcore->bdata[led->name].dev) {
        leddebug(KERN_ERR "LED: led %d GPIO %d has been registered already.\n", led->name, led->gpio);
        goto err;
	} else {
	    ledcore->bdata[led->name].dev = led;
	}
    ret_val = ledcore->trigger_set(led->name, led->default_trig);
    if(ret_val < 0) {
        leddebug(KERN_WARNING "LED:fail to set default trigger.\n");
   	}
err:
    return ret_val;

}

int led_dev_unregister(struct led_dev *led)
{
    int ret_val = -1;

    if((!ledcore) || (!led) || (!ledcore->handler)) {
        leddebug(KERN_ERR "LED:NULL pointer in %s.\n", __func__);
        goto err;
	} else {
		ret_val = 0;
	}	
    if(ledcore->bdata[led->name].dev) {
	    if(led->cur_trig) {
	        led_dev_trigger_set(led->name, led_off_trig);
			led->cur_trig = NULL;
		}
	    ledcore->bdata[led->name].dev = NULL;
   	} else {
	    leddebug(KERN_ERR "LED: led %d GPIO %d was not registered yet.\n", led->name, led->gpio);
	}
err:
    return ret_val;
}

/**************************************************
    提供proc接口操作LED
***************************************************/
static int led_core_proc_write(struct file *filp, const char __user *buf, unsigned long len, void *data )
{
    int ret;
    char str_buf[256];
    led_name name = led_end;
    trig_name trig = led_end_trig;
    int i;

    if(len > 255) {
        goto err;
   	}
    copy_from_user(str_buf, buf, len);
    str_buf[len] = '\0';
    for(i = 0;i < len;i++) {
		if(str_buf[i] < 0x30 || str_buf[i] > 0x39) {
			if(str_buf[i] != 0x20 && str_buf[i] != 0x0a) {
				goto err;
			}
		}
	}
	leddebug("len %d:%s\n",(int)len,str_buf);
	ret = sscanf(str_buf, "%d %d", (int*)&name, (int*)&trig);
	if(ret < 0) {
		goto err;
	}
	leddebug("Set:name %d, trigger %d\n", name, trig);
	ret = led_dev_trigger_set(name, trig);
	if(ret< 0) {
		leddebug("LED: trigger set failed\n");
	}
	goto out;
	
err:
	printk("Usage: echo led_number trigger_number > /proc/led\n");
out:
    return len;
}

static int led_core_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{	
	struct tbs_led_trigger_data *td = NULL;
	trig_name trig;
	int i;
	int len = 0;

	len += sprintf(page + len, "Current leds state:\n");
	for(i = 0; i < led_end; i++) {
		td = &ledcore->bdata[i];
		if(NULL != td->dev) {
			trig = led_dev_trigger_get(td->dev->name);
			if(led_end_trig == trig) {
				trig = td->dev->default_trig;
			}
			len += sprintf(page + len, "Led %02d is running on trigger %02d ttl %10ld\n", 
				(unsigned int)td->dev->name, (unsigned int)trig, td->ttl);
		}
	}
	if(len <= off + count) {
		*eof = 1;
	}
	*start = page + off;
	len -= off;
	if(len > count) {
		len = count;
	}
	if(len < 0) {
		len = 0;
	}

	return len;
}


int __init led_core_init(void)
{
	struct proc_dir_entry *proc_led;
	int i;

	ledcore = kzalloc(sizeof(struct tbs_ledcore), GFP_KERNEL | __GFP_NOFAIL);
	for(i = 0; i < led_end; i++) {
		setup_timer(&ledcore->bdata[i].timer, tbs_led_timer_handler, (unsigned long)&ledcore->bdata[i]);		
	}
	ledcore->led_blinking_handler = blinking_handler;
	ledcore->trigger_set = led_dev_trigger_set;
	//#ifdef CONFIG_DSL_SAR_SUPPORT
	dsl_led_trigger_set = tbs_led_trigger_set;
	//#endif
	#ifdef CONFIG_USB_SUPPORT
	usb_led_trigger_set = tbs_led_trigger_set;
	usb_led_data_blinking = tbs_led_data_blinking;
	#endif
	proc_led = create_proc_entry("led", 0644, NULL);
    proc_led->write_proc = led_core_proc_write;
	proc_led->read_proc = led_core_proc_read;
	printk(KERN_NOTICE "TBS leds core driver initialized\n");
    return 0;
}

void __exit led_core_exit(void)
{
	int i;
	
	remove_proc_entry("led", NULL);
	//#ifdef CONFIG_DSL_SAR_SUPPORT
	dsl_led_trigger_set = NULL;
	//#endif
	#ifdef CONFIG_USB_SUPPORT
	usb_led_trigger_set = NULL;
	usb_led_data_blinking = NULL;
	#endif
	for(i = 0; i < led_end; i++) {
		del_timer_sync(&ledcore->bdata[i].timer);
        if((NULL != ledcore->bdata[i].dev) && (NULL != ledcore->bdata[i].dev->cur_trig)) {
			ledcore->bdata[i].dev->cur_trig = NULL;
			ledcore->bdata[i].dev = NULL;
		}
	}
	memset(ledcore, 0, sizeof(struct tbs_ledcore));
	kfree(ledcore);	
	ledcore = NULL;
}


EXPORT_SYMBOL_GPL(led_hw_handle_register);
EXPORT_SYMBOL_GPL(led_hw_handle_unregister);
EXPORT_SYMBOL_GPL(led_dev_register);
EXPORT_SYMBOL_GPL(led_dev_unregister);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("TBS LED Core");
MODULE_LICENSE("GPL");


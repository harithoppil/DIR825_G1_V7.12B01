/*
  Description: EBTables time match extension kernelspace module.
  Authors:  Song Wang <songw@broadcom.com>, ported from netfilter/iptables
            The following is the original disclaimer.

  This is a module which is used for time matching
  It is using some modified code from dietlibc (localtime() function)
  that you can find at http://www.fefe.de/dietlibc/
  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from: ftp://prep.ai.mit.edu/pub/gnu/GPL
  2001-05-04 Fabrice MARIE <fabrice@netfilter.org> : initial development.
  2001-21-05 Fabrice MARIE <fabrice@netfilter.org> : bug fix in the match code,
     thanks to "Zeng Yu" <zengy@capitel.com.cn> for bug report.
  2001-26-09 Fabrice MARIE <fabrice@netfilter.org> : force the match to be in LOCAL_IN or PRE_ROUTING only.
  2001-30-11 Fabrice : added the possibility to use the match in FORWARD/OUTPUT with a little hack,
     added Nguyen Dang Phuoc Dong <dongnd@tlnet.com.vn> patch to support timezones.
*/
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_time.h>
#include <linux/time.h>

static unsigned char debug = 1;
#define DEBUG_MSG(...) if (debug) printk (KERN_DEBUG "ebt_time: " __VA_ARGS__)

struct ebt_tm
{
	int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
	int tm_min;                   /* Minutes.     [0-59] */
	int tm_hour;                  /* Hours.       [0-23] */
	int tm_mday;                  /* Day.         [1-31] */
	int tm_mon;                   /* Month.       [0-11] */
	int tm_year;                  /* Year - 1900.  */
	int tm_wday;                  /* Day of week. [0-6] */
	int tm_yday;                  /* Days in year.[0-365] */
	int tm_isdst;                 /* DST.         [-1/0/1]*/

	long int tm_gmtoff;           /* we don't care, we count from GMT */
	const char *tm_zone;          /* we don't care, we count from GMT */
};

#ifdef CONFIG_DST
struct tbs_dst_info
{
    time_t dst_start;      //  Daylight saving time begin
    time_t dst_stop;       //  Daylight saving time end
};
extern struct tbs_dst_info * dst;
#define DAY_SECOND       86400
#endif

extern struct timezone sys_tz;

// mask by xgl 2009-08-26, because net\ipv4\netfilter\ipt_time.c has this function.
#if 1
/* seconds per day */
#define SPD 24*60*60

/*
* TBS_TAG: add by chenzhen 2012/10/31
* global=1 timepr is kernel time else global = 0 timepr is local time;
*/ 

void localtime(const time_t *timepr, struct ebt_tm *r, char global) 
{
	time_t i;
	time_t timep;
	static const u_int16_t days_since_year[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
	};

	static const u_int16_t days_since_leapyear[] = {
		0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
	};
	register time_t work;

    if(global)
	    timep = (*timepr) - (sys_tz.tz_minuteswest * 60);
    else
        timep = *timepr;
	work=timep%(SPD);
	r->tm_sec=work%60; work/=60;
	r->tm_min=work%60; r->tm_hour=work/60;
	work=timep/(SPD);
	r->tm_wday=(4+work)%7;
	for (i=1970; ; ++i) {
		register time_t k= (!(i%4) && ((i%100) || !(i%400)))?366:365;
		if (work>k)
			work-=k;
		else
			break;
	}
	r->tm_year=i-1900;

    /*
	 * TBS_TAG: add by chenzhen 2012/10/31
	 */ 
    r->tm_yday=work;
	/*
	 * TBS_TAG: add by Charles Alison(BaiYonghui) 2012/7/25
	 * Description: the day of Feb is 29 in the leap year,
	 *              so it should be add one day from Mar
	 */ 
	if(!(i%4) && ((i%100) || !(i%400)))
	{
		for (i=11; i && days_since_leapyear[i]>work; --i) ;
		r->tm_mday = work-days_since_leapyear[i]+1;
	}
	else
	{
		for (i=11; i && days_since_year[i]>work; --i) ;
		r->tm_mday = work-days_since_year[i]+1;
	}
	/*
	 * TBS_END_TAG
	 */
	/* 
	 * TBS_TAG: add by Charles Alison(BaiYonghui) 2012-7-25 for 
	 *          the begin of the month should be Jan
	 */
	r->tm_mon = i+1;
}
#endif

#ifdef CONFIG_DST

static bool time_check_dst (time_t stamp )
{   
    struct ebt_tm local;
    struct ebt_tm dst_start;
    struct ebt_tm dst_stop;

    if(!dst)
        return false;

    localtime(&stamp, &local, 1);
    localtime(&dst->dst_start, &dst_start, 0);
    localtime(&dst->dst_stop, &dst_stop, 0);   

    if(dst_start.tm_yday> dst_stop.tm_yday)
    {
        if(local.tm_yday < dst_start.tm_yday && local.tm_yday > dst_stop.tm_yday )
            return false;
    }
    else
    {
        if(local.tm_yday < dst_start.tm_yday || local.tm_yday > dst_stop.tm_yday)
            return false;
    }

    if(local.tm_yday == dst_start.tm_yday)
        if((stamp%DAY_SECOND)<(dst->dst_start%DAY_SECOND))
            return false;
    if(local.tm_yday == dst_stop.tm_yday)
        if((stamp%DAY_SECOND)>(dst->dst_stop%DAY_SECOND))
            return false;    
   
    return true;
}

#endif

static bool
ebt_time_mt(const struct sk_buff *skb, struct xt_match_param *par)
{
    const struct ebt_time_info *info = par->matchinfo;   /* match info for rule */
    struct ebt_tm currenttime;                          /* time human readable */
	u_int8_t days_of_week[7] = {64, 32, 16, 8, 4, 2, 1};
	u_int32_t packet_time;
	struct timeval kerneltimeval;
	time_t packet_local_time;

	/* if kerneltime=1, we don't read the skb->timestamp but kernel time instead */
	/* we all use kernel time ,because time precision is not so important, so we not need modify skb struct*/
    #if 0
	if (info->kerneltime)
    #endif
	{
		do_gettimeofday(&kerneltimeval);
		packet_local_time = kerneltimeval.tv_sec;
	}
    #if 0
	else
		packet_local_time = skb->stamp.tv_sec;
    #endif

#ifdef CONFIG_DST
    if(time_check_dst(packet_local_time-60 * sys_tz.tz_minuteswest))
    {
        packet_local_time += 3600;
    }
#endif

	/* Transform the timestamp of the packet, in a human readable form */
	localtime(&packet_local_time, &currenttime, 1);
	DEBUG_MSG("currenttime: Y-%d M-%d D-%d H-%d M-%d S-%d, Day: W-%d\n",
		currenttime.tm_year, currenttime.tm_mon, currenttime.tm_mday,
		currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec,
		currenttime.tm_wday);

	/* check if we match this timestamp, we start by the days... */
	if (info->days_match != 0) {
		if ((days_of_week[currenttime.tm_wday] & info->days_match) != days_of_week[currenttime.tm_wday])
			return false; /* the day doesn't match */
	}

	/* check the time now */
	packet_time = (currenttime.tm_hour * 60 * 60) + currenttime.tm_min * 60 + currenttime.tm_sec;

    if (info->time_start < info->time_stop) {
		if (packet_time < info->time_start ||
		    packet_time > info->time_stop)
			return false;
	} else {
		if (packet_time < info->time_stop &&
		    packet_time > info->time_stop)
			return false;
	}

	/* here we match ! */
	return true;
}

static bool ebt_time_mt_check(const struct xt_mtchk_param *par)
{
	const struct ebt_time_info *info = par->matchinfo;

	/* First, check that we are in the correct hook */
	/* PRE_ROUTING, LOCAL_IN or FROWARD */
#if 0
	if (hookmask
            & ~((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_IN) | (1 << NF_BR_FORWARD) | (1 << NF_BR_LOCAL_OUT)))
	{
		printk("ebt_time: error, only valid for PRE_ROUTING, LOCAL_IN, FORWARD and OUTPUT)\n");
		return -EFAULT;
	}
#endif

#if 0
	/* we use the kerneltime if we are in forward or output */
	//info->kerneltime = 1;

	if (hookmask & ~((1 << NF_BR_FORWARD) | (1 << NF_BR_LOCAL_OUT)))
		/* if not, we use the skb time */
		info->kerneltime = 0;
#endif

	/* Now check the coherence of the data ... */
	if ((info->time_start > (24 * 60 * 60 -1)) ||        /* 23:59:59*/
	    (info->time_stop  > (24 * 60 * 60 -1)))
	{
		printk(KERN_WARNING "ebt_time: invalid argument\n");
		return -EFAULT;
	}

	return 0;
}

static struct xt_match ebt_time_mt_reg __read_mostly = {
    .name           = "time",
    .revision	    = 0,
    .family		    = NFPROTO_BRIDGE,
    .match          = ebt_time_mt,
    .checkentry     = ebt_time_mt_check,
    .matchsize	    = XT_ALIGN(sizeof(struct ebt_time_info)),
    .me             = THIS_MODULE,
};

static int __init ebt_time_init(void)
{
	DEBUG_MSG("ebt_time loading\n");
	return xt_register_match(&ebt_time_mt_reg);
}

static void __exit ebt_time_fini(void)
{
	xt_unregister_match(&ebt_time_mt_reg);
	DEBUG_MSG("ebt_time unloaded\n");
}

module_init(ebt_time_init);
module_exit(ebt_time_fini);

#if 0
MODULE_PARM(debug, "0-1b");
MODULE_PARM_DESC(debug, "debug=1 is turn on debug messages");
#endif
MODULE_AUTHOR("Song Wang <songw@broadcom.com>");
MODULE_DESCRIPTION("Match timestamp");
MODULE_LICENSE("GPL");


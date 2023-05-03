/******************************************************************************
  Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

  �ļ�����: warnlog.h
  �ļ�����: �澯��־��
  �����б�: WARN_LOG
            WARN_setLevel
  �޶���¼:
         1. ����: ����
            ����: 2007-10-15
            ����: �����ļ�
         2. ����: Lvwz
            ����: 2007-11-30
            ����: �޸���־д��syslog�У�������TBS_TRACE���ڿ���ʱDEBUG
         3. ����: ղ��
            ����: 2008-06-18
            ����: ����ģ�� LOG �����壬�ο� src/alp/include/tbstype.h �汾 4642 �� MID ����
******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "tbserror.h"
#include "warnlog.h"
#include "tbstype.h"
#include "autoconf.h"

/* ͨ��mid��ȡ�±� */
#define MID2INDEX(mid) ((mid) & 0x00ff)

#define MAKE_MID_LEVEL(mid, lev)     \
    [MID2INDEX(mid)] = { #mid, mid, lev }

#define MAKE_MID_LEVEL_DEF(mid)     \
    [MID2INDEX(mid)] = { #mid, mid, LOG_NOTICE}


/* ģ�� LOG ������ */
ST_MODCFG g_astModLevel[] =
{
    MAKE_MID_LEVEL(MID_DEFAULT, LOG_DEBUG), /* Ĭ�ϼ��� */

    /* PID_CCP */
    MAKE_MID_LEVEL_DEF(MID_CCP),
    MAKE_MID_LEVEL_DEF(MID_AUTH),           /* ��ȫ��֤ģ�� */
    MAKE_MID_LEVEL_DEF(MID_IGMP),           /* IGMP */
    MAKE_MID_LEVEL_DEF(MID_CMM),            /* CMM */
    MAKE_MID_LEVEL_DEF(MID_LAN),            /* LAN */
    MAKE_MID_LEVEL_DEF(MID_IPT),            /* IPT */
    MAKE_MID_LEVEL_DEF(MID_ETHLAN),         /* LAN����̫�� */
    MAKE_MID_LEVEL_DEF(MID_ETHWAN),         /* WAN����̫�� */
    MAKE_MID_LEVEL_DEF(MID_PPPOE),          /* PPPOE */
    MAKE_MID_LEVEL_DEF(MID_WLAN),           /* WLANģ�� */
    MAKE_MID_LEVEL_DEF(MID_TR069BE),        /* TR069BE */
    MAKE_MID_LEVEL_DEF(MID_DGN),            /* Diagnostics */
    MAKE_MID_LEVEL_DEF(MID_DHCPR),          /* dhcp relay */
    MAKE_MID_LEVEL_DEF(MID_DHCPS),          /* dhcp server */
    MAKE_MID_LEVEL_DEF(MID_TIMER),          /* Timer */
    MAKE_MID_LEVEL_DEF(MID_IPCONN),         /* wan ip connection module */
    MAKE_MID_LEVEL_DEF(MID_FIREWALL),       /* Fire wall */
    MAKE_MID_LEVEL_DEF(MID_SNMPC),          /* SNMP���ù���ģ�� */
#if (defined(CONFIG_RT63365) || defined(CONFIG_RT63368))
    MAKE_MID_LEVEL_DEF(MID_UTMPROXY),       /* MID_UTMPROXY */
    MAKE_MID_LEVEL_DEF(MID_AUTOUPGRADE),    /* MID_AUTOUPGRADE */
#endif
//    MAKE_MID_LEVEL_DEF(MID_MAIL),           /* �ʼ�����ģ�� */
    MAKE_MID_LEVEL_DEF(MID_QOS),            /* QOSģ�� */
    MAKE_MID_LEVEL_DEF(MID_STATIC_ROUTING), /* static routing */
    MAKE_MID_LEVEL_DEF(MID_VDSL),           /* VDSLģ�� */
    MAKE_MID_LEVEL_DEF(MID_DNS),            /* DNSģ�� */
    MAKE_MID_LEVEL_DEF(MID_ALG),            /* ALGģ�� */
    MAKE_MID_LEVEL_DEF(MID_WAN),            /* WANģ�� */
    MAKE_MID_LEVEL_DEF(MID_DROUTE),         /* dynamic routing */
    MAKE_MID_LEVEL_DEF(MID_SNTP),           /* sntp */
    MAKE_MID_LEVEL_DEF(MID_VLAN),           /* vlan */
    MAKE_MID_LEVEL_DEF(MID_USB_MASS),       /* USB mass storage module */
    MAKE_MID_LEVEL_DEF(MID_LOG),            /* LOGGER ģ�� */
    MAKE_MID_LEVEL_DEF(MID_FTPD),           /* FTPD module */
    MAKE_MID_LEVEL_DEF(MID_NATPRIO),        /* NAT ���ȼ� */
    MAKE_MID_LEVEL_DEF(MID_WPS),            /* WPSģ�� */
    MAKE_MID_LEVEL_DEF(MID_ACL),            /* ACL */
    MAKE_MID_LEVEL_DEF(MID_UPNP),           /* UPNP */
    MAKE_MID_LEVEL_DEF(MID_LSVLAN),         /* LSVLAN */
    MAKE_MID_LEVEL_DEF(MID_PORTOFF),        /* PORTOFF:�˿ڸ��� */
    MAKE_MID_LEVEL_DEF(MID_ANTIATTACK),     /* ANTIATTACK:������ */
    MAKE_MID_LEVEL_DEF(MID_DEVINFO),        /* DEVINFOģ�� */
    MAKE_MID_LEVEL_DEF(MID_PORTMAPPING),    /* PortMappingģ�� */
    MAKE_MID_LEVEL_DEF(MID_URLFILTER),      /* URL FILTER */
    MAKE_MID_LEVEL_DEF(MID_ATM),            /* ATMģ�� */
    MAKE_MID_LEVEL_DEF(MID_DSL),            /* DSLģ�� */
    MAKE_MID_LEVEL_DEF(MID_DDNS),           /* DDNSģ�� */
    MAKE_MID_LEVEL_DEF(MID_PROUTE),         /* ����·�� */
    MAKE_MID_LEVEL_DEF(MID_CFG),            /* cfg�ָ�����Ĭ�ϱ����ؼ����� */
    MAKE_MID_LEVEL_DEF(MID_SUPP),           /* SUPPģ�� */
    MAKE_MID_LEVEL_DEF(MID_MACFILTER),      /* MACFILTERģ�� */
    MAKE_MID_LEVEL_DEF(MID_TRACERT),        /* TRACERTģ��*/
    MAKE_MID_LEVEL_DEF(MID_IPPD),           /* IPPDģ��*/
    MAKE_MID_LEVEL_DEF(MID_WEBP),           /* web����ģ�� */
    MAKE_MID_LEVEL_DEF(MID_BRIDGE_FILTER),  /* bridgefilterģ�� */
    MAKE_MID_LEVEL_DEF(MID_SCHED),			/* schedules */
    MAKE_MID_LEVEL_DEF(MID_PORTTRIGGER),    /* �˿ڴ��� */
    MAKE_MID_LEVEL_DEF(MID_IP_ACL),         /* IP_ACL */
    MAKE_MID_LEVEL_DEF(MID_DEFAULTGW),      /* Ĭ������ */
    MAKE_MID_LEVEL_DEF(MID_DIAG),           /* Diagnostics */
    MAKE_MID_LEVEL_DEF(MID_WANSELECT),      /* WAN Select */
    MAKE_MID_LEVEL_DEF(MID_DEVCONFIG),      /* dev config */

    MAKE_MID_LEVEL_DEF(MID_TRADITIONALNAT),     /* MID_TRADITIONALNAT */
    MAKE_MID_LEVEL_DEF(MID_FIREWALLLOG),        /* MID_FIREWALLLOG */
    MAKE_MID_LEVEL_DEF(MID_IPMACFILTER),        /* MID_IPMACFILTER */
    MAKE_MID_LEVEL_DEF(MID_UDPECHO),            /* MID_UDPECHO */
    MAKE_MID_LEVEL_DEF(MID_DOWNLOADDIAG),       /* MID_DOWNLOADDIAG */
    MAKE_MID_LEVEL_DEF(MID_UPLOADDIAG),         /* MID_UPLOADDIAG */
    MAKE_MID_LEVEL_DEF(MID_SAMBA),              /* MID_SAMBA */
    MAKE_MID_LEVEL_DEF(MID_USB3G),              /* MID_USB3G */
    MAKE_MID_LEVEL_DEF(MID_TF_PORTMAPPING),     /* MID_TF_PORTMAPPING */

    MAKE_MID_LEVEL_DEF(MID_SPT),                /* MID_SPT */
    MAKE_MID_LEVEL_DEF(MID_TR064C),             /* MID_TR064C */

//ipv6
    MAKE_MID_LEVEL_DEF(MID_DHCP6S),			/* dhcp6sģ�� */
    MAKE_MID_LEVEL_DEF(MID_V6CONN),			/* ipv6 connectionģ�� */
    MAKE_MID_LEVEL_DEF(MID_RAD),			/* router advertisement daemonģ�� */
    MAKE_MID_LEVEL_DEF(MID_V6SROUTE),	    /* ipv6 static routing */
    MAKE_MID_LEVEL_DEF(MID_IPV6ENABLE),     /* ipv6 enabled */

    MAKE_MID_LEVEL_DEF(MID_TF_FIREWALL),    /* MID_TF_FIREWALL */

    MAKE_MID_LEVEL_DEF(MID_OS_INFO),        /* MID_OS_INFO */
    MAKE_MID_LEVEL_DEF(MID_LPVLAN),         /* MID_LPVLAN */

    MAKE_MID_LEVEL_DEF(MID_DLNA),           /* MID_DLNA */
    MAKE_MID_LEVEL_DEF(MID_IPTUNNEL),       /* MID_IPTUNNEL */
    MAKE_MID_LEVEL_DEF(MID_MLD),			/* MLD IPV6��IGMP */
    MAKE_MID_LEVEL_DEF(MID_TF_GUI),         /* MID_TF_GUI */
    MAKE_MID_LEVEL_DEF(MID_NEW3G),          /* MID_NEW3G */
    MAKE_MID_LEVEL_DEF(MID_NEW3GPIN),       /* MID_NEW3GPIN */
    MAKE_MID_LEVEL_DEF(MID_NEW3GPINRESULT), /* MID_NEW3GPINRESULT */
    MAKE_MID_LEVEL_DEF(MID_NEW3GINFO),      /* MID_NEW3GINFO */
    MAKE_MID_LEVEL_DEF(MID_CTMWBE),         /* MID_CTMWBE */
    MAKE_MID_LEVEL_DEF(MID_WANMIRROR),      /* MID_WANMIRROR */
};

char szPrintBuf[BUFSIZ];

/*
void get_time(char *szTime)
{
    struct tm *stTime;
    time_t timeCount;

    timeCount = time(NULL);
    stTime = (struct tm *)localtime(&timeCount);
    sprintf(szTime, "%02d:%02d:%02d", stTime->tm_hour, stTime->tm_min, stTime->tm_sec);
}
*/


#define SIZEOF(array)       (sizeof(array) / sizeof(array[0]))

unsigned char WARN_GetLevel(unsigned short usMID)
{
    unsigned int i = MID2INDEX(usMID);

    if (usMID == g_astModLevel[i].usMID)
    {
        return g_astModLevel[i].ucLevel;
    }

    return WARN_LEVEL_DEBUG;
}

void WARN_LOG(unsigned char ucLevel, unsigned short usMID, char *fmt, ...)
{
    va_list args;

    /*��֤�澯����*/
    if (ucLevel > WARN_GetLevel(MID_DEFAULT) || ucLevel > WARN_GetLevel(usMID))
    {
        return;
    }

    va_start(args, fmt);
    (void)vsnprintf(szPrintBuf, BUFSIZ, fmt, args);
    va_end (args);

#ifdef CONFIG_TBS_APPS_DEBUG
    printf("[%04x] %s\n", usMID, szPrintBuf);
#endif

    syslog(ucLevel, "[%04x] %s", usMID, szPrintBuf);
}

/* TBS_Log ������ LOG Level */
void TBS_Log(unsigned char ucSeverity, unsigned short usMID,
             const char *pcAccessor, const char *pcMethod, const char *pcPara,
             const char *pcResult, const char *pcRemark, ...)
{
#if 0
    int iLen = 0;
    char *szFormat = "Accessor:[%s] Method:[%s] Para:[%s] Result:[%s] ";
    va_list args;

    snprintf(szPrintBuf + iLen, BUFSIZ - iLen, szFormat,
            pcAccessor ? pcAccessor : "",
            pcMethod ? pcMethod : "",
            pcPara ? pcPara : "",
            pcResult ? pcResult : "");
    iLen = strlen(szPrintBuf);

    va_start(args, pcRemark);
    (void)vsnprintf(szPrintBuf + iLen, BUFSIZ - iLen, pcRemark, args);
    va_end (args);

    #ifdef LOGGER_SYSEVT_LOG
    syslog(LOG_SYSEVT | ucSeverity, "%s", szPrintBuf);
    #else
    syslog(LOG_LOCAL0 | ucSeverity, "%s", szPrintBuf);
    #endif
#else
	return;
#endif
}

/* TBS_Log ������ LOG Level */
void TBS_Log_new(unsigned char ucSeverity1, unsigned char ucSeverity2,const char *pcRemark, ...)
{
    int iLen = 0;
    char *szFormat = "[%d][%d] ";
    va_list args;

    snprintf(szPrintBuf + iLen, BUFSIZ - iLen, szFormat,
            ucSeverity1,
            ucSeverity2);
    iLen = strlen(szPrintBuf);

    va_start(args, pcRemark);
    (void)vsnprintf(szPrintBuf + iLen, BUFSIZ - iLen, pcRemark, args);
    va_end (args);

    #ifdef LOGGER_SYSEVT_LOG
    syslog(LOG_SYSEVT | ucSeverity1, "%s", szPrintBuf);
    #else
    syslog(LOG_LOCAL0 | ucSeverity1, "%s", szPrintBuf);
    #endif
}



int WARN_SetLevel(unsigned short usMID, unsigned char ucLevel)
{
    int iFound = 0;
    unsigned int i = MID2INDEX(usMID);

    if (usMID == g_astModLevel[i].usMID)
    {
        iFound = 1;
        g_astModLevel[i].ucLevel = ucLevel;
        return TBS_SUCCESS;
    }

    if (!iFound)
    {
        //fprintf(stderr, "WARN_SetLevel: Unknown usMID(%04x), ucLevel(%u)\n", usMID, ucLevel);
    }

    return TBS_FAILED;
}

int WARN_SetLevelByName(const char *szModName, unsigned char ucLevel)
    {
    int iFound = 0;
    unsigned int i = 0;

    for (i = 0; i < SIZEOF(g_astModLevel); i++)
    {
        if (g_astModLevel[i].szModName && strcasestr(g_astModLevel[i].szModName, szModName))
        {
            iFound = 1;
            g_astModLevel[i].ucLevel = ucLevel;
            return TBS_SUCCESS;
        }
    }

    if (!iFound)
    {
        fprintf(stderr, "WARN_SetLevelByName: Unknown szMod(%s), ucLevel(%u)\n", szModName, ucLevel);
    }

    return TBS_FAILED;
}



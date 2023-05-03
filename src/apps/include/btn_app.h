/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : btn_app.h
 �ļ����� : ���ļ�ΪӦ�ò��ṩ������ť��Ϣ����Ҫ�ĸ��ֶ���
 �����б� :


 �޶���¼ :
          1 ���� : ����
            ���� : 2008-11-05
            ���� :

**********************************************************************/
#ifndef CM_BTN_H
#define CM_BTN_H

#include <btn.h>
#include "warnlog.h"


/******************************************************************************
*                                 MACRO
******************************************************************************/

/* ������Ϣ�� */
//#define BTN_DEBUG

#ifdef BTN_DEBUG
#define BTN_TRACE(fmt, args...)  COMMON_TRACE(MID_LBT, fmt, ##args)
#else
#define BTN_TRACE(fmt, args...)
#endif

#define MAX_MSGSIZE	    1024
#define BLP_CMM_GROUP   0


/******************************************************************************
*                                 ENUM
******************************************************************************/
/* ��ťģʽ */
typedef enum {
	CFG_APP_BTN_MODE_UNKNOW = 0,
	CFG_APP_BTN_MODE_DIRECT,
	CFG_APP_BTN_MODE_RELEASE_LESS,
	CFG_APP_BTN_MODE_RELEASE_GREATER,
	CFG_APP_BTN_MODE_RELEASE_BETWEEN,
	CFG_APP_BTN_MODE_TIMER,
}APP_BTN_MODE;

/* ��ť�¼� */
typedef enum  {
	APP_BTN_EVEN_DOWN, 		/*ͨ����Ϣ:����CFG_APP_BTN_MODE_DIRECTģʽ*/
	APP_BTN_EVEN_UP,		/*ͨ����Ϣ:����CFG_APP_BTN_MODE_DIRECTģʽ*/
	APP_BTN_EVEN_REBOOT,	/*�Զ�����Ϣ:����reset��ť*/
	APP_BTN_EVEN_RESET,		/*�Զ�����Ϣ:����reset��ť*/
	APP_BTN_WPS_RESET,		/*�Զ�����Ϣ:���ڰ�ʱ�����ֵ�WPS�¼���RESET�¼�*/
	APP_BTN_WPS,
	APP_BTN_EVEN_WLAN0_WPS,	/*�Զ�����Ϣ:��������WPS(2.4G)*/
	APP_BTN_EVEN_WLAN1_WPS,	/*�Զ�����Ϣ:��������WPS(5.8G)*/
	APP_BTN_EVEN_WIFI	/*�Զ�����Ϣ:��������WPS(5.8G)*/
}APP_BTN_EVEN;

struct APP_BTN_MSG {
	btn_name Name;			/* btn_name �����ڵײ�btn.h */
	APP_BTN_EVEN Even;
};

struct APP_BTN_CFG {
	btn_name Name;
	APP_BTN_MODE Mode;
	long int Time;
    long int TimeG;
	APP_BTN_EVEN Even;		/*�Զ�����*/
};


/******************************************************************************
*                               FUNCTION DECLARE                              *
******************************************************************************/
/* ��ʼ��BTN */
int BTN_ModuleInit(void);

/* ע��BTN */
void BTN_ModuleDestory(void);

/* BTN���������� */
int BTN_MainProc(void);

/* BTN��ʱ���� */
int BTN_procTick(void);

/* ��ȡBTN������NETLINK�׽��� */
int BTN_GetMsgSock(void);


#endif

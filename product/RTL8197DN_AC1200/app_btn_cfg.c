/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : app_btn_cfg.c
 �ļ����� : ���ļ�����Ӧ�ò㰴ť������Ϣ�����ļ����ᱻ���ӵ� build/app/ssap/btn Ŀ¼�±���
 �����б� : 

 
 �޶���¼ :
          1 ���� : ����
            ���� : 2008-11-07
            ���� : �����ĵ�

**********************************************************************/

#include <btn.h>
#include <btn_app.h>


struct APP_BTN_CFG AppBtnCfg[] =
{
	//{btn_wps_and_reset,CFG_APP_BTN_MODE_TIMER,0,0,APP_BTN_WPS_RESET},			/* RESET��WPS���ð�ť: WPS�¼�(250ms-3m),Reset�¼�(>=15s)*/
    {btn_wps,CFG_APP_BTN_MODE_RELEASE_GREATER,2000,0,APP_BTN_WPS},/* WPS��ť(2.4G����) */
	{btn_reset,CFG_APP_BTN_MODE_RELEASE_GREATER,5000,0,APP_BTN_EVEN_RESET},/* WPS��ť(2.4G����) */
    //{btn_wps_5g,CFG_APP_BTN_MODE_RELEASE_BETWEEN,250,3000,APP_BTN_EVEN_WLAN1_WPS}, /* WPS��ť(5.8G����) */
    //{btn_wlan,CFG_APP_BTN_MODE_DIRECT,0,0},                           		/* CFG_APP_BTN_MODE_DIRECTģʽ����Ҫ����Even�� */
    //{btn_reset,CFG_APP_BTN_MODE_RELEASE_LESS,2,APP_BTN_EVEN_REBOOT},  		/* RESET��ť:���º��ɿ���С��3�봥��APP_BTN_EVEN_REBOOT�¼� */
    //{btn_reset,CFG_APP_BTN_MODE_RELEASE_GREATER,3000,0,APP_BTN_EVEN_RESET},	/* RESET��ť:���º��ɿ�������5�봥��APP_BTN_EVEN_RESET�¼� */
    //{btn_reset,CFG_APP_BTN_MODE_TIMER,8,APP_BTN_EVEN_RESET},          		/* RESET��ť:���²���8�봥��APP_BTN_EVEN_RESET�¼� */
	{btn_wlan,CFG_APP_BTN_MODE_RELEASE_GREATER,2000,0,APP_BTN_EVEN_WIFI},/* WPS��ť(2.4G����) */
	{btn_end,0,0,0},                                                  			/* btn_end:���������־ */
};


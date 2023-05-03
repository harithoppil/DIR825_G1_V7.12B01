/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : app_btn_cfg.c
 文件描述 : 本文件保存应用层按钮配置信息，此文件将会被连接到 build/app/ssap/btn 目录下编译
 函数列表 : 

 
 修订记录 :
          1 创建 : 张喻
            日期 : 2008-11-07
            描述 : 创建文档

**********************************************************************/

#include <btn.h>
#include <btn_app.h>


struct APP_BTN_CFG AppBtnCfg[] =
{
	//{btn_wps_and_reset,CFG_APP_BTN_MODE_TIMER,0,0,APP_BTN_WPS_RESET},			/* RESET与WPS复用按钮: WPS事件(250ms-3m),Reset事件(>=15s)*/
    {btn_wps,CFG_APP_BTN_MODE_RELEASE_GREATER,2000,0,APP_BTN_WPS},/* WPS按钮(2.4G下行) */
	{btn_reset,CFG_APP_BTN_MODE_RELEASE_GREATER,5000,0,APP_BTN_EVEN_RESET},/* WPS按钮(2.4G下行) */
    //{btn_wps_5g,CFG_APP_BTN_MODE_RELEASE_BETWEEN,250,3000,APP_BTN_EVEN_WLAN1_WPS}, /* WPS按钮(5.8G下行) */
    //{btn_wlan,CFG_APP_BTN_MODE_DIRECT,0,0},                           		/* CFG_APP_BTN_MODE_DIRECT模式不需要定义Even域 */
    //{btn_reset,CFG_APP_BTN_MODE_RELEASE_LESS,2,APP_BTN_EVEN_REBOOT},  		/* RESET按钮:按下后松开，小于3秒触发APP_BTN_EVEN_REBOOT事件 */
    //{btn_reset,CFG_APP_BTN_MODE_RELEASE_GREATER,3000,0,APP_BTN_EVEN_RESET},	/* RESET按钮:按下后松开，大于5秒触发APP_BTN_EVEN_RESET事件 */
    //{btn_reset,CFG_APP_BTN_MODE_TIMER,8,APP_BTN_EVEN_RESET},          		/* RESET按钮:按下不动8秒触发APP_BTN_EVEN_RESET事件 */
	{btn_wlan,CFG_APP_BTN_MODE_RELEASE_GREATER,2000,0,APP_BTN_EVEN_WIFI},/* WPS按钮(2.4G下行) */
	{btn_end,0,0,0},                                                  			/* btn_end:数组结束标志 */
};


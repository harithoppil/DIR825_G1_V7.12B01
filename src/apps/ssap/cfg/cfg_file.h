/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfgapi.h
 �ļ����� : cfgʵ���ļ���д���ñ���ָ��ȹ��ܵ�ͷ�ļ�


 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2007-11-23
            ���� :

**********************************************************************/

#ifndef __CFGFILE_H__
#define __CFGFILE_H__

#include "cfg_api.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif



/******************************************************************************
 *                                 MACRO                                      *
 ******************************************************************************/


#define CFG_DEF_FILE  "/etc/config.xml"     /* ȱʡ�����ļ� */
#define CFG_FULL_FILE "/etc/config_full.xml"
#define CFG_DEBUG_FILE "/tmp/cfg.xml"

#define CFG_WRITE_MOD    0666               /* ��д�����ļ��ı�־ */

#define CFG_FILE_NAME_LEN 32
#define CFG_FILE_KEY_LEN  8

#define MAGICNUM 0x434F4E46UL               /* �����ļ�ǰ��ħ����־"CONF" */



#define CFG_COMM_MTD_LEN   (1024*20)    /* MTD�����ļ�Ԥ�Ƴ��� */

#define CFG_MAX_MTD_LEN   0xffff    /* MTD�����ļ���󳤶� */



/******************************************************************************
 *                                STRUCT                                      *
 ******************************************************************************/



/* �����ļ�ʱ���ʽ */
typedef struct
{
    unsigned long year : 6;
    unsigned long mon  : 4;
    unsigned long date : 5;
    unsigned long hour : 5;
    unsigned long minu : 6;
    unsigned long sec  : 6;

} ST_CFG_TIME;


/* �����ļ���Ÿ�ʽ */
typedef struct
{
    unsigned long ulMagicNum;
    unsigned long ulVer;
    ST_CFG_TIME   stTime;
    char          acKey[CFG_FILE_KEY_LEN];
    unsigned long ulUncompLen;
    unsigned long ulCompLen;
    unsigned long ulCrc;
    char          acData[0];

} ST_CFG_FILE;


/******************************************************************************
 *                               GLOBAL                                       *
 ******************************************************************************/
/*
  ��ԭ����Ĭ������֮�󣬲�������дflash
  ֻ�������������� add by pengyao 20121015
*/
extern int g_bResetedFlag;


/******************************************************************************
 *                               FUNCTION                                     *
 ******************************************************************************/
CFG_RET CFG_ReadFile(const char *pcFile, char **ppcBuf,
                     unsigned long ulPrevSpace, unsigned long ulPostSpace,
                     unsigned long *puLen);

CFG_RET CFG_WriteFile(const char *pcFile, const char *pcBuf, unsigned long ulLen);


/* ��ȡ��ǰ���� */
CFG_RET CFG_ReadCurCfg(const char *pcItem, char **ppcBuf, unsigned long ulPrevSpace,
                       unsigned long ulPostSpace, unsigned long *pulLen);

/* ��ȡȱʡ���� */
CFG_RET CFG_ReadDefaultCfg(char **ppcBuf, unsigned long ulPrevSpace,
                           unsigned long ulPostSpace, unsigned long *pulLen);
CFG_RET CFG_ReadFullCfg(char **ppcBuf, unsigned long ulPrevSpace,
                           unsigned long ulPostSpace, unsigned long *pulLen);


/* �ָ��������� */
CFG_RET CFG_RecoverFactCfg(const char *pcAccessor);


/* ���浱ǰ���� */
CFG_RET CFG_SaveCurCfg(const char *pcItem, char *pcCfg, unsigned long ulLen);


/* �����ǰ���� */
CFG_RET CFG_ClearCurCfg(void);

#ifdef CONFIG_APPS_LOGIC_WANSELECT
CFG_RET CFG_ClearAllTheCfg(void);
#endif

typedef CFG_RET (*FUNC_CFG_SPEC_RECOVER)(void);


/******************************************************************************
 *                                 END                                        *
 ******************************************************************************/

int app_item_get( void *data , char *item_name ,unsigned short *len );
int app_item_save( void *data , char *item_name ,unsigned short len );


#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


#endif /* __CFGFILE_H__ */

/*****************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ph-client.h
 �ļ����� : �����ǿͻ���(peanuthullc)��������ļ�����Ϣ����

 �����б� :

 �޶���¼ :
            ���� :  Kevin
            ���� :  2009-11-18
            ���� :

******************************************************************************/

#ifndef __PH_CLIENT_H
#define __PH_CLIENT_H

#include "stdio.h"

/******************************************************************************
                        *                                 �궨��                                  *
 ******************************************************************************/
 /*�����ǿͻ�����Ϣ(For D-link)*/
 #define PHCLIENT_ID                     9857               /*0X2681*/
 #define PHCLIENT_KEY                   503842356       /*0X1E080634*/
 #define PHCLIENT_VERSION            38726             /*0X9746*/
 #define PHCLIENT_VERSION_STR     "9.7.4.6"          /*VERSION:38726*/
 #define PHCLIENT_INFO		0x26819746	/* �ͻ�����Ϣ��4���ֽڣ�ǰ��λΪǶ��ʽ�Ŀͻ��ţ�����λΪ�ͻ��˰汾��*/
 
 /*���ĵĻ��б�ʶ*/
#define	NEWLINE_R_N	"\015\012"	//\r\n

/*�����������󳤶�*/
 #define MAX_STRING_LEN            256    /*һ��������󳤶�*/
 #define MAX_AUTH_LEN               16
 #define MAX_PASSPORT_LEN        32      /*���ռ��������󳤶�*/
 #define MAX_MD5PSW_LEN          128
 #define MAX_BUF_LEN                 4096
 
 /*�����Ƿ�������Ϣ*/
 #define PH_SERVER_URL                         "hphwebservice.oray.net"   /*WebService address*/
 #define HTTP_PROTOCOL_PORT                "80"
 #define TCP_PROTOCOL_PORT_6060         "6060"
 #define UDP_PROTOCOL_PORT_6060         "6060"
 /*�����ֶ�*/
 #define PH_SERVER_IP                            "PHServer"    /*������IP��ַ*/
 #define PH_USER_TYPE                           "UserType"    /*�û�����*/

 /*�ո�*/
 #define CHAR_BLANK                          32         

 /*�����Ƿ�������Ӧ��*/
 #define PH_RESPOND_LEN                 3      /*��������Ӧ��Ϊ3λ*/
 #define PH_TCP_OK                          "220"
 #define PH_AUTH_ROUTE_OK            "334"
 #define PH_AUTH_OK                       "250" 
 #define PH_AUTH_FAIL                     "535"
 #define PH_REGISTER_FAIL               "507"       /*no name registered*/
 #define PH_UPDATE_OK                    "50"


 /*�����ǿͻ�������*/
 #define PHC_AUTH_ROUTE                        "AUTH ROUTER"
 #define PHC_CNFM                                   "CNFM"
 #define PHC_QUIT                                    "QUIT"
 #define PHC_UPDATE                                10
 #define PHC_LOGOUT                                11
  #define PH_UPDATE_FAIL                         1000
 #define PH_LOGOUT_OK                            51

/*���û����ͼ����������*/
#define STANDARD_USER                      0
#define ADVANCED_USER                     1
#define STANDARD_HEARTBEAT            60
#define ADVANCED_HEARTBEAT           30

/******************************************************************************
                        *                                 ö�����Ͷ���                                    *
 ******************************************************************************/
/*����״̬*/
typedef enum
{
    PROC_UPDATE_FAIL = -3,
    PROC_AUTH_FAIL = -2,
    PROC_LOGIN_FAIL = -1,
    PROC_NORMAL = 0,          /* NORMAL*/
} PROC_STATUS;


/******************************************************************************
                        *                                �ṹ�嶨��                                  *
 ******************************************************************************/
/*�����ǿͻ��˸�����Ϣ�ṹ��*/
typedef struct
{
    long lSessionID;
    long lOptionCode;
    long lSerialNum;
    long lCheckSum;
    long lAppend;
}ST_PH_UDP_SEND_DATE;

typedef struct
{
    ST_PH_UDP_SEND_DATE stUDPDate;
    long lIP;
}ST_PH_UDP_REC_DATE;


 /*****************************************************************************
                        *                            ��������                          *
 ******************************************************************************/
static void Usage(char *psProName);
static int PH_init(void);
static int PH_login(const char *pcsPassport, const char *pcsPassword);
static int GetUserInfo(char *psBuf, const char* pcsPassport, const char *pcsPassword);
static int GetValueFromXML(const char *pcsMsg, const char *pcsKeyword, char *psBuf, int iBufLen);

#endif /* __PH_CLIENT_H */

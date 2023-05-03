/*****************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ph-client.c
 �ļ����� : �����ǿͻ���(peanuthullc)���̵�ʵ��

 �����б� :

 �޶���¼ :
            ���� :  Kevin
            ���� :  2009-11-18
            ���� :  ����

            ��ע :  �ó������������л����������ֽ���Ϊ��˵������
                           ������л����������ֽ���ΪС�ˣ�����Ҫ��������
                           ���ִ���������ֽ���
******************************************************************************/
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ph-client.h"
#include "ph_encrypt.h"
#include "ph_socket.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

/******************************************************************************
                        *                               ȫ�ֱ���                               *
 ******************************************************************************/
 /*�����Ƿ�����IP��ַ*/
static char g_szServerIP[MAX_STRING_LEN] = {0};

/*�����Ƿ��������ص���ս��*/
static unsigned char g_uszKey[MAX_AUTH_LEN] = {0};

/*�����ǿͻ��˸�����Ϣ�ṹ��*/
ST_PH_UDP_SEND_DATE g_stClientInfo;

/*�����ǿͻ�������*/
static int g_iClientType;

/*�����ǿͻ��˸���״̬*/
static int g_iClientState;
/*****************************************************************************
                        *                                 ��������                            *
 ******************************************************************************/
/******************************************************************************
����: ��long�ͱ������иߵ��ֽ�ת��
����: lOldValue               input          ��Ҫת���ı���
����: ת���������ֵ
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static long LongOrderRevers(long lOldValue)
{
    struct stLongStruct
    {
        unsigned long uiZero:8;
        unsigned long uiOne:8;
        unsigned long uiTwo:8;
        unsigned long uiThree:8;
    };

    unsigned char ucTemp = '\0';

    struct stLongStruct *pstNewValue = (struct stLongStruct*)&lOldValue;

    ucTemp = pstNewValue->uiZero;
    pstNewValue->uiZero = pstNewValue->uiThree;
    pstNewValue->uiThree = ucTemp;

    ucTemp = pstNewValue->uiOne;
    pstNewValue->uiOne = pstNewValue->uiTwo;
    pstNewValue->uiTwo = ucTemp;

    return *(long *)pstNewValue;
}
/******************************************************************************
����: ���ַ�����ǰ��ȥ��ָ�����ַ�
����: psString               input          Դ�ַ���
             cTrim                   input         ��Ҫȥ�����ַ�
����: ��
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static void StrTrim(char *psString, char cTrim)
{
    char *psTemp = psString;
    char *psBegin = NULL;
    int i = 0;
    int iLen = strlen(psString);

    if (psString)
    {
        /*ȥ��ĩβ*/
        for (i = iLen-1; i >=0; i--) 
        {
            if (cTrim != psTemp[i])
            {
                psTemp[i+1] = '\0';
                break;
            }
        }

        /*ȥ��ǰ��*/
        for (i = 0; i < iLen; i++)
        {
            if (cTrim != psTemp[i])
            {
                psBegin = psTemp+i;
                break;
            }
        }

        if (psBegin && (psBegin != psTemp) )
        {
            while(*psBegin)
                *psTemp++ = *psBegin++;
               
                *psTemp = '\0';
        }
       
    }/*end of  if (psString)*/

    return;
}
/******************************************************************************
����: ��ָ����socket�������У�ע����������ص�����
����: fd                input          socket������
             psBuf           input         ���������ص�У����
����: �ɹ�/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int RegisterDomain(int fd, char *psBuf)
{
    int iLen = 0;
    char *pDomain =  NULL;
    char szCMD[MAX_STRING_LEN] = {0};
    char szBuffer[MAX_STRING_LEN] = {0};
    char szDomain[MAX_BUF_LEN] = {0};

    PH_TRACE_INTO_FUNC;

    /*�ж��Ƿ���֤�ɹ�*/
    if (0 == strncmp(psBuf,PH_AUTH_FAIL,PH_RESPOND_LEN))
    {
        PH_TRACE("Authentication failure.\n");
        close(fd);
        return PROC_AUTH_FAIL;
    }

    if (0 == strncmp(psBuf,PH_AUTH_OK,PH_RESPOND_LEN))
    {
        pDomain = (char *)strstr(psBuf, NEWLINE_R_N);
        pDomain += strlen(NEWLINE_R_N);
        pDomain = (char *)strtok(pDomain, NEWLINE_R_N);

        /*ע����������ص���������*/
        while(pDomain)
        {
            /*�������ע������*/
            sprintf(szCMD, "REGI A %s"NEWLINE_R_N, pDomain);
            strcat(szDomain,pDomain);
            strcat(szDomain,";");
            TCP_SendTo(fd, szCMD);

            /*��ȡע����*/
            iLen = TCP_ReadFromServer(fd, szBuffer, MAX_STRING_LEN);
            if (iLen > 0)
            {
                PH_TRACE("Register domain result :%s\n",szBuffer);
                if ( 0 != strncmp(szBuffer,PH_AUTH_OK,PH_RESPOND_LEN) )
                {
                    PH_TRACE("Register domain failed.\n");
                    close(fd);
                    return PROC_AUTH_FAIL;
                }
            }

            /*������ȡʣ�µ�����*/
            pDomain = (char *)strtok(NULL, NEWLINE_R_N);

            /*���û���������������˳�ѭ��*/
	    if ('.' == pDomain[0])	
	        break;
        }/*end of while(pDomain)*/
	
    }/*end of  if (0 == strncmp(psBuf,PH_AUTH_OK,3))*/

    /*�����ע�������������TBSƽ̨PC���̽��м��*/
    printf("Register Domain Name:%send",szDomain);

    PH_TRACE_OUT_FUNC;

    return PROC_NORMAL;
}
/******************************************************************************
����:����Ϣ�л�ȡָ���ֶε�ֵ
����: pcsMsg                input          ��ѯ����Ϣ����
             psKeyword           input          �ֶε�����
             psBuf                  output        ָ���ֶλ�ȡ��ֵ
             iBufLen                input          �����ֶ�ֵ����󳤶�  
����: �ֶεĳ���/0
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int GetValueFromXML(const char *pcsMsg, const char *pcsKeyword, char *psBuf, int iBufLen)
{
    char *psBegin= NULL;
    char *psEnd = NULL;
    char szTemp[MAX_STRING_LEN] = {0};
    int iLen = 0;

    PH_TRACE_INTO_FUNC;

    /*���ɹؼ��ֶε�ǰ׺*/
    snprintf(szTemp, sizeof(szTemp), "<%s>", pcsKeyword); 
    psBegin = strstr(pcsMsg, szTemp);
    if (psBegin)
    {
       psBegin += strlen(szTemp);

        /*���ɹؼ��ֵĺ�׺*/
        snprintf(szTemp, sizeof(szTemp), "</%s>", pcsKeyword);
        psEnd = strstr(psBegin, szTemp);
       
        iLen = (psEnd - psBegin);
        if ( (iLen > 0) && (iLen < iBufLen) )
        {
            strncpy(psBuf, psBegin, iLen);
            psBuf[iLen] = '\0';
        }
    }

    PH_TRACE_OUT_FUNC;
   
    return iLen;
}
/******************************************************************************
����: ���컨���ǿͻ���(peanuthullc)��֤����
����: psBuf                   output        ������ı�������
             pcsPassport          input          ����
             pcsPassword         input          ����(����)
             pcusAuthKey         input          ���������ص� ��ս��
����: ���ĵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int GetAuthInfo(char *psBuf, const char* pcsPassport, const char *pcsPassword, 
                                                                                 const unsigned char *pcusAuthKey)
{
    int iLen = 0;
    int iMoveBit = 0;
    long lEmbedAuthKey = 0;
    long lServerTime = 0;
    long lClientInfo = 0;
    long lReversal = 0;
    unsigned char uszMD5Password[MAX_PASSPORT_LEN] = {0};
    unsigned char szBuf[MAX_STRING_LEN] = {0};
    unsigned char *pCurrent = NULL;

    PH_TRACE_INTO_FUNC;

    /*ʹ��hmac_md5��������*/
    hmac_md5((char *)pcusAuthKey, (char *)pcsPassword, strlen(pcsPassword), uszMD5Password);
    PH_TRACE("uszMD5Password=%s,strlen(uszMD5Password)=%d\n",uszMD5Password,strlen(uszMD5Password));

    /*���У�����Ƕ����֤�빲4���ֽڣ�ΪǶ����֤�����������
    ǰʱ������õ���������ʱ������ս���ĵ����ֽں��4���ֽڣ�
    �㷨��Ƕ����֤�����ʱ���ȡ�����л������ѭ������һ��λ����
    ������λ�����÷�����ʱ������30��*/

    /*�ӷ��������ص���ս���л�ȡ������ʱ��ʱ��Ӧ��ע�������ֽ���*/
    lServerTime = *((long*)(pcusAuthKey + 6));
    lServerTime = LongOrderRevers(lServerTime);
    
    lServerTime |= ~(PHCLIENT_KEY);

    iMoveBit = lServerTime % 30; 
    lEmbedAuthKey = (lServerTime << (32 - iMoveBit)) | ((lServerTime >> iMoveBit) & ~(0xffffffff << (32 - iMoveBit)));

    /*�ͻ�����ϢҲ��4���ֽڣ�ǰ��λΪǶ��ʽ�Ŀͻ��ţ�����λΪ
    �ͻ��˰汾��*/
    lClientInfo = PHCLIENT_INFO;


    /*�˺���+һ���ո�+����Ƕ����֤��+�ͻ�����Ϣ+���ܴ�*/
    sprintf(szBuf, "%s ", pcsPassport);
    pCurrent = szBuf + strlen(szBuf);

    /*��дǶ����֤��Ϳͻ�����Ϣʱ����Ҫע�������ֽ���*/
    lReversal= LongOrderRevers(lEmbedAuthKey);
    memcpy(pCurrent, &lReversal, 4);
    pCurrent += 4;

    lReversal = LongOrderRevers(lClientInfo);
    memcpy(pCurrent, &lReversal, 4);
    pCurrent += 4;
    
    memcpy(pCurrent, uszMD5Password,strlen(uszMD5Password));
    pCurrent +=strlen(uszMD5Password);
    *pCurrent = '\0';
    iLen = pCurrent - szBuf;

    PH_TRACE("szBuf=%s,strlen(szBuf)=%d\n",szBuf,iLen);

    /*ʹ��Base64���м���*/
    Base64_Encode(szBuf, iLen, psBuf);
    strcat(psBuf,NEWLINE_R_N);

    PH_TRACE_OUT_FUNC;

    return strlen(psBuf);
}

/******************************************************************************
����: ���컨���ǿͻ���(peanuthullc)��¼����
����: psBuf                   output        ������ı�������
             pcsPassport          input          ����
             pcsPassword         input          ����(����)
����: ���ĵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int GetUserInfo(char *psBuf, const char* pcsPassport, const char *pcsPassword)
{
    int iLen = 0;
    char *pCurrent = psBuf;
    char szPswCrypt[MAX_MD5PSW_LEN] = {0};

    PH_TRACE_INTO_FUNC;

    /*ʹ��MD5���ܻ�������*/
    MD5String((char *)pcsPassword, szPswCrypt);
    PH_TRACE("szPswCrypt=%s\n",szPswCrypt);

    /*���챨������*/
    snprintf(pCurrent, MAX_BUF_LEN, "<?xml version=\"1.0\" encoding=\"utf-8\"?>"NEWLINE_R_N);
    iLen = strlen(psBuf);
    pCurrent += iLen;

    snprintf(pCurrent, MAX_BUF_LEN, "<soap:Envelope "	\
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "	\
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "	\
		"xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"	\
		"<soap:Header>"	\
		"<AuthHeader xmlns=\"http://tempuri.org/\">"	\
		"<Account>%s</Account>"	\
		"<Password>%s</Password>"	\
		"</AuthHeader>"	\
		"</soap:Header>"	\
		"<soap:Body>"	\
		"<GetMiscInfo xmlns=\"http://tempuri.org/\">"	\
		"<clientversion>%s</clientversion>"	\
		"</GetMiscInfo>"	\
		"</soap:Body>"	\
		"</soap:Envelope>", pcsPassport, szPswCrypt, PHCLIENT_VERSION_STR);

    iLen += strlen(pCurrent);

    PH_TRACE_OUT_FUNC;
   
    return iLen;
}

/******************************************************************************
����: ��ɶ�̬ע��ĸ��º�����״̬��⡣
����: ��
����: �ɹ�/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int PH_update(void)
{
    int fd = 0;
    int iRet = 0;
    int iMissHeartPacket =  0;
    ST_PH_UDP_SEND_DATE stClientInfoTemp;
    ST_PH_UDP_REC_DATE stServerInfoTemp;
    blf_ctx stCTX;
    struct hostent *pstHost;
    struct sockaddr_in stSocketAddr;

    PH_TRACE_INTO_FUNC;

    /*����UDP Server����*/
    fd = UDP_connect(atoi(UDP_PROTOCOL_PORT_6060) );
    if (fd  <= 0 )
    {
        PH_TRACE("Setup UDP connection failed.\n");
        return -1;
    }
 
    stSocketAddr.sin_family = AF_INET;
    pstHost = gethostbyname(g_szServerIP);
    if (0 == pstHost) 
    {
        PH_TRACE("gethostbyname server failed.\n");
        return -1;
    }   
    stSocketAddr.sin_addr = *(struct in_addr *)*pstHost->h_addr_list;
    stSocketAddr.sin_port = htons(atoi(UDP_PROTOCOL_PORT_6060));
   
    while(1)
    {
        /*��д�ͻ��˽ṹ����Ϣ*/
        if (PHC_UPDATE == g_iClientState)
            g_stClientInfo.lOptionCode = PHC_UPDATE;
        else
            g_stClientInfo.lOptionCode = PHC_LOGOUT;
           
        g_stClientInfo.lCheckSum = (g_stClientInfo.lOptionCode +g_stClientInfo.lSerialNum)*(-1);
        g_stClientInfo.lAppend = 0;

        PH_TRACE("g_stClientInfo.lSessionID =%ld\n",g_stClientInfo.lSessionID);
        PH_TRACE("g_stClientInfo.lOptionCode =%ld\n",g_stClientInfo.lOptionCode);
        PH_TRACE("g_stClientInfo.lSerialNum =%ld\n",g_stClientInfo.lSerialNum);
        PH_TRACE("g_stClientInfo.lCheckSum =%ld\n",g_stClientInfo.lCheckSum);
        PH_TRACE("g_stClientInfo.lAppend =%ld\n",g_stClientInfo.lAppend);

        /*ʹ��Blow Fish����*/
        stClientInfoTemp = g_stClientInfo;
        blf_key(&stCTX, g_uszKey, sizeof(g_uszKey));
        blf_enc(&stCTX, (unsigned long *)&stClientInfoTemp.lOptionCode, 4);

        /*�����ֽ���ת��*/
        stClientInfoTemp.lSessionID = LongOrderRevers( stClientInfoTemp.lSessionID);
        stClientInfoTemp.lOptionCode = LongOrderRevers( stClientInfoTemp.lOptionCode);
        stClientInfoTemp.lSerialNum= LongOrderRevers( stClientInfoTemp.lSerialNum);
        stClientInfoTemp.lCheckSum= LongOrderRevers( stClientInfoTemp.lCheckSum);
        stClientInfoTemp.lAppend= LongOrderRevers( stClientInfoTemp.lAppend);

        /*���͸��±���*/
        iRet = UDP_SendTo(fd, (void *)&stClientInfoTemp, sizeof(stClientInfoTemp), &stSocketAddr);
        if (iRet >= 0)
        {
            PH_TRACE("UDP_SendTo iRet=%d\n",iRet);
            iRet = UDP_ReadFromServer(fd, (void *)&stServerInfoTemp, sizeof(stServerInfoTemp), &stSocketAddr);
            if (iRet > 0)
            {
                /*��ʧ��������������*/
                iMissHeartPacket = 0;

                PH_TRACE("UDP_ReadFromServer iRet=%d\n",iRet);
                /*�����ֽ���ת��*/
                stServerInfoTemp.stUDPDate.lSessionID = LongOrderRevers(stServerInfoTemp.stUDPDate.lSessionID);
                stServerInfoTemp.stUDPDate.lOptionCode= LongOrderRevers(stServerInfoTemp.stUDPDate.lOptionCode);
                stServerInfoTemp.stUDPDate.lSerialNum= LongOrderRevers(stServerInfoTemp.stUDPDate.lSerialNum);
                stServerInfoTemp.stUDPDate.lCheckSum= LongOrderRevers(stServerInfoTemp.stUDPDate.lCheckSum);
                stServerInfoTemp.stUDPDate.lAppend= LongOrderRevers(stServerInfoTemp.stUDPDate.lAppend);
                stServerInfoTemp.lIP = LongOrderRevers(stServerInfoTemp.lIP);
              
                if (g_stClientInfo.lSessionID != stServerInfoTemp.stUDPDate.lSessionID)
                {
                    /*Ӧ�ò��������*/
                    PH_TRACE("get back session incorrect.\n");
                    goto UPDATE_FAIL;
                }
                else
                {
                    /*ʹ��Blow Fish����*/
                    blf_key(&stCTX, g_uszKey, sizeof(g_uszKey));
                    blf_dec(&stCTX, (unsigned long *)&stServerInfoTemp.stUDPDate.lOptionCode, 4);

                    PH_TRACE("stServerInfoTemp.stUDPDate.lSessionID =%ld\n",stServerInfoTemp.stUDPDate.lSessionID);
                    PH_TRACE("stServerInfoTemp.stUDPDate.lOptionCode =%ld\n",stServerInfoTemp.stUDPDate.lOptionCode);
                    PH_TRACE("stServerInfoTemp.stUDPDate.lSerialNum =%ld\n",stServerInfoTemp.stUDPDate.lSerialNum);
                    PH_TRACE("stServerInfoTemp.stUDPDate.lCheckSum =%ld\n",stServerInfoTemp.stUDPDate.lCheckSum);
                    PH_TRACE("stServerInfoTemp.stUDPDate.lAppend =%ld\n",stServerInfoTemp.stUDPDate.lAppend);
                    PH_TRACE("stServerInfoTemp.ip=%x\n",stServerInfoTemp.lIP);

                    /*����Ǹ�����Ϣ*/
                    if (PHC_UPDATE == g_iClientState)
                    {
                        /*����ʧ��*/
                        if (PH_UPDATE_FAIL == stServerInfoTemp.stUDPDate.lOptionCode)
                        {
                           PH_TRACE("get the incorrect update respond optioncode.\n");
                           goto UPDATE_FAIL;
                        }
                        else
                        {
                            if (g_stClientInfo.lSerialNum == stServerInfoTemp.stUDPDate.lSerialNum)
                                g_stClientInfo.lSerialNum++;
                            else if ((g_stClientInfo.lSerialNum - stServerInfoTemp.stUDPDate.lSerialNum) < 3)
                                g_stClientInfo.lSerialNum = stServerInfoTemp.stUDPDate.lSerialNum;
                            else
                            {
                                PH_TRACE("get the incorrect update respond serial number.\n");
                                goto UPDATE_FAIL;
                            }
                        }/*end of  if (PH_UPDATE_FAIL == stClientInfoTemp.lOptionCode)*/
                    }/*end of if (PHC_UPDATE == g_iClientState)*/
                    else
                    {
                        /*ע���ɹ�*/
                        if (PH_LOGOUT_OK == stServerInfoTemp.stUDPDate.lOptionCode)
                        {
                            PH_TRACE("Logout OK\n");
                            close(fd);
                            return PROC_NORMAL;
                        }
                        else
                        {
                            continue;
                        }

                    }/*end of if (PHC_UPDATE == g_iClientState)*/

                    /*�����û������ͣ���������°�*/
                    if (ADVANCED_USER == g_iClientType)
                        sleep(ADVANCED_HEARTBEAT);
                    else
                        sleep(STANDARD_HEARTBEAT);           
                }/*end of  if (g_stClientInfo.lSessionID != stClientInfoTemp.lSessionID)*/
               
            }
            else
            {
                if (PHC_LOGOUT== g_iClientState)
                {
                    PH_TRACE("Logout OK\n");
                    close(fd);
                    return PROC_NORMAL;
                }
                
                /*����ͻ�������5�����������ͳ�ȥ��û���յ��������Ļ�Ӧ��
                Ӧ�����һ������������������ȴ�20����ж��������ӳ������⡣
                �ͻ�����������ִ��Э��2.2�������µ�¼��ע��������*/
                iMissHeartPacket++;

                if (4 == iMissHeartPacket)
                    sleep(20);

                if (4 < iMissHeartPacket)
                {
                    PH_TRACE("the Internet does not work\n");
                    goto UPDATE_FAIL;
                }
            }/*end of if (iRet >= 0)*/
           
        }/*end of  if (iRet >= 0)*/
        
    }/*end of  while(1)*/
    
    PH_TRACE_OUT_FUNC;

UPDATE_FAIL:
    PH_TRACE("Update failed.\n");
    if (fd)
        close(fd);
        
   return PROC_UPDATE_FAIL;
}
/******************************************************************************
����: �ͻ�������ͨ���˺���֤���ܽ����������롢�޸ġ�ע��Ȳ�����
����: pcsPassport         input     ������
             pcsPassword        input    ����(����)
����: �ɹ�/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int PH_auth(const char *pcsPassport, const char *pcsPassword)
{
    int fd = 0;
    int iRet = 0;
    int iLen = 0;
    char szBuffer[MAX_BUF_LEN] = {0};
    unsigned char uszBase64Key[MAX_STRING_LEN] = {0};

    PH_TRACE_INTO_FUNC;

    /*����TCP ����*/
    fd = TCP_connect( g_szServerIP, TCP_PROTOCOL_PORT_6060);
    if ( fd < 0 )
    {
        PH_TRACE("Setup TCP connection failed.\n");
        return -1;
    }

    /*��ȡ�������Ļ�Ӧ*/
    iLen = TCP_ReadFromServer(fd,szBuffer,MAX_BUF_LEN);
    if (iLen > 0)
    {
        /*���Ӳ��ɹ���ֱ���˳�*/
        if (0 != strncmp(szBuffer,PH_TCP_OK,PH_RESPOND_LEN))
        {
            PH_TRACE("Connect TCP 6060 failed.\n");
            iRet = -1;
            goto OUT;
        }
    }

    /*����AUTH ROUTER*/
    snprintf(szBuffer, MAX_BUF_LEN, PHC_AUTH_ROUTE NEWLINE_R_N);
    TCP_SendTo(fd, szBuffer);

    /*��ȡ���������ص���ս��*/
    iLen = TCP_ReadFromServer(fd,szBuffer,MAX_BUF_LEN);
    if (iLen > 0)
    {
        /*���Ӳ��ɹ���ֱ���˳�*/
        if (0 != strncmp(szBuffer,PH_AUTH_ROUTE_OK,PH_RESPOND_LEN))
        {
            PH_TRACE("Get AUTH ROUTER respond failed.\n");
            iRet = -1;
            goto OUT;
        }
        else
        {
            /*��ȡ��ս��*/
            sscanf(szBuffer, "%*d %s", uszBase64Key);
            PH_TRACE("server szBase64Key =%s\n",uszBase64Key);

            /*������ս��*/
            iLen = Base64_Decode(uszBase64Key,strlen(uszBase64Key),g_uszKey);
            PH_TRACE("server szKey=%s,strlen(uszKey)=%d\n",g_uszKey,strlen(g_uszKey));        
        }
    }/*end of  if (iLen > 0)*/

    /*����У�鱨��*/
    iLen = GetAuthInfo(szBuffer,pcsPassport,pcsPassword,g_uszKey);
    TCP_SendTo(fd, szBuffer);

    /*��ȡУ����*/
    iLen = TCP_ReadFromServer(fd,szBuffer,MAX_BUF_LEN);
    if (iLen <= 0)
    {
        PH_TRACE("Get auth result failed.\n");
        iRet = -1;
        goto OUT;
    }
    else
    {
        /*ע������*/
        iRet = RegisterDomain(fd,szBuffer);
        if (PROC_NORMAL != iRet)
        goto OUT;
    }

    /*����ȷ��*/
    sprintf(szBuffer, PHC_CNFM NEWLINE_R_N);
    TCP_SendTo(fd,szBuffer);

    /*��ȡ�Ự��źͳ�ʼ���*/
    iLen = TCP_ReadFromServer(fd,szBuffer,MAX_BUF_LEN);
    if (iLen > 0)
    {
        /*����ע��ɹ���ֱ���˳�*/
        if (0 == strncmp(szBuffer,PH_REGISTER_FAIL, PH_RESPOND_LEN))
        {
            PH_TRACE("No name registered.\n");
            iRet = -1;
            goto OUT;
        }
        else if (0 == strncmp(szBuffer,PH_AUTH_OK, PH_RESPOND_LEN))
        {
            /*����ȷ�ϳɹ�����ȡ�Ự��ź����к�*/
            sscanf(szBuffer,"250 %ld %ld", &g_stClientInfo.lSessionID, &g_stClientInfo.lSerialNum);
        }
        else
        {
            PH_TRACE("Register failed Return Code:[%d].\n", atoi(szBuffer));
            iRet = -1;
            goto OUT;
        }
    }/*end of  if (iLen > 0)*/

    /*�������λỰ*/
    sprintf(szBuffer,PHC_QUIT NEWLINE_R_N);
    TCP_SendTo(fd,szBuffer);

    /*��ȡ���������ؽ��*/
    iLen = TCP_ReadFromServer(fd,szBuffer,MAX_BUF_LEN);
    if (iLen > 0)
    {
        /*����Ѿ�����Ҫ��*/
        PH_TRACE("Quit result %s\n",szBuffer);
    }

    iRet = PROC_NORMAL;

OUT:
   if (fd)
       close(fd);

   return iRet;
}

/******************************************************************************
����: �ڵ�½�����Ƿ���֮ǰȡ���û�Ӧ��ʹ�õĻ����Ƿ�����
             �ĵ�ַ���Լ����û������͡�
����: pcsPassport         input     ������
             pcsPassword        input    ����(����)
����: PROC_NORMAL/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int PH_login(const char *pcsPassport, const char *pcsPassword)
{
    int iRet =  0;
    int iLen = 0;
    int fd = 0;
    char szHeadBuf[MAX_BUF_LEN] = {0};
    char szBodyBuf[MAX_BUF_LEN] = {0};
    char szTemp[MAX_STRING_LEN] = {0};
  
    PH_TRACE_INTO_FUNC;

    /*����TCP����*/
    fd = TCP_connect( PH_SERVER_URL, HTTP_PROTOCOL_PORT);
    if ( fd < 0 )
    {
        PH_TRACE("Setup TCP connection failed.\n");
        return -1;
    }

    /*�����¼����*/
    iLen = GetUserInfo(szBodyBuf,pcsPassport,pcsPassword);
    PH_TRACE("szBuf=%s\n",szBodyBuf);

    /*���챨��ͷ��*/
    snprintf(szHeadBuf, MAX_BUF_LEN, "POST /userinfo.asmx HTTP/1.1"NEWLINE_R_N	\
                       "Host: %s"NEWLINE_R_N	\
                       "Content-Type: text/xml; charset=utf-8"NEWLINE_R_N	\
                       "Content-Length: %d"NEWLINE_R_N	\
                       "SOAPAction: \"http://tempuri.org/GetMiscInfo\""NEWLINE_R_N NEWLINE_R_N,
                       PH_SERVER_URL, iLen);

    /*���͵�¼����*/
    TCP_SendTo(fd,szHeadBuf);
    TCP_SendTo(fd,szBodyBuf);

    /*��ȡ�������Ļ�Ӧ*/
    iLen = TCP_ReadFromServer(fd,szBodyBuf,MAX_BUF_LEN);
    if (iLen > 0)
    {
        /*��ȡ������IP*/
        iRet = GetValueFromXML(szBodyBuf,PH_SERVER_IP,g_szServerIP,MAX_STRING_LEN);
        if(iRet > 0)
        {
            StrTrim(g_szServerIP,CHAR_BLANK);
            PH_TRACE("Get server ip:%s\n",g_szServerIP);
        }
        else
        {
            PH_TRACE("Get server ip failed.\n");
            iRet = -1;
            goto OUT;
        }

        /*��ȡ�û�����*/
        iRet = GetValueFromXML(szBodyBuf,PH_USER_TYPE,szTemp,MAX_STRING_LEN);
        if(iRet > 0)
        {
            StrTrim(szTemp,CHAR_BLANK);
            PH_TRACE("Get user type:%s\n",szTemp);

            if (1 == atoi(szTemp))
                g_iClientType = ADVANCED_USER;
            else
                g_iClientType = STANDARD_USER;
        }
        else
        {
            PH_TRACE("Get user type failed.\n");
            iRet = -1;
            goto OUT;
        }

        iRet =  PROC_NORMAL;
    }/*end of if (iLen > 0)*/

OUT:
    if (fd)
        close (fd);
   
    PH_TRACE_OUT_FUNC;

    return iRet;
}
/******************************************************************************
����: �����ǿͻ���(peanuthullc)���̳�ʼ������
����: ��   
����: PROC_NORMAL/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static int PH_init(void)
{
    PH_TRACE_INTO_FUNC;

    PH_TRACE_OUT_FUNC;

    return PROC_NORMAL;
}
 /******************************************************************************
����: �����ǿͻ���(peanuthullc)ʹ��˵��
����: pszProName         input      ������   
����: ��
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
static void Usage(char *psProName)
{
    PH_TRACE_INTO_FUNC;

    fprintf(stderr, "Usage:\n\t"
		        "%s passport password\n"
		        "\nNotes:\n\tThe max length of passport or password is 16 Bytes.\n\n"
		        "",psProName);

    PH_TRACE_OUT_FUNC;

    return ;
}

/******************************************************************************
����: �źŴ�����
����: ��     
����: ��
����: Kevin 
ʱ��: 2010-02-23
*******************************************************************************/
static void SigalHandler(int iSigNum)
{
    PH_TRACE_INTO_FUNC;

    PH_TRACE("The signal is %d\n",iSigNum);

    g_iClientState = PHC_LOGOUT;

    PH_TRACE_OUT_FUNC;

    return ;
}
/******************************************************************************
����: ����źŵĺ������ӱ�
����: ��     
����: ��
����: Kevin 
ʱ��: 2010-02-23
*******************************************************************************/
static void SignalInit(void)
{
    signal(SIGHUP, SigalHandler);
    signal(SIGINT, SigalHandler);
    signal(SIGQUIT, SigalHandler);
    signal(SIGKILL, SigalHandler);
    signal(SIGTERM, SigalHandler);

    return;
}
/******************************************************************************
����: �����ǿͻ���(peanuthullc)������ں���
����: argc         input      
             argv[]      input      
����: ���̵�����״̬
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int main(int argc, char *argv[])
{
    int iProcStatus =  PROC_NORMAL;
    char szPassport[MAX_PASSPORT_LEN] = {0};
    char szPassword[MAX_PASSPORT_LEN] = {0};

    PH_TRACE_INTO_FUNC;

    /*�ж���������Ƿ���ȷ*/
    if (argc != 3)
    {
        Usage(argv[0]);
        return -1;
    }

    strcpy(szPassport,argv[1]);
    strcpy(szPassword,argv[2]);

    /*ע���źŴ�����*/
    SignalInit();

    /*���̳�ʼ��*/
    iProcStatus = PH_init();
    if (PROC_NORMAL != iProcStatus)
    {
        PH_TRACE("Init process failed.\n");
        return iProcStatus;
    }

    do
    {
        /*��¼�����Ƿ�����*/
        iProcStatus = PH_login(szPassport,szPassword);
        if (PROC_NORMAL != iProcStatus)
        {
            PH_TRACE("Login PH failed.\n");
            return PROC_LOGIN_FAIL;
        }

        /*��֤��ע������*/
        iProcStatus = PH_auth(szPassport,szPassword);
        if (PROC_NORMAL != iProcStatus)
        {
            PH_TRACE("Auth PH failed.\n");
            return PROC_AUTH_FAIL;
        }

        g_iClientState = PHC_UPDATE;
        iProcStatus = PH_update();
    }while(PROC_UPDATE_FAIL == iProcStatus);
    
    return iProcStatus;
}

#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif

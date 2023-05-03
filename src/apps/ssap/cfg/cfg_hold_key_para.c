/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_hold_key_para.c
 �ļ����� : cfg �ָ�����Ĭ�ϱ����ؼ��������ܵ�ʵ��
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2009-04-06
            ���� :

**********************************************************************/


#include "cfg_api.h"
#include "cfg_file.h"
#include "flash_layout.h"
#include "cfg_prv.h"


#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

/* ���ùؼ����������� */
#define CFG_TR069_TREE "ManagementServer"
#define CFG_SNMP_TREE  "X_TWSZ-COM_SNMPAgent"
#define CFG_TELECOM_ACCOUNT "DeviceInfo.X_TWSZ-COM_TeleComAccount"

#define CFG_KEY_CONN_MAGIC   "KEYC"

#define CFG_STR_ARR(apc)  apc, sizeof(apc)/sizeof(apc[0])

//#define CFG_WAN_DEV_NEED_BUILD(ulDevIndex) (1 != ulDevIndex)
#define CFG_WAN_DEV_NEED_BUILD(ulDevIndex)  (ulDevIndex > 0)

/* ����ĺ��жϵ�������, �е�����³��������Ѿ���һ��wan�豸,
   ���ؼ��������ǻ��ڸ��豸ʱ, �����ʡ��һЩ�����Ͷ��� */


/* �ص��ӿ�: ��ʼ���ϲ��� */
/*
TR069BE ��ʼ��ʱ��Ҫ�ж� �ϴεĹ������ӻ��ڲ���, �����������ÿ�
*/

/*************************************************************************
Function:      CFG_RET CFG_TreeMerge(void *pvBase, void *pvAdd)
Description:   ���������ϲ�
Calls:         ��
Data Accessed:
Data Updated:
Input:         pvBase, ��������ַ
               pvAdd, �ϲ����������ĵ�ַ
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeMerge(void *pvBase, void *pvAdd)
{
    scew_element *pstBase = pvBase;
    scew_element *pstAdd = pvAdd;
    scew_element *pstNext = NULL;
    scew_element *pstParent = NULL;
    scew_element *pstNew = NULL;
    CFG_RET ret = CFG_OK;
    const char *pcAttrVal = NULL;
    const char *pcVal = NULL;

    pstNext = scew_element_next(pstAdd, NULL);
    if (NULL == pstNext)
    {
        return CFG_OK;
    }

    while (1)
    {
        do
        {
            pstAdd = pstNext;
            pstNew = scew_element_by_name(pstBase, scew_element_name(pstAdd));
            if (NULL == pstNew)
            {
                ret = CFG_DupSingleNode(pstAdd, &pstNew);
                if (CFG_OK != ret)
                {
                    CFG_ERR(ret);
                    return ret;
                }

                CFG_NODE_ATTR(pstNew, ATTR_TYPE, pcAttrVal);
                if (0 == strcmp(pcAttrVal, TYPE_CLASS)
                      || 0 == strcmp(pcAttrVal, TYPE_OBJECT))
                {
                    (void)scew_element_add_elem(pstBase, pstNew);
                }
                else
                {
                    (void)scew_element_insert_elem(pstBase, NULL, pstNew);
                }
            }
            else
            {
                pcVal = scew_element_contents(pstAdd);
                if (NULL != pcVal)
                {
                    (void)scew_element_set_contents(pstNew, pcVal);
                }
            }
            pstBase = pstNew;

        } while (pstNext = scew_element_next(pstAdd, NULL), NULL != pstNext);

        while (1)
        {
            pstParent = pstAdd->parent;
            if (NULL == pstParent)
            {
                return CFG_OK;
            }
            pstBase = pstBase->parent;
            pstNext = scew_element_next(pstParent, pstAdd);
            if (NULL != pstNext)
            {
                break;
            }
            pstAdd = pstParent;
        }
    }
}


/*************************************************************************
Function:      CFG_RET CFG_InitMergeKeyTree(void)
Description:   ��ʼ���ϲ���
Calls:         ��
Data Accessed:
Data Updated:
Input:         ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        ��ʼ��ʱ�ӱ���Ĺؼ����ж�ȡ����,�ٺϲ�����ǰ��������
*************************************************************************/
CFG_RET CFG_InitMergeKeyTree(void)
{
    CFG_RET ret = CFG_OK;
    char *pcBuf = NULL;
    unsigned long ulLen = 0;
    scew_tree *pstKeyTree = NULL;

    /* ��ȡ�ؼ��� */
    ret = CFG_ReadCurCfg(TBS_APP_KEY_TREE, &pcBuf, 0, 256, &ulLen); /* �������ָ����ʲô��Ŀ */
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return CFG_OK;
    }

    pcBuf[ulLen] = '\0';

    /* ���Ӷ����ֵ�ת�� */
    CFG_FileTransMean(pcBuf, ulLen);

    /* ���������� */
    ret = CFG_XmlInitPaser(&pstKeyTree, (const char *)pcBuf);
    if (CFG_OK != ret)
    {
        free(pcBuf);
        CFG_ERR(ret);
        return ret;
    }
    free(pcBuf);

    /* ���ڵ�����ת���ȥ */
    ret = CFG_TreeTransBack(pstKeyTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* �뵱ǰ���ϲ� */
    ret = CFG_TreeMerge(scew_tree_root(g_pstCfgTree), scew_tree_root(pstKeyTree));
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    scew_tree_free(pstKeyTree);
    return ret;
}




/*************************************************************************
Function:      CFG_RET CFG_DupTreeList(char * const apcTreeList[],
                       unsigned long ulCount, scew_tree **ppstTree)
Description:   ���������б�
Calls:         ��
Data Accessed:
Data Updated:
Input:         apcTreeList, ���������б�
               ulCount, apcTreeList �ĸ���
Output:        ppstTree, ������Ƴ�������ָ��
Return:        0, �ɹ�
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_DupTreeList(char aacTreeList[][CFG_MAX_PATH_LEN], unsigned long ulCount,
                        scew_tree **ppstTree)
{
    scew_tree *pstKeyTree = NULL;
    scew_element *pstRoot = NULL;
    scew_element *pstNewRoot = NULL;
    const char *pcRoot = NULL;
    char acRoot[CFG_MAX_PATH_LEN];
    CFG_RET ret = CFG_OK;
    unsigned long i = 0;
    scew_element *pstKey = NULL;
    scew_element *pstKeyDup = NULL;

    pstRoot = scew_tree_root(g_pstCfgTree);
    pcRoot = scew_element_name(pstRoot);
    sprintf(acRoot, "<%s> </%s>", pcRoot, pcRoot);
    ret = CFG_XmlInitPaser(&pstKeyTree, acRoot);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    for (i = 0; i < ulCount; i++)
    {
        char *pcNext = aacTreeList[i];
        char *pcTemp = NULL;

        pstNewRoot = scew_tree_root(pstKeyTree);
        pstKey = pstRoot;

        while (pcTemp = CFG_StrChrTok(pcNext, DELIMIT_C, &pcNext),
               NULL != pcTemp)
        {
            pstKey = scew_element_by_name(pstKey, pcTemp);
            CFG_ASSERT(NULL != pstKey);

            if (NULL != pcNext)
            {
                /* �� pstNewRoot ���½��ڵ� */
                pstNewRoot = scew_element_add(pstNewRoot, pcTemp);
                CFG_ASSERT(NULL != pstNewRoot);
            }
            else
            {
                break;
            }
        }

        ret = CFG_DupAccess(pstKey, &pstKeyDup);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }

        (void)scew_element_add_elem(pstNewRoot, pstKeyDup);
    }

    *ppstTree = pstKeyTree;

    return ret;
}

/* ָ����ǰ�������ӵĽڵ� */
#define CFG_MNG_CONN "InternetGatewayDevice.ManagementServer.X_TWSZ-COM_DefaultManageConnectionService"

typedef struct
{
    const char *pcVlan;
    const char *pcPrio;
    const char *pcUntag;
} ST_WAN_CONN;

/* wan�����豸��Ҫ����Ľڵ��б� */
char *s_apcCfgWanConnDevKeyLeaf[] =
{
    "X_TWSZ-COM_VLANID",
    "X_TWSZ-COM_VLANPriority",
    "X_TWSZ-COM_VLANUntag"
};

typedef struct
{
    const char *pcATMSustainableCellRate;
    const char *pcATMMaximumBurstSize;
    const char *pcATMPeakCellRate;
    const char *pcATMQoS;
    const char *pcATMAAL;
    const char *pcATMEncapsulation;
    const char *pcDestinationAddress;
    const char *pcLinkType;
    const char *pcLinkStatus;
    const char *pcEnable;
    const char *pcATMCellDelayVarition;
    const char *pcATMMinimumCellRate;
} ST_WAN_ATM;

char *s_apcCfgWanAtmKeyLeaf[] =
{
    "ATMSustainableCellRate",
    "ATMMaximumBurstSize",
    "ATMPeakCellRate",
    "ATMQoS",
    "ATMAAL",
    "ATMEncapsulation",
    "DestinationAddress",
    "LinkType",
    "LinkStatus",
    "Enable",
    "X_TWSZ-COM_ATMCellDelayVarition",
    "X_TWSZ-COM_ATMMinimumCellRate"
};

typedef struct
{
    const char *pcEnable;
    const char *pcName;

    const char *pcDnsEnable;
    const char *pcDnsOver;
    const char *pcUsrDns;
    const char *pcConnType;
    const char *pcSvcType;

    const char *pcAddrType;
    const char *pcIp;
    const char *pcMask;
    const char *pcGw;
    const char *pcDns;
} ST_WAN_IP_CONN;


/* ip��������Ҫ����ֵ�Ľڵ��б�, dhcp�����, ����4���ǲ���Ҫ�� */
char *s_apcCfgWanIpConnKeyLeaf[] =
{
    "Enable",
    "Name",

    "DNSEnabled",
    "DNSOverrideAllowed",
    "X_TWSZ-COM_UsrDNSServers",
    "ConnectionType",
    "X_TWSZ-COM_ServiceList",

    "AddressingType",
    "ExternalIPAddress",
    "SubnetMask",
    "DefaultGateway",
    "DNSServers"
};

typedef struct
{
    const char *pcEnable;
    const char *pcName;

    const char *pcDnsEnable;
    const char *pcDnsOver;
    const char *pcUsrDns;
    const char *pcConnType;
    const char *pcSvcType;

    const char *pcConnAct;
    const char *pcUsrName;
    const char *pcPwd;
    const char *pcMaxMRU;
    const char *pcTrigger;

} ST_WAN_PPP_CONN;


/* ip��������Ҫ����ֵ�Ľڵ��б� */
char *s_apcCfgWanPppConnKeyLeaf[] =
{
    "Enable",
    "Name",

    "DNSEnabled",
    "DNSOverrideAllowed",
    "X_TWSZ-COM_UsrDNSServers",
    "ConnectionType",
    "X_TWSZ-COM_ServiceList",

    "X_TWSZ-COM_ConnectionAction",
    "Username",
    "Password",
    "MaxMRUSize",
    "ConnectionTrigger"
};

#define CFG_WAN_SET_COUNT  (sizeof(ST_WAN_CONN)/sizeof(char *))

#define CFG_WAN_ATM_COUNT  (sizeof(ST_WAN_ATM)/sizeof(char *))

#define CFG_IP_SET_COUNT  (sizeof(ST_WAN_IP_CONN)/sizeof(char *))

#define CFG_PPP_SET_COUNT  (sizeof(ST_WAN_PPP_CONN)/sizeof(char *))


enum
{
    CFG_STATE_NULL,
    CFG_STATE_WAN_VLAN,
    CFG_STATE_WAN_CONN
} ;

typedef struct
{
    char *pcBuf;
    char *pcPath;
    unsigned char ucWanConnDevLen;
    unsigned char ucType;
    unsigned char ucWanConnLen;
    unsigned long ulConnDevIndex;

    unsigned long ulWanSetLen;
    unsigned long ulWanAtmLen;
    unsigned long ulWanConnSetLen;
    ST_WAN_CONN stWanConn;
    ST_WAN_ATM stWanAtm;
    ST_WAN_IP_CONN stIpConn;
    ST_WAN_PPP_CONN stPppConn;

} ST_CFG_KEY_CONN;

ST_CFG_KEY_CONN s_stCfgKeyConn;
unsigned char s_ucCfgState = CFG_STATE_NULL;

#define CFG_GET_ULONG(ptr, i)        (((unsigned long *)ptr)[i])




/* �ص��ӿ�: �ָ�����Ĭ�ϱ����ؼ����� */
/*
������ǰ�������ӵĹؼ�����
������������
*/
#define CFG_EXPECT_KEY_TREE_LEN  10000

/*************************************************************************
Function:      CFG_RET CFG_HoldKeyValWhenRecover(void)
Description:   �ָ�����Ĭ�ϱ����ؼ������Ľӿ�
Calls:         ��
Data Accessed:
Data Updated:
Input:         ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        ���ﱣ������������: �ؼ���, �ؼ����Ӳ���
*************************************************************************/
CFG_RET CFG_HoldKeyValWhenRecover(void)
{
    char aapcTreeList[][CFG_MAX_PATH_LEN] = {CFG_TR069_TREE, CFG_SNMP_TREE,
                                        "DeviceInfo.X_TWSZ-COM_TeleComAccount"};
    ST_CFG_DUMP_BUF stDumpBuf = {NULL, CFG_EXPECT_KEY_TREE_LEN, 0, 0};
    CFG_RET ret = CFG_OK;
    scew_tree *pstKeyTree = NULL;
    char acKeyConn[CFG_MAX_PATH_LEN];
    unsigned long ulLen = sizeof(acKeyConn);
    char *pcPos = NULL;
    unsigned long ulConnDevIndex = 0;
    ST_WAN_CONN stWanConn;
    ST_WAN_ATM stWanAtm;
    ST_WAN_IP_CONN stIpConn;
    ST_WAN_PPP_CONN stPppConn;
    char *pcBuf = NULL;
    unsigned long i = 0;
    char **ppcStr = NULL;

    char cTmp = 0;

    /* ������������, ����һ������ */
    ret = CFG_DupTreeList(CFG_STR_ARR(aapcTreeList), &pstKeyTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* dump, ѹ����д��flash */
    stDumpBuf.pcBuf = malloc(CFG_EXPECT_KEY_TREE_LEN);
    if (NULL == stDumpBuf.pcBuf)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    g_pcCfgAccessor = NULL;
    (void)CFG_TreeDumpAccess(scew_tree_root(pstKeyTree), &stDumpBuf, 0);
    if (stDumpBuf.ulInfactLen != stDumpBuf.ulPos)
    {
        free(stDumpBuf.pcBuf);
        CFG_REDUMP_FILE(stDumpBuf, scew_tree_root(pstKeyTree));
    }
    scew_tree_free(pstKeyTree);

    ret = CFG_SaveCurCfg(TBS_APP_KEY_TREE, stDumpBuf.pcBuf, stDumpBuf.ulInfactLen);  /* ���Ӳ�����ʾʹ�ùؼ�item */
    if (CFG_OK != ret)
    {
        free(stDumpBuf.pcBuf);
        CFG_ERR(ret);
        return ret;
    }
//    free(stDumpBuf.pcBuf);

    /* ȡ��ǰ�������� */
    ret = CFG_GetLeafValAndType(CFG_MNG_CONN, acKeyConn, &ulLen, NULL, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
    if ('\0' == acKeyConn[0])/* Ϊ�� */
    {
        return CFG_OK;
    }

    pcPos = strstr(acKeyConn, "WANConnectionDevice");
    CFG_ASSERT(NULL != pcPos);

    pcPos += sizeof("WANConnectionDevice");
    ulConnDevIndex = (unsigned long)strtol(pcPos, &pcPos, 10);
    pcPos++;

//gxw / 2014-09-16 / ��ʱ�رգ�DSL��أ�����д���ü�
#if 0
    if (CFG_WAN_DEV_NEED_BUILD(ulConnDevIndex))
    {
        cTmp = pcPos[0];
        pcPos[0] = '\0';
        ret = CFG_GetValToStrArr(acKeyConn, &stWanConn,
                                 CFG_STR_ARR(s_apcCfgWanConnDevKeyLeaf));
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }
        {
            char acPath[CFG_MAX_PATH_LEN];
            strcpy(acPath, acKeyConn);
            strcat(acPath, "WANDSLLinkConfig");
            ret = CFG_GetValToStrArr(acPath, &stWanAtm,
                                 CFG_STR_ARR(s_apcCfgWanAtmKeyLeaf));
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
                return ret;
            }
        }
        pcPos[0] = cTmp;
    }
#endif

    if ('I' == pcPos[3])
    {
        ret = CFG_GetValToStrArr(acKeyConn, &stIpConn,
                                 CFG_STR_ARR(s_apcCfgWanIpConnKeyLeaf));
    }
    else if ('P' == pcPos[3])
    {
        ret = CFG_GetValToStrArr(acKeyConn, &stPppConn,
                                 CFG_STR_ARR(s_apcCfgWanPppConnKeyLeaf));
    }
    else
    {
/*lint -save -e774*/
        CFG_ASSERT(0);
/*lint -restore*/
    }

    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* ����������ַ���, ��ʽ: name=val */
    pcBuf = stDumpBuf.pcBuf;
    ulLen = (unsigned long)sprintf(pcBuf, CFG_KEY_CONN_MAGIC" " "%s", acKeyConn);
    pcBuf += ulLen + 1;
    if (CFG_WAN_DEV_NEED_BUILD(ulConnDevIndex))
    {
        ppcStr = (char **)&stWanConn;
        for (i = 0; i < CFG_WAN_SET_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBuf, "%s", ppcStr[i]);
            pcBuf += ulLen + 1;
        }
        ppcStr = (char **)&stWanAtm;
        for (i = 0; i < CFG_WAN_ATM_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBuf, "%s", ppcStr[i]);
            pcBuf += ulLen + 1;
        }
    }
    if ('I' == pcPos[3])
    {
        ppcStr = (char **)&stIpConn;
        for (i = 0; i < CFG_IP_SET_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBuf, "%s", ppcStr[i]);
            pcBuf += ulLen + 1;
        }
    }
    else
    {
        ppcStr = (char **)&stPppConn;
        for (i = 0; i < CFG_PPP_SET_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBuf, "%s", ppcStr[i]);
            pcBuf += ulLen + 1;
        }
    }

    /* д��flash */
    ret = CFG_WriteFile(TBS_APP_KEY_CONN, stDumpBuf.pcBuf,
                        (unsigned long)(pcBuf - stDumpBuf.pcBuf));
    free(stDumpBuf.pcBuf);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}

CFG_RET CFG_MsgProc(ST_MSG *pstAckMsg);


/* �ص��ӿ�: ��ʼ���������� */
/*************************************************************************
Function:      CFG_RET CFG_InitKeyConn(void)
Description:   ��ʼ��ʱ�����ؼ����ӵĲ���,������
Calls:         ��
Data Accessed:
Data Updated:
Input:         ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        ������ɺ�ʼ��һ������: �½�wan�豸
*************************************************************************/
CFG_RET CFG_InitKeyConn(void)
{
    CFG_RET ret = CFG_OK;
    char *pcBuf = NULL;
    unsigned long ulLen = 2048;
    char *pcPos = NULL;
    char *pcNext = NULL;
    char **ppcStr = NULL;
    unsigned long i = 0;

    /* ע����Ϣ������ */
    ret = MSG_RegModule(MID_CFG, (FUN_RECEIVE)CFG_MsgProc);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
    memset(&s_stCfgKeyConn, 0, sizeof(s_stCfgKeyConn));

    pcBuf = malloc(ulLen);
    if (NULL == pcBuf)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }

    s_stCfgKeyConn.pcBuf = pcBuf;

    /* ��ȡ�ؼ����Ӳ��� */
    ret = CFG_ReadFile(TBS_APP_KEY_CONN, &pcBuf, 0, 0, &ulLen);
    if (CFG_OK != ret)
    {
        free(pcBuf);
        CFG_ERR(ret);
        return CFG_OK;
    }
    pcNext = pcBuf + sizeof(CFG_KEY_CONN_MAGIC);

    /* ���ħ�� */
    if (0 != strncmp(pcBuf, CFG_KEY_CONN_MAGIC, sizeof(CFG_KEY_CONN_MAGIC) - 1))
    {
        free(pcBuf);
        return CFG_OK;
    }

    /* �������������Ϣ */
    s_stCfgKeyConn.pcPath = pcNext;
    pcNext += strlen(pcNext) + 1;

    pcPos = strstr(s_stCfgKeyConn.pcPath, "WANConnectionDevice");
    CFG_ASSERT(NULL != pcPos);

    pcPos += sizeof("WANConnectionDevice");
    s_stCfgKeyConn.ucWanConnDevLen = pcPos - s_stCfgKeyConn.pcPath;
    s_stCfgKeyConn.ulConnDevIndex = (unsigned long)strtol(pcPos, &pcPos, 10);
    pcPos++;
    if (CFG_WAN_DEV_NEED_BUILD(s_stCfgKeyConn.ulConnDevIndex))
    {
        ppcStr = (char **)&s_stCfgKeyConn.stWanConn;
        for (i = 0; i < CFG_WAN_SET_COUNT; i++)
        {
            ulLen = strlen(pcNext);
            s_stCfgKeyConn.ulWanSetLen += ulLen + strlen(s_apcCfgWanConnDevKeyLeaf[i]);
            ppcStr[i] = pcNext;
            pcNext += ulLen + 1;
        }
        ppcStr = (char **)&s_stCfgKeyConn.stWanAtm;
        for (i = 0; i < CFG_WAN_ATM_COUNT; i++)
        {
            ulLen = strlen(pcNext);
            s_stCfgKeyConn.ulWanAtmLen += ulLen + strlen(s_apcCfgWanAtmKeyLeaf[i]);
            ppcStr[i] = pcNext;
            pcNext += ulLen + 1;
        }
    }
    if ('I' == pcPos[3])
    {
        s_stCfgKeyConn.ucType = 0;
        pcPos += sizeof("WANIPConnection");
        ppcStr = (char **)&s_stCfgKeyConn.stIpConn;
        for (i = 0; i < CFG_IP_SET_COUNT; i++)
        {
            ulLen = strlen(pcNext);
            s_stCfgKeyConn.ulWanConnSetLen += ulLen + strlen(s_apcCfgWanIpConnKeyLeaf[i]);
            ppcStr[i] = pcNext;
            pcNext += ulLen + 1;
        }
    }
    else
    {
        s_stCfgKeyConn.ucType = 1;
        pcPos += sizeof("WANPPPConnection");
        ppcStr = (char **)&s_stCfgKeyConn.stPppConn;
        for (i = 0; i < CFG_PPP_SET_COUNT; i++)
        {
            ulLen = strlen(pcNext);
            s_stCfgKeyConn.ulWanConnSetLen += ulLen + strlen(s_apcCfgWanPppConnKeyLeaf[i]);
            ppcStr[i] = pcNext;
            pcNext += ulLen + 1;
        }
    }
    s_stCfgKeyConn.ucWanConnLen = pcPos - s_stCfgKeyConn.pcPath;

    ret = CFG_MsgProc(NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}


STATIC unsigned short s_usCfgMsgID = 0;
#define CFG_MSGID ((MID_CFG << 16) | s_usCfgMsgID++)


#define CFG_MSGHEAD(pstMsg, type) \
{ \
    pstMsg->stMsgHead.usSrcMID = MID_CFG; \
    pstMsg->stMsgHead.usDstMID = MID_CMM; \
    pstMsg->stMsgHead.ulMsgID = CFG_MSGID; \
    pstMsg->stMsgHead.usMsgType = type; \
}


#define CFG_CREATE_MSG(pstMsg, len, type) \
{ \
    pstMsg = MSG_CreateMessage(len); \
    if (NULL == pstMsg) \
    { \
        CFG_ERR(ERR_CREATE_MSG_FAIL); \
        return ERR_CREATE_MSG_FAIL; \
    } \
    CFG_MSGHEAD(pstMsg, type); \
}



/*************************************************************************
Function:      CFG_RET CFG_MkAndSendConnSetMsg(unsigned long ulCount, char * const *apcLeaf,
                             char **apcVal, char *pcKey, unsigned long ulKeyLen,
                              unsigned long ulIndex)
Description:   ���첢�����������ӵ���Ϣ
Calls:         ��
Data Accessed:
Data Updated:
Input:         ulCount, Ҫ���õ�Ҷ�Ӹ���
               apcLeaf, Ҷ�ӽڵ��б�
               apcVal, �ڵ�ֵ�б�
               pcKey, ��ʶ���ӵ�Э������,
               ulKeyLen, pcKey�ĳ���
               ulIndex, Ҫ���õ����ӵ�����
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_MkAndSendConnSetMsg(unsigned long ulCount, char * const *apcLeaf,
                             char **apcVal, char *pcKey, unsigned long ulKeyLen,
                              unsigned long ulIndex)
{
    unsigned long ulLen = 0;
    ST_MSG *pstMsg = NULL;
    char *pcBody = NULL;
    CFG_RET ret = CFG_OK;
    char **ppcStr = NULL;
    unsigned long i = 0;
    char cTmp = 0;

    ulLen = s_stCfgKeyConn.ulWanConnSetLen
            + (s_stCfgKeyConn.ucWanConnDevLen + 2 + 16
                + ulKeyLen + 16) * ulCount;

    /* �����Ϣ����vlan */
    CFG_CREATE_MSG(pstMsg, ulLen, MSG_CMM_SET_VAL);

    pcBody = pstMsg->szMsgBody;
    CFG_SET_ULONG(pcBody, 0, ulCount);

    cTmp = s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen];
    s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = '\0';
    pcBody += sizeof(unsigned long);
    for (i = 0; i < ulCount; i++)
    {
        ppcStr = apcVal;
        ulLen = (unsigned long)sprintf(pcBody, "%s%lu.%s.%lu.%s=%s",
                        s_stCfgKeyConn.pcPath, s_stCfgKeyConn.ulConnDevIndex,
                        pcKey, ulIndex, apcLeaf[i], ppcStr[i]);
        pcBody += ulLen + 1;
    }
    s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = cTmp;
    pstMsg->stMsgHead.ulBodyLength = (unsigned long)(pcBody - pstMsg->szMsgBody);
    ret = MSG_SendMessage(pstMsg);
    MSG_ReleaseMessage(pstMsg);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}


/*************************************************************************
Function:      CFG_RET CFG_AddAckMsgProc(ST_MSG *pstAckMsg)
Description:   �������Ӧ����Ϣ
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstAckMsg, Ӧ����Ϣָ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        �����ǰ������豸, ����һ�������豸������;
               �����ǰ���������, ����һ��������������
*************************************************************************/
CFG_RET CFG_AddAckMsgProc(ST_MSG *pstAckMsg)
{
    unsigned long ulResult = 0;
    char *pcBody = pstAckMsg->szMsgBody;
    unsigned long ulIndex = 0;
    CFG_RET ret = CFG_OK;
    ST_MSG *pstMsg = NULL;
    unsigned long ulLen = 0;
    char cTmp = 0;
    unsigned long i = 0;
    char **ppcStr = NULL;

    /* ��鷵��ֵ */
    ulResult = CFG_GET_ULONG(pcBody, 0);
    if (0 != ulResult)
    {
        ret = ulResult;
        CFG_ERR(ret);
        return ret;
    }

    /* ȡindex */
    ulIndex = CFG_GET_ULONG(pcBody, 1);

    if (CFG_STATE_WAN_VLAN == s_ucCfgState) /* ��ǰ��vlan */
    {
        s_stCfgKeyConn.ulConnDevIndex = ulIndex;

        ulLen = s_stCfgKeyConn.ulWanSetLen + s_stCfgKeyConn.ulWanAtmLen
                + (s_stCfgKeyConn.ucWanConnDevLen + 2 + 16) * CFG_WAN_SET_COUNT
                + (s_stCfgKeyConn.ucWanConnDevLen + 2 + 16 + 20) * CFG_WAN_ATM_COUNT;

        /* �����Ϣ����vlan */
        CFG_CREATE_MSG(pstMsg, ulLen, MSG_CMM_SET_VAL);

        pcBody = pstMsg->szMsgBody;
        CFG_SET_ULONG(pcBody, 0, CFG_WAN_SET_COUNT + CFG_WAN_ATM_COUNT);

        cTmp = s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen];
        s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = '\0';
        pcBody += sizeof(unsigned long);
        ppcStr = (char **)&s_stCfgKeyConn.stWanConn;
        for (i = 0; i < CFG_WAN_SET_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBody, "%s%lu.%s=%s", s_stCfgKeyConn.pcPath,
                            ulIndex, s_apcCfgWanConnDevKeyLeaf[i], ppcStr[i]);
            pcBody += ulLen + 1;
        }
        ppcStr = (char **)&s_stCfgKeyConn.stWanAtm;
        for (i = 0; i < CFG_WAN_ATM_COUNT; i++)
        {
            ulLen = (unsigned long)sprintf(pcBody, "%s%lu.WANDSLLinkConfig.%s=%s", s_stCfgKeyConn.pcPath,
                            ulIndex, s_apcCfgWanAtmKeyLeaf[i], ppcStr[i]);
            pcBody += ulLen + 1;
        }

        s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = cTmp;
        pstMsg->stMsgHead.ulBodyLength = (unsigned long)(pcBody - pstMsg->szMsgBody);
        ret = MSG_SendMessage(pstMsg);
        MSG_ReleaseMessage(pstMsg);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
        }
    }
    else if (CFG_STATE_WAN_CONN == s_ucCfgState)  /* ��ǰ��wan���� */
    {
        /* �����Ϣ����wan���� */
        if (0 == s_stCfgKeyConn.ucType)
        {
            unsigned long ulCount = CFG_IP_SET_COUNT;

            if (0 == strcmp("DHCP", s_stCfgKeyConn.stIpConn.pcAddrType))
            {
                ulCount -= 4;
            }

            ret = CFG_MkAndSendConnSetMsg(ulCount, s_apcCfgWanIpConnKeyLeaf,
                             (char **)&s_stCfgKeyConn.stIpConn, "WANIPConnection",
                             sizeof("WANIPConnection"), ulIndex);
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
            }
        }
        else
        {
            ret = CFG_MkAndSendConnSetMsg(CFG_PPP_SET_COUNT, s_apcCfgWanPppConnKeyLeaf,
                             (char **)&s_stCfgKeyConn.stPppConn, "WANPPPConnection",
                             sizeof("WANPPPConnection"), ulIndex);
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
            }
        }
    }
    else
    {
        ;
    }

    return ret;
}



/*************************************************************************
Function:      CFG_RET CFG_SetAckMsgProc(ST_MSG *pstAckMsg)
Description:   ��������Ӧ����Ϣ
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstAckMsg, Ӧ����Ϣָ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        �����ǰ�������豸, ����һ��������ӵĲ���;
               �����ǰ����������, ����һ���������ñ������
*************************************************************************/
CFG_RET CFG_SetAckMsgProc(ST_MSG *pstAckMsg)
{
    CFG_RET ret = CFG_OK;
    unsigned long ulResult = 0;
    char *pcBody = NULL;
    ST_MSG *pstMsg = NULL;
    char pcTmp = 0;

    if (NULL == pstAckMsg)
    {
        /* ����wan�����Ϣ */
        if (CFG_WAN_DEV_NEED_BUILD(s_stCfgKeyConn.ulConnDevIndex))
        {
            pcTmp = s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen];

            CFG_CREATE_MSG(pstMsg, s_stCfgKeyConn.ucWanConnDevLen + 1, MSG_CMM_ADD_NODE);

            s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = '\0';
            strcpy(pstMsg->szMsgBody, s_stCfgKeyConn.pcPath);
            s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = pcTmp;
            ret = MSG_SendMessage(pstMsg);
            MSG_ReleaseMessage(pstMsg);
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
            }
            s_ucCfgState = CFG_STATE_WAN_VLAN;
            return ret;
        }
    }

    if (NULL != pstAckMsg)
    {
        pcBody = pstAckMsg->szMsgBody;

        /* ��鷵��ֵ */
        ulResult = CFG_GET_ULONG(pcBody, 0);
    }
    if (0 != ulResult)
    {
        ret = ulResult;
        CFG_ERR(ret);
        return ret;
    }
    if (NULL == pstAckMsg || CFG_STATE_WAN_VLAN == s_ucCfgState)  /* ��ǰ��vlan */
    {
        char *apcProt[] = {"WANIPConnection", "WANPPPConnection"};
        unsigned long ulLen = 0;

        /* �����Ϣ: ���wan���� */
        pcTmp = s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnLen];

        CFG_CREATE_MSG(pstMsg, s_stCfgKeyConn.ucWanConnLen + 1, MSG_CMM_ADD_NODE);

        s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = '\0';
        ulLen = (unsigned long)sprintf(pstMsg->szMsgBody, "%s%lu.%s.", s_stCfgKeyConn.pcPath,
                                 s_stCfgKeyConn.ulConnDevIndex, apcProt[s_stCfgKeyConn.ucType]);
        s_stCfgKeyConn.pcPath[s_stCfgKeyConn.ucWanConnDevLen] = pcTmp;
        pstMsg->stMsgHead.ulBodyLength = ulLen + 1;
        ret = MSG_SendMessage(pstMsg);
        MSG_ReleaseMessage(pstMsg);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
        }
        s_ucCfgState = CFG_STATE_WAN_CONN;
        return ret;
    }
    else if (CFG_STATE_WAN_CONN == s_ucCfgState) /* ��ǰ��wan���� */
    {
        const char *pcName = NULL;
        const char *pcPos = NULL;
        char acNewIndex[16] = {0};

        if (0 == s_stCfgKeyConn.ucType)
        {
            pcName = s_stCfgKeyConn.stIpConn.pcName;
        }
        else
        {
            pcName = s_stCfgKeyConn.stPppConn.pcName;
        }
        pcPos = strchr(pcName, '_');
        CFG_ASSERT(NULL != pcPos);
        memcpy(acNewIndex, pcName, (unsigned long)(pcPos - pcName));

        (void)CFG_SetNodeVal(
            "InternetGatewayDevice.WANDevice.1.X_TWSZ-COM_WANConnectionIndex",
            acNewIndex, NULL);

        /* ������� */
        CFG_CREATE_MSG(pstMsg, 0, MSG_CMM_SAVE_CFG);
        ret = MSG_SendMessage(pstMsg);
        MSG_ReleaseMessage(pstMsg);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
        }
    }

    return ret;
}

/*************************************************************************
Function:      CFG_RET CFG_SetAckMsgProc(ST_MSG *pstAckMsg)
Description:   ������Ӧ����Ϣ
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstAckMsg, Ӧ����Ϣָ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:        ����ɹ�, ������ؼ�����
*************************************************************************/
CFG_RET CFG_SaveAckMsgProc(const ST_MSG *pstAckMsg)
{
    CFG_RET ret = CFG_OK;
    unsigned long ulResult = CFG_GET_ULONG(pstAckMsg->szMsgBody, 0);

    /* �ͷ��ڴ� */
    free(s_stCfgKeyConn.pcBuf);

    if (0 != ulResult)
    {
        CFG_ERR(ulResult);
        return ulResult;
    }

    /* ���������Ϣ */
    ret = CFG_WriteFile(TBS_APP_KEY_TREE, "    ", 4);
    ret = CFG_WriteFile(TBS_APP_KEY_CONN, "    ", 4 );

    return ret;
}


/*************************************************************************
Function:      CFG_RET CFG_MsgProc(ST_MSG *pstAckMsg)
Description:   ���������ؽ����̵���Ϣ������
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstAckMsg, Ӧ����Ϣָ��
Output:        ��
Return:        0, �ɹ�
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_MsgProc(ST_MSG *pstAckMsg)
{
    CFG_RET ret = CFG_OK;

    if (NULL == pstAckMsg)
    {
        ret = CFG_SetAckMsgProc(pstAckMsg);
    }
    else
    {
        switch (pstAckMsg->stMsgHead.usMsgType)
        {
        case MSG_CMM_ADD_NODE_ACK:
            ret = CFG_AddAckMsgProc(pstAckMsg);
            break;
        case MSG_CMM_SET_VAL_ACK:
            ret = CFG_SetAckMsgProc(pstAckMsg);
            break;
        case MSG_CMM_SAVE_CFG_ACK:
            ret = CFG_SaveAckMsgProc(pstAckMsg);
            break;
        default:
            break;
        }
    }

    return ret;
}



#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


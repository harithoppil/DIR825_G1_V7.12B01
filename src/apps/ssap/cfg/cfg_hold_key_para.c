/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfg_hold_key_para.c
 文件描述 : cfg 恢复出厂默认保留关键参数功能的实现
 函数列表 :

 修订记录 :
          1 创建 : 陈跃东
            日期 : 2009-04-06
            描述 :

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

/* 两棵关键子树的名称 */
#define CFG_TR069_TREE "ManagementServer"
#define CFG_SNMP_TREE  "X_TWSZ-COM_SNMPAgent"
#define CFG_TELECOM_ACCOUNT "DeviceInfo.X_TWSZ-COM_TeleComAccount"

#define CFG_KEY_CONN_MAGIC   "KEYC"

#define CFG_STR_ARR(apc)  apc, sizeof(apc)/sizeof(apc[0])

//#define CFG_WAN_DEV_NEED_BUILD(ulDevIndex) (1 != ulDevIndex)
#define CFG_WAN_DEV_NEED_BUILD(ulDevIndex)  (ulDevIndex > 0)

/* 这里的宏判断的作用是, 有的情况下出厂配置已经有一个wan设备,
   当关键连接正是基于该设备时, 则可以省略一些参数和动作 */


/* 回调接口: 初始化合并树 */
/*
TR069BE 初始化时需要判断 上次的管理连接还在不在, 不在了则将其置空
*/

/*************************************************************************
Function:      CFG_RET CFG_TreeMerge(void *pvBase, void *pvAdd)
Description:   将两棵树合并
Calls:         无
Data Accessed:
Data Updated:
Input:         pvBase, 宿主树地址
               pvAdd, 合并进来的树的地址
Output:        无
Return:        0, 成功
               其它, 失败
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
Description:   初始化合并树
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        无
Return:        0, 成功
               其它, 失败
Others:        初始化时从保存的关键树中读取出来,再合并到当前配置树中
*************************************************************************/
CFG_RET CFG_InitMergeKeyTree(void)
{
    CFG_RET ret = CFG_OK;
    char *pcBuf = NULL;
    unsigned long ulLen = 0;
    scew_tree *pstKeyTree = NULL;

    /* 读取关键树 */
    ret = CFG_ReadCurCfg(TBS_APP_KEY_TREE, &pcBuf, 0, 256, &ulLen); /* 传入参数指定用什么条目 */
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return CFG_OK;
    }

    pcBuf[ulLen] = '\0';

    /* 增加对数字的转义 */
    CFG_FileTransMean(pcBuf, ulLen);

    /* 解析配置树 */
    ret = CFG_XmlInitPaser(&pstKeyTree, (const char *)pcBuf);
    if (CFG_OK != ret)
    {
        free(pcBuf);
        CFG_ERR(ret);
        return ret;
    }
    free(pcBuf);

    /* 将节点名称转义回去 */
    ret = CFG_TreeTransBack(pstKeyTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* 与当前树合并 */
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
Description:   复制子树列表
Calls:         无
Data Accessed:
Data Updated:
Input:         apcTreeList, 子树名称列表
               ulCount, apcTreeList 的个数
Output:        ppstTree, 输出复制出的子树指针
Return:        0, 成功
               其它, 失败
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
                /* 往 pstNewRoot 上新建节点 */
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

/* 指定当前管理连接的节点 */
#define CFG_MNG_CONN "InternetGatewayDevice.ManagementServer.X_TWSZ-COM_DefaultManageConnectionService"

typedef struct
{
    const char *pcVlan;
    const char *pcPrio;
    const char *pcUntag;
} ST_WAN_CONN;

/* wan连接设备需要保存的节点列表 */
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


/* ip连接所需要保存值的节点列表, dhcp情况下, 后面4个是不需要的 */
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


/* ip连接所需要保存值的节点列表 */
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




/* 回调接口: 恢复出厂默认保留关键参数 */
/*
保留当前管理连接的关键参数
保留两棵子树
*/
#define CFG_EXPECT_KEY_TREE_LEN  10000

/*************************************************************************
Function:      CFG_RET CFG_HoldKeyValWhenRecover(void)
Description:   恢复出厂默认保留关键参数的接口
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        无
Return:        0, 成功
               其它, 失败
Others:        这里保留两部分数据: 关键树, 关键连接参数
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

    /* 复制两棵子树, 做成一棵子树 */
    ret = CFG_DupTreeList(CFG_STR_ARR(aapcTreeList), &pstKeyTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* dump, 压缩并写入flash */
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

    ret = CFG_SaveCurCfg(TBS_APP_KEY_TREE, stDumpBuf.pcBuf, stDumpBuf.ulInfactLen);  /* 增加参数表示使用关键item */
    if (CFG_OK != ret)
    {
        free(stDumpBuf.pcBuf);
        CFG_ERR(ret);
        return ret;
    }
//    free(stDumpBuf.pcBuf);

    /* 取当前管理连接 */
    ret = CFG_GetLeafValAndType(CFG_MNG_CONN, acKeyConn, &ulLen, NULL, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
    if ('\0' == acKeyConn[0])/* 为空 */
    {
        return CFG_OK;
    }

    pcPos = strstr(acKeyConn, "WANConnectionDevice");
    CFG_ASSERT(NULL != pcPos);

    pcPos += sizeof("WANConnectionDevice");
    ulConnDevIndex = (unsigned long)strtol(pcPos, &pcPos, 10);
    pcPos++;

//gxw / 2014-09-16 / 暂时关闭，DSL相关，需进行代码裁剪
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

    /* 输出到连续字符串, 格式: name=val */
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

    /* 写入flash */
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


/* 回调接口: 初始化生成连接 */
/*************************************************************************
Function:      CFG_RET CFG_InitKeyConn(void)
Description:   初始化时读出关键连接的参数,并解析
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        无
Return:        0, 成功
               其它, 失败
Others:        解析完成后开始第一步操作: 新建wan设备
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

    /* 注册消息处理函数 */
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

    /* 读取关键连接参数 */
    ret = CFG_ReadFile(TBS_APP_KEY_CONN, &pcBuf, 0, 0, &ulLen);
    if (CFG_OK != ret)
    {
        free(pcBuf);
        CFG_ERR(ret);
        return CFG_OK;
    }
    pcNext = pcBuf + sizeof(CFG_KEY_CONN_MAGIC);

    /* 检查魔数 */
    if (0 != strncmp(pcBuf, CFG_KEY_CONN_MAGIC, sizeof(CFG_KEY_CONN_MAGIC) - 1))
    {
        free(pcBuf);
        return CFG_OK;
    }

    /* 发送连接添加消息 */
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
Description:   构造并发送设置连接的消息
Calls:         无
Data Accessed:
Data Updated:
Input:         ulCount, 要设置的叶子个数
               apcLeaf, 叶子节点列表
               apcVal, 节点值列表
               pcKey, 标识连接的协议类型,
               ulKeyLen, pcKey的长度
               ulIndex, 要设置的连接的索引
Output:        无
Return:        0, 成功
               其它, 失败
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

    /* 组包消息设置vlan */
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
Description:   处理添加应答消息
Calls:         无
Data Accessed:
Data Updated:
Input:         pstAckMsg, 应答消息指针
Output:        无
Return:        0, 成功
               其它, 失败
Others:        如果当前是添加设备, 则下一步进行设备的配置;
               如果当前是添加连接, 则下一步进行连接设置
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

    /* 检查返回值 */
    ulResult = CFG_GET_ULONG(pcBody, 0);
    if (0 != ulResult)
    {
        ret = ulResult;
        CFG_ERR(ret);
        return ret;
    }

    /* 取index */
    ulIndex = CFG_GET_ULONG(pcBody, 1);

    if (CFG_STATE_WAN_VLAN == s_ucCfgState) /* 当前在vlan */
    {
        s_stCfgKeyConn.ulConnDevIndex = ulIndex;

        ulLen = s_stCfgKeyConn.ulWanSetLen + s_stCfgKeyConn.ulWanAtmLen
                + (s_stCfgKeyConn.ucWanConnDevLen + 2 + 16) * CFG_WAN_SET_COUNT
                + (s_stCfgKeyConn.ucWanConnDevLen + 2 + 16 + 20) * CFG_WAN_ATM_COUNT;

        /* 组包消息设置vlan */
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
    else if (CFG_STATE_WAN_CONN == s_ucCfgState)  /* 当前在wan连接 */
    {
        /* 组包消息设置wan连接 */
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
Description:   处理设置应答消息
Calls:         无
Data Accessed:
Data Updated:
Input:         pstAckMsg, 应答消息指针
Output:        无
Return:        0, 成功
               其它, 失败
Others:        如果当前是设置设备, 则下一步添加连接的操作;
               如果当前是设置连接, 则下一步进行配置保存操作
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
        /* 发送wan添加消息 */
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

        /* 检查返回值 */
        ulResult = CFG_GET_ULONG(pcBody, 0);
    }
    if (0 != ulResult)
    {
        ret = ulResult;
        CFG_ERR(ret);
        return ret;
    }
    if (NULL == pstAckMsg || CFG_STATE_WAN_VLAN == s_ucCfgState)  /* 当前在vlan */
    {
        char *apcProt[] = {"WANIPConnection", "WANPPPConnection"};
        unsigned long ulLen = 0;

        /* 组包消息: 添加wan连接 */
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
    else if (CFG_STATE_WAN_CONN == s_ucCfgState) /* 当前在wan连接 */
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

        /* 组包保存 */
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
Description:   处理保存应答消息
Calls:         无
Data Accessed:
Data Updated:
Input:         pstAckMsg, 应答消息指针
Output:        无
Return:        0, 成功
               其它, 失败
Others:        如果成功, 则清除关键配置
*************************************************************************/
CFG_RET CFG_SaveAckMsgProc(const ST_MSG *pstAckMsg)
{
    CFG_RET ret = CFG_OK;
    unsigned long ulResult = CFG_GET_ULONG(pstAckMsg->szMsgBody, 0);

    /* 释放内存 */
    free(s_stCfgKeyConn.pcBuf);

    if (0 != ulResult)
    {
        CFG_ERR(ulResult);
        return ulResult;
    }

    /* 清空数据信息 */
    ret = CFG_WriteFile(TBS_APP_KEY_TREE, "    ", 4);
    ret = CFG_WriteFile(TBS_APP_KEY_CONN, "    ", 4 );

    return ret;
}


/*************************************************************************
Function:      CFG_RET CFG_MsgProc(ST_MSG *pstAckMsg)
Description:   进行连接重建过程的消息处理函数
Calls:         无
Data Accessed:
Data Updated:
Input:         pstAckMsg, 应答消息指针
Output:        无
Return:        0, 成功
               其它, 失败
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


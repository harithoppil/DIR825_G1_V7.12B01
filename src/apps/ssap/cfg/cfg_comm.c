/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfg_comm.c
 文件描述 : cfg模块中一些全局通用的子函数和初始化相关函数的实现
 函数列表 :

 修订记录 :
          1 创建 : 陈跃东
            日期 : 2007-10-15
            描述 :
          2 修改 : 陈跃东
            日期 : 2009-07-06
            描述 : 增加函数CFG_StrChrTok, 并用之替换 strtok

**********************************************************************/

#include <unistd.h>
#include <time.h>
#include "cfg_file.h"
#include "cfg_prv.h"
#include "autoconf.h"
#include "tbsutil.h"
#include "tbsmsg.h"
#include "flash_layout.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

//telefonica 需求，恢复出厂设置时灯，全部闪烁。
#define CONFIG_LED_FACTORT_FLASHING


/* 配置树 */
scew_tree *g_pstCfgTree = NULL;

const char *g_pcCfgAccessor = NULL;

#define MAX_PATH_LEN 257

#ifndef CPE_PREFIX
#define CPE_PREFIX  "X_TWSZ-COM"
#endif

/*************************************************************************
Function:      unsigned char CFG_IsAccessorIn(const char *pcAccessList,
                                       const char *pcAccessor)
Description:   查询访问者是否在访问者列表中
Calls:         无
Data Accessed:
Data Updated:
Input:         pcAccessList, 访问者列表
               pcAccessor, 访问者
Output:        无
Return:        1, 存在
               0, 不存在
Others:
*************************************************************************/
unsigned char CFG_IsAccessorIn(const char *pcAccessList, const char *pcAccessor)
{
    const char *pcBegin = pcAccessList;
    long ret = 0;
    unsigned char ucState = 0;

    if ('\0' == pcAccessor[0])  /* 为空字符串, 表示访问者为TR069 */
    {
        return 1;
    }

    while (1)
    {
        if ((' ' == *pcAccessList) || ('\0' == *pcAccessList))
        {
            if (1 == ucState)
            {
                ucState = 0;
                ret = strncmp(pcBegin, pcAccessor,
                               (unsigned long)(pcAccessList - pcBegin));
                if (0 == ret)
                {
                    return 1;
                }
            }

            if ('\0' == *pcAccessList)
            {
                break;
            }

            pcBegin = pcAccessList + 1;
        }
        else
        {
            if (0 == ucState)
            {
                ucState = 1;
                pcBegin = pcAccessList;
            }
        }

        pcAccessList++;
    }

    return 0;
}



/*************************************************************************
Function:      char *cfg_strdup(const char *pcPath)
Description:   CFG模块用来复制路径参数,
Calls:         无
Data Accessed:
Data Updated:
Input:         pcPath, 要复制的路径
Output:        无
Return:        复制后字符串的地址
Others:        表面上是重新分配了内存, 实际上用的是静态变量, 目的是减少malloc的次数
*************************************************************************/
char *cfg_strdup(const char *pcPath)
{
    static char acPath[CFG_MAX_PATH_LEN];

    if (strlen(pcPath) >= CFG_MAX_PATH_LEN)
    {
        CFG_ERR(ERR_CFG_BUF_NOT_ENOUGH);

        return NULL;
    }

    strcpy(acPath, pcPath);

    return acPath;
}




/*
取得节点, 子函数, pcPath 可写
*/


/*************************************************************************
Function:      void CFG_InitNodeInfo(ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
Description:  初始化节点信息
Calls:         无
Data Accessed:
Data Updated:
Input:         ulMask, 节点信息掩码标志
Output:        pstNodeInfo, 初始化后的节点信息结构
Return:        无
Others:
*************************************************************************/
void CFG_InitNodeInfo(ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
{
    if (ulMask & CFG_NODE_INFO_MID)
    {
        pstNodeInfo->usMID = 0;
    }
    if (ulMask & CFG_NODE_INFO_STANDARD)
    {
        pstNodeInfo->ucStandard = 1;
    }
    if (ulMask & CFG_NODE_INFO_NOTI)
    {
        pstNodeInfo->ucNoti = 0;
    }
    if (ulMask & CFG_NODE_INFO_ACCESSLIST)
    {
        pstNodeInfo->pcAccessList = "Subscriber";
    }
    if (ulMask & CFG_NODE_INFO_MWNOTI_ITMS)
    {
        pstNodeInfo->ucMwNotiItms = 0;
    }
    if (ulMask & CFG_NODE_INFO_MWNOTI_MW)
    {
        pstNodeInfo->ucMwNotiMw= 0;
    }
}

/*************************************************************************
Function:      void CFG_UpdateNodeInfo(const scew_element *pstNode,
                        ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
Description:  更新节点信息结构
Calls:         无
Data Accessed:
Data Updated:
Input:         pstNode, 节点地址
               ulMask, 节点信息掩码标志
Output:        pstNodeInfo, 更新后的节点信息结构
Return:        无
Others:
*************************************************************************/
void CFG_UpdateNodeInfo(const scew_element *pstNode,
       ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
{
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    if (ulMask & CFG_NODE_INFO_MID)
    {
        pstAttr = scew_attribute_by_name(pstNode, ATTR_MID);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
            pstNodeInfo->usMID = strtol(pcAttrVal, NULL, CFG_MID_BASE);
        }
    }
    if (ulMask & CFG_NODE_INFO_STANDARD)
    {
        if (0 != pstNodeInfo->ucStandard)
        {
            pstAttr = scew_attribute_by_name(pstNode, ATTR_STANDARD);
            if (NULL != pstAttr)
            {
                pcAttrVal = scew_attribute_value(pstAttr);
                pstNodeInfo->ucStandard = (unsigned char )(pcAttrVal[0] - '0');
            }
        }
    }
    if (ulMask & CFG_NODE_INFO_NOTI)
    {
        pstAttr = scew_attribute_by_name(pstNode, ATTR_NOTI);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
            pstNodeInfo->ucNoti = (unsigned char )(pcAttrVal[0] - '0');
        }
    }
    if (ulMask & CFG_NODE_INFO_ACCESSLIST)
    {
        pstAttr = scew_attribute_by_name(pstNode, ATTR_ACCESS_LIST);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
            pstNodeInfo->pcAccessList = pcAttrVal;
        }
    }

    pcAttrVal = NULL;
    if ((CFG_NODE_INFO_MWNOTI_ITMS & ulMask) || (CFG_NODE_INFO_MWNOTI_MW & ulMask))
    {
        pstAttr = scew_attribute_by_name(pstNode, ATTR_MWNOTI);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
        }
    }
    if (ulMask & CFG_NODE_INFO_MWNOTI_ITMS && NULL != pcAttrVal)
    {
        pstNodeInfo->ucMwNotiItms = (unsigned char )(pcAttrVal[0] - '0');
    }
    if (ulMask & CFG_NODE_INFO_MWNOTI_MW && NULL != pcAttrVal)
    {
        pstNodeInfo->ucMwNotiMw = (unsigned char )(pcAttrVal[1] - '0');
    }
}


/*************************************************************************
Function:      long CFG_GetNode(const void *pstTree, char *pcPath, void **ppvNode)
Description:   查找节点
Calls:
Data Accessed:
Data Updated:
Input:         pstTree, 树根
               pcPath, 路径,
Output:        ppvNode, 节点指针
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_GetNode(const void *pvTree, char *pcPath, void **ppvNode,
                    ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
{
    char *pszTmp = NULL;
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    if (NULL == pvTree)
    {
        pvTree = g_pstCfgTree;
    }

    pszTmp = CFG_StrChrTok(pcPath, DELIMIT_C, &pcPath);

    pstNode = scew_tree_root(pvTree);
    if (NULL != pszTmp)
    {
        /* 如果有提供节点路径, 检查根节点是否匹配 */
        ret = strcmp(scew_element_name(pstNode), pszTmp);
        if (0 != ret)
        {
            /* 节点不存在 */
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", pszTmp);
            return ERR_CFG_PATH_NOT_EXSITED;
        }
        CFG_InitNodeInfo(pstNodeInfo, ulMask);
        CFG_UpdateNodeInfo(pstNode, pstNodeInfo, ulMask);

        while (pszTmp = CFG_StrChrTok(pcPath, DELIMIT_C, &pcPath),
               NULL != pszTmp)
        {
            /* 增加 对 pszTmp 作转义 */
            CFG_NUM_TO_NODE(pszTmp);

            pstNode = scew_element_by_name(pstNode, pszTmp);
            if (NULL == pstNode)
            {
                /* 节点不存在 */
                CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", pszTmp);
                return ERR_CFG_PATH_NOT_EXSITED;
            }

            CFG_UpdateNodeInfo(pstNode, pstNodeInfo, ulMask);
        }
    }
    else
    {
        CFG_InitNodeInfo(pstNodeInfo, ulMask);
        CFG_UpdateNodeInfo(pstNode, pstNodeInfo, ulMask);
    }

    *ppvNode = pstNode;
    return CFG_OK;
}


CFG_RET CFG_InitMergeKeyTree(void);
CFG_RET CFG_GetUsbInitPath(char *pcPath);

#ifdef CONFIG_REPLACE_NODE_PREFIX

#define CFG_PREFIX_REPLACE(pcBuf, ulLen, pcOld, pcNew) \
{ \
    char *pcRet = NULL; \
 \
    pcRet = tbsStringReplaceWithMalloc(pcBuf, pcOld, pcNew); \
    free(pcBuf); \
    pcBuf = pcRet;\
    ulLen = strlen(pcBuf); \
    /*重新分配内存时一定要多2K用来转义*/ \
    pcBuf = realloc(pcBuf, ulLen+1+CFG_NUM_TRANS_MEAN_LEN); \
}

#else

#define CFG_PREFIX_REPLACE(pcBuf, ulLen, pcOld, pcNew) (void)0

#endif
/***********************************************************
接口:   采用指定接口读取配置数据然后解析成一个树
参数:   ppstTree, 输出解析成的树
        pcPath, 读取数据的路径
        pfnFunc, 读取函数
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_InitTreeByFunc(scew_tree **ppstTree, const char *pcPath,
                           FUNC_CFG_READ_CFG pfnFunc)
{
    char *pcCfg = NULL;
    unsigned long ulLen = 0;
    CFG_RET ret = CFG_OK;

    ret = pfnFunc(pcPath, &pcCfg, 0, CFG_NUM_TRANS_MEAN_LEN, &ulLen);
    if (CFG_OK != ret)   /* 失败 */
    {
        CFG_ERR(ret);
        return ret;
    }
    pcCfg[ulLen] = '\0';

#ifdef CONFIG_APPS_LOGIC_USB_MASS
    CFG_PREFIX_REPLACE(pcCfg, ulLen, CONFIG_PRODUCT_PREFIX, CPE_PREFIX);
#endif


    /* 增加对数字的转义 */
    CFG_FileTransMean(pcCfg, ulLen);

    /* 解析配置树 */
    ret = CFG_XmlInitPaser(ppstTree, (const char *)pcCfg);
    free(pcCfg);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* 将节点名称转义回去 */
    ret = CFG_TreeTransBack(*ppstTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ret = CFG_FillDataTreeExAttrs();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ret = CFG_FillDataTreeExNodes();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* 删除需要重启删除的节点 */
//    CFG_RmvDeletingNode(*ppstTree);

    return CFG_OK;
}


/***********************************************************
接口:   设置无线初始参数
参数:   pcDev, 2.4G和5G无线
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_WlanInit(const char *pcDev)
{
    #define WLAN_NODE_BEACON_TYPE          "BeaconType"
    #define WLAN_NODE_WEPKEY               "WEPKey"
    #define NODE_WLAN_PSK_EXPRESSION		"X_TWSZ-COM_PSKExpression"
    #define WLAN_NODE_KEY_PASSPHRASE       "KeyPassphrase"
    #define WLAN_NODE_PRE_SHARED_KEY       "PreSharedKey"
    #define MAX_WPAPSK_LEN       	133
    #define AUTH_LOGIN_NAME_LEN     64
    #define ITEM_DATA_LEN               255

    char PassWd[MAX_WPAPSK_LEN] = {0};
    char szValue[ITEM_DATA_LEN] = {0};
    char szPath[MAX_PATH_LEN] = {0};
    const char *pszBeaconType = NULL;
    const char *pszPassword = NULL;
    const char *pszPSKExpression = NULL;
    int iRet = 0;
    unsigned short len;
    int iWlanIdx = 1;

    len = MAX_WPAPSK_LEN;
    if(safe_strcmp(pcDev, "2.4G") == 0)
    {
        iWlanIdx = 1;
        iRet = app_item_get(PassWd , TBS_WLAN_PASSWORD ,&len);
    }
    else if(safe_strcmp(pcDev, "5G") == 0)
    {
        iWlanIdx = 5; 
        iRet = app_item_get(PassWd , TBS_WLAN_PASSWORD5G ,&len);
    }
    else
    {
        return CFG_FAIL;
    }
    
    /* 从FLASH读取第一个VAP的加密模式和密钥 */
    

    if((ERROR_ITEM_OK == iRet) && (strlen(PassWd) > 0))
    {
        sprintf(szPath, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d.%s", iWlanIdx, WLAN_NODE_BEACON_TYPE);
        iRet = CFG_GetNodeValPtr(szPath, &pszBeaconType, NULL);

        if (iRet != CFG_OK)
        {
            CFG_ERR("Error: Get %s value failed.\n", WLAN_NODE_BEACON_TYPE);
            return CFG_FAIL;
        }

        if (strcmp(pszBeaconType, "Basic") == 0)
        {
            if(strlen(PassWd) == 5 || strlen(PassWd) == 13 || strlen(PassWd) == 10 || strlen(PassWd) == 26)
            {
                sprintf(szPath, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d.WEPKey.1.%s", iWlanIdx, WLAN_NODE_WEPKEY);
                CFG_SetNodeVal(szPath, PassWd, NULL);
            }
            else
            {
                CFG_ERR("Error: Set WEP password failed.\n");
                //return CFG_FAIL;
            }
        }
        else if ((strcmp(pszBeaconType, "WPA") == 0) || (strcmp(pszBeaconType, "11i") == 0) || (strcmp(pszBeaconType, "WPAand11i") == 0))
        {
            sprintf(szPath, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d.%s", iWlanIdx, NODE_WLAN_PSK_EXPRESSION);
            iRet = CFG_GetNodeValPtr(szPath, &pszPSKExpression, NULL);
			
            if (iRet != CFG_OK)
            {
                CFG_ERR("Error: Get %s value failed.\n", NODE_WLAN_PSK_EXPRESSION);
                return CFG_FAIL;
            }

            /* 若PSKExpression为KeyPassphrase方式*/
            if (strcmp(pszPSKExpression, WLAN_NODE_KEY_PASSPHRASE) == 0)
			{	
                if(strlen(PassWd) <= 63 && strlen(PassWd) >= 8)
                {
                    sprintf(szPath, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d.PreSharedKey.1.%s", iWlanIdx, WLAN_NODE_KEY_PASSPHRASE);
                    CFG_SetNodeVal(szPath, PassWd, NULL);
                }
                else
                {
                    CFG_ERR("Error: Set WPA password failed.\n");
                    //return CFG_FAIL;
                }
            }
            /* 若PSKExpression为PreSharedKey方式*/
            else if (strcmp(pszPSKExpression, WLAN_NODE_PRE_SHARED_KEY) == 0)
            {	
                if(strlen(PassWd) == 64)
                {
                    sprintf(szPath, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d.PreSharedKey.1.%s", iWlanIdx, WLAN_NODE_PRE_SHARED_KEY);
                    CFG_SetNodeVal(szPath, PassWd, NULL);
                }
                else
                {
                    CFG_ERR("Error: Set WPA password failed.\n");
                    //return CFG_FAIL;
            	}
            }
        }
    }

    return CFG_OK;
}

/***********************************************************
接口:   读取默认配置的函数
参数:   pcFile, 文件名
        ppcBuf, 输出缓冲区地址
        ulPrevSpace, 要求缓冲区之前有多少空间
        ulPostSpace, 要求缓冲区之后有多少空间
        pulLen, 输出读取到的文件长度
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_ReadDefaultCfgByPath(const char *pcFile, char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
{
    (void)pcFile;
    return CFG_ReadDefaultCfg(ppcBuf, ulPrevSpace, ulPostSpace, pulLen);
}

#define SOFTWARE_VERSION_NODE       "InternetGatewayDevice.DeviceInfo.SoftwareVersion"
#define MODEL_NAME_NODE       "InternetGatewayDevice.DeviceInfo.ModelName"

/* 基本接口 */
/***********************************************************
接口:   配置库初始化
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_LibInit(void)
{
    CFG_RET ret = CFG_OK;
    char szPath[MAX_PATH_LEN] = {0};
    int iRet = 0;

    /* 初始化 mid树 */
    ret = CFG_MIDTreeInit();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* 从 保存的配置初始化树 */
    ret = CFG_InitTreeByFunc(&g_pstCfgTree, NULL, CFG_ReadCurCfg);
    if (CFG_OK != ret)   /* 失败 */
    {
        /* 从 缺省配置初始化树 */
        ret = CFG_InitTreeByFunc(&g_pstCfgTree, NULL, CFG_ReadDefaultCfgByPath);
        //printf("----------------read cfg from default config\n");
#ifdef CONFIG_APPS_SSAP_PROTEST
		//wfj del it 20160325
        //CFG_WlanInit("2.4G");
        //CFG_WlanInit("5G");
#endif
    }
    else
    {
#ifdef CONFIG_APPS_LOGIC_USB_MASS
        char acPath[CFG_MAX_FILE_LEN] = {0};

        /* 调用从usb初始化的函数 */
        ret = CFG_GetUsbInitPath(acPath);
        if (CFG_OK == ret && '\0' != acPath[0])
        {
            scew_tree *pstTreeSave = g_pstCfgTree;

            ret = CFG_InitTreeByFunc(&g_pstCfgTree, acPath, CFG_ReadCurCfg);
            if (CFG_OK == ret)
            {
                scew_tree_free(pstTreeSave);
                ret = CFG_SaveCfg();
            }

            /* 如果从u盘中读取失败，则重新从保存的配置初始化树 */
            if (CFG_OK != ret)
            {
                ret = CFG_InitTreeByFunc(&g_pstCfgTree, NULL, CFG_ReadCurCfg);
            }
        }
#endif
    }
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

#ifdef CONFIG_APPS_SSAP_HOLD_KEY_PARA
    ret = CFG_InitMergeKeyTree();  /* 调用 初始化合并关键树的操作 */
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
#endif

#ifdef _CFG_DEBUG
    (void)CFG_ListNoTypeNode();
//    (void)CFG_PrintToExcel(); exit(0);
#endif

    return CFG_OK;
}


/* 配置库去初始化 */
/***********************************************************
接口:   配置库配置库去初始化
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_LibFinal(void)
{
    if (NULL != g_pstCfgTree)
    {
        scew_tree_free(g_pstCfgTree);
        g_pstCfgTree = NULL;
    }
    (void)CFG_MIDTreeFinal();
    return CFG_OK;
}




/******************************************************************************
  函数名称: XML_InitPaser
  功能说明: 使用指定XML内容初始化XML分析器
  输入参数: pszXMLContent  XML完整内容, 只读
  输出参数: 无
  返 回 值: CFG_NOERROR表示成功,
            CFG_ERR_PASER_ERROR：解析器解释出错, 一般是XML格式错误
  修订记录:
         1. 作者: zhaihaichen
            日期: 2007-9-4
            内容: 创建文件
 ******************************************************************************/
CFG_RET CFG_XmlInitPaser(scew_tree **ppstTree, const char *pszXMLContent)
{
    scew_parser* pParser = NULL;
    pParser = scew_parser_create();

    scew_parser_ignore_whitespaces(pParser, 1);

    /* Loads an XML file */
    if (!scew_parser_load_buffer(pParser, pszXMLContent, strlen(pszXMLContent)))
    {
        scew_error code = scew_error_code();
        CFG_ERR(ERR_CFG_INTERNAL_ERR, "Unable to load file (error #%d: %s)\n",
                 code, scew_error_string(code));
        if (code == scew_error_expat)
        {
            #ifdef _CFG_DEBUG
            enum XML_Error expat_code = scew_error_expat_code(pParser);
            #endif
            CFG_ERR(ERR_CFG_INTERNAL_ERR, "Expat error #%d (line %d, column %d): %s\n",
                     expat_code,
                     scew_error_expat_line(pParser),
                     scew_error_expat_column(pParser),
                     scew_error_expat_string(expat_code));
        }

        /* 说明, 经过反复验证, 这里的 scew_parser_free 并不能释放所有的内存.
           这些内存都是在scew_parser_load_buffer中被分配的.
           如果单元测试走到这个分支, 可不必检查内存泄露问题.
        */

        /* Frees the SCEW parser */
        scew_parser_free(pParser);

        return ERR_CFG_FILE_FOTMAT;
    }

    /* 解析XML树 */
    *ppstTree = scew_parser_tree(pParser);

    /* Frees the SCEW parser */
    scew_parser_free(pParser);

    return CFG_OK;
}


/*************************************************************************
Function:      void CFG_FileTransMean(char *pcBuf, unsigned long ulLen)
Description:   文件内容转义, 主要是把数字节点名转为非数字节点名
Calls:
Data Accessed:
Data Updated:
Input:         pcBuf, 文件内容
               ulLen, 内容长度
Output:        pcBuf, 解析后的内容
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
void CFG_FileTransMean(char *pcBuf, unsigned long ulLen)
{
    char *pcBufWork = pcBuf;
    unsigned long ulLenWork = ulLen;
    unsigned char ucState = 0;

    while ('\0' != *pcBufWork && ulLenWork > 0)
    {
        switch (*pcBufWork)
        {
            case '<':
                ucState = 1;
                break;
            case ' ':
            case '\t':
            case '/':
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (1 == ucState)
                {
                    MEM_BACKCPY(pcBufWork, pcBufWork + 1, ulLenWork);
                    *pcBufWork = '_';
                    pcBufWork++;
                    pcBufWork[ulLenWork] = '\0';

                    ucState = 0;
                }
                break;
            default:
                ucState = 0;
                break;
        }

        pcBufWork++;
        ulLenWork--;
    }
}

/***********************************************************
接口:   备份配置文件
参数:   pcFileContent: 缓冲区
        pulLen: 输入缓冲区长度, 函数结束会输出实际是多大的长度
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_GetCfgFile(char *pcFile, unsigned long *pulLen,
                       unsigned long ulCompress, unsigned long ulOverWrite,
                       const char *pcAccessor)
{
    scew_element *pElement = NULL;
    ST_CFG_DUMP_BUF stDumpBuf = {NULL, 0, 0, 0};
    char *pcFileBuf = NULL;
    CFG_RET ret = CFG_OK;

    if (NULL == pcFile || NULL == pulLen)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    if (1 == ulCompress)  /* 压缩形式 */
    {
         /* 文件已经存在 */
        ret = access(pcFile, F_OK);
        if (CFG_OK == ret)
        {
            if (0 == ulOverWrite) /* 无覆盖标志 */
            {
                /* 返回 错误: 文件已经存在 */
                CFG_ERR(ERR_FILE_OPEN_EXSITED);
                return ERR_FILE_OPEN_EXSITED;
            }
            else
            {
                /* 删除文件 */
                ret = remove(pcFile);
                if (CFG_OK != ret)
                {
                    int err = errno;
                    if (EACCES == err)
                    {
                        ret = ERR_FILE_NOT_ALLOWED;
                    }
                    else
                    {
                        ret = ERR_FILE_RM_UNKOWN;
                    }
                    CFG_ERR(ret);
                    return ret;
                }
            }
        }
    }

    pcFileBuf = malloc(CFG_MAX_FILE_LEN);
    if (NULL == pcFileBuf)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }

    stDumpBuf.pcBuf = pcFileBuf;
    stDumpBuf.ulLen = CFG_MAX_FILE_LEN;
    pElement = scew_tree_root(g_pstCfgTree);

    /* dump 树 */
    g_pcCfgAccessor = pcAccessor;
    (void)CFG_TreeDumpAccess(pElement, &stDumpBuf, 0);
    if (stDumpBuf.ulInfactLen != stDumpBuf.ulPos)
    {
        free(stDumpBuf.pcBuf);
        CFG_REDUMP_FILE(stDumpBuf, pElement);
    }
    stDumpBuf.pcBuf[stDumpBuf.ulInfactLen] = '\0';

    CFG_PREFIX_REPLACE(stDumpBuf.pcBuf, stDumpBuf.ulInfactLen,
                       CPE_PREFIX, CONFIG_PRODUCT_PREFIX);

    if (1 == ulCompress)  /* 压缩形式 */
    {
        ret = CFG_SaveCurCfg(pcFile, stDumpBuf.pcBuf, stDumpBuf.ulInfactLen);
    }
    else
    {
        ret = CFG_WriteFile(pcFile, stDumpBuf.pcBuf, stDumpBuf.ulInfactLen);
    }
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    pcFile = pcFile;
    *pulLen = *pulLen;

    free(stDumpBuf.pcBuf);
    return ret;
}


/***********************************************************
接口:   更新压缩配置文件
参数:   pcFileContent: 配置文件内容
        ulLen: 内容长度
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_SetCompressCfgFile(const char *pcFile, unsigned long ulLen)
{
    int iRet = 0;
    scew_tree * pstCfgTree = NULL;
    if(!pcFile || pcFile[0] ==0)
    {
        return CFG_FAIL;
    }
    pstCfgTree = g_pstCfgTree;
    g_pstCfgTree = NULL;
    iRet = CFG_InitTreeByFunc(&g_pstCfgTree, pcFile, CFG_ReadCurCfg);
    (void)ulLen;
    
    if (CFG_OK != iRet)   /* 失败 */
    {
        CFG_ERR(iRet);
        if(g_pstCfgTree)
            scew_tree_free(g_pstCfgTree);
        g_pstCfgTree = pstCfgTree;
        return iRet;
    }
    
    iRet = CFG_SaveCfg();
    if (CFG_OK != iRet)   /* 失败 */
    {
        CFG_ERR(iRet);
        if(g_pstCfgTree)
            scew_tree_free(g_pstCfgTree);
        g_pstCfgTree = pstCfgTree;
        return iRet;
    }
    scew_tree_free(pstCfgTree);
    
    return iRet;
}

/***********************************************************
接口:   更新配置文件
参数:   pcFileContent: 配置文件内容
        ulLen: 内容长度
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_SetCfgFile(const char *pcFile, unsigned long ulLen)
{
    unsigned long ulNewLen = 0;
    scew_tree *pstNewTree = NULL;
    long ret = CFG_OK;
    char *pcFileBuf = NULL;

    scew_tree *g_pstCfgTreeTmp = NULL;
    
    (void)ulLen;
    if (NULL == pcFile)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    ret = CFG_ReadFile(pcFile, &pcFileBuf, 0, CFG_NUM_TRANS_MEAN_LEN, &ulNewLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    pcFileBuf[ulNewLen] = '\0';

    CFG_PREFIX_REPLACE(pcFileBuf, ulNewLen, CONFIG_PRODUCT_PREFIX, CPE_PREFIX);

    /* 增加对数字的转义 */
    CFG_FileTransMean(pcFileBuf, ulNewLen);

    /* 解析新树 */
    ret = CFG_XmlInitPaser(&pstNewTree, (const char *)pcFileBuf);
    if (CFG_OK != ret)
    {
        free(pcFileBuf);

        CFG_ERR(ret);
        return ret;
    }

    free(pcFileBuf);

    g_pstCfgTreeTmp = g_pstCfgTree;
    g_pstCfgTree = pstNewTree;

    ret = CFG_FillDataTreeExAttrs();

    g_pstCfgTree = g_pstCfgTreeTmp;
    if (CFG_OK != ret)
    {
        scew_tree_free(pstNewTree);
        CFG_ERR(ret);
        return ret;
    }

#ifdef TBS_E8_TELECOM
	unsigned int i = 0;
    ST_CFG_LOCAL_RECOVER_KEY_TREE astKeyTree[] =
    {
        /* SSID--1～SSID--8 相关参数 */
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.1", NULL},
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.2", NULL},
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.3", NULL},
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.4", NULL},
	#if 0
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.5", NULL},
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.6", NULL},
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.7", NULL},
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.8", NULL},
	#endif
    };
	if (NULL == g_pstCfgTree)   /* 未初始化 */
    {
        /* 初始化配置树 */
        ret = CFG_LibInit();
        CFG_GOTO_ERR_PROC(ret, RELKEY);
    }

	 /* 挨个复制要保留的子树 */
    for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        (void)CFG_DelNodeToPtr(astKeyTree[i].acPath, &astKeyTree[i].pvTree);
    }
#endif


    /* 释放旧树 */
    scew_tree_free(g_pstCfgTree);

    /* 将全局树指针指向新树 */
    g_pstCfgTree = pstNewTree;

    ret = CFG_TreeTransBack(g_pstCfgTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

#ifdef TBS_E8_TELECOM
	/* 挨个合并上去 */
    for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        char *pcNext = astKeyTree[i].acPath + sizeof("InternetGatewayDevice");
        char *pcTemp = NULL;
        scew_element *pstNode = scew_tree_root(g_pstCfgTree);
        scew_element *pstParent = NULL;

        if (NULL == astKeyTree[i].pvTree)
        {
            continue;
        }
        while (pcTemp = CFG_StrChrTok(pcNext, DELIMIT_C, &pcNext),
               NULL != pcTemp)
        {
            CFG_ASSERT(NULL != pstNode);
            pstParent = pstNode;
            pstNode = scew_element_by_name(pstNode, pcTemp);
        }

        CFG_ASSERT(NULL != pstParent);  /* 这句的作用是消除lint告警 */

        if (NULL == pstNode) /*如果节点不存在则需要合并保存的节点*/
        {
            (void)scew_element_add_elem(pstParent, astKeyTree[i].pvTree);
        }

        astKeyTree[i].pvTree = NULL;
    }
RELKEY:
	for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        if (NULL == astKeyTree[i].pvTree)
        {
            continue;
        }
        scew_element_del(astKeyTree[i].pvTree);
    }
#endif
    /*写文件 */
    ret = CFG_SaveCfg();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}


#ifdef CONFIG_LAST_SAVE_CFG

/***********************************************************
接口:   读取压缩文件更新配置文件
参数:   pcFile: 压缩文件路径
        ulLen: 内容长度
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_SetCompressCfgFile(const char *pcFile, unsigned long ulLen)
{
    unsigned long ulNewLen = 0;
    scew_tree *pstNewTree = NULL;
    long ret = CFG_OK;
    char *pcFileBuf = NULL;

    (void)ulLen;
    if (NULL == pcFile)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    ret = CFG_ReadCurCfg(pcFile, &pcFileBuf, 0, CFG_NUM_TRANS_MEAN_LEN, &ulNewLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
    
    pcFileBuf[ulNewLen] = '\0';
    CFG_PREFIX_REPLACE(pcFileBuf, ulNewLen, CONFIG_PRODUCT_PREFIX, CPE_PREFIX);
    /* 增加对数字的转义 */
    CFG_FileTransMean(pcFileBuf, ulNewLen);

    /* 解析新树 */
    ret = CFG_XmlInitPaser(&pstNewTree, (const char *)pcFileBuf);
    
    if (CFG_OK != ret)
    {
        free(pcFileBuf);

        CFG_ERR(ret);
        return ret;
    }

    free(pcFileBuf);

    /* 释放旧树 */
    scew_tree_free(g_pstCfgTree);

    /* 将全局树指针指向新树 */
    g_pstCfgTree = pstNewTree;

    ret = CFG_TreeTransBack(g_pstCfgTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /*写文件 */
    ret = CFG_SaveCfg();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }
    return ret;
}
#endif


/***********************************************************
接口:   写配置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_SaveCfg(void)
{
    unsigned long ulLen = CFG_MAX_FILE_LEN;
    ST_CFG_DUMP_BUF stDumpBuf = {NULL, 0, 0, 0};
    scew_element *pstNode = NULL;
    long ret = CFG_OK;

    g_pcCfgAccessor = NULL;
    pstNode = scew_tree_root(g_pstCfgTree);

    stDumpBuf.pcBuf = malloc(ulLen);
    if (NULL == stDumpBuf.pcBuf)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    stDumpBuf.ulLen = ulLen;

    /* 将节点树dump 出来, */
    (void)CFG_TreeDumpAccess(pstNode, &stDumpBuf, 0);
    if (stDumpBuf.ulInfactLen != stDumpBuf.ulPos)
    {
        free(stDumpBuf.pcBuf);
        CFG_REDUMP_FILE(stDumpBuf, pstNode);
    }

    /* 调用file接口保存 */
    ret = CFG_SaveCurCfg(NULL, stDumpBuf.pcBuf, stDumpBuf.ulInfactLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    free(stDumpBuf.pcBuf);

    /* Added by yjs 20100115 config change so notify snmpd */
    FILE *fp;
    fp = fopen( "/var/tmp/cfgchg.smp", "w" );
	if( fp != NULL )
	{
		fprintf( fp, "%d", 1 );
		fclose( fp );
	}

    if( ( fp = fopen( "/var/tmp/tr064_change", "w" ) ) != NULL )
    {
        if( fp != NULL )
		{
			fprintf( fp, "%d", 1 );
			fclose( fp );
		}
    }

    return ret;
}


/* 恢复出厂设置 */
/***********************************************************
接口:   恢复出厂设置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_RecoverFactSet(const char *pcAccessor)
{
    long ret = CFG_OK;

    /* 将出厂配置设置为当前配置 */
    ret = CFG_RecoverFactCfg(pcAccessor);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}


/* 清除当前配置 */
/***********************************************************
接口:   清除当前配置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_ClearCfg(void)
{
    CFG_RET ret = CFG_OK;

    ret = CFG_ClearCurCfg();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    /*
     设置Reset flag,不允许再写配置
     只能重启后再配置 add by pengyao 20121015
    */
    g_bResetedFlag = 1;
	
	/* 设置首页跳到恢复出厂页面*/
    ret = app_item_save("1", TBS_WiZARD_FLAG, 2);

    return ret;
}

#ifdef CONFIG_APPS_LOGIC_WANSELECT

/* 清除所有配置 */
/***********************************************************
接口:   清除所有配置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_ClearAllCfg(void)
{
    CFG_RET ret = CFG_OK;

    ret = CFG_ClearAllTheCfg();
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}
#endif

/***********************************************************
接口:   判断节点是否为数字节点
参数:   pcNode, 节点名
返回值: 0:不是
        其它:是
***********************************************************/
unsigned char CFG_IsNodeNum(const char *pcNode)
{
    while ('\0' != *pcNode)
    {
        if (*pcNode > '9' || *pcNode < '0')
        {
            return 0;
        }
        pcNode++;
    }

    return 1;
}

/***********************************************************
接口:   类似于strtok的功能, 专用于cfg模块解析以'.'分隔的路径
参数:   pcStr, 路径
        cKey, 分隔符
        ppcNext, 下一段路径
返回值: 解析到的节点名
***********************************************************/
char *CFG_StrChrTok(char *pcStr, char cKey, char **ppcNext)
{
    char *pcPos = NULL;

    if (NULL == pcStr || '\0' == pcStr[0]
        || (cKey == pcStr[0] && '\0' == pcStr[1]))
    {
        return NULL;
    }

    pcPos = strchr(pcStr, cKey);
    if (NULL != pcPos)
    {
        *pcPos = '\0';
        if (cKey == pcPos[1])
        {
            /* 这里之所以返回一个空字符串作为名称,
               是因为检测到路径中存在连续两个点号的情况.
               这里返回空字符串的目的是希望在外层寻找节点时出错.
               从而产生错误应答.
               而且必须在前一个节点解析的地方返回, 因为形如 xxx.xxx.. 这样的路径,
               在后一个节点处理时无法跟单个的 "." 路径区分 */

            pcStr = "";
        }
        else
        {
            *ppcNext = pcPos + 1;
        }
    }
    else
    {
        *ppcNext = NULL;
    }

    return pcStr;
}

/*************************************************************************
Function:      scew_element *scew_element_insert_elem(scew_element* element,
                                 scew_element *left, scew_element* new_elem)
Description:   将节点到另一节点指定子节点的后面
Calls:         无
Data Accessed:
Data Updated:
Input:         element, 宿主节点地址
               left, 插入的位置
               new_elem, 新插入的节点地址
Output:        无
Return:        new_elem
Others:
*************************************************************************/
scew_element *scew_element_insert_elem(scew_element* element,
                           scew_element *left, scew_element* new_elem)
{
    scew_element* current = NULL;

    element->n_children++;

    new_elem->parent = element;
    if (element->last_child == NULL)
    {
        element->last_child = new_elem;
    }
    else
    {
        if (NULL != left)
        {
            if (NULL != left->right)
            {
                left->right->left = new_elem;
                new_elem->right = left->right;
            }
            else
            {
                element->last_child = new_elem;
            }
            new_elem->left = left;
            left->right = new_elem;
        }
        else
        {
            current = element->child;
            current->left = new_elem;
            new_elem->right = current;
        }
    }
    if (NULL == left || NULL == element->child)
    {
        element->child = new_elem;
    }

    return new_elem;
}



#ifdef _CFG_DEBUG

CFG_RET CFG_PrintNode(const void *pvTree, const char *pcPath,
                      unsigned long ulLen, void *pvFile)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_DUMP_BUF stDumpBuf = {NULL, 0, 0, 0};
    char *pcPathTmp = NULL;

    if (NULL == pvFile)
    {
        pvFile = stdout;
    }

    CFG_DUP_PATH(pcPathTmp, pcPath);

    /* 查找节点 */
    ret = CFG_GetNode(pvTree, pcPathTmp, (void **)(void *)&pstNode, NULL, 0);
    CFG_FREE_PATH(pcPathTmp);
    if (CFG_OK != ret || NULL == pstNode)  /* 没有找到 */
    {
        printf("Node not failed\n");
        return  ret;
    }

    stDumpBuf.pcBuf = malloc(ulLen);
    if (NULL == stDumpBuf.pcBuf)
    {
        printf("Malloc buffer failed\n");
        return ERR_CFG_MALLOC_FAIL;
    }
    stDumpBuf.ulLen = ulLen;

    g_pcCfgAccessor = NULL;
    (void)CFG_TreePrintAccess(pstNode, &stDumpBuf, 0);
    if (stDumpBuf.ulPos == stDumpBuf.ulLen)
    {
        stDumpBuf.pcBuf[stDumpBuf.ulPos - 1] = '\0';
    }
    else
    {
        stDumpBuf.pcBuf[stDumpBuf.ulPos] = '\0';
    }
    fprintf(pvFile, "%s", stDumpBuf.pcBuf);
    fprintf(pvFile, "\n");

    free(stDumpBuf.pcBuf);
    return CFG_OK;
}

#endif




#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif



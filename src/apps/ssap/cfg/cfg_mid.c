/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfg_mid.c
 文件描述 : cfg 模块中跟 逻辑树相关的操作函数
 函数列表 :

 修订记录 :
          1 创建 : 陈跃东
            日期 : 2008-04-06
            描述 :
          2 修改 : 陈跃东
            日期 : 2008-11-06
            描述 : 之前的mid树主要是存放mid之用, 现在改为了存放整个配置模型

**********************************************************************/

#include "cfg_file.h"
#include "cfg_prv.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif
#ifndef UINT_MAX
#define UINT_MAX (~0U)
#endif
#ifndef INT_MAX
#define INT_MAX ((int )(~0U>>1))
#endif
#ifndef INT_MIN
#define INT_MIN (-INT_MAX - 1)
#endif

/* MID 树 */
STATIC scew_tree *s_pstCfgMIDTree = NULL;

/*************************************************************************
Function:      CFG_RET CFG_RegMIDToMIDTree(const char *pcPath, unsigned short usMID)
Description:   将MID注册到逻辑树上
Calls:         无
Data Accessed:
Data Updated:
Input:         pcPath, 注册路径, 可以是通配路径
               usMID, 要注册的mid
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_RegMIDToMIDTree(const char *pcPath, unsigned short usMID)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element *pstNode = NULL;
    scew_element *pstChild = NULL;
    long ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    char acMID[32];

    if (NULL == pcPath || 0 == usMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* 复制路径 */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pstNode = scew_tree_root(s_pstCfgMIDTree);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);
    if (NULL != pszTmp)
    {
        /* 如果有提供节点路径, 检查根节点是否匹配 */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstNode);

        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            if (CFG_IS_NODE_NUM(pszTmp))
            {
                CFG_FREE_PATH(pcPathTmp);
                CFG_ERR(ERR_CFG_NOT_SUPPORT);
                return ERR_CFG_NOT_SUPPORT;
            }

            NODE_WILDCARD_CVT(pszTmp);

            /* 增加 对 pszTmp 作转义 */
            CFG_NUM_TO_NODE(pszTmp);

            pstChild = scew_element_by_name(pstNode, pszTmp);
            if (NULL == pstChild)
            {
                pstChild = scew_element_add(pstNode, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);
                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }
            }
            pstNode = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);
    sprintf(acMID, "%04x", usMID);

    CFG_SETATTR(pstNode, ATTR_MID, acMID);

    return CFG_OK;
}


/*************************************************************************
Function:      CFG_RET CFG_AddCareMID(const scew_element *pstNode, unsigned short usMID)
Description:   往一个节点上添加Care mid
Calls:         无
Data Accessed:
Data Updated:
Input:         pstNode, 节点地址
               usMID, 要添加的mid
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_AddCareMID(const scew_element *pstNode, unsigned short usMID)
{
    char acMID[32];
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    unsigned long ulOldLen = 0;
    unsigned short usMIDOld = 0;
    char acMIDList[CFG_MAX_MIDLIST_LEN];

    sprintf(acMID, "%04x", usMID);
    pstAttr = scew_attribute_by_name(pstNode, ATTR_CAREMID);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        ulOldLen = strlen(pcAttrVal);

        /* 先检查是否重复 */
        if (ulOldLen >= 4)
        {
            const char *pcPos = pcAttrVal;
            do
            {
                usMIDOld = strtol(pcPos, NULL, CFG_MID_BASE);
                if (usMID == usMIDOld)
                {
                    return CFG_OK;
                }
                if ('\0' == pcPos[4])
                {
                    break;
                }
                pcPos += 5;

            } while (1);
        }

        /* 将新MID添加到 末尾 */
        strcpy(acMIDList, pcAttrVal);
        acMIDList[ulOldLen] = ' ';
        strcpy(&acMIDList[ulOldLen + 1], acMID);

        /* 将属性值设置 */
        pcAttrVal = scew_attribute_set_value(pstAttr, acMIDList);
        if (NULL == pcAttrVal)
        {
            CFG_ERR(ERR_CFG_INTERNAL_ERR);
            return ERR_CFG_INTERNAL_ERR;
        }
    }
    else
    {
        CFG_ADDATTR(pstNode, ATTR_CAREMID, acMID, ((void)0));
    }

    return CFG_OK;
}



/*************************************************************************
Function:      CFG_RET CFG_RegMIDToMIDTree(const char *pcPath, unsigned short usMID)
Description:   将CareMID注册到逻辑树上
Calls:         无
Data Accessed:
Data Updated:
Input:         pcPath, 注册路径, 可以是通配路径
               usMID, 要注册的mid
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_RegCareMIDToMIDTree(const char *pcPath, unsigned short usMID)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element *pstNode = NULL;
    scew_element *pstChild = NULL;
    long ret = CFG_OK;

    if (NULL == pcPath || 0 == usMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* 复制路径 */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pstNode = scew_tree_root(s_pstCfgMIDTree);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);
    if (NULL != pszTmp)
    {
        /* 如果有提供节点路径, 检查根节点是否匹配 */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstNode);

        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            if (CFG_IS_NODE_NUM(pszTmp))
            {
                CFG_FREE_PATH(pcPathTmp);
                CFG_ERR(ERR_CFG_NOT_SUPPORT);
                return ERR_CFG_NOT_SUPPORT;
            }

            NODE_WILDCARD_CVT(pszTmp);

            /* 增加 对 pszTmp 作转义 */
            CFG_NUM_TO_NODE(pszTmp);

            pstChild = scew_element_by_name(pstNode, pszTmp);
            if (NULL == pstChild)
            {
                pstChild = scew_element_add(pstNode, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);
                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }
            }
            pstNode = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

    CFG_ADD_CARE_MID(pstNode, usMID);

    return CFG_OK;
}


/*************************************************************************
Function:      long CFG_UpdateMaxIdx(const scew_element *pstNode)
Description:   刷新节点的最大实例号
Calls:         无
Data Accessed:
Data Updated:
Input:         pstNode, 节点指针
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_UpdateMaxIdx(const scew_element *pstNode)
{
    scew_element *pstChild = NULL;
    unsigned long ulMaxIdx = 0;
    unsigned long ulIdx = 0;
    const char *pcName = NULL;
    const char *pcMaxIdx = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    pstAttr = scew_attribute_by_name(pstNode, ATTR_MAX_IDX);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        ulMaxIdx = (unsigned long)strtol(pcAttrVal, NULL, 10);
    }
    else
    {
        ulMaxIdx = 0;
    }

    while (pstChild = scew_element_next(pstNode, pstChild),
           NULL != pstChild)
    {
        pcName = scew_element_name(pstChild);
//        pcName++;
        ulIdx = (unsigned long)strtol(pcName, NULL, 10);
        if (ulIdx > ulMaxIdx)
        {
            ulMaxIdx = ulIdx;
            pcMaxIdx = pcName;
        }
    }

    if (NULL != pcMaxIdx)
    {
        CFG_SETATTR(pstNode, ATTR_MAX_IDX, pcMaxIdx);
    }
    return CFG_OK;
}


/*************************************************************************
Function:      CFG_RET CFG_MIDTreeInit(void)
Description:   逻辑树初始化
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_MIDTreeInit(void)
{
    char *pcRoot = NULL;
    CFG_RET ret = CFG_OK;
    unsigned long ulLen = 0;

    ret = CFG_ReadFullCfg(&pcRoot, 0, 256, &ulLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }
    pcRoot[ulLen] = '\0';
#if 0
    pcRoot = "<InternetGatewayDevice></InternetGatewayDevice>";
#endif

    /* 增加对数字的转义 */
    CFG_FileTransMean(pcRoot, ulLen);

    ret = CFG_XmlInitPaser(&s_pstCfgMIDTree, pcRoot);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }
    free(pcRoot);

    /* 将节点名称转义回去 */
    ret = CFG_TreeTransBack(s_pstCfgMIDTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    return ret;
}

/*************************************************************************
Function:      CFG_RET CFG_MIDTreeFinal(void)
Description:   逻辑树释放
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        无
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_MIDTreeFinal(void)
{
    if (NULL != s_pstCfgMIDTree)
    {
        scew_tree_free(s_pstCfgMIDTree);
        s_pstCfgMIDTree = NULL;
    }
    return CFG_OK;
}


CFG_RET CFG_AddMIDPath(const char *pcPath, unsigned short usMID);

/***********************************************************
接口:   注册节点的MID
参数:   pcPath, 节点路径, 如果以'.'结尾, 则表示其下所有子节点都归属这个MID
        usMID: 模块id
返回值: 0:成功
        其它:失败
备注:   如果同一个节点被注册多次,以最后一次的为准.
***********************************************************/
CFG_RET CFG_RegisterMID(const char *pcPath, unsigned short usMID)
{
    CFG_RET ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    char acMID[32];

    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;

    if (NULL == pcPath || 0 == usMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    ret = CFG_AddMIDPath(pcPath, usMID);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* 将信息保存到MID树 */
    ret = CFG_RegMIDToMIDTree(pcPath, usMID);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ret = CFG_GetWildMIDNodesPtr(pcPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    sprintf(acMID, "%04x", usMID);

    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;

        CFG_SETATTR(apstTree[ulLoop], ATTR_MID, acMID);
    }

    return CFG_OK;
}





/***********************************************************
接口:   注册关心此节点的MID
参数:   pcPath, 节点路径
        usMID: 模块id
返回值: 0:成功
        其它:失败
备注:
1．pcPath必须到叶子节点, 也就是说CareMID的作用范围为指定节点.
2．如果同一个路径被注册多次, 每个都有效.
3．节点路径支持通配符.在通配符的基础上若再指明特定实例属于被另一个模块关心,
则该实
例对两个模块都有效.

***********************************************************/
CFG_RET CFG_RegisterCareMID(const char *pcPath, unsigned short usMID)
{
    CFG_RET ret = CFG_OK;

    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;

    if (NULL == pcPath || 0 == usMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* 将信息保存到MID树 */
    ret = CFG_RegCareMIDToMIDTree(pcPath, usMID);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ret = CFG_GetWildMIDNodesPtr(pcPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;

        CFG_ADD_CARE_MID(apstTree[ulLoop], usMID);
    }

    return CFG_OK;
}

typedef CFG_RET (*FUNC_VAL_TYPE_CHECK)(const char *pcVal);



#define CFG_IS_ALPHA(c) (('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define CFG_IS_HEX_ALPHA(c) (('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define CFG_IS_SIGN(c) ('-' == (c) || (c) == '+')
#define CFG_IS_PLUS_SIGN(c) ('+' == (c))
#define CFG_IS_MINUS_SIGN(c) ('-' == (c))
#define CFG_IS_NUM(c) ('0' <= (c) && (c) <= '9')
#define CFG_IS_PLUS_NUM(c) ('0' < (c) && (c) <= '9')
#define CFG_IS_ZERO_NUM(c) ('0' == (c))
#define CFG_IS_BOOL_NUM(c) ('0' == (c) || (c) == '1')
#define CFG_IS_END(c) ('\0' == (c))


/***********************************************************
接口:   对象类节点值合法性检查
参数:   pcVal, 具体值
返回值: 非法
备注:
***********************************************************/
CFG_RET CFG_objectValTypeCheck(const char *pcVal)
{
    (void)pcVal;

    return ERR_CFG_INVALID_OBJ_VAL;
}

/***********************************************************
接口:   string类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:成功
备注:
***********************************************************/
CFG_RET CFG_stringValTypeCheck(const char *pcVal, unsigned int nLen)
{
    if (strlen(pcVal) > nLen)
    {
        return ERR_STR_TOO_LONG;
    }

    return CFG_OK;
}


/***********************************************************
接口:   int类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:合法,
        其它, 非法
备注:
***********************************************************/
CFG_RET CFG_intValTypeCheck(const char *pcVal, int iMin, int iMax)
{
    /*
    1.第一个字符为有效数字(1-9)或者'-',
    2.后面字符全部为数字
    3.除符号外必须以有效数字开头
    */
    return tbsCheckIntRangeEx(pcVal, iMin, iMax);
}

/***********************************************************
接口:   unsigned int类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:合法,
        其它, 非法
备注:
***********************************************************/
CFG_RET CFG_unsignedIntValTypeCheck(const char *pcVal, unsigned int nMin, unsigned int nMax)
{
    /*
    1.后面字符全部为数字
    2.必须以有效数字开头
    */
    return tbsCheckUIntRangeEx(pcVal, nMin, nMax);
}

/***********************************************************
接口:   boolean类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:合法,
        其它, 非法
备注:
***********************************************************/
CFG_RET CFG_booleanValTypeCheck(const char *pcVal)
{
    /*
    1.必须是'0'/'1', 或者 'true'/'false'
    */
    return tbsCheckEnableEx(pcVal);
}


/***********************************************************
接口:   date类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:成功
备注:
***********************************************************/
CFG_RET CFG_dateTimeValTypeCheck(const char *pcVal)
{
    /* 形如 0000-00-02T03:04:05 */

    (void)pcVal;

    return CFG_OK;
}

/***********************************************************
接口:   base64类节点值合法性检查
参数:   pcVal, 具体值
返回值: 0:合法,
        其它, 非法
备注:
***********************************************************/
CFG_RET CFG_base64ValTypeCheck(const char *pcVal, unsigned int nLen)
{
    /* 每个字符必须是数字或者 a-f A-F */
    const char *pcPos = pcVal;

    while (!CFG_IS_END(*pcPos))
    {
        if (!CFG_IS_NUM(*pcPos) && !CFG_IS_HEX_ALPHA(*pcPos))
        {
            return ERR_CFG_INVALID_HEX;
        }

        pcPos++;
    }

    if (pcPos - pcVal > nLen)
    {
        return ERR_STR_TOO_LONG;
    }

    return CFG_OK;
}

/* 节点类型枚举 */
typedef enum
{
    EN_CFG_VAL_TYPE_NONE = 0,
    EN_CFG_VAL_TYPE_OBJ,
    EN_CFG_VAL_TYPE_STR,
    EN_CFG_VAL_TYPE_INT,
    EN_CFG_VAL_TYPE_UINT,
    EN_CFG_VAL_TYPE_BOOL,
    EN_CFG_VAL_TYPE_DATE,
    EN_CFG_VAL_TYPE_BASE64,
} EN_CFG_VAL_TYPE;

/* 节点类型参数 */
typedef struct
{
    EN_CFG_VAL_TYPE enType;
    union
    {
        struct
        {
            unsigned int nMaxLen;
        } str;
        struct
        {
            int iMin;
            int iMax;
        } sint;
        struct
        {
            unsigned int nMin;
            unsigned int nMax;
        } uint;
    }u;
} ST_CFG_VAL_TYPE_CHECK;


#define CFG_TYPE_CHECK_ITEM(pcType, enType) \
    {#pcType, sizeof(#pcType) - 1, enType}

/* 类型匹配数组 */
static struct
{
    const char *pcType;
    unsigned int nTypeLen;
    EN_CFG_VAL_TYPE enType;
} s_astCfgValTypeCheck[] =
{
    CFG_TYPE_CHECK_ITEM(object,         EN_CFG_VAL_TYPE_OBJ),
    CFG_TYPE_CHECK_ITEM(string,         EN_CFG_VAL_TYPE_STR),
    CFG_TYPE_CHECK_ITEM(int,            EN_CFG_VAL_TYPE_INT),
    CFG_TYPE_CHECK_ITEM(unsignedInt,    EN_CFG_VAL_TYPE_UINT),
    CFG_TYPE_CHECK_ITEM(boolean,        EN_CFG_VAL_TYPE_BOOL),
    CFG_TYPE_CHECK_ITEM(dateTime,       EN_CFG_VAL_TYPE_DATE),
    CFG_TYPE_CHECK_ITEM(base64,         EN_CFG_VAL_TYPE_OBJ)
};


/* string/string(64)/string(64K) */
CFG_RET CFG_ParseSize(const char *pcPos, unsigned int *pnSize)
{
    const char *pcStart = NULL;
    const char *pcEnd = NULL;
    int ret = CFG_OK;

    if('\0' == *pcPos)
    {
        *pnSize = UINT_MAX;
    }
    else if ('(' == *pcPos)
    {
        pcPos++;
        pcEnd = strchr(pcPos, ')');
        if (pcEnd && '\0' == *(pcEnd+1))
        {
            pcStart = pcPos;
            *pnSize = strtoul(pcPos, (char **)&pcPos, 10);
            if (pcStart == pcPos)
            {
                return ERR_CFG_INVALID_TYPE;
            }

            if ('k' == *pcPos || 'K' == *pcPos)
            {
                (*pnSize) *= 1024;
                pcPos++;
            }
            else if (pcEnd == pcPos)
            {
            }
            else
            {
                return ERR_CFG_INVALID_TYPE;
            }

            if (pcPos != pcEnd)
            {
                ret = ERR_CFG_INVALID_TYPE;
            }
        }
        else
        {
            ret = ERR_CFG_INVALID_TYPE;
        }
    }
    else
    {
        ret = ERR_CFG_INVALID_TYPE;
    }

    return ret;
}

/* int/int[:4094]/int[-1:]/int[-1:4094] */
CFG_RET CFG_ParseInt(const char *pcPos, int *piMin, int *piMax)
{
    const char *pcEnd = pcPos;
    int ret = CFG_OK;

    if('\0' == *pcPos)
    {
        *piMin = INT_MIN;
        *piMax = INT_MAX;
    }
    else if ('[' == *pcPos)
    {
        pcPos++;
        pcEnd = strchr(pcPos, ']');
        if (pcEnd && '\0' == *(pcEnd+1))
        {
            if (':' == *pcPos)
            {
                *piMin = INT_MIN;
                pcPos++;

                *piMax = strtoul(pcPos, (char **)&pcPos, 10);
            }
            else
            {
                *piMin = strtoul(pcPos, (char **)&pcPos, 10);
                if (':' != *pcPos)
                {
                    return ERR_CFG_INVALID_TYPE;
                }
                pcPos++;

                if (pcEnd == pcPos)
                {
                    *piMax = INT_MAX;
                }
                else
                {
                    *piMax = strtoul(pcPos, (char **)&pcPos, 10);
                }
            }

            if (pcPos != pcEnd)
            {
                ret = ERR_CFG_INVALID_TYPE;
            }
        }
        else
        {
            ret = ERR_CFG_INVALID_TYPE;
        }
    }
    else
    {
        ret = ERR_CFG_INVALID_TYPE;
    }

    if (*piMin > *piMax)
    {
        ret = ERR_CFG_INVALID_TYPE;
    }

    return ret;
}

/* unsignedInt/unsignedInt[:4094]/unsignedInt[1:]/unsignedInt[1:4094] */
CFG_RET CFG_ParseUInt(const char *pcPos, unsigned int *pnMin, unsigned int *pnMax)
{
    const char *pcEnd = pcPos;
    int ret = CFG_OK;

    if('\0' == *pcPos)
    {
        *pnMin = 0;
        *pnMax = UINT_MAX;
    }
    else if ('[' == *pcPos)
    {
        pcPos++;
        pcEnd = strchr(pcPos, ']');
        if (pcEnd && '\0' == *(pcEnd+1))
        {
            if (':' == *pcPos)
            {
                *pnMin = 0;
                pcPos++;

                *pnMax = strtoul(pcPos, (char **)&pcPos, 10);
            }
            else
            {
                *pnMin = strtoul(pcPos, (char **)&pcPos, 10);
                if (':' != *pcPos)
                {
                    return ERR_CFG_INVALID_TYPE;
                }
                pcPos++;

                if (pcEnd == pcPos)
                {
                    *pnMax = UINT_MAX;
                }
                else
                {
                    *pnMax = strtoul(pcPos, (char **)&pcPos, 10);
                }
            }

            if (pcPos != pcEnd)
            {
                ret = ERR_CFG_INVALID_TYPE;
            }
        }
        else
        {
            ret = ERR_CFG_INVALID_TYPE;
        }
    }
    else
    {
        ret = ERR_CFG_INVALID_TYPE;
    }

    if (*pnMin > *pnMax)
    {
        ret = ERR_CFG_INVALID_TYPE;
    }

    return ret;
}
/***********************************************************
接口:   将节点的type属性装换成type结构体
参数:   pcType, 类型值
        pstType, 类型结构
返回值: 0:成功
备注:
***********************************************************/
CFG_RET CFG_CnvtValType(const char *pcType, ST_CFG_VAL_TYPE_CHECK *pstType)
{
    int i = 0;
    int ret = CFG_OK;

    const char *pcPos = pcType;
    memset(pstType, 0, sizeof(ST_CFG_VAL_TYPE_CHECK));

    for (i = 0; i < ARRAY_SIZE(s_astCfgValTypeCheck); i++)
    {
        if (0 == strncmp(pcType, s_astCfgValTypeCheck[i].pcType,
                          s_astCfgValTypeCheck[i].nTypeLen))
        {
            pstType->enType = s_astCfgValTypeCheck[i].enType;
            pcPos += s_astCfgValTypeCheck[i].nTypeLen;
            break;
        }
    }

    switch(pstType->enType)
    {
        case EN_CFG_VAL_TYPE_OBJ:
            break;
        case EN_CFG_VAL_TYPE_BASE64:
        case EN_CFG_VAL_TYPE_STR:
            ret = CFG_ParseSize(pcPos, &(pstType->u.str.nMaxLen));
            break;
        case EN_CFG_VAL_TYPE_INT:
            ret = CFG_ParseInt(pcPos, &(pstType->u.sint.iMin), &(pstType->u.sint.iMax));
            break;
        case EN_CFG_VAL_TYPE_UINT:
            ret = CFG_ParseUInt(pcPos, &(pstType->u.uint.nMin), &(pstType->u.uint.nMax));
            break;
        case EN_CFG_VAL_TYPE_BOOL:
            break;
        case EN_CFG_VAL_TYPE_DATE:
            break;
        default:
            ret = ERR_CFG_INVALID_TYPE;
            break;
    }

    return ret;
}

/***********************************************************
接口:   根据节点类型，检查节点值合法性的总入口
参数:   pstType, 节点类型结构
        pcVal, 节点值
返回值: 0:成功
备注:
***********************************************************/
CFG_RET CFG_CheckValByType(const char *pcVal, const char *pcType)
{
    CFG_RET ret = CFG_OK;
    ST_CFG_VAL_TYPE_CHECK stType;

    ret = CFG_CnvtValType(pcType, &stType);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    switch(stType.enType)
    {
        case EN_CFG_VAL_TYPE_OBJ:
            ret = CFG_objectValTypeCheck(pcVal);
            break;
        case EN_CFG_VAL_TYPE_STR:
            ret = CFG_stringValTypeCheck(pcVal, stType.u.str.nMaxLen);
            break;
        case EN_CFG_VAL_TYPE_INT:
            ret = CFG_intValTypeCheck(pcVal, stType.u.sint.iMin, stType.u.sint.iMax);
            break;
        case EN_CFG_VAL_TYPE_UINT:
            ret = CFG_unsignedIntValTypeCheck(pcVal, stType.u.uint.nMin, stType.u.uint.nMax);
            break;
        case EN_CFG_VAL_TYPE_BOOL:
            ret = CFG_booleanValTypeCheck(pcVal);
            break;
        case EN_CFG_VAL_TYPE_DATE:
            ret = CFG_dateTimeValTypeCheck(pcVal);
            break;
        case EN_CFG_VAL_TYPE_BASE64:
            ret = CFG_base64ValTypeCheck(pcVal, stType.u.str.nMaxLen);
            break;
        default:
            ret = ERR_CFG_INVALID_TYPE;
            break;
    }

    return ret;
}


/***********************************************************
接口:   根据节点路径，检查节点值合法性
参数:   pcPath: 节点路径,
        pcVal: 节点值,
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_CheckNodeVal(const char *pcPath, const char *pcVal)
{
    const scew_element *pstNode = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcType = NULL;
    CFG_RET ret = CFG_OK;

    /* 复制路径并查找节点 */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    pstAttr = scew_attribute_by_name(pstNode, ATTR_TYPE);
    if (NULL != pstAttr)
    {
        pcType = scew_attribute_value(pstAttr);

        ret = CFG_CheckValByType(pcVal, pcType);
    }

    return ret;
}



/***********************************************************
接口:   获取节点所属的MID和Care MID 列表
参数:   pcPath, 节点路径
       pcAccessor, 访问者。业务代理模块使用时可直接传NULL
       pusMid, 输出MID
       pusCareMIDList, 关心的MID列表, 以0作为结束符.
       ppvNode, 输出子树. 模块使用时可直接传NULL
       pcVal, 将会被设置成的值, 如果不为空这里主要用来检查值的类型是否匹配.
返回值: 0:成功
        其它:失败, 含无权限访问的情况
备注:
***********************************************************/
CFG_RET CFG_GetNodeMIDList(const char *pcPath, const char *pcAccessor,
                    unsigned short *pusMID, unsigned short * pusCareMIDList,
                    unsigned long *pulCareMIDCount,
                    void **ppvNode, const char *pcVal)
{
    /* 先在节点树中查找并验证访问权限 */
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    const char *pcValOld = NULL;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD
                         | CFG_NODE_INFO_ACCESSLIST
                         | CFG_NODE_INFO_MID;

    if (NULL == pcPath || NULL == pusMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    /* 如果值一致, 则不检查权限 */
    if (NULL != pcVal)
    {
        pcValOld = scew_element_contents(pstNode);
        if (NULL == pcValOld)
        {
            pcValOld = "";
        }
        if ((NULL == pcValOld) || (0 != strcmp(pcValOld, pcVal)))
        {
            CFG_SET_RIGHT_CHECK(pstNode, &stNodeInfo, pcAccessor, CFG_ADDABLE_C);

            /*
              这里屏蔽cfg自己的节点值检查，是因为在这里检查所返回的错误码描述不太具体，
              所以改成在后面的公共处理函数CommonSetMsgHandler中找不到模块定义的检查函数之后，
              才调用cfg的通用检查函数
            */
            #if 0
            pstAttr = scew_attribute_by_name(pstNode, ATTR_TYPE);
            if (NULL != pstAttr)
            {
                pcAttrVal = scew_attribute_value(pstAttr);

                /* 检查设置成的值与其类型是否合格 */
                ret = CFG_CheckValByType(pcVal, pcAttrVal);
                if (CFG_OK != ret)
                {
                    CFG_ERR(ret, "%s %s %s", pcPath, pcVal, pcAttrVal);

                    return ret;
                }
            }
            #endif
        }
    }
    else
    {
        CFG_SET_RIGHT_CHECK(pstNode, &stNodeInfo, pcAccessor, CFG_ADDABLE_C);
    }

    *pusMID = stNodeInfo.usMID;

    if (NULL != pusCareMIDList)
    {
        *pulCareMIDCount = 0;
        CFG_GET_CARE_MID(pstNode, pusCareMIDList, pulCareMIDCount);
    }
    if (NULL != ppvNode)
    {
        *ppvNode = pstNode;
    }

    return CFG_OK;
}




/***********************************************************
接口:   获取节点所属的MID
参数:   pcPath, 节点路径
       pcAccessor, 访问者。模块使用时可直接传NULL
       pusMid, 输出MID
       pucAccessable, 输出访问者是否具有权限, 模块使用时可直接传NULL
       ppvNode, 输出子树. 模块使用时可直接传NULL
返回值: 0:成功
        其它:失败
备注:   模块使用时一般按照如下方式即可
       CFG_GetNodeMID(pcPath, NULL, &usMID, NULL, NULL)
***********************************************************/
CFG_RET CFG_GetNodeMID(const char *pcPath, unsigned short *pusMID)
{
    return CFG_GetNodeMIDList(pcPath, NULL, pusMID, NULL, NULL, NULL, NULL);
}











/***********************************************************
接口:   添加节点
参数:   pcPath: 节点路径
        pcType: 节点类型, 可以为空
        pcVal: 节点值, 可以为空
        ucIsStandard, 是否标准节点
        ucNeedSave, 是否需要保存
        ucWritable, 对TR069节点是否可写,
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_AddNode(const char *pcPath, const char *pcType, const char *pcVal,
                 unsigned char ucIsStandard, unsigned char ucNeedSave,
                 unsigned char ucWritable)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element *pstChild = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    long ret = CFG_OK;
    const char *pcValNew = NULL;

    scew_element *pstBig = NULL;
    scew_element *pstMatch = NULL;
    const char *pcMatchMID = NULL;
    unsigned char ucNewNode = 0;
    char acMIDList[CFG_MAX_MIDLIST_LEN];

    if (NULL == pcPath || NULL == pcType || '\0' == pcType[0])
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    acMIDList[0] = '\0';

    pstBig = scew_tree_root(g_pstCfgTree);
    pstMatch = scew_tree_root(s_pstCfgMIDTree);

    /* 复制路径 */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* 如果有提供节点路径, 检查根节点是否匹配 */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstBig);

        /* 逐级解析路径 */
        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            /* 增加 对 pszTmp 作转义 */
            CFG_NUM_TO_NODE(pszTmp);

            /* 取得小树上匹配分支 */
            if (NULL != pstMatch)
            {
                if (CFG_IS_NODE_NUM(pszTmp))  /* 当前节点是数字 */
                {
                    /* 直接查找通配分支 */
                    pstMatch = scew_element_by_name(pstMatch, NODE_WILDCARD);
                }
                else
                {
                    /* 精确查找 */
                    pstMatch = scew_element_by_name(pstMatch, pszTmp);
                }
            }
            /* 在大树上寻找节点 */
            pstChild = scew_element_by_name(pstBig, pszTmp);
            if (NULL == pstChild)  /* 不存在 */
            {
                /* 新建 */
                pstChild = scew_element_add(pstBig, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);

                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }

                ucNewNode = 1;

                if (CFG_IS_NODE_NUM(pszTmp))  /* 节点名是数字 */
                {
                    /* 更新最大index */
                    ret = CFG_UpdateMaxIdx(pstBig);
                    if (CFG_OK != ret)
                    {
                        CFG_FREE_PATH(pcPathTmp);

                        CFG_ERR(ret);
                        return ret;
                    }
                }

                CFG_ADDATTR(pstBig, ATTR_TYPE, TYPE_OBJECT, (CFG_FREE_PATH(pcPathTmp)));

                if (NULL != pstMatch)  /* 小树上匹配分支不为空 */
                {
                    pstAttr = scew_attribute_by_name(pstMatch, ATTR_MID);
                    if (NULL != pstAttr)     /* 该分支上有注册MID */
                    {
                        pcMatchMID = scew_attribute_value(pstAttr);

                        /* 增加大树本节点的MID */
                        CFG_ADDATTR(pstChild, ATTR_MID, pcMatchMID, CFG_FREE_PATH(pcPathTmp));
                    }
                }
            }
            pstBig = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

#if 0
    if (!ucNewNode)  /* 节点不是新增加 */
    {
        /* 报告节点已经存在 */
        CFG_ERR(CFG_NODE_EXISTED);
        return CFG_NODE_EXISTED;
    }
#endif

    /* 更新各个属性 */
    if (NULL != pcVal)
    {
        pcValNew = scew_element_set_contents(pstBig, pcVal);
        if (NULL == pcValNew)
        {
            CFG_ERR(ERR_CFG_INTERNAL_ERR);
            return ERR_CFG_INTERNAL_ERR;
        }
    }

    CFG_SETATTR(pstBig, ATTR_TYPE, pcType);

    if (!ucIsStandard )
    {
        /* 设置为非标准 */
        CFG_SETATTR(pstBig, ATTR_STANDARD, "0");
    }
    if (!ucNeedSave)
    {
        /* 设置为无须保存 */
        CFG_SETATTR(pstBig, ATTR_NEEDSAVE, "0");
    }
    if (!ucWritable)
    {
        char acWritable[] = "0";

        acWritable[0] += ucWritable;

        /* 设置为只读 */
        CFG_SETATTR(pstBig, ATTR_WRITABLE, acWritable);
    }

    /* 增加CareMID */
    if (NULL != pstMatch && ucNewNode)
    {
        /* 取CareMID */
        pstAttr = scew_attribute_by_name(pstMatch, ATTR_CAREMID);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);

            /* 增加CareMID */
            strcpy(acMIDList, pcAttrVal);
        }
    }
    if (acMIDList[0] != '\0' && ucNewNode)
    {
        CFG_ADDATTR(pstBig, ATTR_CAREMID, acMIDList, ((void)0));
    }

    return CFG_OK;
}

/***********************************************************
接口:   添加节点
参数:   pcPath: 节点路径
        pcType: 节点类型, 可以为空
        pcVal: 节点值, 可以为空
        ucIsStandard, 是否标准节点
        ucNeedSave, 是否需要保存
        ucWritable, 对TR069节点是否可写,
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_AddNodeEx(const char *pcPath, const char *pcType, const char *pcVal,
                 unsigned char ucIsStandard, unsigned char ucNeedSave,
                 unsigned char ucWritable, unsigned char ucNotify)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element *pstChild = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    long ret = CFG_OK;
    const char *pcValNew = NULL;

    scew_element *pstBig = NULL;
    scew_element *pstMatch = NULL;
    const char *pcMatchMID = NULL;
    unsigned char ucNewNode = 0;
    char acMIDList[CFG_MAX_MIDLIST_LEN];

    if (NULL == pcPath || NULL == pcType || '\0' == pcType[0])
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    acMIDList[0] = '\0';

    pstBig = scew_tree_root(g_pstCfgTree);
    pstMatch = scew_tree_root(s_pstCfgMIDTree);

    /* 复制路径 */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* 如果有提供节点路径, 检查根节点是否匹配 */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstBig);

        /* 逐级解析路径 */
        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            /* 增加 对 pszTmp 作转义 */
            CFG_NUM_TO_NODE(pszTmp);

            /* 取得小树上匹配分支 */
            if (NULL != pstMatch)
            {
                if (CFG_IS_NODE_NUM(pszTmp))  /* 当前节点是数字 */
                {
                    /* 直接查找通配分支 */
                    pstMatch = scew_element_by_name(pstMatch, NODE_WILDCARD);
                }
                else
                {
                    /* 精确查找 */
                    pstMatch = scew_element_by_name(pstMatch, pszTmp);
                }
            }
            /* 在大树上寻找节点 */
            pstChild = scew_element_by_name(pstBig, pszTmp);
            if (NULL == pstChild)  /* 不存在 */
            {
                /* 新建 */
                pstChild = scew_element_add(pstBig, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);

                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }

                ucNewNode = 1;

                if (CFG_IS_NODE_NUM(pszTmp))  /* 节点名是数字 */
                {
                    /* 更新最大index */
                    ret = CFG_UpdateMaxIdx(pstBig);
                    if (CFG_OK != ret)
                    {
                        CFG_FREE_PATH(pcPathTmp);

                        CFG_ERR(ret);
                        return ret;
                    }
                }

                CFG_ADDATTR(pstBig, ATTR_TYPE, TYPE_OBJECT, (CFG_FREE_PATH(pcPathTmp)));

                if (NULL != pstMatch)  /* 小树上匹配分支不为空 */
                {
                    pstAttr = scew_attribute_by_name(pstMatch, ATTR_MID);
                    if (NULL != pstAttr)     /* 该分支上有注册MID */
                    {
                        pcMatchMID = scew_attribute_value(pstAttr);

                        /* 增加大树本节点的MID */
                        CFG_ADDATTR(pstChild, ATTR_MID, pcMatchMID, CFG_FREE_PATH(pcPathTmp));
                    }
                }
            }
            pstBig = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

#if 0
    if (!ucNewNode)  /* 节点不是新增加 */
    {
        /* 报告节点已经存在 */
        CFG_ERR(CFG_NODE_EXISTED);
        return CFG_NODE_EXISTED;
    }
#endif

    /* 更新各个属性 */
    if (NULL != pcVal)
    {
        pcValNew = scew_element_set_contents(pstBig, pcVal);
        if (NULL == pcValNew)
        {
            CFG_ERR(ERR_CFG_INTERNAL_ERR);
            return ERR_CFG_INTERNAL_ERR;
        }
    }

    CFG_SETATTR(pstBig, ATTR_TYPE, pcType);

    if (!ucIsStandard )
    {
        /* 设置为非标准 */
        CFG_SETATTR(pstBig, ATTR_STANDARD, "0");
    }
    if (!ucNeedSave)
    {
        /* 设置为无须保存 */
        CFG_SETATTR(pstBig, ATTR_NEEDSAVE, "0");
    }
    if (!ucWritable)
    {
        char acWritable[] = "0";

        acWritable[0] += ucWritable;

        /* 设置为只读 */
        CFG_SETATTR(pstBig, ATTR_WRITABLE, acWritable);
    }

    if (ucNotify)
    {
        if (1 == ucNotify)
        {
            CFG_SETATTR(pstBig, ATTR_NOTI, "1");
        }
        else if(2 == ucNotify)
        {
            CFG_SETATTR(pstBig, ATTR_NOTI, "2");
        }
    }

    /* 增加CareMID */
    if (NULL != pstMatch && ucNewNode)
    {
        /* 取CareMID */
        pstAttr = scew_attribute_by_name(pstMatch, ATTR_CAREMID);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);

            /* 增加CareMID */
            strcpy(acMIDList, pcAttrVal);
        }
    }
    if (acMIDList[0] != '\0' && ucNewNode)
    {
        CFG_ADDATTR(pstBig, ATTR_CAREMID, acMIDList, ((void)0));
    }

    return CFG_OK;
}


/***********************************************************
接口:   删除节点
参数:   pcPath: 节点路径
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_DelNode(const char *pcPath)
{
    scew_element *pstNode = NULL;
    scew_element *pstFather = NULL;
    CFG_RET ret = CFG_OK;

    if (NULL == pcPath)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* 复制路径并查找节点 */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    pstFather = pstNode->parent;
    scew_element_del(pstNode);
    (void)CFG_UpdateObjCurInstCount(pstFather);

    return CFG_OK;
}


/***********************************************************
接口:   获取mid树指针
参数:   无
返回值: mid树指针
***********************************************************/
void *CFG_GetMIDTree(void)
{
    return s_pstCfgMIDTree;
}





#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif





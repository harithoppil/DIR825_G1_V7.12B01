/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_mid.c
 �ļ����� : cfg ģ���и� �߼�����صĲ�������
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2008-04-06
            ���� :
          2 �޸� : ��Ծ��
            ���� : 2008-11-06
            ���� : ֮ǰ��mid����Ҫ�Ǵ��mid֮��, ���ڸ�Ϊ�˴����������ģ��

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

/* MID �� */
STATIC scew_tree *s_pstCfgMIDTree = NULL;

/*************************************************************************
Function:      CFG_RET CFG_RegMIDToMIDTree(const char *pcPath, unsigned short usMID)
Description:   ��MIDע�ᵽ�߼�����
Calls:         ��
Data Accessed:
Data Updated:
Input:         pcPath, ע��·��, ������ͨ��·��
               usMID, Ҫע���mid
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pstNode = scew_tree_root(s_pstCfgMIDTree);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);
    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
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

            /* ���� �� pszTmp ��ת�� */
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
Description:   ��һ���ڵ������Care mid
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstNode, �ڵ��ַ
               usMID, Ҫ��ӵ�mid
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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

        /* �ȼ���Ƿ��ظ� */
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

        /* ����MID��ӵ� ĩβ */
        strcpy(acMIDList, pcAttrVal);
        acMIDList[ulOldLen] = ' ';
        strcpy(&acMIDList[ulOldLen + 1], acMID);

        /* ������ֵ���� */
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
Description:   ��CareMIDע�ᵽ�߼�����
Calls:         ��
Data Accessed:
Data Updated:
Input:         pcPath, ע��·��, ������ͨ��·��
               usMID, Ҫע���mid
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pstNode = scew_tree_root(s_pstCfgMIDTree);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);
    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
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

            /* ���� �� pszTmp ��ת�� */
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
Description:   ˢ�½ڵ�����ʵ����
Calls:         ��
Data Accessed:
Data Updated:
Input:         pstNode, �ڵ�ָ��
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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
Description:   �߼�����ʼ��
Calls:         ��
Data Accessed:
Data Updated:
Input:         ��
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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

    /* ���Ӷ����ֵ�ת�� */
    CFG_FileTransMean(pcRoot, ulLen);

    ret = CFG_XmlInitPaser(&s_pstCfgMIDTree, pcRoot);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }
    free(pcRoot);

    /* ���ڵ�����ת���ȥ */
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
Description:   �߼����ͷ�
Calls:         ��
Data Accessed:
Data Updated:
Input:         ��
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
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
�ӿ�:   ע��ڵ��MID
����:   pcPath, �ڵ�·��, �����'.'��β, ���ʾ���������ӽڵ㶼�������MID
        usMID: ģ��id
����ֵ: 0:�ɹ�
        ����:ʧ��
��ע:   ���ͬһ���ڵ㱻ע����,�����һ�ε�Ϊ׼.
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

    /* ����Ϣ���浽MID�� */
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
�ӿ�:   ע����Ĵ˽ڵ��MID
����:   pcPath, �ڵ�·��
        usMID: ģ��id
����ֵ: 0:�ɹ�
        ����:ʧ��
��ע:
1��pcPath���뵽Ҷ�ӽڵ�, Ҳ����˵CareMID�����÷�ΧΪָ���ڵ�.
2�����ͬһ��·����ע����, ÿ������Ч.
3���ڵ�·��֧��ͨ���.��ͨ����Ļ���������ָ���ض�ʵ�����ڱ���һ��ģ�����,
���ʵ
��������ģ�鶼��Ч.

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

    /* ����Ϣ���浽MID�� */
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
�ӿ�:   ������ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: �Ƿ�
��ע:
***********************************************************/
CFG_RET CFG_objectValTypeCheck(const char *pcVal)
{
    (void)pcVal;

    return ERR_CFG_INVALID_OBJ_VAL;
}

/***********************************************************
�ӿ�:   string��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�ɹ�
��ע:
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
�ӿ�:   int��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�Ϸ�,
        ����, �Ƿ�
��ע:
***********************************************************/
CFG_RET CFG_intValTypeCheck(const char *pcVal, int iMin, int iMax)
{
    /*
    1.��һ���ַ�Ϊ��Ч����(1-9)����'-',
    2.�����ַ�ȫ��Ϊ����
    3.���������������Ч���ֿ�ͷ
    */
    return tbsCheckIntRangeEx(pcVal, iMin, iMax);
}

/***********************************************************
�ӿ�:   unsigned int��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�Ϸ�,
        ����, �Ƿ�
��ע:
***********************************************************/
CFG_RET CFG_unsignedIntValTypeCheck(const char *pcVal, unsigned int nMin, unsigned int nMax)
{
    /*
    1.�����ַ�ȫ��Ϊ����
    2.��������Ч���ֿ�ͷ
    */
    return tbsCheckUIntRangeEx(pcVal, nMin, nMax);
}

/***********************************************************
�ӿ�:   boolean��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�Ϸ�,
        ����, �Ƿ�
��ע:
***********************************************************/
CFG_RET CFG_booleanValTypeCheck(const char *pcVal)
{
    /*
    1.������'0'/'1', ���� 'true'/'false'
    */
    return tbsCheckEnableEx(pcVal);
}


/***********************************************************
�ӿ�:   date��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_dateTimeValTypeCheck(const char *pcVal)
{
    /* ���� 0000-00-02T03:04:05 */

    (void)pcVal;

    return CFG_OK;
}

/***********************************************************
�ӿ�:   base64��ڵ�ֵ�Ϸ��Լ��
����:   pcVal, ����ֵ
����ֵ: 0:�Ϸ�,
        ����, �Ƿ�
��ע:
***********************************************************/
CFG_RET CFG_base64ValTypeCheck(const char *pcVal, unsigned int nLen)
{
    /* ÿ���ַ����������ֻ��� a-f A-F */
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

/* �ڵ�����ö�� */
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

/* �ڵ����Ͳ��� */
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

/* ����ƥ������ */
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
�ӿ�:   ���ڵ��type����װ����type�ṹ��
����:   pcType, ����ֵ
        pstType, ���ͽṹ
����ֵ: 0:�ɹ�
��ע:
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
�ӿ�:   ���ݽڵ����ͣ����ڵ�ֵ�Ϸ��Ե������
����:   pstType, �ڵ����ͽṹ
        pcVal, �ڵ�ֵ
����ֵ: 0:�ɹ�
��ע:
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
�ӿ�:   ���ݽڵ�·�������ڵ�ֵ�Ϸ���
����:   pcPath: �ڵ�·��,
        pcVal: �ڵ�ֵ,
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_CheckNodeVal(const char *pcPath, const char *pcVal)
{
    const scew_element *pstNode = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcType = NULL;
    CFG_RET ret = CFG_OK;

    /* ����·�������ҽڵ� */
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
�ӿ�:   ��ȡ�ڵ�������MID��Care MID �б�
����:   pcPath, �ڵ�·��
       pcAccessor, �����ߡ�ҵ�����ģ��ʹ��ʱ��ֱ�Ӵ�NULL
       pusMid, ���MID
       pusCareMIDList, ���ĵ�MID�б�, ��0��Ϊ������.
       ppvNode, �������. ģ��ʹ��ʱ��ֱ�Ӵ�NULL
       pcVal, ���ᱻ���óɵ�ֵ, �����Ϊ��������Ҫ�������ֵ�������Ƿ�ƥ��.
����ֵ: 0:�ɹ�
        ����:ʧ��, ����Ȩ�޷��ʵ����
��ע:
***********************************************************/
CFG_RET CFG_GetNodeMIDList(const char *pcPath, const char *pcAccessor,
                    unsigned short *pusMID, unsigned short * pusCareMIDList,
                    unsigned long *pulCareMIDCount,
                    void **ppvNode, const char *pcVal)
{
    /* ���ڽڵ����в��Ҳ���֤����Ȩ�� */
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

    /* ���ֵһ��, �򲻼��Ȩ�� */
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
              ��������cfg�Լ��Ľڵ�ֵ��飬����Ϊ�������������صĴ�����������̫���壬
              ���Ըĳ��ں���Ĺ���������CommonSetMsgHandler���Ҳ���ģ�鶨��ļ�麯��֮��
              �ŵ���cfg��ͨ�ü�麯��
            */
            #if 0
            pstAttr = scew_attribute_by_name(pstNode, ATTR_TYPE);
            if (NULL != pstAttr)
            {
                pcAttrVal = scew_attribute_value(pstAttr);

                /* ������óɵ�ֵ���������Ƿ�ϸ� */
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
�ӿ�:   ��ȡ�ڵ�������MID
����:   pcPath, �ڵ�·��
       pcAccessor, �����ߡ�ģ��ʹ��ʱ��ֱ�Ӵ�NULL
       pusMid, ���MID
       pucAccessable, ����������Ƿ����Ȩ��, ģ��ʹ��ʱ��ֱ�Ӵ�NULL
       ppvNode, �������. ģ��ʹ��ʱ��ֱ�Ӵ�NULL
����ֵ: 0:�ɹ�
        ����:ʧ��
��ע:   ģ��ʹ��ʱһ�㰴�����·�ʽ����
       CFG_GetNodeMID(pcPath, NULL, &usMID, NULL, NULL)
***********************************************************/
CFG_RET CFG_GetNodeMID(const char *pcPath, unsigned short *pusMID)
{
    return CFG_GetNodeMIDList(pcPath, NULL, pusMID, NULL, NULL, NULL, NULL);
}











/***********************************************************
�ӿ�:   ��ӽڵ�
����:   pcPath: �ڵ�·��
        pcType: �ڵ�����, ����Ϊ��
        pcVal: �ڵ�ֵ, ����Ϊ��
        ucIsStandard, �Ƿ��׼�ڵ�
        ucNeedSave, �Ƿ���Ҫ����
        ucWritable, ��TR069�ڵ��Ƿ��д,
����ֵ: 0:�ɹ�
        ����:ʧ��
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

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstBig);

        /* �𼶽���·�� */
        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            /* ���� �� pszTmp ��ת�� */
            CFG_NUM_TO_NODE(pszTmp);

            /* ȡ��С����ƥ���֧ */
            if (NULL != pstMatch)
            {
                if (CFG_IS_NODE_NUM(pszTmp))  /* ��ǰ�ڵ������� */
                {
                    /* ֱ�Ӳ���ͨ���֧ */
                    pstMatch = scew_element_by_name(pstMatch, NODE_WILDCARD);
                }
                else
                {
                    /* ��ȷ���� */
                    pstMatch = scew_element_by_name(pstMatch, pszTmp);
                }
            }
            /* �ڴ�����Ѱ�ҽڵ� */
            pstChild = scew_element_by_name(pstBig, pszTmp);
            if (NULL == pstChild)  /* ������ */
            {
                /* �½� */
                pstChild = scew_element_add(pstBig, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);

                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }

                ucNewNode = 1;

                if (CFG_IS_NODE_NUM(pszTmp))  /* �ڵ��������� */
                {
                    /* �������index */
                    ret = CFG_UpdateMaxIdx(pstBig);
                    if (CFG_OK != ret)
                    {
                        CFG_FREE_PATH(pcPathTmp);

                        CFG_ERR(ret);
                        return ret;
                    }
                }

                CFG_ADDATTR(pstBig, ATTR_TYPE, TYPE_OBJECT, (CFG_FREE_PATH(pcPathTmp)));

                if (NULL != pstMatch)  /* С����ƥ���֧��Ϊ�� */
                {
                    pstAttr = scew_attribute_by_name(pstMatch, ATTR_MID);
                    if (NULL != pstAttr)     /* �÷�֧����ע��MID */
                    {
                        pcMatchMID = scew_attribute_value(pstAttr);

                        /* ���Ӵ������ڵ��MID */
                        CFG_ADDATTR(pstChild, ATTR_MID, pcMatchMID, CFG_FREE_PATH(pcPathTmp));
                    }
                }
            }
            pstBig = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

#if 0
    if (!ucNewNode)  /* �ڵ㲻�������� */
    {
        /* ����ڵ��Ѿ����� */
        CFG_ERR(CFG_NODE_EXISTED);
        return CFG_NODE_EXISTED;
    }
#endif

    /* ���¸������� */
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
        /* ����Ϊ�Ǳ�׼ */
        CFG_SETATTR(pstBig, ATTR_STANDARD, "0");
    }
    if (!ucNeedSave)
    {
        /* ����Ϊ���뱣�� */
        CFG_SETATTR(pstBig, ATTR_NEEDSAVE, "0");
    }
    if (!ucWritable)
    {
        char acWritable[] = "0";

        acWritable[0] += ucWritable;

        /* ����Ϊֻ�� */
        CFG_SETATTR(pstBig, ATTR_WRITABLE, acWritable);
    }

    /* ����CareMID */
    if (NULL != pstMatch && ucNewNode)
    {
        /* ȡCareMID */
        pstAttr = scew_attribute_by_name(pstMatch, ATTR_CAREMID);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);

            /* ����CareMID */
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
�ӿ�:   ��ӽڵ�
����:   pcPath: �ڵ�·��
        pcType: �ڵ�����, ����Ϊ��
        pcVal: �ڵ�ֵ, ����Ϊ��
        ucIsStandard, �Ƿ��׼�ڵ�
        ucNeedSave, �Ƿ���Ҫ����
        ucWritable, ��TR069�ڵ��Ƿ��д,
����ֵ: 0:�ɹ�
        ����:ʧ��
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

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstBig);

        /* �𼶽���·�� */
        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            /* ���� �� pszTmp ��ת�� */
            CFG_NUM_TO_NODE(pszTmp);

            /* ȡ��С����ƥ���֧ */
            if (NULL != pstMatch)
            {
                if (CFG_IS_NODE_NUM(pszTmp))  /* ��ǰ�ڵ������� */
                {
                    /* ֱ�Ӳ���ͨ���֧ */
                    pstMatch = scew_element_by_name(pstMatch, NODE_WILDCARD);
                }
                else
                {
                    /* ��ȷ���� */
                    pstMatch = scew_element_by_name(pstMatch, pszTmp);
                }
            }
            /* �ڴ�����Ѱ�ҽڵ� */
            pstChild = scew_element_by_name(pstBig, pszTmp);
            if (NULL == pstChild)  /* ������ */
            {
                /* �½� */
                pstChild = scew_element_add(pstBig, pszTmp);
                if (NULL == pstChild)
                {
                    CFG_FREE_PATH(pcPathTmp);

                    CFG_ERR(ERR_CFG_INTERNAL_ERR);
                    return ERR_CFG_INTERNAL_ERR;
                }

                ucNewNode = 1;

                if (CFG_IS_NODE_NUM(pszTmp))  /* �ڵ��������� */
                {
                    /* �������index */
                    ret = CFG_UpdateMaxIdx(pstBig);
                    if (CFG_OK != ret)
                    {
                        CFG_FREE_PATH(pcPathTmp);

                        CFG_ERR(ret);
                        return ret;
                    }
                }

                CFG_ADDATTR(pstBig, ATTR_TYPE, TYPE_OBJECT, (CFG_FREE_PATH(pcPathTmp)));

                if (NULL != pstMatch)  /* С����ƥ���֧��Ϊ�� */
                {
                    pstAttr = scew_attribute_by_name(pstMatch, ATTR_MID);
                    if (NULL != pstAttr)     /* �÷�֧����ע��MID */
                    {
                        pcMatchMID = scew_attribute_value(pstAttr);

                        /* ���Ӵ������ڵ��MID */
                        CFG_ADDATTR(pstChild, ATTR_MID, pcMatchMID, CFG_FREE_PATH(pcPathTmp));
                    }
                }
            }
            pstBig = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

#if 0
    if (!ucNewNode)  /* �ڵ㲻�������� */
    {
        /* ����ڵ��Ѿ����� */
        CFG_ERR(CFG_NODE_EXISTED);
        return CFG_NODE_EXISTED;
    }
#endif

    /* ���¸������� */
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
        /* ����Ϊ�Ǳ�׼ */
        CFG_SETATTR(pstBig, ATTR_STANDARD, "0");
    }
    if (!ucNeedSave)
    {
        /* ����Ϊ���뱣�� */
        CFG_SETATTR(pstBig, ATTR_NEEDSAVE, "0");
    }
    if (!ucWritable)
    {
        char acWritable[] = "0";

        acWritable[0] += ucWritable;

        /* ����Ϊֻ�� */
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

    /* ����CareMID */
    if (NULL != pstMatch && ucNewNode)
    {
        /* ȡCareMID */
        pstAttr = scew_attribute_by_name(pstMatch, ATTR_CAREMID);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);

            /* ����CareMID */
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
�ӿ�:   ɾ���ڵ�
����:   pcPath: �ڵ�·��
����ֵ: 0:�ɹ�
        ����:ʧ��
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

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    pstFather = pstNode->parent;
    scew_element_del(pstNode);
    (void)CFG_UpdateObjCurInstCount(pstFather);

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ȡmid��ָ��
����:   ��
����ֵ: mid��ָ��
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





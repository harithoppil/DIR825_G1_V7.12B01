/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_accessor.c
 �ļ����� : cfg ģ���и��ֱ�����������ʵ��,
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2007-11-23
            ���� :
**********************************************************************/


#include "cfg_prv.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif



#define CFG_TREE_GET_OPT(pstNode, pfnGetOpt, pcAbsPath, ucLen, pvData) \
{ \
    ret = pfnGetOpt(pstNode, pcAbsPath, ucLen, pvData); \
    if (CFG_OK != ret) \
    { \
        return ret; \
    } \
}


/*************************************************************************
Function:      long CFG_TreeAccess(const scew_element *pstNode)
Description:   ��ͨ�ı��������ĺ���(����ȡ����ȡֵ���ô˺���)
Calls:
Data Accessed: g_pfnCfgTreeOpt
Data Updated:  g_acCfgAbsPath
Input:         pstNode, Ҫ����������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeGetAccess(const scew_element *pstNode, FUNC_GET_OPT pfnGetOpt,
                                          char *pcAbsPath, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    const char *pcNodeName = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_TREE_STACK_NODE astStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    /* ��ʼ��ջ */
    astStack[0].ucPathLen = strlen(pcAbsPath);
    astStack[0].pstPath = pstNodeWork;
    ucStackTop = 1;

    /* ���������Ƿ��׼ */
    CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, ((void)0));
    if (NULL == pstNodeWork)
    {
        CFG_ERR(ERR_CFG_NOT_STANDARD, "%s", pcAbsPath);
        return ERR_CFG_NOT_STANDARD;
    }

    /* ���� */
    CFG_TREE_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
            astStack[ucStackTop - 1].ucPathLen, pvData);

    while (1)
    {
        if (NULL != pstNodeWork)
        {
            while (pstChild = scew_element_next(pstNodeWork, pstChild),
                   NULL != pstChild)   /* ȡ�ӽڵ�, NULL !=�ӽڵ� */
            {
                /* ��Ϊ�ӽڵ� */
                pstNodeWork = pstChild;
                pstChild = NULL;

                /* ����·�� */
                pcNodeName = scew_element_name(pstNodeWork);
                CFG_NODE_TO_NUM(pcNodeName);
                pcAbsPath[astStack[ucStackTop - 1].ucPathLen] = DELIMIT_C;
                strcpy(pcAbsPath + astStack[ucStackTop - 1].ucPathLen + 1, pcNodeName);

                /* ��ջ */
                astStack[ucStackTop].pstPath = pstNodeWork;
                astStack[ucStackTop].ucPathLen
                    = astStack[ucStackTop - 1].ucPathLen  + strlen(pcNodeName) + 1;
                ucStackTop++;

                /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
                CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, break);

                /* ���� */
                CFG_TREE_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                       astStack[ucStackTop - 1].ucPathLen, pvData);
            }
        }

        /* ջ�� */
        if (1 == ucStackTop)
        {
            break;
        }

        /* ��ջ, �ָ�·�� */
        ucStackTop--;
        pstNodeWork = astStack[ucStackTop].pstPath;
        pcAbsPath[astStack[ucStackTop].ucPathLen] = '\0';

        /* ȡ�ֵ� */
        pstParent = astStack[ucStackTop - 1].pstPath;
        pstNodeWork = scew_element_next(pstParent, pstNodeWork);
        if (NULL != pstNodeWork)/* �ֵܲ�Ϊ�� */
        {
            /* ����·�� */
            pcNodeName = scew_element_name(pstNodeWork);
            CFG_NODE_TO_NUM(pcNodeName);
            pcAbsPath[astStack[ucStackTop - 1].ucPathLen] = DELIMIT_C;
            strcpy(pcAbsPath + astStack[ucStackTop - 1].ucPathLen + 1, pcNodeName);

            /* ��ջ */
            astStack[ucStackTop].pstPath = pstNodeWork;
            astStack[ucStackTop].ucPathLen
                = astStack[ucStackTop - 1].ucPathLen  + strlen(pcNodeName) + 1;
            ucStackTop++;

            /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
            CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, continue);

            /* ���� */
            CFG_TREE_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                     astStack[ucStackTop - 1].ucPathLen, pvData);
        }
    }

    return CFG_OK;
}


/*************************************************************************
Function:      long CFG_NextLevelAccess(const scew_element *pstNode)
Description:   �����²�ڵ�ĺ���, ����ֻȡ�²�ڵ������б�ʱ
Calls:
Data Accessed: g_pfnCfgTreeOpt
Data Updated:  g_acCfgAbsPath
Input:         pstNode, Ҫ����������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeGetNextLevelAccess(const scew_element *pstNode,
            FUNC_GET_OPT pfnGetOpt, char *pcAbsPath, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    scew_element *pstChild = NULL;
    scew_element *pstChildPrev = NULL;
    long ret = CFG_OK;
    unsigned char ucLen = strlen(pcAbsPath);
    const char *pcNodeName = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    while (pstChild = scew_element_next(pstNodeWork, pstChildPrev),
           NULL != pstChild)
    {
        pstChildPrev = pstChild;

        /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
        CFG_PASS_NOSTANDARD(pstChild, g_pcCfgAccessor, continue);

        /* �Խڵ�������ת�� */
        pcNodeName = scew_element_name(pstChild);
        CFG_NODE_TO_NUM(pcNodeName);

        /* ���¾���·�� */
        pcAbsPath[ucLen] = DELIMIT_C;
        strcpy(&pcAbsPath[ucLen + 1], pcNodeName);

        /* ���� */
        CFG_TREE_GET_OPT(pstChild, pfnGetOpt, pcAbsPath,
                ucLen + strlen(pcNodeName) + 1, pvData);
    }

    return CFG_OK;
}





#define CFG_TREE_SET_OPT(pstNode, pfnSetOpt, pvData) \
{ \
    ret = pfnSetOpt(pstNode, pvData); \
    if (CFG_OK != ret) \
    { \
        return ret; \
    } \
}



/* ���������ò���ʱ�ı�������, ֮���Բ��Ǹ�ȡֵ��ͬһ������,
   ��Ϊȡֵʱ��Ҫ��¼����·��,������ʱ���� */
/*************************************************************************
Function:      long CFG_GetNode(const void *pstTree, char *pcPath, void **ppvNode)
Description:   ���������ò���ʱ�ı�������, ֮���Բ��Ǹ�ȡֵ��ͬһ������,
                 ��Ϊȡֵʱ��Ҫ��¼����·��,������ʱ����
Calls:         g_pfnCfgTreeSetOpt
Data Accessed:
Data Updated:
Input:         pstNode, Ҫ����������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeSetAccess(const scew_element *pstNode,
                          FUNC_SET_OPT pfnSetOpt, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    long ret = CFG_OK;
    const scew_element *apstStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    apstStack[0] = pstNode;
    ucStackTop++;

    /* ���������Ƿ��׼ */
    CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, ((void)0));
    if (NULL == pstNodeWork)
    {
        CFG_ERR(ERR_CFG_NOT_STANDARD, "%s", scew_element_name(pstNode));
        return ERR_CFG_NOT_STANDARD;
    }

    /* ���� */
    CFG_TREE_SET_OPT(pstNodeWork, pfnSetOpt, pvData);

    while (1)
    {
        if (NULL != pstNodeWork)
        {
            while (pstChild = scew_element_next(pstNodeWork, pstChild),
                   NULL != pstChild)   /* ȡ�ӽڵ�, NULL !=�ӽڵ� */
            {
                /* ��Ϊ�ӽڵ� */
                pstNodeWork = pstChild;
                pstChild = NULL;

                /* ��ջ */
                apstStack[ucStackTop] = pstNodeWork;
                ucStackTop++;

                /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
                CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, break);

                /* ���� */
                CFG_TREE_SET_OPT(pstNodeWork, pfnSetOpt, pvData);
            }
        }

        /* ջ�� */
        if (1 == ucStackTop)
        {
            break;
        }

        /* ��ջ, �ָ�·�� */
        ucStackTop--;
        pstNodeWork = apstStack[ucStackTop];

        /* ȡ�ֵ� */
        pstParent = apstStack[ucStackTop - 1];
        pstNodeWork = scew_element_next(pstParent, pstNodeWork);
        if (NULL != pstNodeWork)/* �ֵܲ�Ϊ�� */
        {
            /* ��ջ */
            apstStack[ucStackTop] = pstNodeWork;
            ucStackTop++;

            /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
            CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, continue);

            /* ���� */
            CFG_TREE_SET_OPT(pstNodeWork, pfnSetOpt, pvData);
        }
    }

    return CFG_OK;
}







/*************************************************************************
Function:      void CFG_FillDumpBuf(const char *pcBuf)
Description:   ���ַ�������� pstDumpBuf �еĻ�����
Calls:
Data Accessed: pstDumpBuf
Data Updated:
Input:         pcBuf, Ҫ������ַ���
Output:        ��
Return:        ��
Others:
*************************************************************************/
void CFG_FillDumpBuf(const char *pcBuf, unsigned long ulLen, ST_CFG_DUMP_BUF *pstDumpBuf)
{
    if ((pstDumpBuf->ulInfactLen == pstDumpBuf->ulPos)
        && (pstDumpBuf->ulLen - pstDumpBuf->ulPos > ulLen))
    {
        CFG_MemCpy(pstDumpBuf->pcBuf + pstDumpBuf->ulPos, pcBuf, ulLen);
        pstDumpBuf->ulPos += ulLen;
    }

    pstDumpBuf->ulInfactLen += ulLen;
}



/*************************************************************************
Function:      int CFG_EscapeXML(char *buf, int len, const char *text, const char **ppcNext)
Description:   �ַ�ת��
Calls:
Data Accessed: ��
Data Updated:  ��
Input:         buf, ���ת�����ַ���
               len, text �ĳ���
               text, ԭʼ�ַ���
               ppcNext, ת�����һ���ַ���
Output:        ��
Return:        0
Others:
*************************************************************************/
int CFG_EscapeXML(char *buf, int len, const char *text, const char **ppcNext)
{
    int c = 0;

    while(*text && len > 0)
    {
        switch(*text)
        {
            case '<':
                *buf++ = '&';
                *buf++ = 'l';
                *buf++ = 't';
                *buf++ = ';';
                len -= 4;
                c += 4;
                break;
            case '>':
                *buf++ = '&';
                *buf++ = 'g';
                *buf++ = 't';
                *buf++ = ';';
                len -= 4;
                c += 4;
                break;
            case '&':
                *buf++ = '&';
                *buf++ = 'a';
                *buf++ = 'm';
                *buf++ = 'p';
                *buf++ = ';';
                len -= 5;
                c += 5;
                break;
            case '"':
                *buf++ = '&';
                *buf++ = 'q';
                *buf++ = 'u';
                *buf++ = 'o';
                *buf++ = 't';
                *buf++ = ';';
                len -= 6;
                c += 6;
                break;
            default:
                *buf++ = *text;
                len--;
                c++;
        }
        text++;
    }

    if (NULL != ppcNext)
    {
        if (*text != '\0')
        {
            *ppcNext = text;
        }
        else
        {
            *ppcNext = NULL;
        }
    }

    return c;
}


/*************************************************************************
Function:      void CFG_TreeDumpAccess(const scew_element *pElement)
Description:   ��xml��ʽ dump����
Calls:
Data Accessed: pstDumpBuf
Data Updated:
Input:         pElement, Ҫdump������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeDumpAccess(const scew_element *pElement,
                 ST_CFG_DUMP_BUF *pstDumpBuf, unsigned char ucTable)
{
    scew_element *pChild = NULL;
    XML_Char const* pContents = NULL;
    scew_attribute* pstAttr = NULL;
    const char *pcAttrVal = NULL;
    char acLocalBuffer[1024];
    const char *pcNodeName = NULL;
    unsigned char i = 0;
    unsigned long ulLen = 0;
    char *pcBuf = NULL;

    if (NULL == pElement)
    {
        return ERR_CFG_PARA_INVALID;
    }

    /* ���ݷ����߽������� */
    CFG_GET_RIGHT_CHECK(pElement, g_pcCfgAccessor);

    /* �������뱣��Ľڵ� */
    CFG_PASS_NONEEDSAVE(pElement);

    /**
     * Prints the starting element tag with its attributes.
     */
    pcNodeName = scew_element_name(pElement);
    CFG_NODE_TO_NUM(pcNodeName);

    acLocalBuffer[0] = '\n';
    for (i = 0; i < ucTable; i++)
    {
        acLocalBuffer[i+1] = '\t';
    }
    acLocalBuffer[i+1] = '\0';
    CFG_FillDumpBuf(acLocalBuffer, i+1, pstDumpBuf);

    ulLen = (unsigned long)sprintf(acLocalBuffer, "<%s", pcNodeName);
    CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);

    pstAttr = NULL;
    while (pstAttr = scew_attribute_next(pElement, pstAttr),
           NULL != pstAttr)
    {
        if (0 == strcmp(ATTR_MID, scew_attribute_name(pstAttr))
            || 0 == strcmp(ATTR_CAREMID, scew_attribute_name(pstAttr)))
        {
            continue;
        }

        pcBuf = acLocalBuffer;
        ulLen = (unsigned long)sprintf(acLocalBuffer," %s=\"",
                                   scew_attribute_name(pstAttr));
        pcBuf += ulLen;
        ulLen = (unsigned long)CFG_EscapeXML(pcBuf,
                                 sizeof(acLocalBuffer) - ulLen,
                                 scew_attribute_value(pstAttr), NULL);
        pcBuf += ulLen;
        strcpy(pcBuf, "\"");

        CFG_FillDumpBuf(acLocalBuffer,
            (unsigned long)((pcBuf - acLocalBuffer) + 1),pstDumpBuf);
    }
    CFG_FillDumpBuf(">", 1, pstDumpBuf);

    /**
     * Call print_element function again for each child of the
     * current element.
     */
    pChild = NULL;
    while (pChild = scew_element_next(pElement, pChild),
           NULL != pChild)
    {
        (void)CFG_TreeDumpAccess(pChild, pstDumpBuf, ucTable + 1);
    }

    if (0 < scew_element_count(pElement))
    {
        acLocalBuffer[0] = '\n';
        for (i = 0; i < ucTable; i++)
        {
            acLocalBuffer[i+1] = '\t';
        }
        acLocalBuffer[i+1] = '\0';
        CFG_FillDumpBuf(acLocalBuffer, i + 1, pstDumpBuf);
    }

    pContents = scew_element_contents(pElement);

    /* Prints element's content. */
    if (pContents != NULL)
    {
        const char *pcNext = pContents;
        while (NULL != pcNext)
        {
            ulLen = (unsigned long)CFG_EscapeXML(acLocalBuffer,
                           sizeof(acLocalBuffer), pcNext, &pcNext);
            CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);
        }
    }

    /**
     * Prints the closing element tag.
     */
    ulLen = (unsigned long)sprintf(acLocalBuffer,"</%s>", pcNodeName);
    CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);

    return CFG_OK;
}


#ifdef _CFG_DEBUG
/*************************************************************************
Function:      void CFG_TreeDumpAccess(const scew_element *pElement)
Description:   ��xml��ʽ dump����
Calls:
Data Accessed: pstDumpBuf
Data Updated:
Input:         pElement, Ҫdump������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreePrintAccess(const scew_element *pstNode,
                     ST_CFG_DUMP_BUF *pstDumpBuf, unsigned char ucTable)
{
    scew_element *pChild = NULL;
    XML_Char const* pContents = NULL;
    scew_attribute* pstAttr = NULL;
    const char *pcAttrVal = NULL;
    char acLocalBuffer[1024];
    const char *pcNodeName = NULL;
    unsigned char i = 0;
    unsigned long ulLen = 0;

    if (NULL == pstNode)
    {
        return ERR_CFG_PARA_INVALID;
    }

    /* ���ݷ����߽������� */
    CFG_GET_RIGHT_CHECK(pstNode, g_pcCfgAccessor);

    /* �������뱣��Ľڵ� */

    /**
     * Prints the starting element tag with its attributes.
     */
    pcNodeName = scew_element_name(pstNode);
    CFG_NODE_TO_NUM(pcNodeName);

    acLocalBuffer[0] = '\n';
    for (i = 0; i < ucTable; i++)
    {
        acLocalBuffer[i+1] = '\t';
    }
    acLocalBuffer[i+1] = '\0';
    CFG_FillDumpBuf(acLocalBuffer, i+1, pstDumpBuf);


    ulLen = (unsigned long)sprintf(acLocalBuffer, "<%s", pcNodeName);
    CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);

    pstAttr = NULL;
    while (pstAttr = scew_attribute_next(pstNode, pstAttr),
           NULL != pstAttr)
    {
/*      if (0 == strcmp(ATTR_MID, scew_attribute_name(pstAttr))
            || 0 == strcmp(ATTR_CAREMID, scew_attribute_name(pstAttr)))
        {
            continue;
        } */
        ulLen = (unsigned long)sprintf(acLocalBuffer," %s=\"%s\"",
                  scew_attribute_name(pstAttr), scew_attribute_value(pstAttr));
        CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);
    }
    CFG_FillDumpBuf(">", 1, pstDumpBuf);

    /**
     * Call print_element function again for each child of the
     * current element.
     */
    pChild = NULL;
    while (pChild = scew_element_next(pstNode, pChild),
           NULL != pChild)
    {
        (void)CFG_TreePrintAccess(pChild, pstDumpBuf, ucTable + 1);
    }

    if (0 < scew_element_count(pstNode))
    {
        acLocalBuffer[0] = '\n';
        for (i = 0; i < ucTable; i++)
        {
            acLocalBuffer[i+1] = '\t';
        }
        acLocalBuffer[i+1] = '\0';
        CFG_FillDumpBuf(acLocalBuffer, i + 1, pstDumpBuf);
    }

    pContents = scew_element_contents(pstNode);

    /* Prints element's content. */
    if (pContents != NULL)
    {
        CFG_FillDumpBuf(pContents, strlen(pContents), pstDumpBuf);
    }

    /**
     * Prints the closing element tag.
     */
    ulLen = (unsigned long)sprintf(acLocalBuffer,"</%s>", pcNodeName);
    CFG_FillDumpBuf(acLocalBuffer, ulLen, pstDumpBuf);

    return CFG_OK;
}
#endif




/* �ǵݹ��������ջ */
ST_CFG_TREE_STACK_NODE s_astCfgTreeStack[CFG_ACCESS_STACK_DEEPTH];

unsigned char s_ucCfgTreeStackPos = 0;    /* �ǵݹ��������ջ�� */

/* �ǵݹ������ʱ��ž���·�� */
char  s_acCfgSavePath[CFG_MAX_PATH_LEN];

/* ��ʾ��ʼ����ʱ�Ƿ��������ʼ, ��CFG_SetAccessBasicInfo ������ */
unsigned char s_ucCfgCurIsRoot = 0;


/*************************************************************************
Function:      CFG_RET CFG_SetMIDAccessInfo(const char *pcPath, const void **ppvTree,
                           unsigned short *pusMID, const char *pcAccessor,
                           char **ppcPathSave)
Description:   ���÷ǵݹ����MID�Ļ�����Ϣ
Calls:
Data Accessed:
Data Updated:  s_astCfgTreeStack
               s_ucCfgTreeStackPos
               s_ucCfgCurIsRoot
               s_acCfgSavePath
Input:         pcPath, �ڵ�·��
               pcAccessor, ������
Output:        ppvTree, ����ָ��, ���pcPath��ʾΪȡ����
               pusMID, �ڵ�������mid
               ppcPathSave, ���ȫ·���Ļ�������ַ
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_SetMIDAccessInfo(const char *pcPath, const void **ppvTree,
           unsigned short *pusMID, const char *pcAccessor,
           char **ppcPathSave)
{
    CFG_RET ret = CFG_OK;
    unsigned long ulLen = 0;
    unsigned short usMID = 0;
    void *pvTree = NULL;

    ulLen = strlen(pcPath);
    if (ulLen > 0 && TREE_FLAG_C == pcPath[ulLen - 1])
    {
        ret = CFG_GetNodeMIDList(pcPath, pcAccessor,
                            &usMID, NULL, NULL, &pvTree, NULL);
        if (CFG_OK == ret)
        {
            if (TREE_FLAG_C == pcPath[0])   /* ��pcPath = "." ʱ, ��ʾȡ�� */
            {
                s_astCfgTreeStack[0].ucPathLen = 0;
            }
            else
            {
                strcpy(s_acCfgSavePath, pcPath);
                s_astCfgTreeStack[0].ucPathLen = (ulLen - 1)
                                - strlen(scew_element_name(pvTree));
            }
            s_astCfgTreeStack[0].stNodeInfo.usMID = usMID;  /* ע��, �����ֵ������,
                                          ����, ���ýڵ㼰����û��MIDʱ,
                                          ����������������Ҷ�Ӷ�û��MID����� */
            s_astCfgTreeStack[0].pstPath = NULL;
            s_ucCfgTreeStackPos = 1;
            s_ucCfgCurIsRoot = 1;   /* ��ʾ���濪ʼ�����Ľڵ��Ǹ����� */

            *ppvTree = pvTree;
            *pusMID = usMID;
        }
    }
    else
    {
        ret = CFG_GetNodeMIDList(pcPath, pcAccessor,
                       &usMID, NULL, NULL, &pvTree, NULL);
        if (CFG_OK == ret)
        {
            if ('\0' == pcPath[0])
            {
                strcpy(s_acCfgSavePath, scew_element_name(pvTree));
            }

            *ppvTree = NULL;
            *pusMID = usMID;
        }
    }

    *ppcPathSave = s_acCfgSavePath;

    return ret;
}



/*************************************************************************
Function:      void CFG_GetPathLenAndMID(scew_element *pstPath, unsigned
short *pusMID,
                                                  unsigned char *pucLen)
Description:   �ڷǵݹ���������ڸ���ջȡ�õ�ǰ�ڵ��·����MID
Calls:
Data Accessed:
Data Updated:  s_astCfgTreeStack
               s_ucCfgTreeStackPos
               s_acCfgSavePath
Input:         pstPath, �ڵ�ָ��
Output:        pusMID, �ڵ�������mid
               pucLen, ·������
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
void CFG_GetPathLenAndMID(const scew_element *pstPath, unsigned short *pusMID,
                                                  unsigned char *pucLen)
{
    const scew_element *pstPathWork = pstPath;
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;
    unsigned short usMID = 0;
    unsigned char  ucFormerLen = 0;
    unsigned char  ucPathLen = 0;
    const char *pName = NULL;

    usMID = s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].stNodeInfo.usMID;
    ucFormerLen = s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].ucPathLen;

    pstAttr = scew_attribute_by_name(pstPathWork, ATTR_MID);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        usMID = strtol(pcAttrVal, NULL, CFG_MID_BASE);
    }

    /* �Խڵ���ת�� */
    pName = scew_element_name(pstPathWork);
    CFG_NODE_TO_NUM(pName);

    strcpy(s_acCfgSavePath + ucFormerLen, pName);
    ucPathLen = ucFormerLen + strlen(pName);
    pstAttr = scew_attribute_by_name(pstPathWork, ATTR_TYPE);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        if (NULL != pcAttrVal
            && (0 == strcmp(pcAttrVal, TYPE_CLASS)
                || 0 == strcmp(pcAttrVal, TYPE_OBJECT)))
        {
            s_acCfgSavePath[ucPathLen] = DELIMIT_C;
            ucPathLen++;
            s_acCfgSavePath[ucPathLen] = '\0';
        }
    }

    *pusMID = usMID;
    *pucLen = ucPathLen;
}



/*************************************************************************
Function:     void *CFG_GetNextNode(void *pvPath, unsigned short *pusMID,
                                                   unsigned char *pucLen)
Description:   �ǵݹ����MID, ȡ��һ��MID
Calls:
Data Accessed: s_astCfgTreeStack
               s_ucCfgTreeStackPos
               s_ucCfgCurIsRoot
               s_acCfgSavePath
Data Updated:  s_astCfgTreeStack
               s_ucCfgTreeStackPos
               s_ucCfgCurIsRoot
               s_acCfgSavePath
Input:         pvPath, ��ǰ�ڵ�ָ��
Output:        pusMID, �ڵ�������mid
               pucLen, ·������
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
const void *CFG_GetNextMID(const void *pvPrev, unsigned short *pusMID,
                                    unsigned char *pucLen)
{
    const scew_element *pstParent = NULL;
    const scew_element *pstChild = NULL;
    const scew_element *pstPathWork = pvPrev;
    unsigned short usMID = 0;
    unsigned char  ucPathLen = 0;

    if (NULL == pstPathWork)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return NULL;
    }

    while (1)
    {
        /* ȡ�ֵ� */
        if (0 == s_ucCfgCurIsRoot)
        {
            if (1 == s_ucCfgTreeStackPos)  /* ջ�� */
            {
                return NULL;
            }
            pstParent = s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].pstPath;
            pstPathWork = scew_element_next(pstParent, pstPathWork);
        }
        else  /* ��ǰ��һ����Ѱ�������ĵ�һ��Ҷ�ڵ�, ���, ������ȡ�ֵܵĲ��� */
        {
            s_ucCfgCurIsRoot = 0;
        }
        if (NULL != pstPathWork)   /* NULL != �ֵ� */
        {
            while (pstChild = scew_element_next(pstPathWork, NULL),
                   NULL != pstChild)  /* ȡ�ӽڵ�, NULL != �ӽڵ� */
            {
                /* ȡMID */
                CFG_GetPathLenAndMID(pstPathWork, &usMID, &ucPathLen);

                /* ��ջ */
                s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath = pstPathWork;
                s_astCfgTreeStack[s_ucCfgTreeStackPos].stNodeInfo.usMID = usMID;
                s_astCfgTreeStack[s_ucCfgTreeStackPos].ucPathLen = ucPathLen;
                s_ucCfgTreeStackPos++;

                pstPathWork = pstChild;
            }

            /* ȡMID */
            CFG_GetPathLenAndMID(pstPathWork, &usMID, &ucPathLen);

            /* MID ��ջ���ڵ��MID��һ�� */
            if (usMID != s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].stNodeInfo.usMID)
            {
                /* ��� */
                *pusMID = usMID;
                *pucLen = ucPathLen;

                return pstPathWork;
            }
        }
        else
        {
            /* ��ջ */
            --s_ucCfgTreeStackPos;
            pstPathWork = s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath;
            usMID = s_astCfgTreeStack[s_ucCfgTreeStackPos].stNodeInfo.usMID;
            ucPathLen = s_astCfgTreeStack[s_ucCfgTreeStackPos].ucPathLen;
            s_acCfgSavePath[ucPathLen] = '\0';

            /* MID ��ջ���ڵ��MID��һ��, ����ջ�� */
            if (usMID != s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].stNodeInfo.usMID
                || 1 == s_ucCfgTreeStackPos)
            {
                /* ��� */
                *pusMID = usMID;
                *pucLen = ucPathLen;

                return pstPathWork;
            }
        }
    }

    /* return NULL; */  /* �޷����е� */
}









#define CFG_CHECK_AND_DEL_NODE(pstNode, expt) \
{ \
    pstAttr = scew_attribute_by_name(pstNode, ATTR_STARTDEL); \
    if (NULL != pstAttr) \
    { \
        pcAttrVal = scew_attribute_value(pstAttr); \
        if ('1' == pcAttrVal[0]) \
        { \
            void *pvLeft = pstNode->left; \
            scew_element_del(pstNode); \
            pstNode = pvLeft; \
            if (NULL == pstNode) \
            { \
                expt; \
            } \
            continue; \
        } \
    } \
}





/*************************************************************************
Function:      void CFG_RmvDeletingNode(scew_tree *pstTree)
Description:   ɾ������ʱ��Ҫɾ���Ľڵ�
Calls:
Data Accessed: ��
Data Updated:  ��
Input:         pstTree, Ҫ��������ָ��
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
void CFG_RmvDeletingNode(scew_tree *pstTree)
{
    scew_element *pstNodeWork = NULL;
    scew_element *pstChild = NULL;
    scew_element *pstNodeNext = NULL;
    scew_element *pstParent = NULL;
    scew_element *apstStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;

    scew_tree *pstTreeWork = pstTree;
    if (NULL == pstTreeWork)
    {
        pstTreeWork = g_pstCfgTree;
    }

    pstNodeWork = scew_tree_root(pstTreeWork);
    apstStack[1] = pstNodeWork;
    ucStackTop++;

    while (1)
    {
        if (NULL != pstNodeWork)
        {
            while (pstChild = scew_element_next(pstNodeWork, pstChild),
                   NULL != pstChild)   /* ȡ�ӽڵ�, NULL !=�ӽڵ� */
            {
                CFG_CHECK_AND_DEL_NODE(pstChild, break);

                /* ��Ϊ�ӽڵ� */
                pstNodeWork = pstChild;
                pstChild = NULL;

                /* ��ջ */
                apstStack[ucStackTop] = pstNodeWork;
                ucStackTop++;

            }
        }

        /* ջ�� */
        if (1 == ucStackTop)
        {
            break;
        }

        /* ��ջ, �ָ�·�� */
        ucStackTop--;
        pstNodeWork = apstStack[ucStackTop];

        /* ȡ�ֵ� */
        pstParent = apstStack[ucStackTop - 1];

        while (pstNodeNext = scew_element_next(pstParent, pstNodeWork),
               NULL != pstNodeNext)
        {
            CFG_CHECK_AND_DEL_NODE(pstNodeNext, (void)0);

            pstNodeWork = pstNodeNext;

            /* ��ջ */
            apstStack[ucStackTop] = pstNodeWork;
            ucStackTop++;

            break;
        }
    }

    return;
}



#define CFG_TREE_ATTR_GET_OPT(pstNode, pfnGetOpt, pcAbsPath, ucLen, pstNodeInfo, pvData) \
{ \
    ret = pfnGetOpt(pstNode, pcAbsPath, ucLen, pstNodeInfo, pvData); \
    if (CFG_OK != ret) \
    { \
        return ret; \
    } \
}

/*************************************************************************
Function:      long CFG_TreeAccess(const scew_element *pstNode)
Description:   ��ͨ�ı��������ĺ���(����ȡ����ȡֵ���ô˺���)
Calls:
Data Accessed: g_pfnCfgTreeOpt
Data Updated:  g_acCfgAbsPath
Input:         pstNode, Ҫ����������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_TreeAttrGetAccess(const scew_element *pstNode, FUNC_ATTR_GET_OPT pfnGetOpt,
                       char *pcAbsPath, const ST_CFG_NODE_INFO *pstNodeInfo, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    const char *pcNodeName = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_TREE_STACK_NODE astStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    /* ��ʼ��ջ */
    astStack[0].ucPathLen = strlen(pcAbsPath);
    astStack[0].pstPath = pstNodeWork;
    astStack[0].stNodeInfo = *pstNodeInfo;
    ucStackTop = 1;

    /* ���������Ƿ��׼ */
    CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, ((void)0));
    if (NULL == pstNodeWork)
    {
        CFG_ERR(ERR_CFG_NOT_STANDARD, "%s", pcAbsPath);
        return ERR_CFG_NOT_STANDARD;
    }

    /* ���� */
    CFG_TREE_ATTR_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
            astStack[ucStackTop - 1].ucPathLen,
            &astStack[ucStackTop - 1].stNodeInfo, pvData);

    while (1)
    {
        if (NULL != pstNodeWork)
        {
            while (pstChild = scew_element_next(pstNodeWork, pstChild),
                   NULL != pstChild)   /* ȡ�ӽڵ�, NULL !=�ӽڵ� */
            {
                /* ��Ϊ�ӽڵ� */
                pstNodeWork = pstChild;
                pstChild = NULL;

                /* ����·�� */
                pcNodeName = scew_element_name(pstNodeWork);
                CFG_NODE_TO_NUM(pcNodeName);
                pcAbsPath[astStack[ucStackTop - 1].ucPathLen] = DELIMIT_C;
                strcpy(pcAbsPath + astStack[ucStackTop - 1].ucPathLen + 1, pcNodeName);

                /* ��ջ */
                astStack[ucStackTop].pstPath = pstNodeWork;
                astStack[ucStackTop].ucPathLen
                    = astStack[ucStackTop - 1].ucPathLen  + strlen(pcNodeName) + 1;

                astStack[ucStackTop].stNodeInfo
                    = astStack[ucStackTop - 1].stNodeInfo;
                CFG_UpdateNodeInfo(pstNode, &astStack[ucStackTop].stNodeInfo,
                     CFG_NODE_INFO_NOTI | CFG_NODE_INFO_ACCESSLIST);

                ucStackTop++;

                /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
                CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, break);

                /* ���� */
                CFG_TREE_ATTR_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                       astStack[ucStackTop - 1].ucPathLen,
                       &astStack[ucStackTop - 1].stNodeInfo, pvData);
            }
        }

        /* ջ�� */
        if (1 == ucStackTop)
        {
            break;
        }

        /* ��ջ, �ָ�·�� */
        ucStackTop--;
        pstNodeWork = astStack[ucStackTop].pstPath;
        pcAbsPath[astStack[ucStackTop].ucPathLen] = '\0';

        /* ȡ�ֵ� */
        pstParent = astStack[ucStackTop - 1].pstPath;
        pstNodeWork = scew_element_next(pstParent, pstNodeWork);
        if (NULL != pstNodeWork)/* �ֵܲ�Ϊ�� */
        {
            /* ����·�� */
            pcNodeName = scew_element_name(pstNodeWork);
            CFG_NODE_TO_NUM(pcNodeName);
            pcAbsPath[astStack[ucStackTop - 1].ucPathLen] = DELIMIT_C;
            strcpy(pcAbsPath + astStack[ucStackTop - 1].ucPathLen + 1, pcNodeName);

            /* ��ջ */
            astStack[ucStackTop].pstPath = pstNodeWork;
            astStack[ucStackTop].ucPathLen
                = astStack[ucStackTop - 1].ucPathLen  + strlen(pcNodeName) + 1;

            astStack[ucStackTop].stNodeInfo
                = astStack[ucStackTop - 1].stNodeInfo;
            CFG_UpdateNodeInfo(pstNode, &astStack[ucStackTop].stNodeInfo,
                 CFG_NODE_INFO_NOTI | CFG_NODE_INFO_ACCESSLIST);

            ucStackTop++;

            /* �ڷ�������Tr069�������, �Ź��Ǳ�׼�ڵ� */
            CFG_PASS_NOSTANDARD(pstNodeWork, g_pcCfgAccessor, continue);

            /* ���� */
            CFG_TREE_ATTR_GET_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                     astStack[ucStackTop - 1].ucPathLen,
                     &astStack[ucStackTop - 1].stNodeInfo, pvData);
        }
    }

    return CFG_OK;
}

#ifdef _CFG_DEBUG

/* ͨ��mid��ȡ�±� */
#define MID2INDEX(mid) ((mid) & 0x00ff)

CFG_RET CFG_AddBranchToSepTree(const void *pstBranch, unsigned short usMID,
                               void **ppvTreeList)
{
    unsigned long i = 0;
    void *pvNode = NULL;
    CFG_RET ret = CFG_OK;
    void *pvTree = ppvTreeList[MID2INDEX(usMID)];
    unsigned long ulEnd = s_ucCfgTreeStackPos + 1;

    if (NULL == pvTree)
    {
        ret = CFG_DupSingleNode(s_astCfgTreeStack[1].pstPath,
                                (scew_element **)(void *)&pvNode);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }
        ppvTreeList[MID2INDEX(usMID)] = pvNode;
        pvTree = pvNode;
    }

    s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath = pstBranch;
    for (i = 2; i < ulEnd; i++)
    {
        pvNode = scew_element_by_name(pvTree,
                scew_element_name(s_astCfgTreeStack[i].pstPath));
        if (NULL == pvNode) /* ������ */
        {
            /* ���ƽڵ� */
            ret = CFG_DupSingleNode(s_astCfgTreeStack[i].pstPath,
                                    (scew_element **)(void *)&pvNode);
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
                return ret;
            }

            /* ��ӵ� */
            (void)scew_element_add_elem(pvTree, pvNode);
        }

        /* ���½ڵ� */
        pvTree = pvNode;
    }

    return CFG_OK;
}


CFG_RET CFG_TreeSepAccess(const char *pcPath, void **ppvTreeList)
{
    const scew_element *pstParent = NULL;
    const scew_element *pstChild = NULL;
    const scew_element *pstPathWork = NULL;
    unsigned short usMID = 0;
    unsigned char  ucPathLen = 0;
    CFG_RET ret = CFG_OK;

    ret = CFG_SetMIDAccessInfo(pcPath, (const void **)(void *)&pstPathWork, &usMID,
                               NULL, (char **)(void *)&pcPath);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    while (1)
    {
        /* ȡ�ֵ� */
        if (0 == s_ucCfgCurIsRoot)
        {
            if (1 == s_ucCfgTreeStackPos)  /* ջ�� */
            {
                return CFG_OK;
            }
            pstParent = s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].pstPath;
            pstPathWork = scew_element_next(pstParent, pstPathWork);
        }
        else  /* ��ǰ��һ����Ѱ�������ĵ�һ��Ҷ�ڵ�, ���, ������ȡ�ֵܵĲ��� */
        {
            s_ucCfgCurIsRoot = 0;
        }
        if (NULL != pstPathWork)   /* NULL != �ֵ� */
        {
            while (pstChild = scew_element_next(pstPathWork, NULL),
                   NULL != pstChild)  /* ȡ�ӽڵ�, NULL != �ӽڵ� */
            {
                /* ȡMID */
                CFG_GetPathLenAndMID(pstPathWork, &usMID, &ucPathLen);

                /* ��ջ */
                s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath = pstPathWork;
                s_astCfgTreeStack[s_ucCfgTreeStackPos].stNodeInfo.usMID = usMID;
                s_astCfgTreeStack[s_ucCfgTreeStackPos].ucPathLen = ucPathLen;
                s_ucCfgTreeStackPos++;

                pstPathWork = pstChild;
            }

            /* ȡMID */
            CFG_GetPathLenAndMID(pstPathWork, &usMID, &ucPathLen);

            ret = CFG_AddBranchToSepTree(pstPathWork, usMID, ppvTreeList);
            if (CFG_OK != ret)
            {
                CFG_ERR(ret);
                return ret;
            }
        }
        else
        {
            /* ��ջ */
            --s_ucCfgTreeStackPos;
            pstPathWork = s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath;
        }
    }

    /* return ERR_CFG_FAIL; */  /* �޷����е� */
}

typedef struct
{
    unsigned long usMID;
    char *pcName;

} ST_MOD_NAME;


void CFG_SepTree(void)
{
    CFG_RET ret = CFG_OK;
    void *apvTree[64] = {NULL};
    unsigned long i = 0;
    unsigned long j = 0;
    void *pvTmp = g_pstCfgTree;
    FILE *fp = NULL;
    char acFile[256];
    scew_tree stTree;

ST_MOD_NAME astModName[] =
{
    {0,                   "public"         },
    {MID_AUTH,            "auth"           },
    {MID_IGMP,            "igmp"           },
    {MID_CMM,             "cmm"            },
    {MID_LAN,             "lan"            },
    {MID_IPT,             "ipt"            },
    {MID_ETHLAN,          "ethlan"         },
    {MID_ETHWAN,          "ethwan"         },
    {MID_PPPOE,           "pppoe"          },
    {MID_WLAN,            "wlan"           },
    {MID_TR069BE,         "tr069be"        },
    {MID_DGN,             "ping"           },
    {MID_DHCPR,           "dhcpr"          },
    {MID_DHCPS,           "dhcps"          },
    {MID_TIMER,           "timer"          },
    {MID_IPCONN,          "ipconn"         },
    {MID_FIREWALL,        "firewall"       },
    {MID_SNMPC,           "snmpc"          },
    {MID_QOS,             "qos"            },
    {MID_SROUTE,  		  "sroute"         },
    {MID_STATIC_ROUTING,  "staticrouting"  },
    {MID_VDSL,            "vdsl"           },
    {MID_DNS,             "dns"            },
    {MID_ALG,             "alg"            },
#if (defined(CONFIG_RT63365) || defined(CONFIG_RT63368))
    {MID_UTMPROXY,		  "utmproxy"	   },
    {MID_AUTOUPGRADE,    "autoupgrade"     },
#endif
    {MID_WAN,             "wan"            },
    {MID_DROUTE,          "droute" 		   },
    {MID_SNTP,            "sntp"           },
    {MID_VLAN,            "vlan"           },
    {MID_USB_MASS,        "usbmass"        },
    {MID_LOG,             "logger"         },
    {MID_FTPD,            "ftpd"           },
    {MID_NATPRIO,         "natprio"        },
    {MID_WPS,             "wps"            },
    {MID_ACL,             "acl"            },
    {MID_UPNP,            "upnp"           },
    {MID_LSVLAN,          "lanside_vlan"   },
    {MID_PORTOFF,         "portoff"        },
    {MID_ANTIATTACK,      "antiattack"     },
    {MID_SPT,      		  "shareprotection"},
    {MID_DEVINFO,         "devinfo"        },
    {MID_PORTMAPPING,     "portmapping"    },
    {MID_URLFILTER,       "urlfilter"      },
    {MID_ATM,             "atm"            },
    {MID_DSL,             "dsl"            },
    {MID_DDNS,            "ddns"           },
    {MID_PROUTE,          "proute"         },
    {MID_CFG,             "cfg"            },
    {MID_SUPP,            "supp"           },
    {MID_MACFILTER,       "macfilter"      },
    {MID_TRACERT,         "tracert"        },
    {MID_IPPD,            "ippd"           },
    {MID_WEBP,            "webp"           },
    {MID_BRIDGE_FILTER,   "brfilter"       },
    {MID_SCHED,           "sched"          },
    {MID_PORTTRIGGER,     "porttriger"     },
    {MID_IP_ACL,          "ipacl"          },
    {MID_DEFAULTGW,       "defaultgw"      },
    {MID_DIAG,            "diag"           },
    {MID_WANSELECT,       "wanselect"      },
    {MID_DEVCONFIG,       "devconfig"      },
    {MID_TRADITIONALNAT,  "traditionnat"   },
    {MID_FIREWALLLOG,     "firewalllog"    },
    {MID_IPMACFILTER,     "ipmacfilter"    },
    {MID_UDPECHO,         "udpecho"        },
    {MID_DOWNLOADDIAG,    "downloaddiag"   },
    {MID_UPLOADDIAG,      "uploaddiag"     },
    {MID_SAMBA,           "samba"          },
    {MID_USB3G,           "usb3g"          },
    {MID_TF_PORTMAPPING,  "tf_portmapping" },
    {MID_TF_FIREWALL,     "tf_firewall"    },
    {MID_OS_INFO,         "osinfo"         },
    {MID_WEB,             "web"            },
    {MID_TR069FE,         "tr069fe"        },
    {MID_TF_GUI,		  "tf_gui"		   }
};

    /* ������ָ�� */
    g_pstCfgTree = CFG_GetMIDTree();

    ret = CFG_TreeSepAccess(".", apvTree);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return;
    }

    /* �������� */
    g_pstCfgTree = pvTmp;

    /* ��ӡ�� */
    for (i = 0; i < sizeof(apvTree)/sizeof(apvTree[0]); i++)
    {
        if (NULL != apvTree[i])
        {
            for (j = 0; j < sizeof(astModName)/sizeof(astModName[0]); j++)
            {
                if (MID2INDEX(astModName[j].usMID) == (unsigned short)i)
                {
                    break;
                }
            }

            sprintf(acFile, "../../../../../src/apps/logic/%s/%s.xml",
                           astModName[j].pcName, astModName[j].pcName);

            stTree.root = apvTree[i];

            fp = fopen(acFile, "w");
            ret = CFG_PrintNode(&stTree, "", 0xffff * 2, fp);
            if (0 != ret)
            {
                printf("i = %lu\n", i);
            }
            if (NULL != fp)
            {
                fclose(fp);
            }
        }
    }

    /* �ͷ��� */
    for (i = 0; i < sizeof(apvTree)/sizeof(apvTree[0]); i++)
    {
        if (NULL != apvTree[i])
        {
            scew_element_del(apvTree[i]);
        }
    }
}

#endif



#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif





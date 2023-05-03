
/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_r2.c
 �ļ����� : cfg ģ�����r2�汾���Ż������ӵĺ���
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2008-11-23
            ���� :
**********************************************************************/

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


#include "cfg_api.h"
#include "cfg_file.h"
#include "cfg_prv.h"

/* �������ؽڵ�ֵ = NULL ����� */
STATIC const char *s_pcCfgStringNull = "";


/* ��ͬ���͵����ֵ�ĺ��� */
typedef CFG_RET (*FUNC_VAL_TYPE_GET)(const char *pcVal, void *pvOut);


typedef struct
{
    const char *pcType;
    unsigned char ucTypeLen;
    unsigned char aucSpace[3];
    FUNC_VAL_TYPE_GET pfnValTypeGet;

} ST_CFG_VAL_TYPE_GET;


/***********************************************************
�ӿ�:   ��ȡ������ڵ�ֵ
����:   pcVal, ����ֵ
        pvOut, ���ֵ�ĵ�ַ, �����������""�ĵ�ַ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_objectValTypeGet(const char *pcVal, void *pvOut)
{
    (void)pcVal;

    *(char **)pvOut = (char *)s_pcCfgStringNull;
    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡ�ַ�����ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_stringValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal)
    {
        *(char **)pvOut = (char *)s_pcCfgStringNull;
    }
    else
    {
        *(char **)pvOut = (char *)pcVal;
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ȡint��ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ, ���ת�����ֵ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_intValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal)
    {
        *(int *)pvOut = 0;
    }
    else
    {
        *(int *)pvOut = strtol(pcVal, NULL, 10);
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡunsigned int��ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ, ���ת�����ֵ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_unsignedIntValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal)
    {
        *(int *)pvOut = 0;
    }
    else
    {
        *(int *)pvOut = strtol(pcVal, NULL, 10);
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡboolean��ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ, ���ת�����ֵ
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_booleanValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal || '0' == pcVal[0])
    {
        *(int *)pvOut = 0;
    }
    else
    {
        *(int *)pvOut = 1;
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ȡtime��ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ, ʵ���ϸ�string����һ��
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_dateTimeValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal)
    {
        *(char **)pvOut = (char *)s_pcCfgStringNull;
    }
    else
    {
        *(char **)pvOut = (char *)pcVal;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡbase64��ڵ�ֵ
����:   pcVal, ����ֵ��ַ
        pvOut, ���ֵ�ĵ�ַ, ʵ���ϸ�string����һ��
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_base64ValTypeGet(const char *pcVal, void *pvOut)
{
    if (NULL == pcVal)
    {
        *(char **)pvOut = (char *)s_pcCfgStringNull;
    }
    else
    {
        *(char **)pvOut = (char *)pcVal;
    }
    return CFG_OK;
}


#define CFG_TYPE_GET_ITEM(type) \
    {#type, sizeof(#type) - 1, {0}, CFG_##type##ValTypeGet}


STATIC ST_CFG_VAL_TYPE_GET s_astCfgValTypeGet[] =
{
    CFG_TYPE_GET_ITEM(object),
    CFG_TYPE_GET_ITEM(string),
    CFG_TYPE_GET_ITEM(int),
    CFG_TYPE_GET_ITEM(unsignedInt),
    CFG_TYPE_GET_ITEM(boolean),
    CFG_TYPE_GET_ITEM(dateTime),
    CFG_TYPE_GET_ITEM(base64)
};

#define CFG_VAL_TYPE_GET_NUM \
              (sizeof(s_astCfgValTypeGet)/sizeof(s_astCfgValTypeGet[0]))



/***********************************************************
�ӿ�:   �Ӻ���, ͨ���ڵ��ַ��ȡһ���ڵ��б�ֵ��һ���ṹ����
����:   pstNode, �ڵ��ַ
        pvStruct, ���ֵ�Ľṹ���ַ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
        ucAllString, 0 ���� 1, ��ʾ�Ƿ�ȫ������string���������.
                     ���������Ŀ����Ϊ�˼ӿ��ٶ�(��Ϊ�����ж�����)
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetValToStructByPtr(const scew_element *pstNode, void *pvStruct,
                                char * const *ppcLeafList,
                                unsigned long ulLeafNum,
                                unsigned char ucAllString)
{
    scew_element *pstChild = NULL;
    CFG_RET ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcType = NULL;
    const char *pcVal = NULL;
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long *pulOut = pvStruct;
    unsigned char ucHasDone = 0;

    for (i = 0; i < ulLeafNum; i++)
    {
        if (NULL == ppcLeafList[i])
        {
            CFG_ERR(ERR_CFG_PARA_INVALID);
            return ERR_CFG_PARA_INVALID;
        }

        if ('\0' == ppcLeafList[i][0])
        {
            continue;
        }
        *(pulOut + i) = 0L;
        ucHasDone = 0;

        pstChild = NULL;
        while (pstChild = scew_element_next(pstNode, pstChild),
               NULL != pstChild)
        {
            if (0 == strcmp(scew_element_name(pstChild), ppcLeafList[i]))
            {
                pcVal = scew_element_contents(pstChild);
                if (1 == ucAllString)
                {
                    ucHasDone = 1;
                    ret = CFG_stringValTypeGet(pcVal, pulOut + i);
                    if (CFG_OK != ret)
                    {
                        CFG_ERR(ret);
                        return ret;
                    }
                    break;
                }

                pstAttr = scew_attribute_by_name(pstChild, ATTR_TYPE);
                if (NULL != pstAttr)
                {
                    pcType = scew_attribute_value(pstAttr);
                    for (j = 0; j < CFG_VAL_TYPE_GET_NUM; j++)
                    {
                        if (0 == strncmp(pcType, s_astCfgValTypeGet[j].pcType,
                                          s_astCfgValTypeGet[j].ucTypeLen))
                        {
                            ucHasDone = 1;
                            ret = s_astCfgValTypeGet[j].pfnValTypeGet
                                           (pcVal, pulOut + i);
                            if (CFG_OK != ret)
                            {
                                CFG_ERR(ret);
                                return ret;
                            }
                            break;
                        }
                    }
                }

                break;
            }
        }
        if (0 == ucHasDone)
        {
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", ppcLeafList[i]);
            return ERR_CFG_PATH_NOT_EXSITED;
        }
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ͨ���ڵ�·����ȡһ���ڵ��б�ֵ��һ���ṹ����,
                �����ֵ���Ǹ�������ת�����
����:   pcPath, �ڵ�·��
        pvStruct, ���ֵ�Ľṹ���ַ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetValToStruct(const char *pcPath, void *pvStruct,
                           char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    const scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    ret = CFG_GetValToStructByPtr(pstNode, pvStruct, ppcLeafList, ulLeafNum, 0);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcPath);
    }

    return ret;
}

/***********************************************************
�ӿ�:   ͨ���ڵ�·����ȡһ���ڵ��б�ֵ��һ���ṹ����,
                ���нڵ�ֵ��string�������
����:   pcPath, �ڵ�·��
        pvStrArr, ���ֵ�Ľṹ���ַ, ����Ķ��ǰ�string����
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetValToStrArr(const char *pcPath, void *pvStrArr,
                           char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    const scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    ret = CFG_GetValToStructByPtr(pstNode, pvStrArr, ppcLeafList, ulLeafNum, 1);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcPath);
    }

    return ret;
}


/* ��ͬ���͵�����ֵ�ĺ��� */
typedef CFG_RET (*FUNC_VAL_TYPE_SET)(scew_element *pstNode, const void *pvVal,
                                     unsigned char *pucChanged);


typedef struct
{
    const char *pcType;
    unsigned char ucTypeLen;
    unsigned char aucSpace[3];
    FUNC_VAL_TYPE_SET pfnValTypeSet;

} ST_CFG_VAL_TYPE_SET;



/***********************************************************
�ӿ�:   ���ö�����ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_objectValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    (void)pstNode;
    (void)pvVal;
    (void)pucChanged;

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ����string��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_stringValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    const char *pcNewVal = *(char * const *)pvVal;
    const char *pcOld =  scew_element_contents(pstNode);

    if (NULL == pcNewVal)
    {
        pcNewVal = s_pcCfgStringNull;
    }

    if (NULL == pcOld || 0 != strcmp(pcNewVal, pcOld))
    {
        pcNewVal = scew_element_set_contents(pstNode, pcNewVal);
        if (NULL == pcNewVal)
        {
            return CFG_FAIL;
        }
        *pucChanged = 1;
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ����int��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ�ĵ�ַ, ����ָ�����һ��int�͵�ֵ
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_intValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    char acIntVal[32];
    const char *pcNewVal = NULL;
    const char *pcOld =  scew_element_contents(pstNode);

    sprintf(acIntVal, "%d", *(int *)pvVal);
    if (NULL == pcOld || 0 != strcmp(acIntVal, pcOld))
    {
        pcNewVal = scew_element_set_contents(pstNode, acIntVal);
        if (NULL == pcNewVal)
        {
            return CFG_FAIL;
        }

        *pucChanged = 1;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ����unsigned int��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ�ĵ�ַ, ����ָ�����һ��unsigned int�͵�ֵ
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_unsignedIntValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    char acIntVal[32];
    const char *pcNewVal = NULL;
    const char *pcOld =  scew_element_contents(pstNode);

    sprintf(acIntVal, "%u", *(unsigned int *)pvVal);
    if (NULL == pcOld || 0 != strcmp(acIntVal, pcOld))
    {
        pcNewVal = scew_element_set_contents(pstNode, acIntVal);
        if (NULL == pcNewVal)
        {
            return CFG_FAIL;
        }
        *pucChanged = 1;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ����boolean��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ�ĵ�ַ, ����ָ�����һ��boolean�͵�ֵ
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_booleanValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    char acBoolVal[2] = "0";
    const char *pcOld =  scew_element_contents(pstNode);

    if (0 != *(int *)pvVal)
    {
        acBoolVal[0] = '1';
    }
    if (NULL == pcOld || 0 != strcmp(acBoolVal, pcOld))
    {
        (void)scew_element_set_contents(pstNode, acBoolVal);
        *pucChanged = 1;
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ����date��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ, ����ָ�����һ���ַ���
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_dateTimeValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    const char *pcNewVal = *(char * const *)pvVal;
    const char *pcOld =  scew_element_contents(pstNode);

    if (NULL == pcNewVal)
    {
        pcNewVal = s_pcCfgStringNull;
    }

    if (NULL == pcOld || 0 != strcmp(pcNewVal, pcOld))
    {
        pcNewVal = scew_element_set_contents(pstNode, pcNewVal);
        if (NULL == pcNewVal)
        {
            return CFG_FAIL;
        }
        *pucChanged = 1;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ����base64��ڵ�ֵ
����:   pstNode, �ڵ�ֵ
        pcVal, Ҫ���óɵ�ֵ, ����ָ�����һ���ַ���
        pucChanged, ���ֵ�Ƿ񱻸ı�, �����������Ҫ��;���� Notification
����ֵ: 0:�ɹ�
��ע:
***********************************************************/
CFG_RET CFG_base64ValTypeSet(scew_element *pstNode, const void *pvVal,
                             unsigned char *pucChanged)
{
    const char *pcNewVal = *(char * const *)pvVal;
    const char *pcOld =  scew_element_contents(pstNode);

    if (NULL == pcNewVal)
    {
        pcNewVal = s_pcCfgStringNull;
    }

    if (NULL == pcOld || 0 != strcmp(pcNewVal, pcOld))
    {
        pcNewVal = scew_element_set_contents(pstNode, pcNewVal);
        if (NULL == pcNewVal)
        {
            return CFG_FAIL;
        }
        *pucChanged = 1;
    }

    return CFG_OK;
}


#define CFG_TYPE_SET_ITEM(type) \
    {#type, sizeof(#type) - 1, {0}, CFG_##type##ValTypeSet}


STATIC ST_CFG_VAL_TYPE_SET s_astCfgValTypeSet[] =
{
    CFG_TYPE_SET_ITEM(object),
    CFG_TYPE_SET_ITEM(string),
    CFG_TYPE_SET_ITEM(int),
    CFG_TYPE_SET_ITEM(unsignedInt),
    CFG_TYPE_SET_ITEM(boolean),
    CFG_TYPE_SET_ITEM(dateTime),
    CFG_TYPE_SET_ITEM(base64)
};

#define CFG_VAL_TYPE_SET_NUM \
              (sizeof(s_astCfgValTypeSet)/sizeof(s_astCfgValTypeSet[0]))


/***********************************************************
�ӿ�:   �Ӻ���, ͨ���ڵ��ַ����һ���ڵ��б�ֵ, ֵ��Դ��һ���ṹ��
����:   pstNode, �ڵ��ַ
        pvStruct, ����ֵ����Դ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���,
        pstInfo, �ڵ���Ϣ, ��Ҫ�������� Notification ����
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   �ú����ڲ��ж� Notification ������
***********************************************************/
CFG_RET CFG_SetValFromStructByPtr(const scew_element *pstNode, const void *pvStruct,
                           char * const *ppcLeafList, unsigned long ulLeafNum,
                           ST_CFG_NODE_INFO *pstInfo)
{
    scew_element *pstChild = NULL;
    CFG_RET ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcType = NULL;
    unsigned long i = 0;
    unsigned long j = 0;
    const unsigned long *pulOut = pvStruct;
    unsigned char ucHasDone = 0;
    unsigned char ucChanged = 0;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD | CFG_NODE_INFO_NOTI
                     | CFG_NODE_INFO_MWNOTI_ITMS | CFG_NODE_INFO_MWNOTI_MW;

    for (i = 0; i < ulLeafNum; i++)
    {
        if (NULL == ppcLeafList[i])
        {
            CFG_ERR(ERR_CFG_PARA_INVALID);
            return ERR_CFG_PARA_INVALID;
        }

        if ('\0' == ppcLeafList[i][0])
        {
            continue;
        }
        ucHasDone = 0;
        pstChild = NULL;
        while (pstChild = scew_element_next(pstNode, pstChild),
               NULL != pstChild)
        {
            if (0 == strcmp(scew_element_name(pstChild), ppcLeafList[i]))
            {
                pstAttr = scew_attribute_by_name(pstChild, ATTR_TYPE);
                if (NULL != pstAttr)
                {
                    pcType = scew_attribute_value(pstAttr);
                    for (j = 0; j < CFG_VAL_TYPE_SET_NUM; j++)
                    {
                        if (0 == strncmp(pcType, s_astCfgValTypeSet[j].pcType,
                                          s_astCfgValTypeSet[j].ucTypeLen))
                        {
                            ucHasDone = 1;
                            ret = s_astCfgValTypeSet[j].pfnValTypeSet
                                         (pstChild, pulOut + i, &ucChanged);
                            if (CFG_OK != ret)
                            {
                                CFG_ERR(ret);
                                return ret;
                            }
                            if (1 == ucChanged)
                            {
                                CFG_UpdateNodeInfo(pstChild, pstInfo, ulMask);
                               (void)CFG_NotiInform(pstChild, pstInfo);
                            }

                            break;
                        }
                    }
                }

                break;
            }
        }
        if (0 == ucHasDone)
        {
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", ppcLeafList[i]);
            return ERR_CFG_PATH_NOT_EXSITED;
        }
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ͨ���ڵ�·������һ���ڵ��б�ֵ, ֵ��Դ��һ���ṹ��
����:   pcPath, �ڵ�·��
        pvStruct, ����ֵ����Դ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_SetValFromStruct(const char *pcPath, const void *pvStruct,
                           char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD | CFG_NODE_INFO_NOTI
                     | CFG_NODE_INFO_MWNOTI_ITMS | CFG_NODE_INFO_MWNOTI_MW;

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    ret = CFG_SetValFromStructByPtr(pstNode, pvStruct,
                  ppcLeafList, ulLeafNum, &stNodeInfo);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcPath);
    }

    return ret;
}

/***********************************************************
�ӿ�:   ͨ���ڵ�·������һ���ڵ��б�ֵ, ֵ��Դ��һ���ַ�����
����:   pcPath, �ڵ�·��
        pvStrArr, ����ֵ����Դ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_SetValFromStrArr(const char *pcPath, void *pvStrArr,
                           char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD | CFG_NODE_INFO_NOTI;

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    ret = CFG_SetValFromStructByPtr(pstNode, pvStrArr,
                  ppcLeafList, ulLeafNum, &stNodeInfo);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcPath);
    }

    return ret;
}



#if 1


/***********************************************************
�ӿ�:   ��ȡһ���ڵ��ֵ�ĵ�ַ, �����͵�ַ
����:   pcPath, �ڵ�·��
        ppcVal, ���ָ��ڵ�ֵ��ָ��
        ppcType, ���ָ��ڵ����͵�ָ��, ���Ϊ��, ���������
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetNodeValPtr(const char *pcPath, const char **ppcVal,
                          const char **ppcType)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    scew_attribute *pstAttr =NULL;
    const char *pcVal = NULL;

    if (NULL == pcPath)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    if (NULL != ppcVal)
    {
        pcVal = scew_element_contents(pstNode);
        if (NULL == pcVal)
        {
            pcVal = "";
        }

        *ppcVal = pcVal;
    }
    if (NULL != ppcType)
    {
        pstAttr = scew_attribute_by_name(pstNode, ATTR_TYPE);
        if (NULL != pstAttr)
        {
            *ppcType = scew_attribute_value(pstAttr);
        }
        else
        {
            *ppcType = "";
        }
    }

    return CFG_OK;
}



#endif



#if 1


/***********************************************************
�ӿ�:   �Ե����ڵ�, ����ýڵ�mid����ָ��mid, �򽫸ýڵ��ֵ ���뵽������
����:   pstNode, �ڵ��ַ
        pcAbsPath, �ڵ�ȫ·��
        ucLen, �ڵ�ȫ·������
        usMID, ���ڵ�������mid
        usMIDBegin, ��ָ����mid
        pvPara, ���������ĵ�ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_XxxSingleNodeVal(const scew_element *pstNode, const char *pcAbsPath,
                 unsigned char ucLen, unsigned short usMID,
                 unsigned short usMIDBegin, void *pvPara)
{
    ST_PARA_VAL **ppstParaValList = (ST_PARA_VAL **)pvPara;
    ST_PARA_VAL *pstParaVal = NULL;
    unsigned long ulBufLen = 0;
    const char *pcVal = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcType = NULL;
    const char *pcAttrVal = "";

    if (usMID != usMIDBegin)
    {
        return CFG_OK;
    }

    /* �������ͽ������� */
    CFG_FILTER_NON_LEAF(pstNode);
    pcType = pcAttrVal;

    pcVal = scew_element_contents(pstNode);
    if (NULL == pcVal)
    {
        pcVal = "";
    }

    ulBufLen = sizeof(ST_PARA_VAL) + strlen(pcType) + 1
              + ucLen + 1 + strlen(pcVal) + 1;
    pstParaVal = malloc(ulBufLen);
    if (NULL == pstParaVal)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    pstParaVal->pcType = (char *)(pstParaVal + 1);
    strcpy(pstParaVal->pcType, pcType);
    pstParaVal->pcName = pstParaVal->pcType + strlen(pstParaVal->pcType) + 1;
    strcpy(pstParaVal->pcName, pcAbsPath);
    pstParaVal->pcVal = pstParaVal->pcName + strlen(pstParaVal->pcName) + 1;
    strcpy(pstParaVal->pcVal, pcVal);

    /* �������� */
    CFG_LIST_APPEND(ppstParaValList, pstParaVal);

    return CFG_OK;
}


#define CFG_TREE_XXX_OPT(pstNode, pfnGetOpt, pcAbsPath, ucLen, usMID, usMIDBegin, pvData) \
{ \
    ret = pfnGetOpt(pstNode, pcAbsPath, ucLen, usMID, usMIDBegin, pvData); \
    if (CFG_OK != ret) \
    { \
        return ret; \
    } \
}

typedef long (*FUNC_XXX_OPT)(const scew_element *pstNode, const char *pcAbsPath,
                 unsigned char ucLen, unsigned short usMID,
                 unsigned short usMIDBegin, void *pvPara);


/***********************************************************
�ӿ�:   ������������ȡ����ָ��MID�Ľڵ��б�
����:   pstNode, �ڵ��ַ
        pstInfo, �ڵ���Ϣ, ������Ҫ�õ����е�usMID, ������ʾpstNode �������ĸ�mid��
        pfnGetOpt, �����ڵ�Ĳ�������
        pcAbsPath, �ڵ�ȫ·���Ĵ�ŵ�ַ
        usMID, ָ��Ҫ��ȡ��MID�Ľڵ��б�
        pvPara, ���������ĵ�ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
long CFG_TreeXxxAccess(const scew_element *pstNode, const ST_CFG_NODE_INFO *pstInfo,
                          FUNC_XXX_OPT pfnGetOpt, char *pcAbsPath,
                          unsigned short usMID, void *pvData)
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
    astStack[0].stNodeInfo.usMID = pstInfo->usMID;
    ucStackTop = 1;

    /* ���� */
    CFG_TREE_XXX_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
            astStack[ucStackTop - 1].ucPathLen, usMID, usMID, pvData);

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
                pstAttr = scew_attribute_by_name(pstNodeWork, ATTR_MID);  /* ȡMID */
                if (NULL == pstAttr)
                {
                    astStack[ucStackTop].stNodeInfo.usMID
                        = astStack[ucStackTop- 1].stNodeInfo.usMID;
                }
                else
                {
                    pcAttrVal = scew_attribute_value(pstAttr);
                    astStack[ucStackTop].stNodeInfo.usMID
                         = strtol(pcAttrVal, NULL, CFG_MID_BASE);
                }
                ucStackTop++;

                /* ���� */
                CFG_TREE_XXX_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                       astStack[ucStackTop - 1].ucPathLen,
                       astStack[ucStackTop- 1].stNodeInfo.usMID, usMID, pvData);
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
            pstAttr = scew_attribute_by_name(pstNodeWork, ATTR_MID);  /* ȡMID */
            if (NULL == pstAttr)
            {
                astStack[ucStackTop].stNodeInfo.usMID
                    = astStack[ucStackTop- 1].stNodeInfo.usMID;
            }
            else
            {
                pcAttrVal = scew_attribute_value(pstAttr);
                astStack[ucStackTop].stNodeInfo.usMID
                     = strtol(pcAttrVal, NULL, CFG_MID_BASE);
            }
            ucStackTop++;

            /* ���� */
            CFG_TREE_XXX_OPT(pstNodeWork, pfnGetOpt, pcAbsPath,
                   astStack[ucStackTop - 1].ucPathLen,
                   astStack[ucStackTop- 1].stNodeInfo.usMID, usMID, pvData);
        }
    }

    return CFG_OK;
}





/***********************************************************
�ӿ�:   ��·��������ȡ����ָ��MID�Ľڵ��б�
����:   pcPath, �ڵ�·��
        usMID, ָ��Ҫ��ȡ��MID�Ľڵ��б�
        ppstParaVal, ���������ĵ�ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetMIDNodes(char *pcPath, unsigned short usMID, ST_PARA_VAL **ppstParaVal)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    ST_PARA_VAL *apstParaValList[2] = {NULL, NULL};
    char acAbsPath[CFG_MAX_PATH_LEN];
    ST_CFG_NODE_INFO stInfo;

    if (NULL == pcPath || NULL == ppstParaVal)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stInfo, CFG_NODE_INFO_MID);

    if ('\0' == pcPath[0] || '.' == pcPath[0])
    {
        strcpy(acAbsPath, scew_element_name(pstNode));
    }
    else
    {
        strcpy(acAbsPath, pcPath);
    }

    /* ��ȫ���� */
    ret = CFG_TreeXxxAccess(pstNode, &stInfo, CFG_XxxSingleNodeVal,
                    acAbsPath, usMID, apstParaValList);
    if (CFG_OK != ret)
    {
        /* �ͷ����� */
        (void)CFG_MemFree(apstParaValList[0]);

        return ret;
    }

    *ppstParaVal = apstParaValList[0];

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ȡһ���ڵ��ȫ·��, ����ķ����Ǵӵ�ǰ�ڵ�һ�����ص����ڵ�
����:   pstNode, �ڵ��ַ
        pcPath, ����Ľڵ�ȫ·��
        pulLen, �������pcPath�Ŀ�д�ĳ���, ����Ľڵ�·���ĳ���
����ֵ: 0:�ɹ�
        ����: ʧ��, ����������
��ע:   �ú������������ڶ�ͨ��·�����в���ʱ, ʡȥ��ÿ��ͨ��ڵ㱣��ȫ·���Ļ�����,
        ��ʱ�任�ռ�
***********************************************************/
CFG_RET CFG_GetFullPath(const scew_element *pstNode, char *pcPath,
                        unsigned long *pulLen)
{
    const char *pcName = NULL;
    unsigned long ulNameLen = 0;
    unsigned long ulLen = *pulLen;
    char *pcPathEnd = pcPath + ulLen;

    while (NULL != pstNode)
    {
        pcName = scew_element_name(pstNode);
        NODE_WILDCARD_RCVT(pcName);
        ulNameLen = strlen(pcName);

        if (pcPathEnd < pcPath + ulNameLen + 1)
        {
            CFG_ERR(ERR_CFG_BUF_NOT_ENOUGH);
            return ERR_CFG_BUF_NOT_ENOUGH;
        }
        --pcPathEnd;
        *pcPathEnd = DELIMIT_C;
        pcPathEnd -= ulNameLen;
        memcpy(pcPathEnd, pcName, ulNameLen);

        pstNode = pstNode->parent;
    }

    pcPath[ulLen - 1] = '\0';

    *pulLen = (unsigned long)(pcPath + ulLen - pcPathEnd) - 1;

    strcpy(pcPath, pcPathEnd);

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ͨ��·����ȡ����ָ��MID�Ľڵ��б�,
����:   pcWildPath, ͨ��·��
        usMID, ָ����MID
        ppstParaVal, ���������ĵ�ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   �����ͨ��·������������MID����ָ����MID
***********************************************************/
CFG_RET CFG_GetWildMIDNodes(char *pcWildPath, unsigned short usMID, ST_PARA_VAL **ppstParaVal)
{
    CFG_RET ret = CFG_OK;

    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;
    ST_PARA_VAL *apstParaValList[2] = {NULL, NULL};
    char acFullPath[CFG_MAX_PATH_LEN];
    unsigned long ulLen = 0;
    ST_CFG_NODE_INFO stInfo;

    char *pcPath = pcWildPath;

    if (NULL == pcPath || 0 == usMID)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    ret = CFG_GetWildMIDNodesPtr(pcPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    stInfo.usMID = usMID;

    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;

        ulLen = CFG_MAX_PATH_LEN;
        ret = CFG_GetFullPath(apstTree[ulLoop], acFullPath, &ulLen);
        if (CFG_OK != ret)
        {
            (void)CFG_MemFree((void *)apstParaValList[0]);

            CFG_ERR(ret);
            return ret;
        }

        ret = CFG_TreeXxxAccess(apstTree[ulLoop], &stInfo, CFG_XxxSingleNodeVal,
                                    acFullPath, usMID, apstParaValList);

        if (CFG_OK != ret)
        {
            (void)CFG_MemFree(apstParaValList[0]);

            CFG_ERR(ret);
            return ret;
        }
    }

    *ppstParaVal = apstParaValList[0];

    return CFG_OK;
}




#endif







#if 1
/***********************************************************
�ӿ�:   �����ڵ�ĸ��ƺ���
����:   pstNode, �ڵ��ַ
        ppstDupNode, �����Ժ�Ľڵ��ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_DupSingleNode(const scew_element *pstNode, scew_element **ppstDupNode)
{
    const char *pcValNew = NULL;
    scew_element *pstDupNode = *ppstDupNode;
    scew_attribute *pstAttr =NULL;
    scew_attribute *pstAttrOld =NULL;

    if (NULL == pstDupNode)
    {
        pstDupNode = scew_element_create(scew_element_name(pstNode));
    }
    else
    {
        pstDupNode = scew_element_add(pstDupNode, scew_element_name(pstNode));
    }
    if (NULL == pstDupNode)
    {
        /* ������� */
        CFG_ERR(ERR_CFG_INTERNAL_ERR);
        return ERR_CFG_INTERNAL_ERR;
    }

    pcValNew = scew_element_contents(pstNode);
    if (NULL != pcValNew)
    {
        pcValNew = scew_element_set_contents(pstDupNode, pcValNew);
        if (NULL == pcValNew)
        {
            CFG_ERR(ERR_CFG_INTERNAL_ERR);
            return ERR_CFG_INTERNAL_ERR;
        }
    }

    pstAttrOld = NULL;
    while (pstAttrOld = scew_attribute_next(pstNode, pstAttrOld),
           NULL != pstAttrOld)
    {
        CFG_ADDATTR(pstDupNode, scew_attribute_name(pstAttrOld),
                    scew_attribute_value(pstAttrOld), (void)0);
    }

    *ppstDupNode = pstDupNode;

    return CFG_OK;
}


#define CFG_TREE_DUP_OPT(pstNode, pfnSetOpt, pstDupNode, apstDupStack, ucDupStackTop) \
{ \
    ret = pfnSetOpt(pstNode, &pstDupNode); \
    if (CFG_OK != ret) \
    { \
        return ret; \
    } \
    apstDupStack[ucDupStackTop] = pstDupNode; \
    ucDupStackTop++; \
}


/***********************************************************
�ӿ�:   ���������ı�������
����:   pstNode, �ڵ��ַ
        ppstDupNode, �����Ժ�Ľڵ��ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   ������"i"���µĽڵ�, ����������һ������ʵ��, Ҳ���ڸ���һ��������
***********************************************************/
CFG_RET CFG_DupAccess(const scew_element *pstNode, scew_element **ppstDupNode)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    const scew_element *apstStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_element *apstDupStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucDupStackTop = 0;
    scew_element *pstDupNode = NULL;
    CFG_RET ret = CFG_OK;

    apstStack[0] = pstNode;
    ucStackTop++;

    /* ���� */
    CFG_TREE_DUP_OPT(pstNodeWork, CFG_DupSingleNode, pstDupNode,
                                 apstDupStack, ucDupStackTop);

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

                if (0 == strcmp(scew_element_name(pstNodeWork), NODE_WILDCARD))
                {
                    pstNodeWork = NULL;
                    ucDupStackTop++;
                    break;
                }

                /* ���� */
                CFG_TREE_DUP_OPT(pstNodeWork, CFG_DupSingleNode, pstDupNode,
                                 apstDupStack, ucDupStackTop);
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
        ucDupStackTop--;
        pstDupNode = apstDupStack[ucDupStackTop-1];

        /* ȡ�ֵ� */
        pstParent = apstStack[ucStackTop - 1];
        pstNodeWork = scew_element_next(pstParent, pstNodeWork);
        if (NULL != pstNodeWork)/* �ֵܲ�Ϊ�� */
        {
            /* ��ջ */
            apstStack[ucStackTop] = pstNodeWork;
            ucStackTop++;

            if (0 == strcmp(scew_element_name(pstNodeWork), NODE_WILDCARD))
            {
                pstNodeWork = NULL;
                ucDupStackTop++;
                continue;
            }

            /* ���� */
            CFG_TREE_DUP_OPT(pstNodeWork, CFG_DupSingleNode, pstDupNode,
                             apstDupStack, ucDupStackTop);
        }
    }

    *ppstDupNode = apstDupStack[0];

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡһ�����������������ʵ������
����:   pstObj, �ڵ��ַ
        pulCount, ��������ĸ���,
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   �ú�����ʵ����ȡ�ڵ��MaxNode����,Ȼ���Ѿ�������ֵ�ҵ�ָ�����ֵ�Ľڵ�,
        Ȼ�󽫽ڵ�ֵ������, ת��unsigned int
***********************************************************/
CFG_RET CFG_GetObjMaxAllowCount(const scew_element *pstObj,
                               unsigned long *pulCount)
{
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    const scew_element *pstNode = NULL;
    const char *pcVal = NULL;

    pstAttr = scew_attribute_by_name(pstObj, ATTR_MAX_NODE);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        pstNode = scew_element_by_name(pstObj->parent, pcAttrVal);
        if (NULL == pstNode)
        {
            /* ����ʧ�� */
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", pcAttrVal);
            return ERR_CFG_PATH_NOT_EXSITED;
        }

        pcVal = scew_element_contents(pstNode);
        *pulCount = (unsigned long)strtol(pcVal, NULL, 10);
    }
    else
    {
        *pulCount = (unsigned long)~0;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ����һ������ǰ��ʵ������
����:   pstObj, �ڵ��ַ
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   �ú�����ʵ����ȡ�ڵ��CurNode����,Ȼ���Ѿ�������ֵ�ҵ��ýڵ�,
        Ȼ�󽫸ýڵ��ֵ���óɵ�ǰ�������������
***********************************************************/
CFG_RET CFG_UpdateObjCurInstCount(const scew_element *pstObj)
{
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    scew_element *pstNode = NULL;
    char acVal[32];

    pstAttr = scew_attribute_by_name(pstObj, ATTR_CUR_NODE);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        pstNode = scew_element_by_name(pstObj->parent, pcAttrVal);
        if (NULL == pstNode)
        {
            /* ����ʧ�� */
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", pcAttrVal);
            return ERR_CFG_PATH_NOT_EXSITED;
        }

        sprintf(acVal, "%lu", (unsigned long)scew_element_count(pstObj));

        (void)scew_element_set_contents(pstNode, acVal);
    }

    return CFG_OK;
}



/* *pulIndex = 0, ��ʾ�Զ�����; �����ʵ�ʵĽ�� */
/***********************************************************
�ӿ�:   Ϊһ������·���Զ����һ��ʵ��
����:   pcPath, �ڵ�·��
        pulIndex, ��*pulIndex = 0, ��ʾ�Զ�����, �����ʵ�ʵĽ��
                  ����, ��ʵ������������ *pulIndex ������
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:   �������"i"���µĽڵ�
***********************************************************/
CFG_RET CFG_AddObjInst(const char *pcPath, unsigned long *pulIndex)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element *pstChild = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    long ret = CFG_OK;

    scew_element *pstBig = NULL;
    scew_element *pstMatch = NULL;

    unsigned long ulIndex = 0;
    char acIndexNode[32];
    unsigned long ulMax = 0;

    if (NULL == pcPath || NULL == pulIndex)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    ulIndex = *pulIndex;

    pstBig = scew_tree_root(g_pstCfgTree);
    pstMatch = scew_tree_root(CFG_GetMIDTree());

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
                CFG_FREE_PATH(pcPathTmp);

                CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s %s", pszTmp, pcPath);
                return ERR_CFG_PATH_NOT_EXSITED;
            }
            pstBig = pstChild;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

    /* �ж�С�����Ƿ���i */
    if (NULL != pstMatch)
    {
        pstMatch = scew_element_by_name(pstMatch, NODE_WILDCARD);
    }
    if (NULL == pstMatch || 0 == scew_element_count(pstMatch))
    {
        /* ���ش��� */
        CFG_ERR(ERR_CFG_INVALID_PATH, "%s", pcPath);
        return ERR_CFG_INVALID_PATH;
    }

    /* ȡ�ڵ������������ */
    ret = CFG_GetObjMaxAllowCount(pstBig, &ulMax);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcPath);
        return ret;
    }
    if (ulMax <= scew_element_count(pstBig))  /* ���������� <= ��ǰ���� */
    {
        /* ����ʧ�� */
        CFG_ERR(ERR_CFG_OBJ_INST_FULL, "%lu %lu %s", ulMax,
                      (unsigned long)scew_element_count(pstBig), pcPath);
        return ERR_CFG_OBJ_INST_FULL;
    }

    if (0 == ulIndex)
    {
        /* ȡ��maxidx */
        pstAttr = scew_attribute_by_name(pstBig, ATTR_MAX_IDX);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
            ulIndex = (unsigned long)strtol(pcAttrVal, NULL, 10);
        }
        ulIndex++;
    }
    sprintf(acIndexNode, "%lu", ulIndex);

    /* �ڵ�ǰ�ڵ��ϲ��ҽڵ� */
    pstChild = scew_element_by_name(pstBig, acIndexNode);
    if (NULL != pstChild)  /* �Ѵ��� */
    {
        /* ������� */
        CFG_ERR(ERR_CFG_INTERNAL_ERR);
        return ERR_CFG_INTERNAL_ERR;
    }

    /* ���ƽṹ */
    ret = CFG_DupAccess(pstMatch, &pstChild);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* �޸Ľڵ��� */
    (void)scew_element_set_name(pstChild, acIndexNode);

    /* ��ӵ������� */
    (void)scew_element_add_elem(pstBig, pstChild);

    ret = CFG_UpdateMaxIdx(pstBig);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* ���µ�ǰ���� */
    (void)CFG_UpdateObjCurInstCount(pstBig);

    /* ������� */
    *pulIndex = ulIndex;

    return CFG_OK;
}


typedef struct tagST_CFG_MID_PATH
{
    unsigned short usMID;
    unsigned char aucSpace[2];

    char *pcPath;

    struct tagST_CFG_MID_PATH *pstNext;

} ST_CFG_MID_PATH;


STATIC ST_CFG_MID_PATH *g_pstCfgMIDPathHead = NULL;
STATIC ST_CFG_MID_PATH *g_pstCfgMIDPathTail = NULL;

/***********************************************************
�ӿ�:   ���һ��MID��Ӧ��ע��·��
����:   pcPath, ע��·��
        usMID,  ·��������MID
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_AddMIDPath(const char *pcPath, unsigned short usMID)
{
    ST_CFG_MID_PATH *pstMIDPath = g_pstCfgMIDPathHead;
    unsigned long ulLen = 0;

    while (NULL != pstMIDPath)
    {
        if (!strcmp(pstMIDPath->pcPath, pcPath))
        {
            pstMIDPath->usMID = usMID;
            return CFG_OK;
        }

        pstMIDPath = pstMIDPath->pstNext;
    }

    ulLen = sizeof(ST_CFG_MID_PATH) + strlen(pcPath) + 1;
    pstMIDPath = malloc(ulLen);
    if (NULL == pstMIDPath)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }

    pstMIDPath->pcPath = (char *)(pstMIDPath + 1);

    pstMIDPath->usMID = usMID;
    strcpy(pstMIDPath->pcPath, pcPath);
    pstMIDPath->pstNext = NULL;

    if (NULL == g_pstCfgMIDPathHead)
    {
        g_pstCfgMIDPathHead = pstMIDPath;
    }
    else
    {
        g_pstCfgMIDPathTail->pstNext = pstMIDPath;
    }
    g_pstCfgMIDPathTail = pstMIDPath;

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��������, ��pstValList ���������� ppstValTotalList β��
����:   ppstValTotalList, �������� ��һ��Ԫ�ر�ʾͷ, �ڶ���Ԫ�ر�־β
        pstValList,  Ҫ��ӵ�����
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
void CFG_ValListCat(ST_PARA_VAL **ppstValTotalList, ST_PARA_VAL *pstValList)
{
    ST_PARA_VAL *pstValListTail = pstValList;

    if (NULL == pstValList)
    {
        return;
    }

    while (NULL != pstValListTail->pstNext)
    {
        pstValListTail = pstValListTail->pstNext;
    }

    if (NULL == ppstValTotalList[1])
    {
        ppstValTotalList[0] = pstValList;
        ppstValTotalList[1] = pstValListTail;
    }
    else
    {
        ppstValTotalList[1]->pstNext = pstValList;
        ppstValTotalList[1] = pstValListTail;
    }
}


/***********************************************************
�ӿ�:   �г���������ָ��MID�����нڵ�
����:   usMID, ָ����MID
        ppstParaVal, ����ڵ��б�
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_ListMIDAllNodes(unsigned short usMID, ST_PARA_VAL **ppstParaVal)
{
    ST_CFG_MID_PATH *pstMIDPath = g_pstCfgMIDPathHead;
    CFG_RET ret = CFG_OK;
    ST_PARA_VAL *apstValTotalList[2] = {NULL, NULL};
    ST_PARA_VAL *pstValList = NULL;

    while (NULL != pstMIDPath)
    {
        if (pstMIDPath->usMID == usMID)
        {
            ret = CFG_GetWildMIDNodes(pstMIDPath->pcPath, usMID, &pstValList);
            if (CFG_OK != ret)
            {
                /* �ͷ�ԭ�����ܱ� */
                (void)CFG_MemFree(apstValTotalList[0]);

                CFG_ERR(ret);
                return ret;
            }

            /* ��������������������� */
            CFG_ValListCat(apstValTotalList, pstValList);
        }

        pstMIDPath = pstMIDPath->pstNext;
    }

    *ppstParaVal = apstValTotalList[0];

    return CFG_OK;
}


#endif

/*
1. get: ȡ�߼���
2. Add: ȡ������

*/

STATIC unsigned short *s_pusCfgMIDList = NULL;
STATIC unsigned long s_ulCfgMIDCount = 0;

/***********************************************************
�ӿ�:   ��MID��ӵ�MID�б���,
����:   usMID, ָ����MID
����ֵ: ��
��ע:   ���������Ҫ������ȡһ�����������е�MID,
        ��һ��Ԫ�ش����������MID
***********************************************************/
void CFG_AddMIDToList(unsigned short usMID)
{
    unsigned long i = 0;

    for (i = 0; i < s_ulCfgMIDCount; i++)
    {
        if (usMID == s_pusCfgMIDList[i])
        {
            return;
        }
    }

    s_pusCfgMIDList[s_ulCfgMIDCount++] = usMID;
}

/***********************************************************
�ӿ�:   ��ʼ��MID�б�
����:   pusMIDList, ʵ�ʴ���б�ĵ�ַ
        usMID, ��һ��MID
����ֵ: ��
��ע:
***********************************************************/
void CFG_InitMIDList(unsigned short *pusMIDList, unsigned short usMID)
{
    s_pusCfgMIDList = pusMIDList;
    if (0 != usMID)
    {
        s_ulCfgMIDCount = 1;
        s_pusCfgMIDList[0] = usMID;
    }
    else
    {
        s_ulCfgMIDCount = 0;
    }
}


/***********************************************************
�ӿ�:   ��ȡ�����ڵ��MID�б�
����:   pstNode, �ڵ��ַ
        pvPara, ������
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetSingleNodeMID(const scew_element *pstNode, void *pvPara)
{
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;
    unsigned short usMID = 0;

    (void)pvPara;

    pstAttr = scew_attribute_by_name(pstNode, ATTR_MID);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        usMID = strtol(pcAttrVal, NULL, 16);
        if (0 != usMID)
        {
            CFG_AddMIDToList(usMID);
        }
    }

    return CFG_OK;
}



/***********************************************************
�ӿ�:   ���������ϻ�ȡһ��·�������е�MID
����:   pcPath, �ڵ�·��
        pusMIDList, ���MID�б�
        pulCount, ���mid�ĸ���
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:   ��Ҫ����ɾ��һ������ʵ��ʱ, ��Ҫ֪ͨ���µ�����ģ��
***********************************************************/
CFG_RET CFG_GetObjMIDListFromDataTree(const char *pcPath,
                unsigned short *pusMIDList, unsigned long *pulCount)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_MID;


    if (NULL == pcPath || NULL == pusMIDList)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    CFG_InitMIDList(pusMIDList, stNodeInfo.usMID);

    ret = CFG_TreeSetAccess(pstNode, CFG_GetSingleNodeMID, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    *pulCount = s_ulCfgMIDCount;

    return ret;
}



/***********************************************************
�ӿ�:   ���߼����ϻ�ȡһ��·�������е�MID
����:   pcPath, �ڵ�·��
        pusMIDList, ���MID�б�
        pulCount, ���mid�ĸ���
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:   ��Ҫ�������һ������ʵ��ʱ, ��Ҫ֪ͨ���µ�����ģ��
***********************************************************/
CFG_RET CFG_GetObjMIDListFromLogicTree(const char *pcPath,
                unsigned short *pusMIDList, unsigned long *pulCount,
                scew_element **ppstNode)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    long ret = CFG_OK;

    scew_element *pstMatch = NULL;
    unsigned short usMID = 0;
    unsigned char ucGetFull = 1;

    if (NULL == pcPath || NULL == pusMIDList || NULL == pulCount)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    pstMatch = scew_tree_root(CFG_GetMIDTree());

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstMatch);

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
            if (NULL == pstMatch)
            {
                /* �����ʾû���ҵ�����·��, ���߼�����,
                   ��һ��Ҫ�󱣴����нڵ�ģ��. ��Ҫ����: ʵ��ģ��, ��midģ��.
                   ���, һ��·���Ҳ�ȫ, �����ҵ����ϼ��ڵ��midΪ׼ */
                ucGetFull = 0;
                break;
#if 0
                CFG_FREE_PATH(pcPathTmp);

                CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s %s ", pszTmp, pcPath);
                return ERR_CFG_PATH_NOT_EXSITED;
#endif
            }

            pstAttr = scew_attribute_by_name(pstMatch, ATTR_MID);
            if (NULL != pstAttr)
            {
                pcAttrVal = scew_attribute_value(pstAttr);
                usMID = strtol(pcAttrVal, NULL, 16);
            }
        }
    }
    CFG_FREE_PATH(pcPathTmp);

    CFG_InitMIDList(pusMIDList, usMID);

    if (1 == ucGetFull)
    {
        ret = CFG_TreeSetAccess(pstMatch, CFG_GetSingleNodeMID, NULL);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }
    }

    *pulCount = s_ulCfgMIDCount;

    if (NULL != ppstNode)
    {
        *ppstNode = pstMatch;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡ�����µ�mid�б�, �ú�����ǰ�����������ķ�װ
����:   ucTree, ��������
        pcPath, �ڵ�·��
        pusMIDList, ���MID�б�
        pulCount, ���mid�ĸ���
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetObjMIDList(unsigned char ucTree, const char *pcPath,
                unsigned short *pusMIDList, unsigned long *pulCount)
{
    g_pcCfgAccessor = NULL;

    if (CFG_DATA_TREE == ucTree)
    {
        return CFG_GetObjMIDListFromDataTree(pcPath, pusMIDList, pulCount);
    }
    else
    {
        return CFG_GetObjMIDListFromLogicTree(pcPath, pusMIDList, pulCount, NULL);
    }
}


CFG_RET CFG_GetNodeMIDFromFullTree(const char *pcPath, unsigned short *pusMIDList,
                                   unsigned long *pulCount)
{
    CFG_RET ret = CFG_OK;
    scew_element *pstNode = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    unsigned short *pulCareList = NULL;

    ret = CFG_GetObjMIDListFromLogicTree(pcPath, pusMIDList, pulCount, &pstNode);
    if (CFG_OK != ret || NULL == pstNode)
    {
        CFG_ERR(ret);
        return ret;
    }

    pulCareList = &pusMIDList[*pulCount];

    CFG_GET_CARE_MID(pstNode, pulCareList, pulCount);

    return CFG_OK;
}



/***********************************************************
�ӿ�:   ���ýڵ�����
����:   pcPath, �ڵ�·��
        pcNewName, �µ�����
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_SetNodeName(const char *pcPath, const char *pcNewName)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    const char *pcName = NULL;

    if (NULL == pcPath || NULL == pcNewName)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    pcName = scew_element_set_name(pstNode, pcNewName);
    if (NULL == pcName)
    {
        CFG_ERR(ERR_CFG_INTERNAL_ERR);
        return ERR_CFG_INTERNAL_ERR;
    }

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ͨ���ڵ��ַ���ýڵ�����
����:   pvNode, �ڵ��ַ
        pcNewName, �µ�����
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_SetNodeNameByPtr(void *pvNode, const char *pcNewName)
{
    const char *pcName = NULL;

    pcName = scew_element_set_name(pvNode, pcNewName);
    if (NULL == pcName)
    {
        CFG_ERR(ERR_CFG_INTERNAL_ERR);
        return ERR_CFG_INTERNAL_ERR;
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ���ڵ�·����Ӧ������������
����:   pcPath, �ڵ�·��
        ppvNode, ��������������ַ
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_DelNodeToPtr(const char *pcPath, void **ppvNode)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    if (NULL == pcPath || NULL == ppvNode)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    if (NULL != pstNode->parent)
    {
        pstNode->parent->n_children--;
        if (pstNode == pstNode->parent->last_child)
        {
            pstNode->parent->last_child = pstNode->left;
        }
        if (pstNode == pstNode->parent->child)
        {
            pstNode->parent->child = pstNode->right;
        }
    }
    if (NULL != pstNode->left)
    {
        pstNode->left->right = pstNode->right;
    }
    if (NULL != pstNode->right)
    {
        pstNode->right->left = pstNode->left;
    }
    pstNode->parent = NULL;
    pstNode->right = NULL;
    pstNode->left = NULL;

    *ppvNode = pstNode;

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��������ӵ�ָ����·����
����:   pcPath, �ڵ�·��
        pvNode, Ҫ��ӵ�����
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_AddNodeFromPtr(const char *pcPath, void *pvNode)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    if (NULL == pcPath || NULL == pvNode)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    (void)scew_element_add_elem(pstNode, pvNode);

    return CFG_OK;
}



/***********************************************************
�ӿ�:   ��ȡͨ��·���Ľڵ��ַ�б�
����:   pcWildPath, �ڵ�ͨ��·��
        apstTree, ����ڵ��ַ�б�,
        ulBufNum, apstTree �������������
        pulHead, �о���ɺ��������ʼԪ���±�
        pulTail, �о���ɺ�����Ľ���Ԫ���±�
        pstMatchInfo, �洢ͨ��·����ͨ�����λ����Ϣ, ��������ں��潫���нڵ��ͨ���index�ҳ���
����ֵ: 0: �ɹ�
        ����: ʧ��
��ע:   �ú����ڲ�����һ�����ƻ��ζ��л���˵�ǹ������еķ���
***********************************************************/
CFG_RET CFG_GetWildMIDNodesPtr(const char *pcWildPath, scew_element **apstTree,
                               unsigned long ulBufNum,
                               unsigned long *pulHead, unsigned long *pulTail,
                               ST_CFG_MATCH_TREE_INFO *pstMatchInfo)
{
    char *pszTmp = NULL;
    char *pcPathTmp = NULL;
    scew_element* pstNode = NULL;
    scew_element* pstChild = NULL;
    long ret = CFG_OK;

    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long ulNewTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;

    unsigned char ucTotalLevel = 0;
    unsigned char ucLevelCount = 0;

    const char *pcPath = pcWildPath;

    pstNode = scew_tree_root(g_pstCfgTree);
    apstTree[ulTail] = pstNode;
    ulTail++;

    /* ����·�� */
    CFG_DUP_PATH(pcPathTmp, pcPath);

    pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp);

    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
        CFG_MATCH_ROOT(pszTmp, pcPathTmp, pcPath, pstNode);
        ucTotalLevel++;

        while (pszTmp = CFG_StrChrTok(pcPathTmp, DELIMIT_C, &pcPathTmp),
               NULL != pszTmp)
        {
            ucTotalLevel++;
            if (0 == strcmp(pszTmp, NODE_WILDCARD_EXP))  /* ͨ��� */
            {
                if (NULL != pstMatchInfo)
                {
                    pstMatchInfo->aucLevelList[ucLevelCount++] = ucTotalLevel;
                }

                ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + ulBufNum;
                ulNewTail = ulTail;
                for (i = ulHead; i < ulLoopEnd; i++)
                {
                    ulLoop = i % ulBufNum;

                    pstChild = NULL;
                    while (pstChild = scew_element_next(apstTree[ulLoop], pstChild),
                           NULL != pstChild)
                    {
                        if (ulNewTail == ulLoop)
                        {
                            /* ����������� */
                            CFG_FREE_PATH(pcPathTmp);
                            return ERR_CFG_WILDCARD_QUE_FULL;
                        }

                        /* �����ָ�� */
                        apstTree[ulNewTail] = pstChild;
                        ulNewTail = (ulNewTail + 1) % ulBufNum;
                    }  /* while */
                } /* for */
            }
            else  /* ��ͨ��ڵ� */
            {
                ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + ulBufNum;
                ulNewTail = ulTail;
                for (i = ulHead; i < ulLoopEnd; i++)
                {
                    ulLoop = i % ulBufNum;

                    /* ���� �� pszTmp ��ת�� */
                    CFG_NUM_TO_NODE(pszTmp);

                    pstChild = scew_element_by_name(apstTree[ulLoop], pszTmp);
                    if (NULL == pstChild)
                    {
                        continue;
                    }

                    apstTree[ulNewTail] = pstChild;
                    ulNewTail = (ulNewTail + 1) % ulBufNum;
                } /* for */
            }

            ulHead = ulTail;
            ulTail = ulNewTail;
        }
    }

    CFG_FREE_PATH(pcPathTmp);

    *pulHead = ulHead;
    *pulTail = ulTail;

    if (NULL != pstMatchInfo)
    {
        pstMatchInfo->ucTotalLevel = ucTotalLevel;
        pstMatchInfo->ucLevelCount = ucLevelCount;
    }

    return CFG_OK;
}


/* �ǵݹ��������ջ */
extern ST_CFG_TREE_STACK_NODE s_astCfgTreeStack[CFG_ACCESS_STACK_DEEPTH];

extern unsigned char s_ucCfgTreeStackPos;    /* �ǵݹ��������ջ�� */

/* �ǵݹ������ʱ��ž���·�� */
extern char  s_acCfgSavePath[CFG_MAX_PATH_LEN];

/*
STATIC ST_CFG_MID_PATH *s_pstCfgCurMatchPath = NULL;
STATIC scew_element *s_apstCfgTreeList[CFG_MAX_WILDCARD_NUM];
STATIC unsigned long s_ulCfgTreeHead = 0;
STATIC unsigned long s_ulCfgTreeTail = 0;
STATIC unsigned long s_ulCfgTreeCur = 0;
*/
STATIC const scew_element *s_pstCfgCurNode = NULL;
STATIC unsigned short s_usCfgListMID = 0;





/***********************************************************
�ӿ�:   Ϊ�о�ָ��mid�����нڵ������׼������
����:   usMID, ָ����mid
����ֵ: ��
��ע:
***********************************************************/
void CFG_MIDInstListBegin(unsigned short usMID)
{
    char *pcParh = NULL;

    s_usCfgListMID = usMID;
    (void)CFG_SetMIDAccessInfo(".", (const void **)(void *)&s_pstCfgCurNode,
                               &usMID, NULL, &pcParh);
}


/***********************************************************
�ӿ�:   Ϊ�о�ָ��mid�����нڵ������׼������, �ú�������ָ��·��
����:   usMID, ָ����mid
        pcPath, ָ����·��
����ֵ: ��
��ע:
***********************************************************/
void CFG_MIDInstListBeginEx(unsigned short usMID, const char *pcPath)
{
    char *pcParh = NULL;

    s_usCfgListMID = usMID;
    (void)CFG_SetMIDAccessInfo(pcPath, (const void **)(void *)&s_pstCfgCurNode,
                               &usMID, NULL, &pcParh);
}




/***********************************************************
�ӿ�:   ��ȡһ���ڵ��µ��ӽڵ���ͬ����ָ��mid�Ľڵ��б�, ͬʱ,
        ��ȡ���ڵ�����һ������ڵ�
����:   pstCurNode, ��ǰ�ڵ��ַ
        pcAbsPath, ��ǰ�ڵ����·����ŵ�ַ,
        ucLen, ��ǰ�ڵ����·������,
        usCurMID, ��ǰ�ڵ������ڵ�MID,
        ppstParaVal, ����ڵ��б�,
        ppstFirstChildObj, ������µĵ�һ������ڵ��ַ,
        pusNextMID, �����һ������ڵ��ַ��MID
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:   һ����˵, �������е����нڵ�������ʱ��ѭһ������, Ҷ����ǰ��, �����ں���,
        �����������ǵ��������, ���Բ�ȡ�˱������²����нڵ������,
***********************************************************/
CFG_RET CFG_NodesLeafList(const scew_element *pstCurNode,
                          char *pcAbsPath,  unsigned ucLen,
                          unsigned short usCurMID,
                          ST_PARA_VAL **ppstParaVal,
                          const scew_element **ppstFirstChildObj,
                          unsigned short *pusNextMID)
{
    const scew_element *pstNodeWork = pstCurNode;
    scew_element *pstChild = NULL;
    scew_element *pstChildPrev = NULL;
    const char *pcNodeName = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcType = "";
    const char *pcMID = "";
    const char *pcVal = "";
    ST_PARA_VAL *pstParaVal = NULL;
    unsigned long ulBufLen = 0;
    ST_PARA_VAL *apstParaValList[2] = {NULL, NULL};

    unsigned short usMID = usCurMID;
    unsigned char ucHasGetObj = 0;
    unsigned char ucNameLen = 0;

    while (pstChild = scew_element_next(pstNodeWork, pstChildPrev),
           NULL != pstChild)
    {
        pstChildPrev = pstChild;
        usMID = usCurMID;

        /* ȡ�ڵ����� */
        pstAttr = scew_attribute_by_name(pstChild, ATTR_TYPE);
        if (NULL != pstAttr)
        {
            pcType = scew_attribute_value(pstAttr);
        }

        /* ȡmid */
        pstAttr = scew_attribute_by_name(pstChild, ATTR_MID);
        if (NULL != pstAttr)
        {
            pcMID = scew_attribute_value(pstAttr);
            usMID = strtol(pcMID, NULL, 16);
        }

        if (0 == strcmp(pcType, TYPE_OBJECT)) /* ���� */
        {
            if (0 == ucHasGetObj) /* ��û��ȡ������ڵ� */
            {
                /* ���ñ�־ */
                ucHasGetObj = 1;
                *ppstFirstChildObj = pstChild;
                *pusNextMID = usMID;
            }
        }
        else
        {
            if (usMID == s_usCfgListMID) /* mid һ�� */
            {
                /* �Խڵ�������ת�� */
                pcNodeName = scew_element_name(pstChild);
                CFG_NODE_TO_NUM(pcNodeName);

                ucNameLen = strlen(pcNodeName);

                /* ���¾���·�� */
//                pcAbsPath[ucLen] = DELIMIT_C;
                strcpy(&pcAbsPath[ucLen], pcNodeName);

                pcVal = scew_element_contents(pstChild);
                if (NULL == pcVal)
                {
                    pcVal = "";
                }

                /* ���� */
                ulBufLen = sizeof(ST_PARA_VAL) + strlen(pcType) + 1
                          + ucLen + ucNameLen + 1 + strlen(pcVal) + 1;
                pstParaVal = malloc(ulBufLen);
                if (NULL == pstParaVal)
                {
                    CFG_ERR(ERR_CFG_MALLOC_FAIL);
                    return ERR_CFG_MALLOC_FAIL;
                }
                pstParaVal->pcType = (char *)(pstParaVal + 1);
                strcpy(pstParaVal->pcType, pcType);
                pstParaVal->pcName = pstParaVal->pcType + strlen(pstParaVal->pcType) + 1;
                strcpy(pstParaVal->pcName, pcAbsPath);
                pstParaVal->pcVal = pstParaVal->pcName + ucNameLen + ucLen + 1;
                strcpy(pstParaVal->pcVal, pcVal);

                /* �������� */
                CFG_LIST_APPEND(apstParaValList, pstParaVal);
            }
        }
    }

    *ppstParaVal = apstParaValList[0];

    return CFG_OK;
}

/***********************************************************
�ӿ�:   ��ȡ���ڵ�ͬ������һ���ֵܽڵ�
����:   pstCurNode, ��ǰ�ڵ��ַ
����ֵ: �ǿ�, ��ʾ��һ���ֵܽڵ�
        ��, ���ʾû����һ���ֵ���
��ע:
***********************************************************/
const scew_element *CFG_GetNextBratherObj(const scew_element *pstCurNode)
{
    const scew_element *pstParent = s_astCfgTreeStack[s_ucCfgTreeStackPos - 1].pstPath;
    const scew_element *pstNext = pstCurNode;
    scew_attribute *pstAttr = NULL;
    const char *pcType = "";

    while (pstNext = scew_element_next(pstParent, pstNext),
           NULL != pstNext)
    {
        pstAttr = scew_attribute_by_name(pstNext, ATTR_TYPE);
        if (NULL != pstAttr)
        {
            pcType = scew_attribute_value(pstAttr);
            if (0 == strcmp(pcType, TYPE_OBJECT)) /* ���� */
            {
                return pstNext;
            }
        }
    }

    return NULL;
}



/***********************************************************
�ӿ�:   ��������, ��ȡ��һ�������� ����ָ��mid�Ľڵ��б�
����:   ppstParaVal, ����ڵ��б�, ���Ϊ��, ����ζ���Ѿ���������
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:   �ú�������ǰ����� CFG_MIDInstListBegin ϵ��
***********************************************************/
CFG_RET CFG_ListMIDNextInstNodes(ST_PARA_VAL **ppstParaVal)
{
    const scew_element *pstNext = NULL;
    CFG_RET ret = CFG_OK;
    ST_PARA_VAL *pstParaVal = NULL;
    unsigned short usMID = 0;
    unsigned short usNextMID = 0;
    unsigned char ucLen = 0;

    *ppstParaVal = NULL;

getlist:
    if (NULL != s_pstCfgCurNode) /* NULL != ��ǰ�ڵ� */
    {
        /* ע: ��ɾ�� CFG_GetNextMID ��, �˺����п�ɾ�������ͽ����ж� */
        CFG_GetPathLenAndMID(s_pstCfgCurNode, &usMID, &ucLen);

        /* ȡ�б� */
        pstNext = NULL;
        ret = CFG_NodesLeafList(s_pstCfgCurNode, s_acCfgSavePath, ucLen,
                                usMID, &pstParaVal, &pstNext, &usNextMID);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }

        /* ��ջ */
        s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath = s_pstCfgCurNode;
        s_astCfgTreeStack[s_ucCfgTreeStackPos].stNodeInfo.usMID = usMID;
        s_astCfgTreeStack[s_ucCfgTreeStackPos].ucPathLen = ucLen;
        s_ucCfgTreeStackPos++;

        /* �ƶ���next */
        s_pstCfgCurNode = pstNext;

        if (NULL == pstParaVal) /* �б�Ϊ�� */
        {
            goto getlist;
        }
        else
        {
            /* ���� */
            *ppstParaVal = pstParaVal;
            return CFG_OK;
        }
    }

pop:
    /* ��ջ */
    --s_ucCfgTreeStackPos;
    if (1 >= s_ucCfgTreeStackPos)  /* ջ�� */
    {
        return CFG_OK;
    }

    s_pstCfgCurNode = s_astCfgTreeStack[s_ucCfgTreeStackPos].pstPath;
    usMID = s_astCfgTreeStack[s_ucCfgTreeStackPos].stNodeInfo.usMID;
    ucLen = s_astCfgTreeStack[s_ucCfgTreeStackPos].ucPathLen;
    s_acCfgSavePath[ucLen] = '\0';

    /* ȡͬ�������ֵ� */
    s_pstCfgCurNode = CFG_GetNextBratherObj(s_pstCfgCurNode);
    if (NULL == s_pstCfgCurNode)/* Ϊ�� */
    {
        goto pop;
    }

    goto getlist;

    /* return CFG_OK; */
}


/***********************************************************
�ӿ�:   ��䵥���ڵ㵽��������
����:   pstNode, �߼����ϵĽڵ��ַ
        pvData, ������
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:   �߼�����һ���ڵ���ܶ�Ӧ�������϶��ʵ���µĽڵ�,
        ������������һ��
***********************************************************/
CFG_RET CFG_FillSingleNode(const scew_element *pstNode, void *pvData)
{
    char acPath[CFG_MAX_PATH_LEN];
    unsigned long ulLen = 0;
    CFG_RET ret = CFG_OK;
    scew_element *pstDup = NULL;
    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;

    /* ��ȡ���ڵ��ȫ·�� */
    ulLen = CFG_MAX_PATH_LEN;
    ret = CFG_GetFullPath(pstNode->parent, acPath, &ulLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* ��ȡ�������ϵĸ��ڵ�ָ���б� */
    ret = CFG_GetWildMIDNodesPtr(acPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* �������� */
    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;

        ret = CFG_DupAccess(pstNode, &pstDup);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
            return ret;
        }

        /* ��ӵ��������� */
        (void)scew_element_add_elem(apstTree[ulLoop], pstDup);
    }

    (void)pvData;

    return CFG_OK;
}

#define CFG_TREE_EX_NODE_OPT(pstNode, pfnSetOpt, pvData, exp) \
{ \
    pcAttrVal = NULL;\
    pstAttr = scew_attribute_by_name(pstNode, ATTR_NEEDSAVE); \
    if (NULL != pstAttr) \
    { \
        pcAttrVal = scew_attribute_value(pstAttr); \
    } \
 \
    if (NULL != pcAttrVal && '0' == pcAttrVal[0]) \
    { \
        ret = pfnSetOpt(pstNode, pvData); \
        if (CFG_OK != ret) \
        { \
            return ret; \
        } \
        pstNode = NULL; \
        exp; \
    } \
}


/***********************************************************
�ӿ�:   ������ڵ�ı�������
����:   pstNode, �߼����ϵĽڵ��ַ
        pfnSetOpt, �����еĲ�������
        pvData, ������
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_TreeExNodeAccess(const scew_element *pstNode,
                          FUNC_SET_OPT pfnSetOpt, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    CFG_RET ret = CFG_OK;
    const scew_element *apstStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    apstStack[0] = pstNode;
    ucStackTop++;

    /* ���� */
    CFG_TREE_EX_NODE_OPT(pstNodeWork, pfnSetOpt, pvData, (void)0);

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

                /* ���� */
                CFG_TREE_EX_NODE_OPT(pstNodeWork, pfnSetOpt, pvData, break);
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

            /* ���� */
            CFG_TREE_EX_NODE_OPT(pstNodeWork, pfnSetOpt, pvData, continue);
        }
    }

    return CFG_OK;
}



/***********************************************************
�ӿ�:   ��䵥���ڵ�������б�
����:   pstNode, �߼����ϵĽڵ��ַ
        pvData, ��Ҫ��ӵ������б�
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_FillSingleAttr(const scew_element *pstNode, void *pvData)
{
    char acPath[CFG_MAX_PATH_LEN];
    unsigned long ulLen = 0;
    CFG_RET ret = CFG_OK;
    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    char *(*papcAttrList)[2] = pvData;
    unsigned long j = 0;

    /* ��ȡ���ڵ��ȫ·�� */
    ulLen = CFG_MAX_PATH_LEN;
    ret = CFG_GetFullPath(pstNode, acPath, &ulLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* ��ȡ�������ϵı��ڵ�ָ���б� */
    ret = CFG_GetWildMIDNodesPtr(acPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    /* ������ӻ����������� */
    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;
        for (j = 0; NULL != papcAttrList[j][0]; j++)
        {
            if (NULL != papcAttrList[j][1])
            {
                CFG_SETATTR(apstTree[ulLoop], papcAttrList[j][0],
                            papcAttrList[j][1]);
            }
        }
    }

    return CFG_OK;
}


#define CFG_TREE_EX_ATTR_OPT(pstNode, pfnSetOpt, pvData) \
{ \
    ucHasAttr = 0; \
    for (i = 0; NULL != pvData[i][0]; i++) \
    { \
        pvData[i][1] = NULL; \
        pstAttr = scew_attribute_by_name(pstNode, pvData[i][0]); \
        if (NULL != pstAttr) \
        { \
            pvData[i][1] = (char *)scew_attribute_value(pstAttr); \
            ucHasAttr = 1; \
        } \
    } \
    if (0 != ucHasAttr) \
    { \
        ret = pfnSetOpt(pstNode, pvData); \
        if (CFG_OK != ret) \
        { \
            return ret; \
        } \
    } \
}


/***********************************************************
�ӿ�:   ���ڵ���������б�ı�������
����:   pstNode, �߼����ϵĽڵ��ַ
        pfnSetOpt, �����еĲ�������
        pvData, �����б�
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_TreeExAttrAccess(const scew_element *pstNode,
                          FUNC_SET_OPT pfnSetOpt, void *pvData)
{
    const scew_element *pstNodeWork = pstNode;
    const scew_element *pstChild = NULL;
    const scew_element *pstParent = NULL;
    CFG_RET ret = CFG_OK;
    const scew_element *apstStack[CFG_ACCESS_STACK_DEEPTH];
    unsigned char ucStackTop = 0;
    scew_attribute *pstAttr =NULL;
    char *(*papcAttrList)[2] = pvData;
    unsigned long i = 0;
    unsigned char ucHasAttr = 0;

    apstStack[0] = pstNode;
    ucStackTop++;

    /* ���� */
    CFG_TREE_EX_ATTR_OPT(pstNodeWork, pfnSetOpt, papcAttrList);

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

                /* ���� */
                CFG_TREE_EX_ATTR_OPT(pstNodeWork, pfnSetOpt, papcAttrList);
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

            /* ���� */
            CFG_TREE_EX_ATTR_OPT(pstNodeWork, pfnSetOpt, papcAttrList);
        }
    }

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ���߼���������ΪNeedSave=0�Ľڵ���䵽��������
����:   ��
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_FillDataTreeExNodes(void)
{
    const scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    pstNode = scew_tree_root(CFG_GetMIDTree());

    ret = CFG_TreeExNodeAccess(pstNode, CFG_FillSingleNode, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}

/***********************************************************
�ӿ�:   ���߼�����һЩ�����б���䵽��������
����:   ��
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_FillDataTreeExAttrs(void)
{
    const scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    char *aapcExAttrList[][2] = {{ATTR_MAX_NODE, NULL},
                                 {ATTR_CUR_NODE, NULL},
                                 {ATTR_STAT, NULL},
                                 {ATTR_CACHE, NULL},
                                 {NULL, NULL}};

    pstNode = scew_tree_root(CFG_GetMIDTree());

    ret = CFG_TreeExAttrAccess(pstNode, CFG_FillSingleAttr, aapcExAttrList);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}



/***********************************************************
�ӿ�:   ����ͨ�������Ϣ��ȡһ���ڵ��Ӧ��ͨ���index
����:   pstNode, �ڵ��ַ
        pstInfo, ͨ����Ϣ, �����ŵ��ǵڼ����ڵ��Ƕ�Ӧͨ��·���ϵ�{i}
        pulIndex, ���ȡ�õ�index�б�
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetNodeIndexList(const scew_element *pstNode,
                             const ST_CFG_MATCH_TREE_INFO *pstInfo,
                             unsigned long *pulIndex)
{
    unsigned char ucTotalLevel = pstInfo->ucTotalLevel;
    unsigned char ucLevelCount = pstInfo->ucLevelCount;
    unsigned long ulIndex = 0;

    do
    {
        if (ucTotalLevel == pstInfo->aucLevelList[ucLevelCount - 1])
        {
            ulIndex = (unsigned long)strtol(scew_element_name(pstNode), NULL, 10);
            pulIndex[ucLevelCount - 1] = ulIndex;
            if (0 == ucLevelCount)
            {
                break;
            }
            ucLevelCount--;
        }

        pstNode = pstNode->parent;
        ucTotalLevel--;

    } while (NULL != pstNode);

    return CFG_OK;
}


/* ��ȡһ��ͨ��·���µ�����ʵ���Ľṹ���� */
/***********************************************************
�ӿ�:   ��ȡһ��ͨ��·���µ�����ʵ���Ľṹ����
����:   pcMatchPath, ͨ��·��
        ppcLeafList, Ҷ�ӽڵ��б�
        ulLeafNum, Ҷ�ӽڵ����
        ppstInstList, ����ṹ����
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetPathInstList(const char *pcMatchPath, char * const *ppcLeafList,
                           unsigned long ulLeafNum,
                           ST_CFG_INST_NODE **ppstInstList)
{
    CFG_RET ret = CFG_OK;
    const char *pcPath = pcMatchPath;
    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;
    ST_CFG_INST_NODE *apstInstList[2] = {NULL, NULL};
    ST_CFG_INST_NODE *pstInst = NULL;
    ST_CFG_MATCH_TREE_INFO stInfo;

    ret = CFG_GetWildMIDNodesPtr(pcPath, apstTree, CFG_MAX_WILDCARD_NUM,
                                 &ulHead, &ulTail, &stInfo);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    ulLoopEnd = (ulTail >= ulHead ) ? ulTail : ulTail + CFG_MAX_WILDCARD_NUM;
    for (i = ulHead; i < ulLoopEnd; i++)
    {
        ulLoop = i % CFG_MAX_WILDCARD_NUM;

        /* �����ڴ�, �������� */
        pstInst = malloc(sizeof(ST_CFG_INST_NODE) + ulLeafNum * sizeof(long));
        if (NULL == pstInst)
        {
            (void)CFG_MemFree(apstInstList[0]);

            CFG_ERR(ERR_CFG_MALLOC_FAIL);
            return ERR_CFG_MALLOC_FAIL;
        }

        if (NULL == apstInstList[0])
        {
            apstInstList[0] = pstInst;
        }
        else
        {
            apstInstList[1]->pstNext = pstInst;
        }
        apstInstList[1] = pstInst;
        pstInst->pstNext = NULL;

        /* ��ȡindex�б� */
        (void)CFG_GetNodeIndexList(apstTree[ulLoop], &stInfo,
                                   pstInst->aulIndexList);

        /* ��ȡ�ṹ�� */
        ret = CFG_GetValToStructByPtr(apstTree[ulLoop], pstInst->acData,
                                      ppcLeafList, ulLeafNum, 0);
        if (CFG_OK != ret)
        {
            (void)CFG_MemFree(apstInstList[0]);

            CFG_ERR(ret, "%s", pcMatchPath);
            return ret;
        }
    }

    *ppstInstList = apstInstList[0];

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ͨ��ͨ��·������index�б��ȡ�ڵ��ַ
����:   pvTree, ��ָ��,
        pcMathPath, ͨ��·��,
        pulIndexList, index�б�
        ppvNode, ����Ľڵ��ַ,
        pstNodeInfo, ����ڵ���Ϣ
        ulMask, Ҫ��ȡ�Ľڵ���Ϣ������
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:
***********************************************************/
CFG_RET CFG_GetNodeByIndex(const void *pvTree, char *pcMathPath,
                    const unsigned long *pulIndexList, void **ppvNode,
                    ST_CFG_NODE_INFO *pstNodeInfo, unsigned long ulMask)
{
    char *pszTmp = NULL;
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    char acName[32];
    unsigned char ucLevel = 0;

    if (NULL == pvTree)
    {
        pvTree = g_pstCfgTree;
    }

    pszTmp = CFG_StrChrTok(pcMathPath, DELIMIT_C, &pcMathPath);

    pstNode = scew_tree_root(pvTree);
    if (NULL != pszTmp)
    {
        /* ������ṩ�ڵ�·��, �����ڵ��Ƿ�ƥ�� */
        ret = strcmp(scew_element_name(pstNode), pszTmp);
        if (0 != ret)
        {
            /* �ڵ㲻���� */
            CFG_ERR(ERR_CFG_PATH_NOT_EXSITED, "%s", pszTmp);
            return ERR_CFG_PATH_NOT_EXSITED;
        }
        CFG_InitNodeInfo(pstNodeInfo, ulMask);
        CFG_UpdateNodeInfo(pstNode, pstNodeInfo, ulMask);

        while (pszTmp = CFG_StrChrTok(pcMathPath, DELIMIT_C, &pcMathPath),
               NULL != pszTmp)
        {
            if (0 == strcmp(pszTmp, NODE_WILDCARD_EXP))
            {
                sprintf(acName, "%lu", pulIndexList[ucLevel++]);
                pszTmp = acName;
            }

            pstNode = scew_element_by_name(pstNode, pszTmp);
            if (NULL == pstNode)
            {
                /* �ڵ㲻���� */
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




/***********************************************************
�ӿ�:   ͨ��ͨ��·����index�б��ȡһ��������Ҷ�ӽڵ�ֵ��һ���ṹ����
����:   pcMatchPath, �ڵ�ͨ��·��
        pulIndexList, index�б�
        pvStruct, ����ֵ����Դ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0, �ɹ�
        ����, ʧ��
��ע:   �ú�����CFG_GetValToStructEx��һ����չ
***********************************************************/
CFG_RET CFG_GetValToStructEx(const char *pcMatchPath,
                           const unsigned long *pulIndexList,
                           void *pvStruct, char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;

    /* ����·�� */
    CFG_DUP_AND_SEARCH_NODE_BY_INDEX(pcMatchPath,
                 pulIndexList, pstNode, NULL, 0);

    ret = CFG_GetValToStructByPtr(pstNode, pvStruct, ppcLeafList, ulLeafNum, 0);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcMatchPath);
    }

    return ret;
}


/***********************************************************
�ӿ�:   ͨ���ڵ�ͨ��·������һ���ڵ��б�ֵ, ֵ��Դ��һ���ṹ��
����:   pcMatchPath, �ڵ�ͨ��·��
        pulIndexList, index�б�
        pvStruct, ����ֵ����Դ
        ppcLeafList, �ڵ��б�, �ڵ���Ϊ"" �Ľ�������
        ulLeafNum, �ڵ����, ʵ������ָ ppcLeafList �е�Ԫ�ظ���
����ֵ: 0:�ɹ�
        ����: ʧ��
��ע:
***********************************************************/
CFG_RET CFG_SetValFromStructEx(const char *pcMatchPath,
                           const unsigned long *pulIndexList,
                           const void *pvStruct, char * const *ppcLeafList,
                           unsigned long ulLeafNum)
{
    scew_element *pstNode = NULL;
    CFG_RET ret = CFG_OK;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD | CFG_NODE_INFO_NOTI
                     | CFG_NODE_INFO_MWNOTI_ITMS | CFG_NODE_INFO_MWNOTI_MW;

    /* ����·�� */
    CFG_DUP_AND_SEARCH_NODE_BY_INDEX(pcMatchPath,
                 pulIndexList, pstNode, &stNodeInfo, ulMask);

    ret = CFG_SetValFromStructByPtr(pstNode, pvStruct,
                 ppcLeafList, ulLeafNum, &stNodeInfo);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%s", pcMatchPath);
    }

    return ret;
}

/***********************************************************
�ӿ�:   ��ȡͨ��·����Ҷ�ӽڵ��б�
����:   pcMatchPath: �ڵ�ͨ��·��,
        ucNextLevel: �ο�Tr069
        ppstParaInfo: ����ڵ���Ϣ����ͷ
        pcAccessor: ������
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_GetMatchNodeName(const char *pcMatchPath,
                             unsigned char ucNextLevel,
                             ST_PARA_VAL **ppstParaInfo, const char *pcAccessor)
{
    CFG_RET ret = CFG_OK;

    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;
    ST_PARA_VAL *apstParaValList[2] = {NULL, NULL};
    char acFullPath[CFG_MAX_PATH_LEN];
    unsigned long ulLen = 0;

    const char *pcPath = pcMatchPath;

    if (NULL == pcPath)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    g_pcCfgAccessor = pcAccessor;

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

        ulLen = CFG_MAX_PATH_LEN;
        ret = CFG_GetFullPath(apstTree[ulLoop], acFullPath, &ulLen);
        if (CFG_OK != ret)
        {
            (void)CFG_MemFree((void *)apstParaValList[0]);

            CFG_ERR(ret);
            return ret;
        }

        /* pcPath ��'.' ��β */
        if (TREE_FLAG_C == pcPath[strlen(pcPath) - 1])
        {
            if (0 == ucNextLevel)
            {
                /* ��ȫ���� */
                ret = CFG_TreeGetAccess(apstTree[ulLoop], CFG_GetSingleNodeName,
                                acFullPath, apstParaValList);
            }
            else
            {
                /* ������һ���ӽڵ� */
                ret = CFG_TreeGetNextLevelAccess(apstTree[ulLoop],
                           CFG_GetSingleNodeName, acFullPath, apstParaValList);
            }
        }
        else
        {
            ret = CFG_GetSingleNodeName(apstTree[ulLoop],
                            acFullPath, strlen(acFullPath), apstParaValList);
        }

        if (CFG_OK != ret)
        {
            (void)CFG_MemFree(apstParaValList[0]);

            CFG_ERR(ret);
            return ret;
        }
    }

    *ppstParaInfo = apstParaValList[0];

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ��ȡ�ڵ�����ֵָ��
����:   pcPath, �ڵ�·��
        pcAttr, ������
        ppcVal, ���ֵ��ָ��
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_GetNodeAttrValPtr(const char *pcPath, const char *pcAttr,
                              const char **ppcVal)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = "";

    if (NULL == pcPath || NULL == pcAttr || NULL == ppcVal)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    pstAttr = scew_attribute_by_name(pstNode, pcAttr);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
    }

    *ppcVal = pcAttrVal;
    return CFG_OK;
}


/***********************************************************
�ӿ�:   �Խڵ�ֵ��������ָ����ֵ, ֻֻ��int��ڵ�������
����:   pcPath, �ڵ�·��
        lVal, Ҫ���ӵ�ֵ
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_IncIntNodeVal(const char *pcPath, long lVal)
{
    const char *pcPos = NULL;
    char acPath[CFG_MAX_PATH_LEN] = {0};
    const char *pcLeaf = NULL;
    CFG_RET ret = CFG_OK;
    long lValOld = 0;
    const char *pcType = NULL;

    /* �ȶ����ͽ����ж� */
    ret = CFG_GetNodeAttrValPtr(pcPath, ATTR_TYPE, &pcType);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    if (0 != strncmp(pcType, "int", 3) && 0 != strncmp(pcType, "unsignedInt", 11))
    {
        CFG_ERR(ERR_CFG_INVALID_TYPE);
        return ERR_CFG_INVALID_TYPE;
    }

    /* ȡ���ϼ�·�� */
    pcPos = pcPath + strlen(pcPath) - 2;
    while (DELIMIT_C != *pcPos)
    {
        pcPos--;
    }
    pcLeaf = pcPos + 1;

    memcpy(acPath, pcPath, (unsigned long)(pcPos - pcPath));
    ret = CFG_GetValToStruct(acPath, &lValOld, (char * const *)&pcLeaf, 1);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    lValOld += lVal;
    ret = CFG_SetValFromStruct(acPath, &lValOld, (char * const *)&pcLeaf, 1);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    return CFG_OK;
}



/***********************************************************
�ӿ�:   ���ݽڵ�˳���ȡ�ڵ�����
����:   pcPath, �ڵ�·��
        ulOrder, �ڵ�˳��, ��1 ��ʼ
        ppstPara, ����ڵ��ȫ·��
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_GetNodeOrderName(const char *pcPath, unsigned long ulOrder,
                             ST_PARA_VAL **ppstPara)
{
    scew_element *pstNode = NULL;
    scew_element *pstChild = NULL;
    CFG_RET ret = CFG_OK;
    unsigned long i = ulOrder;
    char acAbsPath[CFG_MAX_PATH_LEN];
    unsigned long ulBufLen = 0;
    ST_PARA_VAL *apstParaList[2] = {NULL, NULL};

    if (NULL == pcPath || 0 == i || NULL == ppstPara)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    if ('\0' == pcPath[0] || '.' == pcPath[0])
    {
        ulBufLen = (unsigned long)snprintf(acAbsPath, CFG_MAX_PATH_LEN,
                                   "%s.", scew_element_name(pstNode));
    }
    else
    {
        ulBufLen = strlen(pcPath);
        if (TREE_FLAG_C == pcPath[ulBufLen - 1])
        {
            strcpy(acAbsPath, pcPath);
        }
        else
        {
            ulBufLen = (unsigned long)snprintf(acAbsPath, CFG_MAX_PATH_LEN, "%s.", pcPath);
        }
    }

    if (CFG_LAST_NODE == i)
    {
        /* ȡ���һ�� */
        pstChild = pstNode->last_child;
    }
    else
    {
        while (pstChild = scew_element_next(pstNode, pstChild),
               NULL != pstChild)
        {
            i--;
            if (0 == i)
            {
                break;
            }
        }
    }
    if (NULL == pstChild)
    {
        CFG_ERR(ERR_CFG_PATH_NOT_EXSITED);
        return ERR_CFG_PATH_NOT_EXSITED;
    }

    strcat(acAbsPath, scew_element_name(pstChild));
    ulBufLen += strlen(scew_element_name(pstChild));

    ret = CFG_GetSingleNodeName(pstChild, acAbsPath, ulBufLen, apstParaList);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    *ppstPara = apstParaList[0];
    return CFG_OK;
}



/***********************************************************
�ӿ�:   ���ͨ��·������ڵ��µ�ʵ��
����:   pcMatchPath, ͨ��·��
        ucRestIdx, ��ʾ�Ƿ�����ֵ��λ
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_ClearObjInstEx(const char *pcMatchPath, unsigned char ucRestIdx)
{
    CFG_RET ret = CFG_OK;
    const char *pcPath = pcMatchPath;
    scew_element *apstTree[CFG_MAX_WILDCARD_NUM];
    unsigned long ulHead = 0;
    unsigned long ulTail = 0;
    unsigned long i = 0;
    unsigned long ulLoopEnd = 0;
    unsigned long ulLoop = 0;

    scew_element *pstChild = NULL;
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

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

        while (pstChild = scew_element_next(apstTree[ulLoop], NULL),
               NULL != pstChild)
        {
            scew_element_del(pstChild);
        }

        (void)CFG_UpdateObjCurInstCount(apstTree[ulLoop]);

        if (1 == ucRestIdx)
        {
            CFG_SETATTR(apstTree[ulLoop], "MaxIdx", "0");
        }
    }

    return CFG_OK;
}



#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif





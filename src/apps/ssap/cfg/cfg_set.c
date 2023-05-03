/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_set.c
 �ļ����� : cfg ģ����set��ع��ܵ�ʵ��
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2007-11-23
            ���� :
**********************************************************************/

#include <sys/time.h>
#include "cfg_prv.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


/* ������Ի�ȡ֪ͨ�Ľӿ� */
STATIC FUNC_NOTI_INFORM s_pfnCfgNotiInform = NULL;



/*************************************************************************
Function:      CFG_RET CFG_ClearSingleNoti(const scew_element *pstNode, void *pvPara)
Description:   ��������ڵ��֪ͨ����
Calls:
Data Accessed:
Data Updated:
Input:         pstNode, Ҫ����������,
               pvPara, ������
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_ClearSingleNoti(const scew_element *pstNode, void *pvPara)
{
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;

    (void)pvPara;

    /* �������ͽ������� */
    CFG_FILTER_NON_LEAF(pstNode);

    /* ȡ�ڵ�֪ͨ��־���� */
    pstAttr = scew_attribute_by_name(pstNode, ATTR_NEED_NOTI);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
    }
    else
    {
        return CFG_OK;
    }

    if (NEEDNOTI_C == pcAttrVal[0])   /* ֪ͨ��־Ϊ�� */
    {
        /* ���ڵ� neednoti ������Ϊ��  */
        pcAttrVal = scew_attribute_set_value(pstAttr, NONEEDNOTI);
    }

    return CFG_OK;
}



/*************************************************************************
Function:      CFG_RET CFG_SetSingleNodeAttr(const scew_element *pstNode,
                                                        void *pvPara)
Description:   �Ե����ڵ��������ԵĲ����ĺ���
Calls:
Data Accessed:
Data Updated:
Input:         pstNode, Ҫ����������,
               pvPara, ��������, ʵ�ʴ���������������б�
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_SetSingleNodeAttr(const scew_element *pstNode, void *pvPara)
{
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;
    char *(*papcAttrVal)[2] = pvPara;
    unsigned long i = 0;

    while (NULL != papcAttrVal[i][0])
    {
        if (0 == strcmp(papcAttrVal[i][0], ATTR_MWNOTI))
        {
            char acMwNeedNoti[5] = "00";
            pstAttr = scew_attribute_by_name(pstNode, ATTR_MWNOTI);
            if (NULL != pstAttr)
            {
                pcAttrVal = scew_attribute_value(pstAttr);
                strncpy(acMwNeedNoti, pcAttrVal, 2);
            }
            if ('F' != papcAttrVal[i][1][0])
            {
                acMwNeedNoti[0] = papcAttrVal[i][1][0];
            }
            if ('F' != papcAttrVal[i][1][1])
            {
                acMwNeedNoti[1] = papcAttrVal[i][1][1];
            }

            CFG_SETATTR(pstNode, papcAttrVal[i][0], acMwNeedNoti);
        }
        else
        {
            CFG_SETATTR(pstNode, papcAttrVal[i][0], papcAttrVal[i][1]);
        }
        i++;
    }

    return CFG_OK;
}

/*************************************************************************
Function:      CFG_RET CFG_FilterNodeDonotSetActiveNoti(const scew_element *pstNode,
                                                  void *pvPara)
Description:   ���˲�������Ϊ�����ϱ��ĵĽڵ�
Calls:
Data Accessed:
Data Updated:
Input:         pstNode, Ҫ����������,
               pvPara, ��������, ʵ�ʴ���������������б�
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/

CFG_RET CFG_FilterNodeDonotSetActiveNoti(const scew_element *pstNode, void *pvPara)
{
     int idx , i = 0;
     char *(*papcAttrVal)[2] = pvPara;
     char * pcFilterNode[]={"ManufacturerOUI","SerialNumber", "ProductClass"};

     for(idx = 0 ; idx < sizeof(pcFilterNode)/sizeof(pcFilterNode[0]); idx++)
    {
           if(0==strcmp(pcFilterNode[idx], pstNode->name))
	    {
                 while (NULL != papcAttrVal[i][0])
                 {
                      if (0 == strcmp(papcAttrVal[i][0], ATTR_NOTI))
                      {
                              if (ACTIVE_NOTI_C == papcAttrVal[i][1][0])
                              {
                                     //CFG_ERR(ERR_CFG_REJECT_ATTR_SET);
                                     return ERR_CFG_REJECT_ATTR_SET;
                              }
                       }
                       i++;
                  }
            }
     }
     return CFG_OK;
}


/*************************************************************************
Function:      CFG_RET CFG_CheckSingleNodeAttrSet(const scew_element *pstNode,
                                                  void *pvPara)
Description:   �Ե����ڵ��������ԵĽ��м��ĺ���
Calls:
Data Accessed:
Data Updated:
Input:         pstNode, Ҫ����������,
               pvPara, ��������, ʵ�ʴ���������������б�
Output:        ��
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_CheckSingleNodeAttrSet(const scew_element *pstNode, void *pvPara)
{
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;
    char *(*papcAttrVal)[2] = pvPara;
    unsigned long i = 0;
    CFG_RET ret = CFG_OK;
    /* �������ͽ������� */
    CFG_FILTER_NON_LEAF(pstNode);

    /* ���˲�������Ϊ�����ϱ��ĵĽڵ�  */
    ret = CFG_FilterNodeDonotSetActiveNoti(pstNode, pvPara);
    if(ret != CFG_OK)
    {
          return ret;
    }

    pstAttr = scew_attribute_by_name(pstNode, ATTR_STAT);
    if (NULL != pstAttr)
    {
        pcAttrVal = scew_attribute_value(pstAttr);
        if (ISSTAT_C == pcAttrVal[0])
        {
            while (NULL != papcAttrVal[i][0])
            {
                if (0 == strcmp(papcAttrVal[i][0], ATTR_NOTI)
                    || 0 == strcmp(papcAttrVal[i][0], ATTR_MWNOTI))
                {
                    if (ACTIVE_NOTI_C == papcAttrVal[i][1][0]
                        || PASSIVE_NOTI_C == papcAttrVal[i][1][0]

                        || ACTIVE_NOTI_C == papcAttrVal[i][1][1]
                        || PASSIVE_NOTI_C == papcAttrVal[i][1][1])
                    {
                        CFG_ERR(ERR_CFG_ATTR_STAT);

                        return ERR_CFG_ATTR_STAT;
                    }
                }

                i++;
            }
        }
    }

    return CFG_OK;
}




/*************************************************************************
Function:      CFG_RET CFG_ParseAttrVal(char *pcVal, void **pvOut)
Description:   ���������б�, �� xxx="abc def"�����ĸ�ʽ������ָ���
               (������, ����ֵ),
Calls:
Data Accessed:
Data Updated:
Input:         pcVal, �����б�
Output:        pvOut, ����ָ���
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_ParseAttrVal(char *pcVal, void **pvOut)
{
    enum
    {
        NONE= 1,
        NAME,
        MIDDLE,
        VAL,
        VAL_END
    };

    unsigned long ulCount = 1;
    char *pcValWork = pcVal;
    char *(*papcAttrList)[2] = NULL;
    unsigned char ucState = NONE;

    while (0 != *pcValWork)
    {
        if ('=' == *pcValWork)
        {
            ulCount++;
        }
        pcValWork++;
    }
    pcValWork = pcVal;

    papcAttrList = malloc(2 * sizeof(char *) * ulCount);
    if (NULL == papcAttrList)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    ulCount = 0;

    while ('\0' != *pcValWork)
    {
        switch (*pcValWork)
        {
            case '=':
                if (NAME == ucState)
                {
                    ucState = MIDDLE;
                    *pcValWork = '\0';
                }
                break;
            case ' ':
                if (VAL_END == ucState)
                {
                    ucState = NONE;
                }
                break;
            case '\"':
                if (MIDDLE == ucState)
                {
                    ucState = VAL;
                    papcAttrList[ulCount][1] = pcValWork + 1;
                    ulCount++;
                }
                else if (VAL == ucState)
                {
                    ucState = VAL_END;
                    *pcValWork = '\0';
                }
                break;
            default:
                switch (ucState)
                {
                    case NONE:
                        ucState = NAME;
                        papcAttrList[ulCount][0] = pcValWork;
                        break;
                    default:
                        break;
                }
                break;
        }

        pcValWork++;
    }

    papcAttrList[ulCount][0] = NULL;
    papcAttrList[ulCount][1] = NULL;

    *pvOut = papcAttrList;

    return CFG_OK;
}






/* ע��֪ͨ����Ľӿ� */
/*************************************************************************
Function:      CFG_RET CFG_RegNotiInformIntf(FUNC_NOTI_INFORM pfnNotiInform)
Description:   ע��֪ͨ����Ľӿ�
Calls:
Data Accessed:
Data Updated:
Input:         pfnNotiInform, ����ָ��
Output:
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_RegNotiInformIntf(FUNC_NOTI_INFORM pfnNotiInform)
{
    if (NULL == pfnNotiInform)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    s_pfnCfgNotiInform = pfnNotiInform;
    return CFG_OK;
}

#ifdef _TBS_CFG_CTMW
CFG_RET CFG_SendMwNoti(unsigned long ulNotiType, unsigned long ulDst,
                       const char *pcPath, unsigned long ulPathLen,
                       const char *pcValue, unsigned long ulValueLen
)
{
    ST_MSG *pstMsg = NULL;
    unsigned long ulLen = 3 * sizeof(unsigned long);
    char *pcBody = NULL;
    CFG_RET ret = CFG_OK;

    if (CFG_ACTIVE_NOTI == ulNotiType)
    {
        ulLen += ulPathLen + 1;
        ulLen += ulValueLen + 1;
    }

    pstMsg = MSG_CreateMessage(ulLen);
    if (NULL == pstMsg)
    {
        CFG_ERR(ERR_CREATE_MSG_FAIL, "%lu %lu %s %lu",
                      ulNotiType, ulDst, pcPath, ulLen);
        return ERR_CREATE_MSG_FAIL;
    }

    pstMsg->stMsgHead.usSrcMID = MID_CFG;
    pstMsg->stMsgHead.usDstMID = MID_CTMWBE;
    pstMsg->stMsgHead.ulMsgID = 0;
    pstMsg->stMsgHead.usMsgType = MSG_CTMW_NOTI;
    pcBody = pstMsg->szMsgBody;

    CFG_SET_ULONG(pcBody, 0, ulNotiType);
    CFG_SET_ULONG(pcBody, 1, ulDst);
    if (CFG_ACTIVE_NOTI == ulNotiType)
    {
        CFG_SET_ULONG(pcBody, 2, 1);
        pcBody += sizeof(unsigned long) * 3;
        sprintf(pcBody,"%s=%s", pcPath,pcValue);
    }
    else
    {
        CFG_SET_ULONG(pcBody, 2, 0);
    }

    ret = MSG_SendMessage(pstMsg);
    MSG_ReleaseMessage(pstMsg);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret, "%lu %lu %s %lu", ulNotiType, ulDst, pcPath, ulLen);
    }

    return ret;
}
#endif

/*************************************************************************
Function:      CFG_RET CFG_NotiInform(const scew_element *pstNode, const ST_CFG_NODE_INFO *pstInfo)
Description:   ͳһ����ڵ�ı��Notification�ĺ���
Calls:
Data Accessed:
Data Updated:
Input:         pstNode, �ڵ��ַ
               pstInfo, �ڵ���Ϣ.
Output:
Return:        0,�ɹ�;
               ����, ʧ��
Others:
*************************************************************************/
CFG_RET CFG_NotiInform(const scew_element *pstNode, const ST_CFG_NODE_INFO *pstInfo)
{
    scew_attribute *pstAttr =NULL;
    const char *pcAttrVal = NULL;
    const char *pcLimit = NULL;
    const char *pcLast = NULL;
    unsigned char ucNotiType = pstInfo->ucNoti;
    struct timeval stCurTime = {0, 0};
    unsigned long ulLast = 0;
    unsigned long ulLimit = 0;
    CFG_RET ret = CFG_OK;
    char acLst[32];

#ifndef _CFG_TREE_ALL_STANDARD
    if (0 == pstInfo->ucStandard)
    {
        return CFG_OK;
    }
#endif

    if (CFG_ACTIVE_NOTI == ucNotiType || CFG_PASSIVE_NOTI == ucNotiType)    /* �ڵ�Noti����Ϊ�������߱��� */
    {
        /* ��ȡʱ������ */
        pstAttr = scew_attribute_by_name(pstNode, ATTR_INFORM_LIMIT);
        if (NULL != pstAttr)    /* �ɹ� */
        {
            pcLimit = scew_attribute_value(pstAttr);
            ulLimit = (unsigned long)strtol(pcLimit, NULL, 10);

            /* ��ȡ��ǰʱ�� */
            ret = gettimeofday(&stCurTime, NULL);
            if (CFG_OK != ret)
            {
                stCurTime.tv_sec = 0;
                stCurTime.tv_usec = 0;
            }

            /* ��ȡ�ϴ�֪ͨ��ʱ�� */
            pstAttr = scew_attribute_by_name(pstNode, ATTR_LAST_INFORM);
            if (NULL != pstAttr)    /* �ɹ� */
            {
                pcLast = scew_attribute_value(pstAttr);
                ulLast = (unsigned char)strtol(pcLast, NULL, 10);

                /* ����ʱ��� */
                if (ulLimit > stCurTime.tv_sec - ulLast)  /* ʱ��δ�� */
                {
                    /* ֱ�ӷ��� */
                    return CFG_OK;
                }
            }
        }

        /* �ýڵ� �����־Ϊ �� */
        CFG_SETATTR(pstNode, ATTR_NEED_NOTI, NEEDNOTI);

        /* ����֪ͨ */
        ret = s_pfnCfgNotiInform(ucNotiType);
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
        }

        if (NULL != pcLimit)   /* ��ʱ������ */
        {
            /* �����ϴ�֪ͨʱ��Ϊ��ǰʱ�� */
            sprintf(acLst, "%lu", stCurTime.tv_sec);
            CFG_SETATTR(pstNode, ATTR_LAST_INFORM, acLst);
        }
    }

#ifdef _TBS_CFG_CTMW
    if (0 != pstInfo->ucMwNotiItms || 0 != pstInfo->ucMwNotiMw)
    {
        char acPath[CFG_MAX_PATH_LEN];
        unsigned long ulLen = CFG_MAX_PATH_LEN;
        char acMwNeedNoti[5] = "00";
        const char* pcValue = NULL;

        /* ȡȫ·�� */
        (void)CFG_GetFullPath(pstNode, acPath, &ulLen);
        pcValue = scew_element_contents(pstNode);

        pstAttr = scew_attribute_by_name(pstNode, ATTR_MW_NEED_NOTI);
        if (NULL != pstAttr)
        {
            pcAttrVal = scew_attribute_value(pstAttr);
            strncpy(acMwNeedNoti, pcAttrVal, 2);
        }

        if (0 != pstInfo->ucMwNotiItms)
        {
            /* ������Ϣ */
            (void)CFG_SendMwNoti(pstInfo->ucMwNotiItms, 1, acPath, ulLen , pcValue ,strlen(pcValue));
            if (CFG_PASSIVE_NOTI == pstInfo->ucMwNotiItms) /* Ϊ����֪ͨ */
            {
                /* ��¼��� */
                acMwNeedNoti[0] = '1';
            }
        }
        if (0 != pstInfo->ucMwNotiMw)
        {
            /* ������Ϣ */
            (void)CFG_SendMwNoti(pstInfo->ucMwNotiMw, 2, acPath, ulLen , pcValue ,strlen(pcValue));
            if (CFG_PASSIVE_NOTI == pstInfo->ucMwNotiMw) /* Ϊ����֪ͨ */
            {
                /* ��¼��� */
                acMwNeedNoti[1] = '1';
            }
        }
        if ('0' != acMwNeedNoti[0] && '0' != acMwNeedNoti[1])
        {
            CFG_SETATTR(pstNode, ATTR_MW_NEED_NOTI, acMwNeedNoti);
        }
    }
#endif
    return CFG_OK;
}




/***********************************************************
�ӿ�:   ���ýڵ�����
����:   pcPath: �ڵ�·��
        pcVal: Ҫ���óɵ�ֵ
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_SetNodeVal(const char *pcPath, const char *pcVal, const char *pcAccessor)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    const char *pcValSet = NULL;
    const char *pcValOld = NULL;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD | CFG_NODE_INFO_NOTI
                           | CFG_NODE_INFO_MWNOTI_ITMS | CFG_NODE_INFO_MWNOTI_MW;

//  ulMask |= CFG_NODE_INFO_ACCESSLIST;

    if (NULL == pcPath || NULL == pcVal)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }
    (void)pcAccessor;

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    /* ���ֵһ��, �������� */
    pcValOld = scew_element_contents(pstNode);
    if ((NULL != pcValOld) && (0 == strcmp(pcValOld, pcVal)))
    {
        return CFG_OK;
    }

    /* ���ݷ����߹��� */
//    CFG_SET_RIGHT_CHECK(pstNode, &stNodeInfo, pcAccessor, CFG_WRITABLE_C);

    /* Ϊ�ڵ��������� */
    pcValSet = scew_element_set_contents(pstNode, pcVal);
    if (NULL == pcValSet)
    {
        CFG_ERR(ERR_CFG_INTERNAL_ERR);
        return ERR_CFG_INTERNAL_ERR;
    }

    if (NULL == pcAccessor || '\0' != pcAccessor[0])
    {
        (void)CFG_NotiInform(pstNode, &stNodeInfo);
    }

    return CFG_OK;
}






/***********************************************************
�ӿ�:   ���ýڵ�����
����:   pcPath: �ڵ�·��,
        pstAttVal: ����ֵ�ṹ,
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_SetNodeAttrOpt(const ST_PARA_VAL *pstAttSet, FUNC_SET_OPT pfnOpt)
{
    char *pcAttrVal = NULL;
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    unsigned long ulAttrLen = 0;
    char *(*papcAttrVal)[2] = NULL;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD;

    if (NULL == pstAttSet
        || NULL == pstAttSet->pcName
        || NULL == pstAttSet->pcVal)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    CFG_DUP_AND_SEARCH_NODE(pstAttSet->pcName, pstNode, &stNodeInfo, ulMask);
    CFG_GET_RIGHT_CHECK_BY_INFO(&stNodeInfo, g_pcCfgAccessor);

    ulAttrLen = strlen(pstAttSet->pcVal) + 1;
    pcAttrVal = malloc(ulAttrLen);
    if (NULL == pcAttrVal)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }

    /* ������������ֵ */
    strcpy(pcAttrVal, pstAttSet->pcVal);
    ret = CFG_ParseAttrVal(pcAttrVal, (void **)(void *)&papcAttrVal);
    if (CFG_OK != ret)
    {
        free(pcAttrVal);

        CFG_ERR(ret);
        return ret;
    }

    if (pfnOpt == CFG_SetSingleNodeAttr)
    {
        /* ע��, �˴�Ӧ�����·���ͽڵ������Ƿ��Ӧ */

        /* ֱ�ӵ��ò����ӿ� */
        ret = pfnOpt(pstNode, papcAttrVal);
    }
    else
    {
        /* pcPath ��'.' ��β */
        if (TREE_FLAG_C == pstAttSet->pcName[strlen(pstAttSet->pcName) - 1])
        {
            /* ����ȫ�����ӿ� CFG_TreeSetAccess */
            ret = CFG_TreeSetAccess(pstNode, pfnOpt, papcAttrVal);
        }
        else
        {
            /* ֱ�ӵ��ò����ӿ� */
            ret = pfnOpt(pstNode, papcAttrVal);
        }
    }
    free(papcAttrVal);
    free(pcAttrVal);

    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }
    return ret;
}










/***********************************************************
�ӿ�:   ���ýڵ�����
����:   pcPath: �ڵ�·��,
        pstAttVal: ����ֵ�ṹ,
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_SetNodeAttr(const ST_PARA_VAL *pstAttSet, const char *pcAccessor)
{
    g_pcCfgAccessor = pcAccessor;

    return CFG_SetNodeAttrOpt(pstAttSet, CFG_SetSingleNodeAttr);
}


/***********************************************************
�ӿ�:   ���ýڵ�����
����:   pcPath: �ڵ�·��,
        pstAttVal: ����ֵ�ṹ,
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_CheckNodeAttrSet(const ST_PARA_VAL *pstAttSet, const char *pcAccessor)
{
    g_pcCfgAccessor = pcAccessor;

    return CFG_SetNodeAttrOpt(pstAttSet, CFG_CheckSingleNodeAttrSet);
}



/***********************************************************
�ӿ�:   ���֪ͨ
����:   ��
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_ClearNoti(void)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;

    g_pcCfgAccessor = "";
    pstNode = scew_tree_root(g_pstCfgTree);

    ret = CFG_TreeSetAccess(pstNode, CFG_ClearSingleNoti, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }
    return ret;
}



/***********************************************************
�ӿ�:   �������ڵ�����ת���ȥ
����:   pstNode, �ڵ��ַ
        pvPara, ������
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_SingleNodeTransBack(scew_element *pstNode, void *pvPara)
{
    char acNodeName[64];
    const char *pcNodeName = NULL;

    (void)pvPara;

    pcNodeName = scew_element_name(pstNode);
    if (pcNodeName[1] >= '0' && pcNodeName[1] <= '9'
                           && '_' == pcNodeName[0])
    {
        strcpy(acNodeName, &pcNodeName[1]);
        pcNodeName = scew_element_set_name(pstNode, acNodeName);
        if (NULL == pcNodeName)
        {
            CFG_ERR(ERR_CFG_INTERNAL_ERR);
            return ERR_CFG_INTERNAL_ERR;
        }
    }

    return CFG_OK;
}

/* ���ڵ�����ת���ȥ */
/***********************************************************
�ӿ�:   �������ڵ�����ת���ȥ
����:   pstTree, ������ַ
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_TreeTransBack(scew_tree *pstTree)
{
    CFG_RET ret = CFG_OK;
    scew_element *pstNode = NULL;
    scew_tree *pstTreeWork = pstTree;

    if (NULL == pstTreeWork)
    {
        pstTreeWork = g_pstCfgTree;
    }

    g_pcCfgAccessor = NULL;
    pstNode = scew_tree_root(pstTreeWork);
    ret = CFG_TreeSetAccess(pstNode, (FUNC_SET_OPT)CFG_SingleNodeTransBack, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}







/***********************************************************
�ӿ�:   ���ɾ���ڵ�ĺϷ���
����:   pcPath, �ڵ�·��,
        pcAccessor, ������
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_CheckAndSetNodeDeleting(const char *pcPath, const char *pcAccessor)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;
    const char *pcName = NULL;
    ST_CFG_NODE_INFO stNodeInfo;
    unsigned long ulMask = CFG_NODE_INFO_STANDARD
                         | CFG_NODE_INFO_ACCESSLIST;

    if (NULL == pcPath)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, &stNodeInfo, ulMask);

    /* ���ڵ����Ƿ�Ϊ���� */
    pcName = scew_element_name(pstNode);
    if (!CFG_IS_NODE_NUM(pcName))
    {
        CFG_ERR(ERR_CFG_INVALID_PATH);
        return ERR_CFG_INVALID_PATH;
    }

    /* ȡ���ڵ�, ���Ȩ�� */
    CFG_SET_RIGHT_CHECK(pstNode->parent, &stNodeInfo, pcAccessor, CFG_DELETABLE_C);
    CFG_SET_RIGHT_CHECK(pstNode, &stNodeInfo, pcAccessor, CFG_WRITABLE_C);

//    CFG_SETATTR(pstNode, ATTR_STARTDEL, "1");

    return CFG_OK;
}


/***********************************************************
�ӿ�:   ���ýڵ�����ֵ
����:   pcPath, �ڵ�·��,
        pcAttr, ������
        pcVal, Ҫ���óɵ�����ֵ
����ֵ: 0:�ɹ�
        ����:ʧ��
***********************************************************/
CFG_RET CFG_SetNodeAttrVal(const char *pcPath,
         const char *pcAttr, const char *pcVal)
{
    scew_element *pstNode = NULL;
    long ret = CFG_OK;
    scew_attribute *pstAttr = NULL;
    const char *pcAttrVal = NULL;

    if (NULL == pcPath || NULL == pcAttr || NULL == pcVal)
    {
        CFG_ERR(ERR_CFG_PARA_INVALID);
        return ERR_CFG_PARA_INVALID;
    }

    /* ����·�������ҽڵ� */
    CFG_DUP_AND_SEARCH_NODE(pcPath, pstNode, NULL, 0);

    CFG_SETATTR(pstNode, pcAttr, pcVal);

    return CFG_OK;
}




#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif



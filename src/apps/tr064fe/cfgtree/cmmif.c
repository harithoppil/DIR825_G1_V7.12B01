/* cmmif.c */

#include "cmmif.h"
#include <memory.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "tbstype.h"

#define CALL_CMM_TIMEOUT	5 /* seconds */

static unsigned int g_uMsgId = 0;
static int g_iInit = 0;	/* 0 - uninitialized; 1 - initialized success; 2 - initialized failed */

#ifdef TR64IF_DEBUG
void TR64FE_ShowMessage(ST_MSG* pstMsg);
#endif

/*
*	��������: 	CMM_Init
*	��������: 	CMM�ӿ�ģ���ʼ��������
*	����:		void
*	����ֵ  :	CMM_INIT_NONE - ��δ��ʼ����
*				CMM_INIT_SUCCESS - ��ʼ���ɹ���
*				CMM_INIT_FAILED - ��ʼ��ʧ�ܡ�
*				����TR064������롣
*	����    :	����ʯ / 2009-02-8
*/
int CMM_Init()
{
	MSG_RET iRet;
	if (g_iInit==CMM_INIT_NONE) {
		iRet = MSG_RegModule(MID_TR064FE, NULL);	/* ��CMM_Call()���Լ�������Ϣ�����Բ���Ҫ��Ϣѭ�� */
		if (RET_FAILED(iRet)) {
			g_iInit = CMM_INIT_FAILED;
			TR64FE_TRACE("err: register module TR064FE failed (%d)!\n", iRet);
		} else {
			MSG_AllModStartOK();
			g_iInit = CMM_INIT_SUCCESS;
			TR64FE_TRACE("msg: register module TR064FE successed.\n");
		}
	}
	return g_iInit;
}

/******************************************************************
����: ִ��FactoryReset����
����:
����: CMM_SUCCESS: �ɹ�           ����ֵ: ʧ��
��ע:
******************************************************************/
int TR064_PostFactoryReset()
{
    /*  ������Ϣִ��FactoryReset */
    ST_MSG *pstRecoverMsg = NULL;
   
    pstRecoverMsg = (ST_MSG*)MSG_CreateMessage(0);
    if (NULL == pstRecoverMsg)
    {
         return CMM_FAIL;
    }
    pstRecoverMsg->stMsgHead.usSrcMID     = MID_TR064FE;
    pstRecoverMsg->stMsgHead.usDstMID     = MID_CMM;
    pstRecoverMsg->stMsgHead.usMsgType    = MSG_CMM_RECOVER_CFG;
    pstRecoverMsg->stMsgHead.ulBodyLength = 0;

    if(MSG_OK !=MSG_SendMessage(pstRecoverMsg))
    {
        TR64FE_TRACE("TR064_PostFactoryReset: MSG_SendMessage failed.\n");\
		safe_free_msg(pstRecoverMsg);
        return CMM_FAIL;
    };
	
    safe_free_msg(pstRecoverMsg);
    TR64FE_TRACE ("TR069_PostFactoryReset: request sent\n");
    return CMM_SUCCESS;
}


int TR064_PostReboot()
{
    // TODO: ������Ϣִ��Reboot
    ST_MSG *pstRebootMsg = NULL;

    pstRebootMsg = (ST_MSG*)MSG_CreateMessage(0);
    if (NULL == pstRebootMsg)
    {
         return CMM_FAIL;
    }
    pstRebootMsg->stMsgHead.usSrcMID     = MID_TR064FE;
    pstRebootMsg->stMsgHead.usDstMID     = MID_CMM;
    pstRebootMsg->stMsgHead.usMsgType    = MSG_CMM_REBOOT;
    pstRebootMsg->stMsgHead.ulBodyLength = 0;

    if(MSG_OK !=MSG_SendMessage(pstRebootMsg))
    {
        TR64FE_TRACE("TR064_Reboot: MSG_SendMessage failed.\n");
		safe_free_msg(pstRebootMsg);
        return CMM_FAIL ;
    };

    safe_free_msg(pstRebootMsg);
    TR64FE_TRACE("TR069_PostReboot: request sent\n");
    return CMM_SUCCESS;
}

int TR064_AddStrToLink(ST_TR064STR_LINK **ppHead, char *szStr)
{
    ST_TR064STR_LINK *pObjAdd, *pObjTmp;
	
    pObjAdd = malloc(sizeof(ST_TR064STR_LINK));
    if (pObjAdd != NULL)
    {
        pObjAdd->pstrValue = malloc(strlen(szStr) + 1);
        pObjAdd->pNext = NULL;
        strcpy(pObjAdd->pstrValue, szStr);
    }
	
    if (pObjAdd == NULL || pObjAdd->pstrValue == NULL)
    { 
        TR64FE_TRACE("out of memory");
		return CMM_FAIL ;
    }
    
    if (*ppHead == NULL)
        *ppHead = pObjAdd;
    else
    {
        pObjTmp = *ppHead;
        while (pObjTmp->pNext != NULL) pObjTmp = pObjTmp->pNext;
        pObjTmp->pNext = pObjAdd;
    }
    return CMM_SUCCESS;
}

void TR064_DestroyStrLink(ST_TR064STR_LINK *pHead)
{
    while (pHead != 0)
    {
        ST_TR064STR_LINK *pTmp = pHead;
        pHead = pHead->pNext;
        free(pTmp->pstrValue);
        free(pTmp);
    }
}

/*
*	��������: 	CMM_Free
*	��������: 	CMM�ӿ�ģ���ͷź�����
*	����:		void
*	����ֵ  :	void
*	����    :	����ʯ / 2009-02-8
*/
void CMM_Free()
{
	if (g_iInit==CMM_INIT_SUCCESS) {
		MSG_UnregModule(MID_TR064FE, NULL);
		g_iInit = CMM_INIT_NONE;
		TR64FE_TRACE("msg: unregistered module tr064FE.\n");
	}
}

/*
*	��������: 	CMM_CreateMessage
*	��������: 	CMM��Ϣ����������
*	����:		usSrcMID - [in] Դģ��ID
*				usDstMID - [in] Ŀ��ģ��ID
*				usMsgType - [in] ��Ϣ����
*				ulBodyLength - [in] ��Ϣ�峤�ȡ�
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateMessage(
	unsigned short usSrcMID, unsigned short usDstMID,
	unsigned short usMsgType, unsigned long ulBodyLength)
{
	ST_MSG* pMsg = NULL;
	pMsg = MSG_CreateMessage(ulBodyLength);
	if (!pMsg) {
		TR64FE_TRACE("err: %s() failed - MSG_CreateMessage() returned NULL(SrcMID=%04x,DstMID=%04x,MsgType=%04x,BodySize=%08x)!\n",
			__func__, usSrcMID, usDstMID, usMsgType, (unsigned int)ulBodyLength);
		return NULL;
	}
	pMsg->stMsgHead.usSrcMID = usSrcMID;
	pMsg->stMsgHead.usDstMID = usDstMID;
	pMsg->stMsgHead.ulMsgID = ++g_uMsgId;
	pMsg->stMsgHead.usMsgType = usMsgType;
	pMsg->stMsgHead.ulBodyLength = ulBodyLength;
	return pMsg;
}

/*
*	��������: 	CMM_CreateGetValMessage
*	��������: 	����MSG_CMM_GET_VAL��Ϣ��
*	����:		szLeafName - [in] Ҫ��ȡ��ֵ��TR069Ҷ�ӽڵ�ȫ·�����ơ�
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateGetValMessage(const char* szLeafName)
{
	unsigned long ulBodySize = (sizeof(unsigned long)+strlen(szLeafName)+1);
	ST_MSG* pMsg = CMM_CreateMessage(
		MID_TR064FE, MID_CMM,
		MSG_CMM_GET_VAL, ulBodySize);

	if (!pMsg) {
		return NULL;
	}

	/* Body */
	*((unsigned long*)pMsg->szMsgBody) = 1;	/* PathCount */
	strcpy(pMsg->szMsgBody+sizeof(unsigned long), szLeafName);		/* Path1 */

	return pMsg;
}

/*
*	��������: 	CMM_CreateSetValMessage
*	��������: 	����MSG_CMM_SET_VAL��Ϣ��
*	����:		szLeafName - [in] Ҫ������ֵ��TR069Ҷ�ӽڵ�ȫ·�����ơ�
*				szValue - [in] Ҫ���õĽڵ�ֵ��
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateSetValMessage(const char* szLeafName, const char* szValue)
{
	unsigned long ulBodySize = (sizeof(unsigned long)+strlen(szLeafName)+strlen(szValue)+2);
	ST_MSG* pMsg = CMM_CreateMessage(
					MID_TR064FE, MID_CMM,
					MSG_CMM_SET_VAL, ulBodySize);
	if (!pMsg) {
		return NULL;
	}

	/* Body */
	*((unsigned long*)pMsg->szMsgBody) = 1;	/* PathCount */
	sprintf(pMsg->szMsgBody+sizeof(unsigned long), "%s=%s", szLeafName, szValue);	/* Path=Value */
	return pMsg;
}

/*
*	��������: 	CMM_CreateGetNameMessage
*	��������: 	����MSG_CMM_GET_NAME��Ϣ��
*	����:		cNodeName - [in] Ҫ��ȡ��ʵ���б��TR069����ڵ�ȫ·�����ơ�
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateGetNameMessage(const char* cNodeName)
{
	unsigned long ulBodySize = (sizeof(unsigned char)+strlen(cNodeName)+1);
	ST_MSG* pMsg = CMM_CreateMessage(
		MID_TR064FE, MID_CMM,
		MSG_CMM_GET_NAME, ulBodySize);
	if (!pMsg) {
		return NULL;
	}

	/* Body */
	pMsg->szMsgBody[0] = 1;	/* NextLevel */
	strcpy(pMsg->szMsgBody+sizeof(unsigned char), cNodeName);		/* Path */

	return pMsg;
}

/*
*	��������: 	CMM_CreateAddNodeMessage
*	��������: 	����MSG_CMM_ADD_NODE��Ϣ��
*	����:		cNodeName - [in] Ҫ����ʵ����TR069����ڵ�ȫ·�����ơ�
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateAddNodeMessage(const char* cNodeName)
{
	unsigned long ulBodySize = strlen(cNodeName)+1;

	ST_MSG* pMsg = CMM_CreateMessage(
		MID_TR064FE, MID_CMM,
		MSG_CMM_ADD_NODE, ulBodySize);
	if (!pMsg) {
		return NULL;
	}
	/* Body */
	strcpy(pMsg->szMsgBody, cNodeName);		/* Path */

	return pMsg;
}

/*
*	��������: 	CMM_CreateDelNodeMessage
*	��������: 	����MSG_CMM_ADD_NODE��Ϣ��
*	����:		cNodeName - [in] Ҫɾ����TR069����ʵ��ȫ·�����ơ�
*	����ֵ  :	�����Ϣ�����ɹ��������´�����Ϣ��ָ�룻
*				���򷵻�NULL��
*	����    :	����ʯ / 2009-02-8
*/
ST_MSG* CMM_CreateDelNodeMessage(const char* cNodeName)
{
	unsigned long ulBodySize = strlen(cNodeName)+1;
	ST_MSG* pMsg = CMM_CreateMessage(
		MID_TR064FE, MID_CMM,
		MSG_CMM_DEL_NODE, ulBodySize);
	if (!pMsg) {
		return NULL;
	}

	/* Body */
	strcpy(pMsg->szMsgBody, cNodeName);		/* Path */

	return pMsg;
}

/*
*	��������: 	CMM_Call
*	��������: 	��CMM����������Ϣ����ȡ��Ӧ��Ӧ����Ϣ��
*	����:		pReq - [in] �Ѵ����õ�������Ϣָ��
*				ppRsp - [out] ��Ϣָ���ָ�룬���ڱ��淵�ص�Ӧ����Ϣ��ָ�롣
*	����ֵ  :	CMM_SUCCESS - �ɹ���*ppRspΪӦ����Ϣָ��
*				���򣬷��س�����룬*ppRspδ���塣
*				CMM_ERR_SEND - ����������Ϣʧ��
*				CMM_ERR_RECV - ����Ӧ��ʧ��
*				CMM_ERR_TIMEOUT - ����Ӧ��ʱ
*				CMM_FAIL - ��������
*	����    :	����ʯ / 2009-02-8
*/
int CMM_Call(ST_MSG* pReq, ST_MSG** ppRsp)
{ 
	MSG_RET iRet;
	time_t t0, t1, t;

	iRet = MSG_SendMessage(pReq);
	if (iRet!=MSG_OK) {
		TR64FE_TRACE("err: send request to CMM failed (%d)!\n", iRet);
		return CMM_ERR_SEND;
	}

#ifdef TR064IF_DEBUG
	
#endif
	t = t0 = time(NULL);
	t1 = t0+CALL_CMM_TIMEOUT;
	for (;;) {
		t = time(NULL);
		if (t>t1) {	/* �ȴ�Ӧ��ʱ */
			TR64FE_TRACE("err: wait CMM response timeout (%d seconds)!\n", CALL_CMM_TIMEOUT);
			return CMM_ERR_TIMEOUT;
		}
      
		iRet = MSG_ReceiveMessage(ppRsp, t1-t);
		if (iRet!=MSG_OK) {
			TR64FE_TRACE("err: receive response from CMM failed (%d)!\n", iRet);
			return CMM_ERR_RECV;
		}

		/*  �յ�һ����Ϣ */
		if (*ppRsp) 
		{
			ST_MSG* pRsp = *ppRsp;
			if (pRsp->stMsgHead.ulMsgID==pReq->stMsgHead.ulMsgID&&
				pRsp->stMsgHead.usDstMID==pReq->stMsgHead.usSrcMID&&
				pRsp->stMsgHead.usSrcMID==pReq->stMsgHead.usDstMID)
			{
				/* �յ���Ӧ�����������Ӧ����Ϣ���˳�ѭ�� */
#ifdef TR064IF_DEBUG
				TR064_ShowMessage(pRsp);
#endif
				break;
			} else 
			{
                MSG_ReleaseMessage(*ppRsp);
			}
		} else {
			TR64FE_TRACE("err: received a null response, why?!\n");
			return CMM_FAIL;
		}
	}
	return CMM_SUCCESS;
}

/*
*	��������: 	CMM_GetStr
*	��������: 	��CMM��ȡһ���ڵ��ֵ��
*	����:		szLeafName - [in] TR069Ҷ�ӽڵ�ȫ·�����ơ�
*				szValue - [out] ���ڱ���TR069�ڵ�ֵ�Ļ�������
*				uSize - [in] szValue��������С��
*	����ֵ  :	CMM_SUCCESS - �ɹ���szValue�б���ָ���ڵ�ֵ
*				����ʧ��szValueδ���塣
*	����    :	����ƽ / 2008-05-29
*/
int CMM_GetStr(const char* szLeafName, char* szValue, unsigned int uSize, char **pv, int pvLen)
{
    //printf(" szLeafName = %s " , szLeafName ) ;
	MSG_RET iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	char* pcItem = NULL;
	char* pc = NULL;
	u_long ulResult = 0;

	pReq = CMM_CreateGetValMessage(szLeafName);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}

	iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}

	ulResult = *((u_long*)pRsp->szMsgBody);
	if (0!=ulResult) {
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: get node value failed (%s result=%lu)!\n",
			szLeafName, ulResult);
		goto EXIT3;
	}

	pcItem = pRsp->szMsgBody+sizeof(unsigned long)+sizeof(unsigned long);	/* Result + Status */
	//TR64FE_TRACE("msg: %s()--->%s\n", __func__, pcItem);
	pc = strtok(pcItem, "=");
	pc = strtok(NULL, "\n\r");
	if (pc) {
		strncpy(szValue, pc, uSize-1);
		szValue[uSize-1] = 0;
	} else {
		*szValue = 0;
	}
	//TR64FE_TRACE("msg: value='%s'\n", szValue);

       /*map the pv and try to find the matched value*/
       if(pv && pvLen > 1) /*at least pvlen is 2*/
       {
            int found = 0;
            int i = 0;
            char *p = pv[i];
            while(1)
            {
                if(strcasecmp(szValue, pv[i]) == 0) /*compare name: value read from CMM*/
                {
                    found = 1;
                    strcpy(szValue, pv[i+1]); /*retrieve the value: value transfered to SNMP manager*/
                    break;
                }

                i += 2;
                if(i >= pvLen) break;
                
                p = pv[i];
            }
            
            if(found == 0) 
            {
                TR64FE_TRACE("Failed to get %s, because being unmathced!\n", szLeafName);
                //iRet = CMM_FAIL; /*If no item matched the value, return it as successful*/
            }
       }

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

// ����ڵ���Ϣ�Ļ�ȡ
// szPathName �Ե��β 
// ע�Ȿ�������ص��ַ���������Ҫ������ɾ��
ST_TR064STR_LINK* CMM_GetStrs(const char *szPathName, char* aszLeafName[],int nValNum)
{
    ST_TR064STR_LINK *pStrLink = NULL;
	
    if (szPathName == NULL && aszLeafName == NULL)
    {
        TR64FE_TRACE("GetObject Parameter error");
        goto EXIT1;
    }

	int bEndDot = strcmp(rindex(szPathName,'.') ,".") ;

	// �������ȡ����
	int i = 0 ;
	unsigned long ulBodySize = sizeof(unsigned long);
    for( i = 0 ; i < nValNum; i++)
    {
       if(!bEndDot)
        {
            ulBodySize += strlen(szPathName) + strlen(aszLeafName[i]) + 1;
        }else
        {
           ulBodySize += strlen(szPathName) + strlen(aszLeafName[i]) + 2;
        } 
    }
	
	ST_MSG* pReq = CMM_CreateMessage(MID_TR064FE, MID_CMM,MSG_CMM_GET_VAL, ulBodySize);
	if (!pReq) {
		TR64FE_TRACE("Out of Memory");
        goto EXIT1;
	}
  
    char  *pGetMsgBody, *pPosCur;
    pPosCur = pReq->szMsgBody;

    // �������
    *(unsigned long *)pPosCur = nValNum;
    pPosCur += sizeof(unsigned long);

    // �������ȡ����
    int n = 0 ;
    for (i = 0; i < nValNum-1; i++)
    {
        if(!bEndDot)
        {
            pPosCur += sprintf(pPosCur, "%s%s", szPathName, aszLeafName[i]) + 1;
        }else
        {
           pPosCur += sprintf(pPosCur, "%s.%s", szPathName, aszLeafName[i]) + 1;
        } 
    }

    if(!bEndDot)
    {
        pPosCur += sprintf(pPosCur, "%s%s", szPathName, aszLeafName[nValNum-1]);
    }else
    {
       pPosCur += sprintf(pPosCur, "%s.%s", szPathName, aszLeafName[nValNum-1]);
    }

	ST_MSG* pRsp = NULL;
	if (CMM_Call(pReq, &pRsp)!=CMM_SUCCESS) {
		goto EXIT2;
	}

    pGetMsgBody = pPosCur = pRsp->szMsgBody;
    if (*(unsigned long *)pPosCur != 0)   // ulResult !=0 ��ʾ��ȡʧ��
    {   
        TR64FE_TRACE("msg result is error");
        goto EXIT3;
    }
	
    pPosCur += sizeof(unsigned long);

    int nRow = *(unsigned long *)pPosCur;   // count Num
    pPosCur += sizeof(unsigned long);

    // ���û�����ݣ� ����Ҫ���κ����飬 Ҳ����ʧ��
    if (nRow != nValNum)
    {
		TR64FE_TRACE("Get Value num is unrespected");
        goto EXIT3;
    }

    // ����Ӧ����Ϣ�е�ֵ��ȡ�������浽szValRet;
    for (i = 0; i < nValNum; i++)
    {
        pPosCur = strchr(pPosCur, '=');
        if (*pPosCur == '\0' || pPosCur >= pGetMsgBody + pRsp->stMsgHead.ulBodyLength)
        {
            TR064_DestroyStrLink(pStrLink);
			TR64FE_TRACE("Get too less value");
            goto EXIT3;
        }
        TR064_AddStrToLink(&pStrLink, pPosCur + 1);
        pPosCur += strlen(pPosCur) + 1;
    }
EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
    return pStrLink;
}


// ����ڵ���Ϣ�Ļ�ȡ
// ע�Ȿ�������ص��ַ���������Ҫ������ɾ��
ST_TR064STR_LINK* CMM_FullPathGetStrs(char* aszFullName[],int nValNum)
{
    ST_TR064STR_LINK *pStrLink = NULL;
    if (aszFullName == NULL)
    {
        TR64FE_TRACE("GetObject Parameter error");
        goto EXIT1;
    }
    int i = 0 ;
    unsigned long ulBodySize = sizeof(unsigned long) ;
	// �������ȡ����
    for(; i < nValNum; i++) 
    {
       ulBodySize += strlen(aszFullName[i]) + 1;
    }
	
	ST_MSG* pReq = CMM_CreateMessage(
		MID_TR064FE, MID_CMM,
		MSG_CMM_SET_VAL, ulBodySize);
	if (!pReq) {
		TR64FE_TRACE("Out of Memory");
        goto EXIT1;
	}
  
    char  *pGetMsgBody, *pPosCur;
    pPosCur = pReq->szMsgBody;

    // �������
    *(unsigned long *)pPosCur = nValNum;
    pPosCur += sizeof(unsigned long);

    // �������ȡ����
    for (i = 0; i < nValNum-1; i++)
    {
        pPosCur += sprintf(pPosCur, "%s",aszFullName[i]) + 1;
    }
	
	pPosCur += sprintf(pPosCur, "%s",aszFullName[nValNum-1]);

	ST_MSG* pRsp = NULL;
	if (CMM_Call(pReq, &pRsp)!=CMM_SUCCESS) {
		goto EXIT2;
	}

    pGetMsgBody = pPosCur = pRsp->szMsgBody;
    if (*(unsigned long *)pPosCur != 0)   // ulResult !=0 ��ʾ��ȡʧ��
    {   
        TR64FE_TRACE("msg result is error");
        goto EXIT3;
    }
	
    pPosCur += sizeof(unsigned long);

    int nRow = *(unsigned long *)pPosCur;   // count Num
    pPosCur += sizeof(unsigned long);

    // ���û�����ݣ� ����Ҫ���κ����飬 Ҳ����ʧ��
    if (nRow != nValNum)
    {
		TR64FE_TRACE("Get Value num is unrespected");
        goto EXIT3;
    }

    // ����Ӧ����Ϣ�е�ֵ��ȡ�������浽szValRet;
    for (i = 0; i < nValNum; i++)
    {
        pPosCur = strchr(pPosCur, '=');
        if (*pPosCur == '\0' || pPosCur >= pGetMsgBody + pRsp->stMsgHead.ulBodyLength)
        {
            TR064_DestroyStrLink(pStrLink);
			TR64FE_TRACE("Get too less value");
            goto EXIT3;
        }
        TR064_AddStrToLink(&pStrLink, pPosCur + 1);
        pPosCur += strlen(pPosCur) + 1;
    }
EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
    return pStrLink;
}

/*
*	��������: 	CMM_SetStr
*	��������: 	��CMM����һ���ڵ��ֵ��
*	����:		szLeafName - [in] TR069Ҷ�ӽڵ�ȫ·�����ơ�
*				szValue - [in] Ҫ���õĽڵ�ֵ��
*	����ֵ  :	CMM_SUCCESS - �ɹ�
*				����ʧ�ܡ�
*	����    :	����ʯ / 2009-02-8
*/
int CMM_SetStrs(char* ppcPathName[], char* ppcPathValue[] ,int nValNum)
{
    /*
    printf("  ######### CMM_SetStrs () ##############");
    
    for(; i < nValNum; i++)
    {
       printf(" %s = %s \n " , ppcPathName[i] , ppcPathValue[i]);
	   printf( "%d = %d \n" ,  strlen(ppcPathName[i]), strlen(ppcPathValue[i]));
    }
    */
	
    // �������ȡ����
	int iRet = CMM_SUCCESS;
	int i = 0 ;
	unsigned long ulBodySize = sizeof(unsigned long) ;
    for(i=0; i < nValNum; i++)
    {
       ulBodySize += (strlen(ppcPathName[i])+strlen(ppcPathValue[i])+2);
    }
	
	ST_MSG* pReq = CMM_CreateMessage(MID_TR064FE, MID_CMM,MSG_CMM_SET_VAL, ulBodySize);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}

    char  *pGetMsgBody, *pPosCur;
    pPosCur = pReq->szMsgBody;

    // �������
    *(unsigned long *)pPosCur = nValNum;
    pPosCur += sizeof(unsigned long);

    // �������ȡ����
    for (i = 0; i < nValNum-1; i++)
    {
        pPosCur += sprintf(pPosCur, "%s=%s", ppcPathName[i], ppcPathValue[i]) + 1;
    }

	pPosCur += sprintf(pPosCur, "%s=%s", ppcPathName[nValNum-1], ppcPathValue[nValNum-1]);
	
	ST_MSG* pRsp = NULL;
    iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}

	unsigned long ulResult = *((unsigned long*)pRsp->szMsgBody);
	if (0!=ulResult) {	/* Result */
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: set node value failed (%s result=%lu)!\n",
			ppcPathName[0], ulResult);
		goto EXIT3;
	}
	TR64FE_TRACE("msg: %s()--->%s='%s'\n",__func__, ppcPathName[0], ppcPathValue[0]);

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}


/*
*	��������: 	CMM_SetStr
*	��������: 	��CMM����һ���ڵ��ֵ��
*	����:		szLeafName - [in] TR069Ҷ�ӽڵ�ȫ·�����ơ�
*				szValue - [in] Ҫ���õĽڵ�ֵ��
*	����ֵ  :	CMM_SUCCESS - �ɹ�
*				����ʧ�ܡ�
*	����    :	����ʯ / 2009-02-8
*/
int CMM_SetStr(const char* szLeafName, char* szValue, char **pv, int pvLen)
{
	MSG_RET iRet = MSG_OK;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	unsigned long ulResult = 0;
    char t_szValue[128] = {0};
    strncpy(t_szValue, szValue, strlen(szValue));
    if(pv && pvLen>1) /*pvLen is at least 2*/
    {
        int found = 0;
        int i = 0;
        char *p = pv[i];

        while(1)
        {
            if(strcasecmp(t_szValue, pv[i+1]) == 0) /*compare value: value read from snmp manager*/
            {
                found = 1;
                strcpy(t_szValue, pv[i]); /*retrieve the name: value transfered to CMM*/
                break;
            }

            i += 2;
            if(i >= pvLen) return CMM_FAIL;
            p = pv[i];
        }
        
        if(found == 0) 
        {
            TR64FE_TRACE("Failed to set %s, because being unmathced!\n", szLeafName);
            return CMM_FAIL; /*unvariable value*/
        }
    }

	pReq = CMM_CreateSetValMessage(szLeafName, t_szValue);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}

	iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}

	ulResult = *((unsigned long*)pRsp->szMsgBody);
	if (0!=ulResult) {	/* Result */
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: set node value failed (%s result=%lu)!\n",
			szLeafName, ulResult);
		goto EXIT3;
	}
	TR64FE_TRACE("msg: %s()--->%s='%s'\n",__func__, szLeafName, szValue);

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

int CMM_GetUlong(const char* szLeafName, u_long* puValue, char **pv, int pvLen)
{
	char cValue[32];
	int iRet = CMM_SUCCESS;
	iRet = CMM_GetStr(szLeafName, cValue, sizeof(cValue), pv, pvLen);
	if (iRet!=CMM_SUCCESS) {
		return iRet;
	}
	*puValue = (u_long)atoi(cValue);
	return iRet;
}

int CMM_SetUlong(const char* szLeafName, u_long uValue, char **pv, int pvLen)
{
	char cValue[32];
	sprintf(cValue, "%lu", uValue);
	return CMM_SetStr(szLeafName, cValue, pv, pvLen);
}

/*
*	��������: 	CMM_GetInstanceList
*	��������: 	��ȡ�����ʵ���б�
*	����:		cNodeName - [in] �������ƣ�ȫ·������"."������
*	����ֵ  :	NULL - ʧ��
*				���򣬷���һ�����飬ÿ��Ԫ��Ϊָ��ʵ�����Ƶ��ַ�����
*				������NULLԪ�ؽ�����
*	����    :	����ʯ / 2009-02-8
*/
char** CMM_GetInstanceList(const char* cNodeName)
{
	MSG_RET iRet = MSG_OK;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	char** ppList = NULL;
	unsigned long ulResult = 0;
	unsigned long ulResCount = 0;
	unsigned long i = 0;
	char* pcItem = NULL;
	
	pReq = CMM_CreateGetNameMessage(cNodeName);
	if (!pReq) {
		TR64FE_TRACE("err: CMM_CreateGetNameMessage() failed!\n");
		goto ERROR4;
	}

	iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=MSG_OK) {
		TR64FE_TRACE("err: CMM_Call() failed (%d)!\n", iRet);
		goto ERROR3;
	}

	ulResult = *((unsigned long*)pRsp->szMsgBody);
	if (ulResult!=0) {
		TR64FE_TRACE("err: get name list returned %lu!\n", ulResult);
		goto ERROR2;
	}

	/* ʵ���б�ĸ��� */
	//TR64FE_TRACE("msg: %s()--->%s\n", __func__, cNodeName);

	ulResCount = *((unsigned long*)(pRsp->szMsgBody+sizeof(unsigned long)));
	pcItem = pRsp->szMsgBody+sizeof(unsigned long)+sizeof(unsigned long);
	ppList = (char**)malloc((ulResCount+1)*sizeof(*ppList));
	memset(ppList, 0, sizeof(*ppList)*(ulResCount+1));
	for (i = 0; i<ulResCount; i++) {
		unsigned int len = strlen(pcItem);
		char* pcName = strtok(pcItem, " ");	/* ����ֻȡ���ֲ��� */
		pcItem = pcItem+len+1;	/* ָ����һ�� */
		if (!pcName) {
			TR64FE_TRACE("err: get a null name!\n");
			goto ERROR1;
		}
		len = strlen(pcName);
		ppList[i] = (char*)malloc(len+1);
		if (!ppList[i]) {
			TR64FE_TRACE("err: allocate memory for name failed (len=%u)!\n", len+1);
			goto ERROR1;
		}
		strcpy(ppList[i], pcName);
		//TR64FE_TRACE("msg: Instances[%lu] = %s\n",i, ppList[i]);
	}
	ppList[i] = NULL;	/* ��Ϊ֮ǰ�Ѿ�����memset��������0���������ﲻ�Ǳ���� */
	MSG_ReleaseMessage(pRsp);
	MSG_ReleaseMessage(pReq);
	return ppList;

ERROR1:
	CMM_FreeInstanceList(ppList);
ERROR2:
	MSG_ReleaseMessage(pRsp);
ERROR3:
	MSG_ReleaseMessage(pReq);
ERROR4:
	return NULL;
}

/*
*	��������: 	CMM_FreeInstanceList
*	��������: 	�ͷ���CMM_GetInstanceList()������ʵ���б�
*	����:		ppList - [in] ��CMM_GetInstanceList()������ʵ���б�
*	����ֵ  :	void
*	����    :	����ƽ / 2008-05-29
*/
void CMM_FreeInstanceList(char** ppList)
{
	int i;
	for (i = 0; ppList[i]; i++) {
		free(ppList[i]);
	}
	free(ppList);
	ppList = NULL;
}

/*
*	��������: 	CMM_AddInstance
*	��������: 	Ϊ��������һ��ʵ����
*	����:		cNodeName - [in]  �������ƣ�ȫ·������"."������
*				pulInstanceId - [out] ���ڱ������������ʵ���š�
*	����ֵ  :	CMM_SUCCESS - �ɹ���*pulInstanceIdΪ����ʵ���š�
*				������ʧ�ܣ�*pulInstanceIdδ���塣
*	����    :	����ƽ / 2008-05-29
*/
int CMM_AddInstance(const char* cNodeName, unsigned long* pulInstanceId)
{
	MSG_RET iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	u_long ulResult = 0;

	pReq = CMM_CreateAddNodeMessage(cNodeName);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}
	iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}
	ulResult = *((u_long*)pRsp->szMsgBody);
	if (ulResult!=0) {
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: %s() failed %s = %lu\n", __func__, cNodeName, ulResult);
		goto EXIT3;
	}

	*pulInstanceId = *((u_long*)(pRsp->szMsgBody+sizeof(u_long)));
	TR64FE_TRACE("msg: %s()--->%s new instance is %lu\n", __func__, cNodeName, *pulInstanceId);

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

/*
*	��������: 	CMM_DelInstance
*	��������: 	ɾ��ʵ����
*	����:		cInstanceName - [in]  Ҫɾ����ʵ��ȫ·�����ơ�
*	����ֵ  :	CMM_SUCCESS - �ɹ���*pulInstanceIdΪ����ʵ���š�
*				������ʧ�ܣ�*pulInstanceIdδ���塣
*	����    :	����ƽ / 2008-05-29
*/
int CMM_DelInstance(const char* cInstanceName)
{
	MSG_RET iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	u_long ulResult = 0;

	pReq = CMM_CreateDelNodeMessage(cInstanceName);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}
	iRet = CMM_Call(pReq, &pRsp);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}

	ulResult = *((u_long*)pRsp->szMsgBody);
	if (ulResult!=0) {
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: %s() failed %s = %lu\n", __func__, cInstanceName, ulResult);
		goto EXIT3;
	}

	TR64FE_TRACE("msg: %s()--->%s\n", __func__, cInstanceName);

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

int CMM_SaveConfig()
{
	int iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	u_long ulResult = 0;

	pReq = CMM_CreateMessage(MID_TR064FE, MID_CMM, MSG_CMM_SAVE_CFG, 0);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}
	iRet = CMM_Call(pReq, &pRsp);
	if (CMM_SUCCESS!=iRet) {
		goto EXIT2;
	}
	ulResult = *((u_long*)pRsp->szMsgBody);
	if (0!=ulResult) {
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: %s() failed = %lu\n", __func__, ulResult);
		goto EXIT3;
	}
	TR64FE_TRACE("msg: %s()--->%s\n", __func__, "Success");

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

int CMM_RecoverConfig()
{
	int iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;
	ST_MSG* pRsp = NULL;
	u_long ulResult = 0;

	pReq = CMM_CreateMessage(MID_TR064FE, MID_CMM, MSG_CMM_RECOVER_CFG, 0);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}
	iRet = CMM_Call(pReq, &pRsp);
	if (CMM_SUCCESS!=iRet) {
		goto EXIT2;
	}
	ulResult = *((u_long*)pRsp->szMsgBody);
	if (0!=ulResult) {
		iRet = CMM_FAIL;
		TR64FE_TRACE("err: %s() failed = %lu\n", __func__, ulResult);
		goto EXIT3;
	}
	TR64FE_TRACE("msg: %s()--->%s\n", __func__, "Success");

EXIT3:
	MSG_ReleaseMessage(pRsp);
EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

int CMM_Reboot()
{
	int iRet = CMM_SUCCESS;
	ST_MSG* pReq = NULL;

	pReq = CMM_CreateMessage(MID_TR064FE, MID_CMM, MSG_CMM_REBOOT, 0);
	if (!pReq) {
		iRet = CMM_ERR_MEM;
		goto EXIT1;
	}
	iRet = MSG_SendMessage(pReq);
	if (CMM_SUCCESS!=iRet) {
		goto EXIT2;
	}
	TR64FE_TRACE("msg: %s()--->%s\n", __func__, "Success");

EXIT2:
	MSG_ReleaseMessage(pReq);
EXIT1:
	return iRet;
}

/*****************************************************************************
*
* Debug����
*
*****************************************************************************/

#ifdef CMMIF_DEBUG

/*
*	��������: 	MSGTYPE2NAME
*	��������: 	���Ը�������������ֵ��ʽMsgTypeת��Ϊ�ַ����ɶ���ʽ��
*	�������: 	usMsgType - MsgType��ֵ
*	����ֵ  :	��Ӧ��MsgType�ַ������֡��������SNMPC֧�ֵģ�����
*				SNMPC_UNSUPPORTED_MSGTYPE(ddd)��ʽ��
*	����    : 	����ƽ / 2008-04-23
*/
const char* MSGTYPE2NAME(unsigned short usMsgType)
{
	#define MSG_TYPE_NAME(name) case name: return #name

	switch (usMsgType) {
	MSG_TYPE_NAME(MSG_CMM_SET_VAL);
	MSG_TYPE_NAME(MSG_CMM_SET_VAL_ACK);
	MSG_TYPE_NAME(MSG_CMM_GET_VAL);
	MSG_TYPE_NAME(MSG_CMM_GET_VAL_ACK);
	MSG_TYPE_NAME(MSG_CMM_COMMIT);
	MSG_TYPE_NAME(MSG_CMM_CANCEL);
	MSG_TYPE_NAME(MSG_PC_START_PROC);
	MSG_TYPE_NAME(MSG_PC_START_PROC_ACK);
	MSG_TYPE_NAME(MSG_PC_SEND_SIGNAL);
	MSG_TYPE_NAME(MSG_PC_SEND_SIGNAL_ACK);
	MSG_TYPE_NAME(MSG_PC_PROC_STATE);
	MSG_TYPE_NAME(MSG_CMM_UPDATE);
	MSG_TYPE_NAME(MSG_CMM_UPDATE_ACK);
	MSG_TYPE_NAME(MSG_CMM_ADD_NODE);
	MSG_TYPE_NAME(MSG_CMM_ADD_NODE_ACK);
	MSG_TYPE_NAME(MSG_CMM_DEL_NODE);
	MSG_TYPE_NAME(MSG_CMM_DEL_NODE_ACK);
	MSG_TYPE_NAME(MSG_CMM_GET_NAME);
	MSG_TYPE_NAME(MSG_CMM_GET_NAME_ACK);
	MSG_TYPE_NAME(MSG_CMM_SAVE_CFG);
	MSG_TYPE_NAME(MSG_CMM_SAVE_CFG_ACK);
	MSG_TYPE_NAME(MSG_CMM_RECOVER_CFG);
	MSG_TYPE_NAME(MSG_CMM_RECOVER_CFG_ACK);
	MSG_TYPE_NAME(MSG_CMM_REBOOT);
	default:
		{
			static char s_szUnsupported[64];
			sprintf(s_szUnsupported, "SNMPC_UNSUPPORTED_MSGTYPE(%lu)", (unsigned long)usMsgType);
			return s_szUnsupported;
		}
	}
}

/*
*	��������: 	MID2NAME
*	��������: 	���Ը�������������ֵ��ʽMIDת��Ϊ�ַ����ɶ���ʽ��
*	�������: 	usMID - MID��ֵ
*	����ֵ  :	��Ӧ��MID�ַ������֡��������SNMPC֧�ֵģ�����
*				SNMPC_UNSUPPORTED_MID(ddd)��ʽ��
*	����    : 	����ƽ / 2008-04-23
*/
const char* MID2NAME(unsigned short usMID)
{
	#define MID_NAME(name) case name: return #name
	switch (usMID) {
	MID_NAME(MID_TR064FE);
	MID_NAME(MID_TR064BE);
	MID_NAME(MID_PC);
	MID_NAME(MID_CMM);
	default:
		{
			static char s_szUnsupported[64];
			sprintf(s_szUnsupported, "SNMPC_UNSUPPORTED_MID(%lu)", (unsigned long)usMID);
			return s_szUnsupported;
		}
	}
}

/*
*	��������: 	SNMPA_ShowMessage
*	��������: 	�Կɶ���ʽ��ʾ��Ϣ���ݣ�����DEBUG��
*	�������: 	pstMsg - ָ��Ҫ��ʾ��Ϣ��ָ��.
*	����ֵ  :	void
*	����    : 	����ƽ / 2008-04-17
*/
void SNMPA_ShowMessage(ST_MSG* pstMsg)
{
	char* pcPos = (char*)pstMsg->szMsgBody;
	char* pcEnd = (pcPos+pstMsg->stMsgHead.ulBodyLength);

	/* Temp values */
	unsigned long uLong;
	unsigned char uChar;

/*************************************
*
* MACROS for body show.
*
*************************************/

#define INDENTS "                                                                          "
#define INDENT_UNIT	2
#define GET_INDENT(indent) (INDENTS+(sizeof(INDENTS)-1)-((indent)*INDENT_UNIT))

/*
* ����:��ȡ����ʾһ��unsigned long���͵���
* name - �������֣��ַ���
* indent - ��ӡ��Ϣʱ������
*/
#define SHOW_ULONG(name, indent) \
	do {\
		if (pcPos+sizeof(uLong)>pcEnd) { \
			printf("\tBad message - get %s(unsigned long) failed!\n", name); \
			return; \
		} else { \
			GET_ULONG(pcPos, uLong); \
			printf("%s%s=%lu\n", GET_INDENT(indent), name, uLong);\
		} \
	} while (0)

/*
* ����:��ȡ����ʾһ��unsigned char���͵���
* name - �������֣��ַ���
* indent - ��ӡ��Ϣʱ������
*/
#define SHOW_UCHAR(name, indent) \
	do {\
		if (pcPos+sizeof(uChar)>pcEnd) { \
			printf("\tBad message - get %s(unsigned char) failed!\n", name); \
			return; \
		} else { \
			GET_UCHAR(pcPos, uChar); \
			printf("%s%s=%lu\n", GET_INDENT(indent), name, (unsigned long)uChar);\
		} \
	} while (0)

/*
* ����:��ȡ����ʾһ���б�����ݡ�
* name - �б����֣��ַ���
* get_count - ��ȡ�б���Ŀ�����ķ�����GET_ULONG����GET_UCHAR��
* count_type - �б�����������ͣ���get_countһ�£�����Ϊunsigned long����unsigned char��
* indent - ��ӡ��Ϣʱ������
*/
#define SHOW_LIST(name, get_count, count_type, indent) \
	do {\
		count_type count; \
		char* item; \
		count_type i; \
		if (pcPos+sizeof(count)>pcEnd) { \
			printf("%sBad message - get  %sCount failed!\n", GET_INDENT(indent), name); \
			return;\
		}\
		get_count(pcPos, count);\
		printf("%s%sCount=%lu\n", GET_INDENT(indent), name, (unsigned long)count);\
		for (i = 0; i<count; i++) {\
			if (pcPos>=pcEnd) { \
				printf("%sBad message - item count more than actual item count!\n", GET_INDENT(indent+1)); \
				return; \
			} \
			GET_STR(pcPos, item);\
			printf("%s%s%lu='%s'\n", GET_INDENT(indent+1), name, (unsigned long)(i+1), (item?item:"(null)"));\
		}\
	} while (0)

/*
* ����:��ȡ����ʾ�������ʹ�����Ϣ�б�
* indent - ��ӡ��Ϣʱ������
*/
#define SHOW_ERROR(indent) \
	do { \
		SHOW_ULONG("ErrNo", indent); \
		SHOW_LIST("ErrItem", GET_ULONG, unsigned long, indent); \
	} while (0)

/*************************************
*
* MACROS for body show end.
*
*************************************/
	printf("Head:\n");
	printf("%sSrcMID=%s\n", GET_INDENT(1), MID2NAME(pstMsg->stMsgHead.usSrcMID));
	printf("%sDstMID=%s\n", GET_INDENT(1), MID2NAME(pstMsg->stMsgHead.usDstMID));
	printf("%sMsgID=%lu\n", GET_INDENT(1), pstMsg->stMsgHead.ulMsgID);
	printf("%sMsgType=%s\n", GET_INDENT(1), MSGTYPE2NAME(pstMsg->stMsgHead.usMsgType));
	printf("%usMsgType=%u\n", GET_INDENT(1), (unsigned int)pstMsg->stMsgHead.usMsgType);
	printf("%usFirstMID=%02X\n", GET_INDENT(1), (unsigned int)pstMsg->stMsgHead.usFirstMID);
	printf("%sBodyLength=%lu\n", GET_INDENT(1), pstMsg->stMsgHead.ulBodyLength);
	printf("Head.\n");

	printf("Body:\n");

	switch (pstMsg->stMsgHead.usMsgType) {
	case MSG_CMM_SET_VAL:
		SHOW_LIST("SetItem", GET_ULONG, unsigned long, 1);
		break;

	case MSG_CMM_SET_VAL_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {	// failed
			SHOW_ERROR(1);
		} else {	// success
			SHOW_ULONG("Status", 1);
		}
		break;

	case MSG_CMM_GET_VAL:
		SHOW_LIST("Path", GET_ULONG, unsigned long, 1);
		break;

	case MSG_CMM_GET_VAL_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {
			SHOW_ERROR(1);
		} else {
			SHOW_LIST("ResItem", GET_ULONG, unsigned long, 1);
		}

	case MSG_CMM_COMMIT:
	case MSG_CMM_CANCEL:
		// No  message body.
		break;

	case MSG_PC_START_PROC:
		SHOW_UCHAR("SteadyFlag", 1);
		SHOW_LIST("ArgVal", GET_UCHAR, unsigned char, 1);
		SHOW_LIST("MonVal", GET_UCHAR, unsigned char, 1);
		break;

	case MSG_PC_START_PROC_ACK:
		SHOW_ULONG("StartOK", 1);
		SHOW_ULONG("ProcID", 1);
		break;

	case MSG_PC_SEND_SIGNAL:
		SHOW_ULONG("ProcID", 1);
		SHOW_UCHAR("Signal", 1);
		SHOW_UCHAR("NeedRestart", 1);
		SHOW_UCHAR("NeedSteady", 1);
		SHOW_LIST("ArgVal", GET_UCHAR, unsigned char, 1);
		SHOW_LIST("MonVal", GET_UCHAR, unsigned char, 1);
		break;

	case MSG_PC_SEND_SIGNAL_ACK:
		SHOW_ULONG("Result", 1);
		break;

	case MSG_PC_PROC_STATE:
		SHOW_ULONG("ProcID", 1);
		SHOW_UCHAR("ProcState", 1);
		SHOW_UCHAR("SteadyFlag", 1);
		SHOW_UCHAR("IsKilledByUser", 1);
		break;

	case MSG_CMM_UPDATE:
		// No message body
		break;

	case MSG_CMM_UPDATE_ACK:
		SHOW_ULONG("Result", 1);
		break;

	case MSG_CMM_ADD_NODE:
	case MSG_CMM_DEL_NODE:
		printf("%sObjPath=%s\n", GET_INDENT(1), pcPos);
		break;

	case MSG_CMM_ADD_NODE_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {
			SHOW_ERROR(1);
		} else {
			SHOW_ULONG("ObjNo", 1);
			SHOW_ULONG("ObjStatus", 1);
		}
		break;

	case MSG_CMM_DEL_NODE_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {
			SHOW_ERROR(1);
		} else {
			SHOW_ULONG("ObjStatus", 1);
		}
		break;

	case MSG_CMM_GET_NAME:
		SHOW_UCHAR("NextLevel", 1);
		printf("%sPath=%s\n", GET_INDENT(1), pcPos);
		break;

	case MSG_CMM_GET_NAME_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {
			SHOW_ERROR(1);
		} else {
			SHOW_LIST("ResItem", GET_ULONG, unsigned long, 1);
		}
		break;

	case MSG_CMM_SAVE_CFG:
	case MSG_CMM_RECOVER_CFG:
	case MSG_CMM_REBOOT:
		/* no body */
		break;

	case MSG_CMM_SAVE_CFG_ACK:
	case MSG_CMM_RECOVER_CFG_ACK:
		SHOW_ULONG("Result", 1);
		if (uLong) {
			SHOW_ERROR(1);
		}
		break;

	default:
		break;
	}
	printf("Body.\n");
}

#endif

/*������ķ������ܸպ��෴*/
static struct sockaddr_in loginsrv_addrlist; //only for the following method
unsigned char *CHARS_TO_intIPADDR(const char *ipaddr)
{
    unsigned long sock_add_val = inet_addr(ipaddr);
    loginsrv_addrlist.sin_addr.s_addr = 0;
    if(sock_add_val != INADDR_NONE)  
        loginsrv_addrlist.sin_addr.s_addr = htonl(sock_add_val);

    return (unsigned char*)&(loginsrv_addrlist.sin_addr.s_addr);
}
unsigned int *TO_intIP(const char *ipaddr)
{
    unsigned long sock_add_val = inet_addr(ipaddr);
    loginsrv_addrlist.sin_addr.s_addr = 0;
    if(sock_add_val != INADDR_NONE)  
        loginsrv_addrlist.sin_addr.s_addr = htonl(sock_add_val);

    return (loginsrv_addrlist.sin_addr.s_addr);
}


/*�������ֽ��������IP��ַת������.�������ַ�����IP��ַ*/
char *intIPAddr_TO_CHARS(char *pszIpAddr, unsigned long netSortIntIpaddr)
{
    char *ipaddr = inet_ntoa(*(struct in_addr*)&netSortIntIpaddr);
    
    if(ipaddr) strcpy(pszIpAddr, ipaddr);
    else return NULL;

    return pszIpAddr;
}

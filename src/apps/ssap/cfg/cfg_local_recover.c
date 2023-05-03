/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_local_recover.c
 �ļ����� : cfgģ����ʵ�ֱ��ػָ��������ò���

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2009-07-27
            ���� :

**********************************************************************/


#include "cfg_prv.h"
#include "flash_layout.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


/*
������Ϣ:
1. ����·��
2. ������ǰһ���ڵ�
3. ��Ĭ�������ϵ�ǰһ���ڵ�
*/


CFG_RET CFG_ReadDefaultCfgByPath(const char *pcFile, char **ppcBuf,
                     unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen);

CFG_RET CFG_InitTreeByFunc(scew_tree **ppstTree, const char *pcPath,
                           FUNC_CFG_READ_CFG pfnFunc);

CFG_RET CFG_RecoverWanConnSomePara(void);

/***********************************************************
�ӿ�:   ���ػָ�����Ĭ������
����:   ��
����ֵ: 0:�ɹ�
        ����:ʧ��
��ע:   �й����Ź淶Ҫ�󱾵ػָ��뱣��������Ϣ:
        # ITMS URL
        # e ���ն�(e8)��ITMS ��˫��Digest ��֤�û���/������Ϣ��
        # ����ά���ʺŵ��û���/����
        # TR069 ʹ�õ�WAN ���Ӽ��󶨹�ϵ(����PVC/VLAN,��,
          �ɵ�����)
        # ����WAN �����������(����PVC/VLAN,LAN/WLAN ��
          ϵ,�Ž�/·�ɵ�����,PPPoE �û���/����)
        # �����û�����ز���
        # SSID--2��SSID--4 ��ز���
        # SIP PROXY ����,IP ��ַ,�˿ں�,�û��ʺ�,����(������e
          �ն�e8-C ��̬)
          (InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.*)
        # �˿�ӳ����ز���
        # �豸��֤״̬( InternetGatewayDevice.X_TWSZ-COM_UserInfo.Times ,
                        InternetGatewayDevice.X_TWSZ-COM_UserInfo.Limit ,
                        InternetGatewayDevice.X_TWSZ-COM_UserInfo.Status)
        # ONU�߼���ʶ( �����EPON����e���ն�,
                       InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserName,
                       InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserId)
        ���, ���￼��ֱ�ӱ�����������:
        1. wan��������
        2. TR069/snmp����
        3. ���� 2 - 4 ����
        4. ������
        5. ACL
***********************************************************/
CFG_RET CFG_LocalRecover(void)
{
    CFG_RET ret = CFG_OK;
    ST_CFG_LOCAL_RECOVER_KEY_TREE astKeyTree[] =
    {
        /* SSID--2��SSID--4 ��ز��� */
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.2", NULL},
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.3", NULL},
        {"InternetGatewayDevice.LANDevice.1.WLANConfiguration.4", NULL},
	#if 0
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.6", NULL},
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.7", NULL},
		{"InternetGatewayDevice.LANDevice.1.WLANConfiguration.8", NULL},
	#endif

        /* WAN������ز��� */
        {"InternetGatewayDevice.WANDevice", NULL},
        {"InternetGatewayDevice.Layer2Bridging", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_ACL", NULL},

        /* e ���ն�(e8)��ITMS ��˫��Digest ��֤�û���/������Ϣ */
        {"InternetGatewayDevice.ManagementServer", NULL},
        {"InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_MiddlewareMgt", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_SNMPAgent", NULL},

        /* ����ά���ʺŵ��û���/���� */
        {"InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_TeleComAccount", NULL},

        /* �����û�����ز��� */
        {"InternetGatewayDevice.Services.X_TWSZ-COM_MWBAND", NULL},

        /* ONU�߼���ʶ */
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserName", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserId", NULL},
		{"InternetGatewayDevice.WANDevice.1.X_TWSZ-COM_EponInterfaceConfig.Loid", NULL},
        {"InternetGatewayDevice.WANDevice.1.X_TWSZ-COM_EponInterfaceConfig.Password", NULL},

        /* �豸��֤״̬ */
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.Times", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.Limit", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.Status", NULL},
        {"InternetGatewayDevice.X_TWSZ-COM_UserInfo.Result", NULL},

        /* SIP PROXY ����,IP ��ַ,�˿ں�,�û��ʺ�,����(������e���ն�e8-C ��̬) */
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.UserAgentDomain", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.UserAgentPort", NULL},

        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.ProxyServer", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.ProxyServerPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-ProxyServer", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-ProxyServerPort", NULL},

        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.RegistrarServer", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.RegistrarServerPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-RegistrarServer", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-RegistrarServerPort", NULL},

        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.OutboundProxy", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.OutboundProxyPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-OutboundProxy", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Standby-OutboundProxyPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.X_TWSZ-COM_Transport", NULL},
//        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.SIP.DSCPMark", NULL},
        /* add by YangP on 20110830 for Q20492,add the line enable for key value */       
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.Enable", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.Enable", NULL},
        /* add end */
        /* mod by YangP on 20110702 for TR069 ����  */
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.SIP.AuthUserName", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.SIP.AuthPassword", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.SIP.X_TWSZ-COM_CIDName", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.SIP.URI", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.SIP.AuthUserName", NULL},            
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.SIP.AuthPassword", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.SIP.X_TWSZ-COM_CIDName", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.SIP.URI", NULL},
        /* mod end */
        /* add by zhanhy 2011.08.22 for H248 config */
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.MediaGatewayControler", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.MediaGatewayControlerPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.Standby-MediaGatewayControler", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.Standby-MediaGatewayControlerPort", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.DeviceIDType", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.DeviceID", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.PhysicalTermID", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.PhysicalTermIDConfigMethod", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.PhysicalTermIDPrefix", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.PhysicalTermIDAddLen", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.RTPPrefix", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.EphemeralTermIDAddLen", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.EphemeralTermIDStart", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.AuthenticationMethID", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.X_TWSZ-COM_AuthInitKey", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.X_TWSZ-COM_Authmgid", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_TWSZ-COM_H248.X_TWSZ-COM_EphemeralTermNumber", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.1.X_TWSZ-COM_H248.PhysicalTermID", NULL},
        {"InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.X_TWSZ-COM_H248.PhysicalTermID", NULL},
        /*add end zhanhy*/
        /* add by liuyousheng 2011.08.14 17:23 for QIMS20037  ����˫Э��汾��˵������Э��Ϊ�ؼ��������ָ�Ĭ������ʱ���ָ� */
        {"InternetGatewayDevice.Services.VoiceService.1.X_TWSZ-COM_ActiveSignalingProtocol", NULL},
        /* add end liuyousheng */

    };
    unsigned long i = 0;

    if (NULL == g_pstCfgTree)   /* δ��ʼ�� */
    {
        /* ��ʼ�������� */
        ret = CFG_LibInit();
        CFG_GOTO_ERR_PROC(ret, Exit);
    }

    /* ��������Ҫ���������� */
    for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        (void)CFG_DelNodeToPtr(astKeyTree[i].acPath, &astKeyTree[i].pvTree);
    }

    /* �ͷž��� */
    scew_tree_free(g_pstCfgTree);
    g_pstCfgTree = NULL;

    /* ����������� */
    ret = CFG_InitTreeByFunc(&g_pstCfgTree, NULL, CFG_ReadDefaultCfgByPath);
    CFG_GOTO_ERR_PROC(ret, Exit);

    /* �����ϲ���ȥ */
    for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        char *pcNext = astKeyTree[i].acPath + sizeof("InternetGatewayDevice");
        char *pcTemp = NULL;
        scew_element *pstNode = scew_tree_root(g_pstCfgTree);
        scew_element *pstParent = NULL;
        scew_element *pstPrev = NULL;

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
        CFG_ASSERT(NULL != pstParent);  /* ��������������lint�澯 */

        if (NULL != pstNode) /* ���������нڵ� */
        {
            /* ��¼��λ�� */
            pstPrev = pstNode->left;

            /* ɾ���ڵ� */
            scew_element_del(pstNode);

            /* ���½ڵ���ӽ��� */
            (void)scew_element_insert_elem(pstParent, pstPrev, astKeyTree[i].pvTree);
        }
        else
        {
            (void)scew_element_add_elem(pstParent, astKeyTree[i].pvTree);
        }

        astKeyTree[i].pvTree = NULL;
    }

    ret = CFG_RecoverWanConnSomePara();
    CFG_GOTO_ERR_PROC(ret, Exit);

#ifdef CONFIG_APPS_SSAP_PROTEST

#define MAX_SSID_LEN 			33
#define MAX_WPAPSK_LEN       	133
#define AUTH_LOGIN_NAME_LEN     64

	char Ssid[MAX_SSID_LEN] = {0};
	char PassWd[MAX_WPAPSK_LEN] = {0};
	char LoginName[AUTH_LOGIN_NAME_LEN] = {0};
    char LoginPassword[AUTH_LOGIN_NAME_LEN] = {0};
	int iRet = 0;
	unsigned short len;

    /* ��FLASH��ȡ��һ��VAP��ssid */
	len = MAX_SSID_LEN;
	iRet = app_item_get(Ssid , TBS_WLAN_SSID, &len);
    if((ERROR_ITEM_OK == iRet) && (strlen(Ssid) > 0))
    {
		CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSID",
			Ssid, NULL);
    }

    /* ��FLASH��ȡ��һ��VAP�ļ���ģʽ����Կ */
	len = MAX_WPAPSK_LEN;
	iRet = app_item_get(PassWd , TBS_WLAN_PASSWORD, &len);
    if((ERROR_ITEM_OK == iRet) && (strlen(PassWd) > 0))
    {
		if (strlen(PassWd) == 64)
		{
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.X_TWSZ-COM_PSKExpression",
				"PreSharedKey" , NULL);
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.PreSharedKey.1.PreSharedKey",
				PassWd, NULL);
		}
		else
		{
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.X_TWSZ-COM_PSKExpression",
				"KeyPassphrase", NULL);
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.PreSharedKey.1.KeyPassphrase",
				PassWd, NULL);
		}
    }

    /* ��FLASH��ȡ�ն˵��û��� */
    len = AUTH_LOGIN_NAME_LEN;
    iRet = app_item_get(LoginName , TBS_LOGIN_NAME ,&len);
    if(TBS_SUCCESS == ret && strlen(LoginName) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserList.2.UserName", LoginName, NULL);
    }

    /* ��FLASH��ȡ�ն˵����� */
    len = AUTH_LOGIN_NAME_LEN;
    iRet = app_item_get(LoginPassword , TBS_LOGIN_PASSWORD ,&len);
    if(TBS_SUCCESS == ret && strlen(LoginPassword) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserList.2.Password", LoginPassword, NULL);
    }

#endif

	ret = CFG_SaveCfg();
    CFG_GOTO_ERR_PROC(ret, Exit);

    (void)CFG_LibFinal();

Exit:
    for (i = 0; i < sizeof(astKeyTree)/sizeof(astKeyTree[0]); i++)
    {
        if (NULL == astKeyTree[i].pvTree)
        {
            continue;
        }
        scew_element_del(astKeyTree[i].pvTree);
    }

    printf("CFG_LocalRecover: recover config ok\n");
    return ret;
}


/***********************************************************
�ӿ�:   Զ�ָ̻�����Ĭ������
����:   ��
����ֵ: 0:�ɹ�
        ����:ʧ��
��ע:   �й����Ź淶Ҫ��Զ�ָ̻��뱣��������Ϣ:
        # ONU�߼���ʶ�������EPON����e���նˣ�
        # �β�ֵ
***********************************************************/
CFG_RET CFG_RemoteRecover(void)
{
    CFG_RET ret = CFG_OK;
	
#define TEMP_BUF_LEN 128

	char TempBuf[TEMP_BUF_LEN] = {0};

    if (NULL == g_pstCfgTree)   /* δ��ʼ�� */
    {
        /* ��ʼ�������� */
        ret = CFG_LibInit();
        CFG_GOTO_ERR_PROC(ret, Exit);
    }

    /* �ͷž��� */
    scew_tree_free(g_pstCfgTree);
    g_pstCfgTree = NULL;

    /* ����������� */
    ret = CFG_InitTreeByFunc(&g_pstCfgTree, NULL, CFG_ReadDefaultCfgByPath);
    CFG_GOTO_ERR_PROC(ret, Exit);

#ifdef CONFIG_APPS_SSAP_PROTEST

#define MAX_SSID_LEN 			33
#define MAX_WPAPSK_LEN       	133
#define AUTH_LOGIN_NAME_LEN     64

	char Ssid[MAX_SSID_LEN] = {0};
	char PassWd[MAX_WPAPSK_LEN] = {0};
	char LoginName[AUTH_LOGIN_NAME_LEN] = {0};
    char LoginPassword[AUTH_LOGIN_NAME_LEN] = {0};
	int iRet = 0;
	unsigned short len;

    /* ��FLASH��ȡ��һ��VAP��ssid */
	len = MAX_SSID_LEN;
	iRet = app_item_get(Ssid , TBS_WLAN_SSID ,&len);
    if((ERROR_ITEM_OK == iRet) && (strlen(Ssid) > 0))
    {
		CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSID",
			Ssid, NULL);
    }

    /* ��FLASH��ȡ��һ��VAP�ļ���ģʽ����Կ */
	len = MAX_WPAPSK_LEN;
	iRet = app_item_get(PassWd , TBS_WLAN_PASSWORD ,&len);
    if((ERROR_ITEM_OK == iRet) && (strlen(PassWd) > 0))
    {
		if (strlen(PassWd) == 64)
		{
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.X_TWSZ-COM_PSKExpression",
				"PreSharedKey" , NULL);
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.PreSharedKey.1.PreSharedKey",
				PassWd, NULL);
		}
		else
		{
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.X_TWSZ-COM_PSKExpression",
				"KeyPassphrase", NULL);
			CFG_SetNodeVal("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.PreSharedKey.1.KeyPassphrase",
				PassWd, NULL);
		}
    }

    /* ��FLASH��ȡ�ն˵��û��� */
    len = AUTH_LOGIN_NAME_LEN;
    iRet = app_item_get(LoginName , TBS_LOGIN_NAME ,&len);
    if(TBS_SUCCESS == ret && strlen(LoginName) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserList.2.UserName", LoginName, NULL);
    }

    /* ��FLASH��ȡ�ն˵����� */
    len = AUTH_LOGIN_NAME_LEN;
    iRet = app_item_get(LoginPassword , TBS_LOGIN_PASSWORD ,&len);
    if(TBS_SUCCESS == ret && strlen(LoginPassword) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserList.2.Password", LoginPassword, NULL);
    }
#endif

	/* ��FLASH��ȡ�й�����ά��״�ŵ����� */
/*
    len = TEMP_BUF_LEN;
    memset(TempBuf, 0, sizeof(TempBuf));
    iRet = app_item_get(TempBuf , TBS_TELECOM_PASSWORD ,&len);
    if(TBS_SUCCESS == ret && strlen(TempBuf) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_TeleComAccount.Password", TempBuf, NULL);
    }
    */
   /* ��FLASH��ȡONU���߼�ID���� */
    len = TEMP_BUF_LEN;
    memset(TempBuf, 0, sizeof(TempBuf));
    iRet = app_item_get(TempBuf , TBS_ONU_LOGID ,&len);
    if(TBS_SUCCESS == ret && strlen(TempBuf) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserId", TempBuf, NULL);
		CFG_SetNodeVal("InternetGatewayDevice.WANDevice.1.X_TWSZ-COM_EponInterfaceConfig.Password",TempBuf, NULL);
    }
	
	len = TEMP_BUF_LEN;
	memset(TempBuf, 0, sizeof(TempBuf));
    iRet = app_item_get(TempBuf , TBS_ONU_PASSWORD ,&len);
    if(TBS_SUCCESS == ret && strlen(TempBuf) > 0)
    {
        CFG_SetNodeVal("InternetGatewayDevice.X_TWSZ-COM_UserInfo.UserName", TempBuf, NULL);
		CFG_SetNodeVal("InternetGatewayDevice.WANDevice.1.X_TWSZ-COM_EponInterfaceConfig.Loid",TempBuf, NULL);
	}
	
	ret = CFG_SaveCfg();
    CFG_GOTO_ERR_PROC(ret, Exit);

    (void)CFG_LibFinal();

Exit:

    printf("CFG_RemoteRecover: recover config ok\n");
    return ret;
}



CFG_RET CFG_RecoverWanConnSomePara(void)
{
    CFG_RET ret = CFG_OK;
    char *apcProtType[] = {"WANIPConnection.{i}.", "WANPPPConnection.{i}."};
    char *apcLeaf[] = {"X_TWSZ-COM_DMZEnabled", "X_TWSZ-COM_DMZHost"};
    char *apcVal[] = {NULL, ""};
    char acPath[CFG_MAX_PATH_LEN] = "InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.";
    unsigned long ulLen = 60; /* acPath �ĳ��� */
    unsigned long i = 0;
    ST_CFG_INST_NODE *pstList = NULL;
    ST_CFG_INST_NODE *pstNode = NULL;

    for (i = 0; i < sizeof(apcProtType)/sizeof(apcProtType[0]); i++)
    {
        strcat(acPath, apcProtType[i]);
        ret = CFG_GetPathInstList(acPath, NULL, 0, &pstList);
        CFG_COMM_ERR_PROC(ret, "%s", acPath);

        for (pstNode = pstList; NULL != pstNode; pstNode = pstNode->pstNext)
        {
            ret = CFG_SetValFromStructEx(acPath, pstNode->aulIndexList, apcVal,
                                         apcLeaf, 2);
            CFG_GOTO_ERR_PROC(ret, EXIT, "%lu %lu %lu", pstNode->aulIndexList[0],
                            pstNode->aulIndexList[1], pstNode->aulIndexList[2]);
        }

        (void)CFG_MemFree(pstList);
        pstList = NULL;

        strcat(acPath, "X_TWSZ-COM_DDNSConfiguration");
        ret = CFG_ClearObjInstEx(acPath, 1);
        CFG_GOTO_ERR_PROC(ret, EXIT);

        acPath[ulLen] = '\0';
    }

EXIT:
    if (NULL != pstList)
    {
        (void)CFG_MemFree(pstList);
        pstList = NULL;
    }

    return ret;
}






#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif



/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: wancommon.c,v 1.26.20.2 2003/10/31 21:31:35 mthawani Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wancommon.h"
#include "cmmif.h"

int SetEnabledForInternet(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_WanWANCommonInterface"EnabledForInternet",VAR_EnabledForInternet);
}

int WANCommonInterfaceConfig_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   char szAdslValue[CMM_MAX_NODE_LEN] ;
   
   switch (varindex) 
   {
        case VAR_WANAccessType:
           if(CMM_GetStr(TR064_ROOT_WanWANCommonInterface"WANAccessType", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current WANAccessType");
			      return FALSE;
		   }
           break;
        case VAR_TotalBytesSent:
		   if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"TotalBytesSent", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current TotalBytesSent");
			      return FALSE;
		   }
           break;
        case VAR_TotalBytesReceived:
           if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"TotalBytesReceived", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current TotalBytesReceived");
			      return FALSE;
		   }
           break;
        case VAR_TotalPacketsSent:
		   if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"TotalPacketsSent", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current TotalBytesReceived");
			      return FALSE;
		   }
           break;
        case VAR_TotalPacketsReceived:
		   if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"TotalPacketsSent", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current TotalPacketsSent");
			      return FALSE;
		   }
           break;
        case VAR_Layer1UpstreamMaxBitRate:
           if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"Layer1UpstreamMaxBitRate", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current Layer1UpstreamMaxBitRate");
			      return FALSE;
		   }
           break;
        case VAR_Layer1DownstreamMaxBitRate:
           if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"Layer1DownstreamMaxBitRate", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current Layer1DownstreamMaxBitRate");
			      return FALSE;
		   }
           break;
        case VAR_PhysicalLinkStatus:
           if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"PhysicalLinkStatus", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current PhysicalLinkStatus");
			      return FALSE;
		   }
           break;
        case VAR_EnabledForInternet:
           if(CMM_GetStr( TR064_ROOT_WanWANCommonInterface"EnabledForInternet", szAdslValue,sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
		   {
			      TR64FE_TRACE("Could not get current EnabledForInternet");
			      return FALSE;
		   }
           
           break;
   }

   if( strcmp(szAdslValue,""))
   {
   	   strcpy(var->value, szAdslValue);
   }else
   {
       strcpy(var->value, "0");
   }
   	
   return TRUE;
}


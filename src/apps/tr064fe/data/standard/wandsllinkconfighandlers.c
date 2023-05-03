#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

#include "wandsllinkconfig.h"
#include "tr64defs.h"
#include "cmmif.h"

#define GetDslLinkValue(szLeafName) \
do{\
    char szNodeName[CMM_MAX_NODE_LEN];\
    int nWanDev =psvc->device->parent->instance;\
    int nWanDsl= psvc->device->instance;\
    snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.%s",nWanDev,nWanDsl,szLeafName);\
    if(CMM_GetStr(szNodeName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current WANDSLLinkConfig %s ",szLeafName);\
	      return FALSE;\
	}\
	return TRUE; \
}while(0)

int WANDSLLinkConfig_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   switch (varindex) 
   {
     case VAR_Enable:
         GetDslLinkValue("Enable");
         break;

     case VAR_LinkType:
         GetDslLinkValue("LinkType");
         break;

     case VAR_LinkStatus:
	 	 GetDslLinkValue("LinkStatus");
         break;
        
     case VAR_DestinationAddress:
	 	 GetDslLinkValue("DestinationAddress");
         break;
     case VAR_ATMEncapsulation:
	 	 GetDslLinkValue("ATMEncapsulation");
         break;

     case VAR_ATMQoS:
         GetDslLinkValue("ATMEncapsulation");
        break;
     case VAR_ATMPeakCellRate:
        GetDslLinkValue("ATMPeakCellRate");
        break;

     case VAR_ATMMaximumBurstSize:
        GetDslLinkValue("ATMMaximumBurstSize");
        break;

     case VAR_ATMSustainableCellRate:
        GetDslLinkValue("ATMSustainableCellRate");
        break;
   }
  
   return FALSE;
}

int SetDSLEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szLeafName[CMM_MAX_NODE_LEN];
   snprintf(szLeafName ,sizeof(szLeafName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.Enable",
   	                                psvc->device->parent->instance,psvc->device->instance);
   SetTr64Value(szLeafName,VAR_Enable);
}

int SetATMEncapsulation(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szLeafName[CMM_MAX_NODE_LEN];
   snprintf(szLeafName ,sizeof(szLeafName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.ATMEncapsulation",
   	                                psvc->device->parent->instance,psvc->device->instance);
   SetTr64Value(szLeafName,VAR_ATMEncapsulation);
}

int SetDestinationAddress(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szLeafName[CMM_MAX_NODE_LEN];
   snprintf(szLeafName ,sizeof(szLeafName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.DestinationAddress",
   	                                psvc->device->parent->instance,psvc->device->instance);
   SetTr64Value(szLeafName,VAR_DestinationAddress);
}

int SetATMQoS(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szATMQoS[CMM_MAX_NODE_LEN];
   snprintf(szATMQoS ,sizeof(szATMQoS),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.ATMQoS",
   	                                psvc->device->parent->instance,psvc->device->instance);
   char szATMPeakCR[CMM_MAX_NODE_LEN];
   snprintf(szATMPeakCR ,sizeof(szATMPeakCR),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.ATMPeakCellRate",
   	                                psvc->device->parent->instance,psvc->device->instance);

   char szATMMaximumBS[CMM_MAX_NODE_LEN];
   snprintf(szATMMaximumBS ,sizeof(szATMMaximumBS),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.ATMMaximumBurstSize",
   	                                psvc->device->parent->instance,psvc->device->instance);
   char szATMSustainableCR[CMM_MAX_NODE_LEN];
   snprintf(szATMSustainableCR ,sizeof(szATMSustainableCR),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.ATMSustainableCellRate",
   	                                psvc->device->parent->instance,psvc->device->instance);
   
   char* szLeafNames[4]={szATMQoS,szATMPeakCR,szATMMaximumBS,szATMSustainableCR};
   int  relatedVars[4] = {VAR_ATMQoS,VAR_ATMPeakCellRate, VAR_ATMMaximumBurstSize,VAR_ATMSustainableCellRate};
   SetTr64Values(szLeafNames,relatedVars ,4);
}

int SetDSLLinkType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szLeafName[CMM_MAX_NODE_LEN];
   snprintf(szLeafName ,sizeof(szLeafName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.WANDSLLinkConfig.LinkType",
   	                                psvc->device->parent->instance,psvc->device->instance);
   SetTr64Value(szLeafName,VAR_LinkType);
}
      
int GetStatisticsWANDSL(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{   
   /*
    static char AAL5CRCErrors[32];
    char szLeafName[CMM_MAX_NODE_LEN];
    snprintf(szLeafName ,sizeof(szLeafName),TR064_ROOT_WanDevice"%d.WANConnectionDevice.%d.AAL5CRCErrors",
		                           psvc->device->parent->instance,psvc->device->instance);	  
  	if(CMM_GetStr(szLeafName, AAL5CRCErrors,sizeof(AAL5CRCErrors), NULL, 0) != CMM_SUCCESS)
	{
	      TR64FE_TRACE("Could not get currentWANDSLLinkConfig AAL5CRCErrors");
	      return FALSE;\
	}
	*/

    soap_error( uclient, SOAP_ACTIONFAILED );
    return FALSE;
}


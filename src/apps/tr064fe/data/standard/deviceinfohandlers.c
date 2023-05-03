#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "deviceinfoparams.h"
#include "tr64defs.h"

#include "cmmif.h"

extern tr64PersistentData *pTr64Data;
extern void setCurrentState(pTr64PersistentData pData);

#define GetDeviceInfoValue(szLeafName) \
do{\
	char szNodeName[60]={0};\
    snprintf(szNodeName, sizeof(szNodeName), TR064_ROOT_DeviceInfo"%s",szLeafName);\
  	if(CMM_GetStr(szNodeName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current DeviceInfo %s ",szLeafName);\
	      return FALSE;\
	}\
	return TRUE; \
}while(0)

int DeviceInfo_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   switch (varindex) 
   {
      case VAR_Manufacturer:
	  	 GetDeviceInfoValue("Manufacturer");
      break;

      case VAR_ManufacturerOUI:
         GetDeviceInfoValue("ManufacturerOUI");
      break;

      case VAR_ModelName:
         GetDeviceInfoValue("ModelName");
      break;

      case VAR_Description:
         GetDeviceInfoValue("Description");
      break;

      case VAR_ProductClass:
         GetDeviceInfoValue("ProductClass");
      break;
      
       case VAR_SerialNumber:
         GetDeviceInfoValue("SerialNumber");
      break;
      
       case VAR_SoftwareVersion:
         GetDeviceInfoValue("SoftwareVersion");
      break;   

       case VAR_HardwareVersion:
         GetDeviceInfoValue("HardwareVersion");
      break;   

       case VAR_SpecVersion:
         GetDeviceInfoValue("SpecVersion");
      break;      

       case VAR_ProvisioningCode:
         strcpy(var->value, pTr64Data->provisioningCode);
      break;         
       
      case VAR_UpTime:  
         GetDeviceInfoValue("");
      break;
    }

    return TRUE;
}

int SetProvisioningCode(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;

   pParams = findActionParamByRelatedVar(ac,VAR_ProvisioningCode);
   if (pParams != NULL)
   {
      if(pParams->value!=NULL)
      {
         strcpy(pTr64Data->provisioningCode, pParams->value);
      }
      setCurrentState(pTr64Data);
   }
   else
   {
      //cmsLog_error("input pParam error");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   return TRUE;
}

int GetDeviceLog(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   // GetDeviceInfoValue("DeviceLog");
   return TRUE;
}


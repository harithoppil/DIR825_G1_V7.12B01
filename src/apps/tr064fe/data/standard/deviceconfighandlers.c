#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "deviceconfigparams.h"
#include "cmmif.h"



int DeviceConfig_Init(PService psvc, service_state_t state)
{
   switch (state) 
   {
     case SERVICE_CREATE:
      psvc->opaque = NULL;
      break;

     case SERVICE_DESTROY:
      break;
   }
   
   return TRUE;
}

int DeviceConfig_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   switch (varindex) 
   {
	   case VAR_PersistenData:
	      break;

	   case VAR_ConfigFile:
	      break;

	   case VAR_AStatus:
	      strcpy(var->value,"ChangesApplied");
	      break;

	   case VAR_AUUID:
	      strcpy(var->value,"");
	      break;
   }
   return TRUE;
}


int Tr64FactoryReset(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   UPNP_TRACE(("Tr64FactoryReset()\n"));

   if( TR064_PostFactoryReset() != CMM_SUCCESS )
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }
   return TRUE;
}  /* Tr64FactoryReset */

int Tr64Reboot(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   UPNP_TRACE(("Tr64Reboot(entry)\n"));
   
   if( TR064_PostReboot() != CMM_SUCCESS)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }
   return TRUE;
}  /* Tr64Reboot */


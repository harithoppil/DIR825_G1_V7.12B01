#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wandslifcfgparams.h"
#include "tr64defs.h"

#include "cmmif.h"

#define GetWANDSLIfCfgValue(szLeafName) \
do{\
	char szFullLeafName[100]={0};\
    snprintf(szFullLeafName, sizeof(szFullLeafName), TR064_ROOT_WANDSLIfCfg"%s" ,szLeafName);\
  	if(CMM_GetStr(szFullLeafName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current WANDSLInterfaceConfig %s ",szLeafName);\
	      return FALSE;\
	}\
	if( 0 == strcmp(var->value,""))\
	{\
	   strcpy(var->value,"0");\
	}\
}while(0)

int WANDSLInterfaceConfig_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   switch (varindex) 
   {
     case VAR_Enable:
        GetWANDSLIfCfgValue("Enable");
        break;

     case VAR_Status:
	 	GetWANDSLIfCfgValue("Status");
        break;

     case VAR_UpstreamCurrRate:
         GetWANDSLIfCfgValue("UpstreamCurrRate");
        break;

     case VAR_DownstreamCurrRate:
        GetWANDSLIfCfgValue("DownstreamCurrRate");
        break;
  
     case VAR_UpstreamMaxRate:
         GetWANDSLIfCfgValue("UpstreamMaxRate");
        break;
  
     case VAR_DownstreamMaxRate:
         GetWANDSLIfCfgValue("DownstreamMaxRate");
        break;   

     case VAR_UpstreamNoiseMargin:
         GetWANDSLIfCfgValue("UpstreamNoiseMargin");
        break;   

     case VAR_DownstreamNoiseMargin:
         GetWANDSLIfCfgValue("DownstreamNoiseMargin");
        break;      

     case VAR_UpstreamAttenuation:
         GetWANDSLIfCfgValue("UpstreamAttenuation");
        break;
  
     case VAR_DownstreamAttenuation:
         GetWANDSLIfCfgValue("DownstreamAttenuation");
        break;   

     case VAR_DownstreamPower:
         //GetWANDSLIfCfgValue("DownstreamPower");
         strcpy(var->value,"0");
        break;         

     case VAR_UpstreamPower:
         //GetWANDSLIfCfgValue("UpstreamPower");
         strcpy(var->value,"0");
        break;            

     case VAR_ATURVendor:
         //GetWANDSLIfCfgValue("ATURVendor");
         strcpy(var->value,"0");
        break;

     case VAR_ATURCountry:
        //GetWANDSLIfCfgValue("ATURCountry");
        strcpy(var->value,"0");
        break;

     case VAR_ATUCVendor:
         //GetWANDSLIfCfgValue("ATUCVendor");
         strcpy(var->value,"0");
        break;

     case VAR_ATUCCountry:
        //GetWANDSLIfCfgValue("ATUCCountry");
        strcpy(var->value,"0");
        break;

     case VAR_TotalStart:
        strcpy(var->value,"0");
        break;
   	}
	return TRUE; 
}

int SetDSLInterfaceEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_WANDSLIfCfg"Enable",VAR_Enable);
}

int GetStatisticsTotal(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char* aszLeafNames[9]= {"Enable" ,"Stats.Total.ErroredSecs","Stats.Total.SeverelyErroredSecs",
   	                      "Stats.Total.HECErrors","Stats.Total.ATUCHECErrors","Stats.Total.ATUCFECErrors",
   	                      "Stats.Total.CRCErrors","Stats.Total.ATUCCRCErrors","Stats.Total.FECErrors"};
   
   ST_TR064STR_LINK *pStWanDslIfCfgLink;
   pStWanDslIfCfgLink = CMM_GetStrs(TR064_ROOT_WANDSLIfCfg,aszLeafNames,9);
   if(pStWanDslIfCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   static char ErroredSecs[32];
   static char SeverelyErroredSecs[32];
   static char HECErrors[32];
   static char ATUCHECErrors[32];
   static char ATUCFECErrors[32];
   static char CRCErrors[32];
   static char ATUCCRCErrors[32];
   static char FECErrors[32];

   ST_TR064STR_LINK *pTmpLink = pStWanDslIfCfgLink ;

   if( 0 == strcmp(pTmpLink->pstrValue ,"1"))
   {
       pTmpLink = pTmpLink->pNext;
	   
       strcpy(ErroredSecs, pTmpLink->pstrValue);
       pTmpLink = pTmpLink->pNext;

	   strcpy(SeverelyErroredSecs, pTmpLink->pstrValue);
	   pTmpLink = pTmpLink->pNext;

	   strcpy(HECErrors, pTmpLink->pstrValue);
	   pTmpLink = pTmpLink->pNext;

	   strcpy(ATUCHECErrors, pTmpLink->pstrValue);
	   pTmpLink = pTmpLink->pNext;

	   strcpy(ATUCFECErrors, pTmpLink->pstrValue);
	   pTmpLink = pTmpLink->pNext;

	   strcpy(CRCErrors, pTmpLink->pstrValue);
	   pTmpLink = pTmpLink->pNext;
	   
	   strcpy(ATUCCRCErrors, pTmpLink->pstrValue);
       pTmpLink = pTmpLink->pNext;

	   strcpy(FECErrors, pTmpLink->pstrValue);
   }else
   {
       sprintf(ErroredSecs,"%u", 0);
       sprintf(SeverelyErroredSecs,"%u", 0);
       sprintf(HECErrors,"%u", 0);
       sprintf(ATUCHECErrors,"%u", 0);
	   sprintf(ATUCFECErrors,"%u", 0);
       sprintf(CRCErrors,"%u", 0);
       sprintf(ATUCCRCErrors,"%u", 0);
	   sprintf(FECErrors,"%u", 0);
   }
   
   TR064_DestroyStrLink(pStWanDslIfCfgLink);

   int errorinfo = 0 ;
   errorinfo |= OutputCharValueToAC(ac, VAR_ErroredSecs, ErroredSecs);
   errorinfo |= OutputCharValueToAC(ac, VAR_SeverelyErroredSecs, SeverelyErroredSecs);
   errorinfo |= OutputCharValueToAC(ac, VAR_HECErrors, HECErrors);
   errorinfo |= OutputCharValueToAC(ac, VAR_ATUCHECErrors, ATUCHECErrors);
   errorinfo |= OutputCharValueToAC(ac, VAR_ATUCFECErrors, ATUCHECErrors);
   errorinfo |= OutputCharValueToAC(ac, VAR_CRCErrors, CRCErrors);
   errorinfo |= OutputCharValueToAC(ac, VAR_ATUCCRCErrors, ATUCCRCErrors);
   errorinfo |= OutputCharValueToAC(ac, VAR_FECErrors, FECErrors);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetStatisticsShowTime(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    return GetStatisticsTotal( uclient, psvc, ac, args, nargs);
}


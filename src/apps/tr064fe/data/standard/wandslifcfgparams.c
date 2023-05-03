
/*****************************************************************************
//
//  Copyright (c) 2005  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       wandslifcfgparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wandslifcfgparams.h"


static VarTemplate StateVariables[] = { 
   { "Enable", "1", VAR_BOOL }, 
   { "Status", "", VAR_STRING }, 
   { "UpstreamCurrRate", "", VAR_ULONG }, 
   { "DownstreamCurrRate", "", VAR_ULONG }, 
   { "UpstreamMaxRate", "",VAR_ULONG }, 
   { "DownstreamMaxRate", "", VAR_ULONG }, 
   { "UpstreamNoiseMargin", "", VAR_STRING }, 
   { "DownstreamNoiseMargin", "", VAR_STRING },    
   { "UpstreamAttenuation", "", VAR_STRING },    
   { "DownstreamAttenuation", "", VAR_STRING },    
   { "UpstreamPower", "", VAR_LONG },    
   { "DownstreamPower", "", VAR_LONG },    
   { "ATURVendor", "", VAR_STRING },  
   { "ATURCountry", "", VAR_ULONG },  
   { "ATURANSIStd", "", VAR_ULONG },
   { "ATURANSIRev", "", VAR_ULONG },
   { "ATUCVendor", "", VAR_STRING },  
   { "ATUCCountry", "", VAR_ULONG },  
   { "ATUCANSIStd", "", VAR_ULONG },
   { "ATUCANSIRev", "", VAR_ULONG },   
   { "TotalStart", "", VAR_ULONG },      
   { "ShowTimeStart", "", VAR_ULONG },         
   { "LastShowTimeStart", "", VAR_ULONG },
   { "CurrentDayStart", "", VAR_ULONG },         
   { "QuarterHourStart", "", VAR_ULONG },     

   { "Stats.Total.ReceiveBlocks", "", VAR_ULONG },     
   { "Stats.Total.TransmitBlocks", "", VAR_ULONG },        
   { "Stats.Total.CellDelin", "", VAR_ULONG },        
   { "Stats.Total.LinkRetrain", "", VAR_ULONG },     
   { "Stats.Total.InitErrors", "", VAR_ULONG },        
   { "Stats.Total.InitTimeouts", "", VAR_ULONG },
   { "Stats.Total.LossOfFraming", "", VAR_ULONG },     
   { "ErroredSecs", "", VAR_STRING }, //    
   { "SeverelyErroredSecs", "", VAR_STRING }, //  
   { "FECErrors", "", VAR_STRING },     
   { "ATUCFECErrors", "", VAR_STRING },  //   
   { "HECErrors", "", VAR_STRING },  //
   { "ATUCHECErrors", "", VAR_STRING },// 
   { "CRCErrors", "", VAR_STRING },  //      
   { "ATUCCRCErrors", "", VAR_STRING }, //        
   { 0 } 
};


static Action _SetEnable = {
    "SetEnable", SetDSLInterfaceEnable,
   (Param []) {
       {"NewEnable", VAR_Enable, VAR_IN},
       { 0 }
    }
};

static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewEnable", VAR_Enable, VAR_OUT},
       {"NewStatus", VAR_Status, VAR_OUT},
       {"NewUpstreamCurrRate", VAR_UpstreamCurrRate, VAR_OUT},   
       {"NewDownstreamCurrRate", VAR_DownstreamCurrRate, VAR_OUT},   
       {"NewUpstreamMaxRate", VAR_UpstreamMaxRate, VAR_OUT},   
       {"NewDownstreamMaxRate", VAR_DownstreamMaxRate, VAR_OUT},   
       {"NewUpstreamNoiseMargin", VAR_UpstreamNoiseMargin, VAR_OUT},   
       {"NewDownstreamNoiseMargin", VAR_DownstreamNoiseMargin, VAR_OUT},   
       {"NewUpstreamAttenuation", VAR_UpstreamAttenuation, VAR_OUT},   
       {"NewDownstreamAttenuation", VAR_DownstreamAttenuation, VAR_OUT},   
     //  {"NewUpstreamPower", VAR_UpstreamPower, VAR_OUT},   
      // {"NewDownstreamPower", VAR_DownstreamPower, VAR_OUT},                                   
      // {"NewATURVendor", VAR_ATURVendor, VAR_OUT},  
      // {"NewATURCountry", VAR_ATURCountry, VAR_OUT},  
      // {"NewATUCVendor", VAR_ATUCVendor, VAR_OUT},  
      // {"NewATUCCountry", VAR_ATUCCountry, VAR_OUT},  
      // {"NewTotalStart", VAR_TotalStart, VAR_OUT},  
       { 0 }
    }
};

static Action _GetStatisticsTotal = {
    "GetStatisticsTotal", GetStatisticsTotal,
   (Param []) {
       {"NewErroredSecs", VAR_ErroredSecs, VAR_OUT},   
       {"NewSeverelyErroredSecs", VAR_SeverelyErroredSecs, VAR_OUT},   
       {"NewATUCFECErrors", VAR_ATUCFECErrors, VAR_OUT},  
       {"NewATUCHECErrors", VAR_ATUCHECErrors, VAR_OUT},  
       {"NewHECErrors", VAR_HECErrors, VAR_OUT},  
       {"NewCRCErrors", VAR_CRCErrors, VAR_OUT},  
       {"NewATUCCRCErrors", VAR_ATUCCRCErrors, VAR_OUT}, 
       {"NewFECErrors", VAR_FECErrors, VAR_OUT},  
       { 0 }
    }
};

static Action _GetStatisticsShowTime = {
    "GetStatisticsShowTime", GetStatisticsShowTime,
   (Param []) {
       {"NewReceiveBlocks", VAR_ReceiveBlocks, VAR_OUT},
       {"NewTransmitBlocks", VAR_TransmitBlocks, VAR_OUT},
       {"NewCellDelin", VAR_CellDelin, VAR_OUT},   
       {"NewLinkRetrain", VAR_LinkRetrain, VAR_OUT},   
       {"NewInitErrors", VAR_InitErrors, VAR_OUT},   
       {"NewInitTimeouts", VAR_InitTimeouts, VAR_OUT},   
       {"NewLossOfFraming", VAR_LossOfFraming, VAR_OUT},   
       {"NewErroredSecs", VAR_ErroredSecs, VAR_OUT},   
       {"NewSeverelyErroredSecs", VAR_SeverelyErroredSecs, VAR_OUT},   
       {"NewFECErrors", VAR_FECErrors, VAR_OUT},   
       {"NewATUCFECErrors", VAR_ATUCFECErrors, VAR_OUT},   
       {"NewHECErrors", VAR_HECErrors, VAR_OUT},  
       {"NewATUCHECErrors", VAR_ATUCHECErrors, VAR_OUT},  
       {"NewCRCErrors", VAR_CRCErrors, VAR_OUT},  
       {"NewATUCCRCErrors", VAR_ATUCCRCErrors, VAR_OUT},  
       { 0 }
    }
};

static PAction Actions[] = {
   &_SetEnable,
   &_GetInfo,
   &_GetStatisticsTotal,
 //  &_GetStatisticsShowTime,
   NULL
};

ServiceTemplate Template_WANDSLInterfaceConfig = {
   "WANDSLInterfaceConfig:1",
   NULL,
   WANDSLInterfaceConfig_GetVar,    /* state variable handler */
   NULL,  /* SVCXML */
   ARRAYSIZE(StateVariables)-1, StateVariables,
   Actions,
   0,
   "urn:dslforum-org:serviceId:WANDSLInterfaceConfig"
};


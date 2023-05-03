
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
//  Filename:       wandsllinkconfigparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wandsllinkconfig.h"

struct iftab iftable[] = {
   { "nas_", 1 },
   { "ipa_", 2 },
   { "ppp_", 3 },
   { "ppp_", 4 },
   { "nas_", 5 },
   { "unconfigured", 6 },
   { NULL, 0 }
};

char *PossibleLinkTypes_allowedValueList[] = { "EoA",
                                               "IPoA",
                                               "PPPoA",
                                               "PPPoE",
                                               "CIP",
                                               "Unconfigured",
                                               NULL };

char *LinkStatus_allowedValueList[] = { "Down",
                                        "Up",
                                        "Initializing",
                                        "Unavailable",
                                        NULL };

char *Encapsulation_allowedValueList[] = { "LLC",
                                        "VCMUX",
                                        NULL };

char *ATMQoS_allowedValueList[] = { "UBR",
                                    "CBR",
                                    "GFR",
                                    "VBR-nrt",
                                    "VBR-rt",
                                    "UBR+",
                                    "ABR",
                                     NULL };

static VarTemplate StateVariables[] = {
   { "Enable", "", VAR_BOOL },
   { "LinkStatus", "Unavailable", VAR_EVENTED|VAR_STRING|VAR_LIST,(allowedValue) { LinkStatus_allowedValueList } },       
   { "LinkType", "Unconfigured", VAR_STRING|VAR_LIST, (allowedValue) { PossibleLinkTypes_allowedValueList } }, 
   { "AutoConfig", "1", VAR_EVENTED|VAR_BOOL }, //not support 
   { "ModulationType", "", VAR_STRING },   // not support
   { "DestinationAddress", "", VAR_STRING }, 
   { "ATMEncapsulation", "", VAR_STRING|VAR_LIST, (allowedValue) { Encapsulation_allowedValueList } }, 
   { "FCSPreserved", "", VAR_BOOL },   //not support
   { "VCSearchList", "", VAR_STRING }, //not support
   { "ATMAAL", "", VAR_STRING }, 
   { "ATMTransmittedBlocks", "", VAR_ULONG },  //not support
   { "ATMReceivedBlocks", "", VAR_ULONG },  //not support
   { "ATMQoS", "", VAR_STRING|VAR_LIST, (allowedValue) { ATMQoS_allowedValueList }  }, 
   { "ATMPeakCellRate", "", VAR_ULONG }, 
   { "ATMMaximumBurstSize", "", VAR_STRING }, 
   { "ATMSustainableCellRate", "", VAR_STRING }, 
   { "AAL5CRCErrors", "", VAR_ULONG }, //not support
   { "ATMCRCErrors", "", VAR_ULONG },  //not support
   { "ATMHECErrors", "", VAR_ULONG },  //not support
   { 0 } 
};

static Action _GetInfo = { 
   "GetInfo", GetInfo,
   (Param [])    {
      {"NewEnable", VAR_Enable, VAR_OUT},
      {"NewLinkType", VAR_LinkType, VAR_OUT},          
      {"NewLinkStatus", VAR_LinkStatus, VAR_OUT},      
      {"NewDestinationAddress", VAR_DestinationAddress, VAR_OUT},      
      {"NewATMEncapsulation", VAR_ATMEncapsulation, VAR_OUT},      
      {"NewATMQoS", VAR_ATMQoS, VAR_OUT},      
      {"NewATMPeakCellRate", VAR_ATMPeakCellRate, VAR_OUT},      
      {"NewATMMaximumBurstSize", VAR_ATMMaximumBurstSize, VAR_OUT},      
      {"NewATMSustainableCellRate", VAR_ATMSustainableCellRate, VAR_OUT},      
      { 0 }
   }
};

static Action _SetEnable = { 
   "SetEnable", SetDSLEnable,
   (Param [])    {
      {"NewEnable", VAR_Enable, VAR_IN},
      { 0 }
   }
};


static Action _GetDSLLinkInfo = { 
   "GetDSLLinkInfo", GetDSLLinkInfo,
   (Param [])    {
      {"NewLinkType", VAR_LinkType, VAR_OUT},
      { 0 }
   }
};
/* It is not required in Phase 1
static Action _GetAutoConfig = { 
   "GetAutoConfig", GetAutoConfig,
   (Param [])    {
      {"NewAutoConfig", VAR_AutoConfig, VAR_OUT},
      { 0 }
   }
};

static Action _GetModulationType = { 
   "GetModulationType", GetModulationType,
   (Param [])    {
      {"NewModulationType", VAR_ModulationType, VAR_OUT},
      { 0 }
   }
};
*/
static Action _SetDestinationAddress = { 
   "SetDestinationAddress", SetDestinationAddress,
   (Param [])    {
      {"NewDestinationAddress", VAR_DestinationAddress, VAR_IN},
      { 0 }
   }
};

static Action _GetDestinationAddress = { 
   "GetDestinationAddress", GetDestinationAddress,
   (Param [])    {
      {"NewDestinationAddress", VAR_DestinationAddress, VAR_OUT},
      { 0 }
   }
};

static Action _SetATMEncapsulation = { 
   "SetATMEncapsulation", SetATMEncapsulation,
   (Param [])    {
      {"NewATMEncapsulation", VAR_ATMEncapsulation, VAR_IN},
      { 0 }
   }
};

static Action _GetATMEncapsulation = { 
   "GetATMEncapsulation", GetATMEncapsulation,
   (Param [])    {
      {"NewATMEncapsulation", VAR_ATMEncapsulation, VAR_OUT},
      { 0 }
   }
};

static Action _SetATMQoS = { 
   "SetATMQoS", SetATMQoS,
   (Param [])    {
      {"NewATMQoS", VAR_ATMQoS, VAR_IN},
      {"NewATMPeakCellRate", VAR_ATMPeakCellRate, VAR_IN},
      {"NewATMMaximumBurstSize", VAR_ATMMaximumBurstSize, VAR_IN},
      {"NewATMSustainableCellRate", VAR_ATMSustainableCellRate, VAR_IN},      
      { 0 }
   }
};

static Action _SetDSLLinkType = { 
   "SetDSLLinkType", SetDSLLinkType,
   (Param [])    {
      {"NewLinkType", VAR_LinkType, VAR_IN},
      { 0 }
   }
};

static Action _GetStatistics = { 
   "GetStatistics", GetStatisticsWANDSL,
   (Param [])    {
      {"NewATMTransmittedBlocks", VAR_ATMTransmittedBlocks, VAR_OUT},
      {"NewATMReceivedBlocks", VAR_ATMReceivedBlocks, VAR_OUT},
      {"NewAAL5CRCErrors", VAR_AAL5CRCErrors, VAR_OUT},
      {"NewATMCRCErrors", VAR_ATMCRCErrors, VAR_OUT},
      { 0 }
   }
};

static PAction Actions[] = {
   &_SetEnable,
   &_GetInfo,
   &_GetDSLLinkInfo,
   &_SetDestinationAddress,
   &_GetDestinationAddress,
   &_SetATMEncapsulation,
   &_GetATMEncapsulation,
   &_SetATMQoS,
   &_SetDSLLinkType,
//   &_GetStatistics,
   NULL
};


ServiceTemplate Template_WANDSLLinkConfig = {
   "WANDSLLinkConfig:1",
   NULL,
   WANDSLLinkConfig_GetVar,    /* state variable handler */
   NULL,  /* SVCXML */
   ARRAYSIZE(StateVariables)-1, 
   StateVariables,
   Actions,
   0,
   "urn:dslforum-org:serviceId:WANDSLLinkConfig"
};

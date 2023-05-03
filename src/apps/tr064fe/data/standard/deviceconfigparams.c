#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "deviceconfigparams.h"

static VarTemplate StateVariables[] = { 
    { "PersistentData", "", VAR_EVENTED|VAR_STRING },   
    { "ConfigFile", "" ,VAR_EVENTED|VAR_STRING},
    { "A_ARG_TYPE_Status", "ChangesApplied", VAR_EVENTED|VAR_STRING},
    { NULL }
};

static Action _FactoryReset = {
    "FactoryReset", Tr64FactoryReset,
   (Param []) {
       { 0 }
    }
};

static Action _Reboot = {
    "Reboot", Tr64Reboot,
   (Param []) {
       { 0 }
    }
};


static PAction Actions[] = {
   &_FactoryReset,
   &_Reboot,
   NULL
};


ServiceTemplate Template_DeviceConfig = {
    "DeviceConfig:1",
    DeviceConfig_Init, 
    DeviceConfig_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions,
    0,
    "urn:dslforum-org:service:DeviceConfig"
};

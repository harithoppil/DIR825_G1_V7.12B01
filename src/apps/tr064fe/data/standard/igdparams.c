#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

extern int IGDevice_Init(PDevice igdev, device_state_t state);
extern void igd_xml(PDevice pdev, UFILE *up);
extern ServiceTemplate Template_Layer3Forwarding;
extern ServiceTemplate Template_WANCommonInterfaceConfig;
extern ServiceTemplate Template_WANCableLinkConfig;
extern ServiceTemplate Template_WANETHLinkConfig;
extern ServiceTemplate Template_WANDSLLinkConfig;
extern ServiceTemplate Template_WANPPPConnection;
extern ServiceTemplate Template_WANIPConnection;
extern ServiceTemplate Template_LANHostConfigManagement;
extern ServiceTemplate Template_LANEthernetInterfaceConfig;
extern ServiceTemplate Template_WANDSLConnMgt;
extern ServiceTemplate Template_IPPingConfig;
#ifdef USB
extern ServiceTemplate Template_LANUSBInterfaceConfig;
#endif
#ifdef WIRELESS
extern ServiceTemplate Template_WLANConfig;
#endif
extern ServiceTemplate Template_DeviceInfo;
extern ServiceTemplate Template_LANHosts;
extern ServiceTemplate Template_MgtServer;
extern ServiceTemplate Template_WANDSLInterfaceConfig;
extern ServiceTemplate Template_DeviceConfig;
extern ServiceTemplate Template_LANConfigSecurity ;
extern ServiceTemplate Template_WANETHInterfaceConfig;
extern ServiceTemplate Template_TimeServer;
extern ServiceTemplate Template_Layer2Bridging;
extern ServiceTemplate Template_QueueManagement;
/** All devices are supported: InternetGatewayDevice, LANDevice and WANDevice (and sub-devices).
 *  Services are selected in make Menuconfig:
 *     IGD's services:
 *        Layer3Forwarding, DeviceInfo, DeviceConfig, LANConfigSecurity, ManagementServer, Time,
 *        UserInterface.
 *     LANDevice's services:
 *        LANHostConfigMgmt, LANEthernetInterfaceConfig, WLANConfiguration, LANUSBInterfaceConfig,
 *        Hosts.
 *     WANDevice's services:
 *        WANCommonInterfaceConfig, WANDSLInterfaceConfig, WANEthernetInterfaceConfig,
 *        WANDSLConnectionManagement, WANDSLDiagnostics.
 *        WANConnectionDevice's serives:
 *           WANPOTSLinkConfig, WANDSLLinkConfig, WANCableLinkConfig, WANEthernetLinkConfig,
 *           WANIPConnection, WANPPPConnection.
 */

/** IGD's services
 */

PServiceTemplate svcs_igd[] = { 
#ifdef INCLUDE_LAYER3
    &Template_Layer3Forwarding,
#endif
#ifdef INCLUDE_DEVICEINFO 
    &Template_DeviceInfo,
#endif
#ifdef INCLUDE_DEVICECONFIG
    &Template_DeviceConfig,
#endif
#ifdef INCLUDE_LANCONFIGSECURITY
    &Template_LANConfigSecurity,
#endif
#ifdef INCLUDE_MANAGEMENTSERVER 
    &Template_MgtServer,
#endif
#ifdef INCLUDE_TIME
    &Template_TimeServer,
#endif
#ifdef INCLUDE_USERINTERFACE
#endif
#ifdef INCLUDE_QUEUEMANAGEMENT
    &Template_QueueManagement,
#endif
#ifdef INCLUDE_LAYER2BRIDGE
    &Template_Layer2Bridging,
#endif
#ifdef INCLUDE_IPPINGDIAG
    &Template_IPPingConfig,
#endif
};


/* WAN device template */
PServiceTemplate svcs_wandevice[] = { 
#ifdef INCLUDE_WANCOMMONINTERFACE
   &Template_WANCommonInterfaceConfig, 
#endif
#ifdef INCLUDE_WANDSLINTERFACE
   &Template_WANDSLInterfaceConfig,
#endif
#ifdef INCLUDE_WANETHERNETCONFIG
   &Template_WANETHInterfaceConfig,
#endif
#ifdef INCLUDE_WANDSLCONNECTIONMGMT
   &Template_WANDSLConnMgt,
#endif
#ifdef INCLUDE_WANDSLDIAGNOSTICS
#endif
};

/** WAN's services
 */
PServiceTemplate svcs_wanconnection[] = { 

};

/** LAN's services
 */
PServiceTemplate svcs_landevice[] = { 

};

/** WAN's sub devices
*/

DeviceTemplate subdevs_wandevice[] = { 
   {
      "urn:schemas-upnp-org:device:WANConnectionDevice:",
      "WANCONNECTION",
      NULL, /* PFDEVINIT */
      NULL, /* PFDEVXML */
      ARRAYSIZE(svcs_wanconnection), svcs_wanconnection 
   }
};

DeviceTemplate IGDeviceTemplate = {
   "urn:schemas-upnp-org:device:InternetGatewayDevice:1",
   "ROOTUDN",
    (PFDEVINIT)(void*)IGDevice_Init,      /* PFDEVINIT */
   NULL,          /* PFDEVXML */
   ARRAYSIZE(svcs_igd), svcs_igd
};


DeviceTemplate LANDeviceTemplate = {
    "urn:schemas-upnp-org:device:LANDevice:1",
    "LANDEVICEUDN",
    LANDevice_Init,     /* PFDEVINIT */
    NULL, /* PFDEVXML */
    ARRAYSIZE(svcs_landevice), svcs_landevice 
};

DeviceTemplate WANDeviceTemplate = {
   "urn:schemas-upnp-org:device:WANDevice:1",
   "WANDEVICEUDN",
   WANDevice_Init,     /* PFDEVINIT */
   NULL,         /* PFDEVXML */
   ARRAYSIZE(svcs_wandevice), 
   svcs_wandevice, 
};


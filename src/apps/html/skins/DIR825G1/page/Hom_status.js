/*  JavaScript Document:Hom_status  */
//deficeinfo
var G_CurrentLocalTime = "<?get :InternetGatewayDevice.Time.CurrentLocalTime?>";
var G_FirmwareVersion = "<?get :InternetGatewayDevice.DeviceInfo.ModemFirmwareVersion?>";
var G_DeviceUpTime = "<?get :InternetGatewayDevice.DeviceInfo.UpTime?>";

var wan_conn_run_time = 0;
var WlanEnable=[];
WlanEnable[0] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.Enable?>";
WlanEnable[1] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.Enable?>";
WlanEnable[2] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.Enable?>";
WlanEnable[3] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.Enable?>";

//lan
var G_lan_info = []
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement. "DHCPServerEnable IPInterface.1.IPInterfaceIPAddress IPInterface.1.IPInterfaceSubnetMask"
`	var G_lan_dhcp_enable 		= "$01";
	var G_lan_ip_addr     		= "$02";
	var G_lan_mask        		= "$03";
`?>
var G_lan_mac = "<?get :InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1.MACAddress ?>";
var G_wan_mac = "<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.X_TWSZ-COM_MACAddress ?>";
var G_wan_clone_mac = "<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.X_TWSZ-COM_CloneMACAddress ?>";
var G_cable_status = "<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANEthernetLinkConfig.EthernetLinkStatus ?>";
//wan
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<? if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "Enable Name NATEnabled ExternalIPAddress SubnetMask DefaultGateway DNSOverrideAllowed DNSServers AddressingType ConnectionType X_TWSZ-COM_NATType X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection X_TWSZ-COM_DomainName MaxMTUSize X_TWSZ-COM_Hostname X_TWSZ-COM_UsrDNSServers ConnectionStatus ConnectionTime"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.$00.";
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['Name'] 			= "$02";	//Name
			G_WANConn['NATEnabled'] 		= "$03";	//NATEnabled
			G_WANConn['ExternalIPAddress'] 		= "$04";	//ExternalIPAddress
			G_WANConn['SubnetMask'] 		= "$05";	//SubnetMask
			G_WANConn['DefaultGateway'] 		= "$06";	//DefaultGateway
			G_WANConn['DNSOverrideAllowed'] 	= "$07";	//DNSOverrideAllowed
			G_WANConn['DNSServers'] 		= "$08";	//DNSServers
			G_WANConn['AddressingType'] 		= "$09";	//AddressingType
			G_WANConn['ConnectionType'] 		= "$0a";	//ConnectionType
			G_WANConn['NATType'] 			= "$0b";	//X_TWSZ-COM_NATType
			G_WANConn['ServiceList'] 		= "$0c";	//X_TWSZ-COM_ServiceList
			G_WANConn['ProtocolType'] 		= "$0d";	//X_TWSZ-COM_ProtocolType
			G_WANConn['RouteProtocolRx'] 		= "$0e";	// RouteProtocolRx
			G_WANConn['RipDirection'] 		= "$0f";	// RipDirection
			G_WANConn['DomainName'] 		= "$0g";	// X_TWSZ-COM_DomainName
			G_WANConn['MaxMTUSize'] 		= "$0h";	// MaxMTUSize
			G_WANConn['Hostname'] 			= "$0i";	// X_TWSZ-COM_Hostname
			G_WANConn['X_TWSZ-COM_UsrDNSServers']  = "$0j";	//X_TWSZ-COM_UsrDNSServers
			G_WANConn['ConnectionStatus'] 			= "$0k";	// ConnectionStatus
			G_WANConn['ConnectionTime'] 			= "$0l";	// ConnectionTime
		`?>
	`?>

	<? if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "Enable NATEnabled Username PPPAuthenticationProtocol ConnectionTrigger IdleDisconnectTime MaxMRUSize MaxMTUSize PPPLCPEchoRetry X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_NATType PPPLCPEcho X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection ConnectionType DNSServers X_TWSZ-COM_UsrDNSServers X_TWSZ-COM_VPN_CLIENT X_TWSZ-COM_VPN_NETMASK X_TWSZ-COM_VPN_GATEWAY X_TWSZ-COM_VPN_SERVER X_TWSZ-COM_VPN_INTERFACE_REF PPPoEServiceName DNSOverrideAllowed X_TWSZ-COM_VPN_ADDR_MODE ConnectionStatus ExternalIPAddress ConnectionTime RemoteIPAddress LocalNetMask"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.$00.";	//Path		
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['NATEnabled'] 		= "$02";	//NATEnabled
			G_WANConn['Username'] 			= "$03";	//Username
			G_WANConn['Password'] 			= "**********";	//Password
			G_WANConn['PPPAuthenticationProtocol'] 	= "$04";	//PPPAuthenticationProtocol
			G_WANConn['ConnectionTrigger'] 		= "$05";	//ConnectionTrigger
			G_WANConn['IdleDisconnectTime'] 	= "$06";	//IdleDisconnectTime
			G_WANConn['MaxMRUSize'] 		= "$07";	//MaxMRUSize
			G_WANConn['MaxMTUSize'] 		= "$08";	//MaxMTUSize
			G_WANConn['PPPLCPEchoRetry'] 		= "$09";	//PPPLCPEchoRetry
			G_WANConn['StaticIPAddress'] 		= "$0a";	//X_TWSZ-COM_StaticIPAddress
			G_WANConn['NATType'] 			= "$0b";	//X_TWSZ-COM_NATType
			G_WANConn['PPPLCPEcho'] 		= "$0c";	//PPPLCPEcho
			G_WANConn['ServiceList'] 		= "$0d";	//X_TWSZ-COM_ServiceList
			G_WANConn['ProtocolType'] 		= "$0e";	//X_TWSZ-COM_ProtocolType
			G_WANConn['RouteProtocolRx'] 		= "$0f";	// RouteProtocolRx
			G_WANConn['RipDirection'] 		= "$0g";	// RipDirection
			G_WANConn['ConnectionType'] 		= "$0h";	//ConnectionType
			G_WANConn['DNSServers']                = "$0i";	//
			G_WANConn['X_TWSZ-COM_UsrDNSServers']  = "$0j";	//
			G_WANConn['X_TWSZ-COM_VPN_CLIENT']     = "$0k";	//
			G_WANConn['X_TWSZ-COM_VPN_NETMASK']    = "$0l";	//
			G_WANConn['X_TWSZ-COM_VPN_GATEWAY']    = "$0m";	//
			G_WANConn['X_TWSZ-COM_VPN_SERVER']     = "$0n";	//
			G_WANConn['X_TWSZ-COM_VPN_INTERFACE_REF']  = "$0o";	//X_TWSZ-COM_VPN_INTERFACE_REF
			G_WANConn['PPPoEServiceName']               = "$0p"; //PPPoEServiceName
			G_WANConn['DNSOverrideAllowed'] 	= "$0q";	//DNSOverrideAllowed
			G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE'] 	= "$0r";	//X_TWSZ-COM_VPN_ADDR_MODE
			G_WANConn['ConnectionStatus'] 			= "$0s";	// ConnectionStatus
			G_WANConn['ExternalIPAddress'] 			= "$0t";	// ExternalIPAddress
			G_WANConn['ConnectionTime'] 			= "$0u";	// ConnectionTime
			G_WANConn['RemoteIPAddress'] 			= "$0v";	// RemoteIPAddress
			G_WANConn['LocalNetMask'] 			= "$0w";	// LocalNetMask
		`?>
	`?>
`?>
//wlan
var Radio_list=[];
var m=0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_Radio. "Enable Channel AutoChannelEnable Standard  OperatingChannelBandwidth ChannelsInUse"
`	Radio_list[m]=[];
	Radio_list[m][0] 			 = "InternetGatewayDevice.X_TWSZ-COM_Radio.$00.";//path
	Radio_list[m][1]             = "$01";//Enable
	Radio_list[m][2]             = "$02";//Channel
	Radio_list[m][3]             = "$03";//AutoChannelEnable
	Radio_list[m][4]             = "$04";//Standard
	Radio_list[m][5]	         = "$05";//OperatingChannelBandwidth
	Radio_list[m][6]	         = "$06";//ChannelsInUse
	m++;
`?>
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "X_TWSZ-COM_ScheduleListName SSID SSIDAdvertisementEnabled BeaconType WEPEncryptionLevel WEPKeyIndex BasicAuthenticationMode BasicEncryptionModes WPAAuthenticationMode WPAEncryptionModes IEEE11iAuthenticationMode IEEE11iEncryptionModes X_TWSZ-COM_PSKExpression PreSharedKey.1.KeyPassphrase PreSharedKey.1.PreSharedKey X_TWSZ-COM_WPAGroupRekey WEPKey.1.WEPKey X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey WPS.Enable Enable BSSID WPS.ConfigurationState"
        `	
            G_Wireless[n] = ['InternetGatewayDevice.LANDevice.1.WLANConfiguration.$00.', //path
                     '$01',//X_TWSZ-COM_ScheduleListName
                     '$02', //SSID
                     '$03', //SSIDAdvertisementEnabled
                     '$04',//BeaconType
                     '$05',//WEPEncryptionLevel
                     '$06',//WEPKeyIndex
                     '$07', //BasicAuthenticationMode
					 '$08', //BasicEncryptionModes
					 '$09', //WPAAuthenticationMode
					 '$0a', //WPAEncryptionModes
					 '$0b', //IEEE11iAuthenticationMode
					 '$0c', //IEEE11iEncryptionModes
					 '$0d', //X_TWSZ-COM_PSKExpression
					 '$0e', //KeyPassphrase
					 '$0f', //PreSharedKey
					 '$0g', //X_TWSZ-COM_WPAGroupRekey
					 '$0h', //WEPKey.1.WEPKey
					 '$0i', //X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress
					 '$0j', //X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port
					 '$0k', //X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey
					 '$0l', //X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress
					 '$0m', //X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port
					 '$0n', //X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey
					 '$0o', //WPS.Enable
					 '$0p', //Enable
					 '$0q', //BSSID
					 '$0r' //WPS.ConfigurationState
                     ];
    n++;
`?>

var LanHosts = [];
var t = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "HostName MACAddress IPAddress LeaseTimeRemaining VendorClassID AddressSource"
             `	<?if eq `DHCP` `<?echo $26?>` 	
		`	LanHosts[m] = [];
			LanHosts[m][0] = "<?echo $22?>";
			LanHosts[m][1] = "<?echo $23?>";
			LanHosts[m][2] = "<?echo $21?>"=="ZFc1cmJtOTNiZz09"?"unknown":strAnsi2Unicode((Base64.Decode("<?echo $21?>")));
			++m;
			`?>
		`?>
	`?>
`?>



// IGMP
var G_ProxyTables = [];
var m = 0;
<?mget :InternetGatewayDevice.X_TWSZ-COM_IGMPProxy. "Enabled IGMPTableNumberOfEntries"
`   <?if  et $11 1
	`	<?objget :InternetGatewayDevice.X_TWSZ-COM_IGMPProxy.IGMPTable. "GroupAddress Interface State"
		`	G_ProxyTables[m] = [];
			G_ProxyTables[m][0] = IntStr2IP2("$01");
			//G_ProxyTables[m][1] ="$02?";
			//G_ProxyTables[m][2] = "$03?";
			++m;
		`?>
	`?>
`?>

var G_TunConn = [];
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel. "Activated Mode Dynamic RemoteIpv6Address BorderRelayAddress ConnStatus ConnectionTime"
`
	G_TunConn['Path'] 		= ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.$00.";
	G_TunConn['Activated']   = "$01";	//Activated
	G_TunConn['Mode'] 		= "$02";	//Mode
	G_TunConn['Dynamic']   = "$03";	//Dynamic
	G_TunConn['RemoteIpv6Address']   = "$04";	//RemoteIpv6Address
	G_TunConn['BorderRelayAddress']   = "$05";	//BorderRelayAddress
	G_TunConn['ConnStatus']   = "$06";	//BorderRelayAddress
	G_TunConn['ConnectionTime']   = "$07";	//BorderRelayAddress
`?>
// linsd add for dual access
var G_SecondWan = [];
<?mget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1. "X_TWSZ-COM_DualAccessEnable X_TWSZ-COM_DualAccessAddressType X_TWSZ-COM_DualAccessIPAddress X_TWSZ-COM_DualAccessSubnetMask X_TWSZ-COM_DualAccessStatus"
`
	G_SecondWan['X_TWSZ-COM_DualAccessEnable']  = "$01";	//X_TWSZ-COM_DhcpUseUnicast
	G_SecondWan['X_TWSZ-COM_DualAccessAddressType']  = "$02";	//X_TWSZ-COM_DhcpUseUnicast
	G_SecondWan['X_TWSZ-COM_DualAccessIPAddress']  = "$03";	//X_TWSZ-COM_DhcpUseUnicast
	G_SecondWan['X_TWSZ-COM_DualAccessSubnetMask']  = "$04";	//X_TWSZ-COM_DhcpUseUnicast
	G_SecondWan['X_TWSZ-COM_DualAccessStatus']  = "$05";	
`?>


/*�����ַ�������ΪIP��ַ*/
/*"10000E0" ->224.0.0.1*/
function IntStr2IP(str_ip)
{
	var str;
	var tt = new Array();
	var num = parseInt(str_ip, 16);

	tt[3] = (num >>> 24) >>> 0;
    tt[2] = ((num << 8) >>> 24) >>> 0;
    tt[1] = (num << 16) >>> 24;
    tt[0] = (num << 24) >>> 24;
	str = String(tt[0]) + "." + String(tt[1]) + "." + String(tt[2]) + "." + String(tt[3]);
	return str;
} 

/*�����ַ�������ΪIP��ַ*/
/*"EF010203" ->239.1.2.3*/
function IntStr2IP2(str_ip)
{
	var str;
	var tt = new Array();
	var num = parseInt(str_ip, 16);

	tt[0] = (num >>> 24) >>> 0;
    tt[1] = ((num << 8) >>> 24) >>> 0;
    tt[2] = (num << 16) >>> 24;
    tt[3] = (num << 24) >>> 24;
	str = String(tt[0]) + "." + String(tt[1]) + "." + String(tt[2]) + "." + String(tt[3]);
	return str;
} 


//协议类型
function findProtocol(){	
	var ConnType = G_WANConn['Path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";
	if(ConnType == "IP"){
		if(G_WANConn['ConnectionType'] == "IP_Bridged"){
			return "Bridge";
		}else{
			if(G_WANConn['AddressingType'] == 'Static')
				return "Static IP";
			else
				return "DHCP Client";
		}		
	}else if(ConnType == "PPP"){
		if(G_WANConn['ConnectionType']=="PPTP_Relay")
		{
			return "PPTP";
		}else if(G_WANConn['ConnectionType']=="L2TP_Relay"){
			return "L2TP";			
		}else{
			return "PPPoE";
		}
	}
}


function DHCP_Renew()
{
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 	: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	},true);
	$F(':' + G_WANConn['Path'] + 'X_TWSZ-COM_DhcpServerReleaseRenew', 'Renew');
	$('uiPostForm').submit();
	$('st_wan_dhcp_renew').disabled= true;
}

function DHCP_Release()
{
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 	: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	},true);
	$F(':' + G_WANConn['Path'] + 'X_TWSZ-COM_DhcpServerReleaseRenew', 'Release');
	$('uiPostForm').submit();
	$('st_wan_dhcp_release').disabled= true;
}

function PPP_Connect() {
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 	: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	var DevicePath = G_WANConn['Path'];
	$F(":" + DevicePath + "X_TWSZ-COM_ConnectionAction", 	"Connect");
	$('uiPostForm').submit();
	$('st_wan_ppp_connect').disabled= true;	
}

function PPP_Disconnect() {
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 	: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	var DevicePath = G_WANConn['Path'];
	$F(":" + DevicePath + "X_TWSZ-COM_ConnectionAction", 	"Disconnect");
	$('uiPostForm').submit();	
	$('st_wan_ppp_disconnect').disabled= true;
}

function initial_DSLITE() {

	if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] == "4in6")
	{
		$("wan_ethernet_dslite_block").style.display = "";
		$("wan_ethernet_block").style.display = "none";	

	var WanConnected = (G_TunConn['ConnStatus'] == "Connected" ? true : false);
	var wan_uptime_sec = "0";
	var wan_uptime_min = "0";
	var wan_uptime_hour = "0";
	var wan_uptime_day = "0";
	if(WanConnected && G_TunConn['ConnectionTime'] != "-1")
	{
		wan_conn_run_time = G_DeviceUpTime - G_TunConn['ConnectionTime'];
		wan_uptime_sec = wan_conn_run_time%60;
		wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
		wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
		wan_uptime_day = Math.floor(wan_conn_run_time/86400);
	}
	
	setJSONValue({
				'st_dslite_networkstatus'	: WanConnected ? SEcode[8004] : SEcode[8005],
				'st_dslite_wancable' 		: G_cable_status=="Up" ? SEcode[8004] : SEcode[8005],
				'st_dslite_wantype' 		: "DS-Lite",
				'st_dslite_connection_uptime'	: wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026,
				'st_dslite_wan_mac' 		: G_wan_clone_mac == "" ? G_wan_mac : G_wan_clone_mac,
				'st_aftrserver' 			: G_TunConn['RemoteIpv6Address'],
				'st_dslite_dhcp6opt' 		    : G_TunConn['Dynamic'] = "1" ? data_languages.Public.innerHTML.Public001 : data_languages.Public.innerHTML.Public002
	});

	}


}

function initial_WAN() {

	if (G_WANConn['Enable'] == "0")
	{
		return ;
	}
	var WanConnected = (G_WANConn['ConnectionStatus'] == "Connected" ? true : false);

	if(G_WANConn['ConnectionTrigger'] == "OnDemand" && (G_WANConn['ExternalIPAddress'] == "10.64.64.64"||G_WANConn['ExternalIPAddress'] == ""))
		WanConnected = false;
	if(findProtocol()=="DHCP Client")
	{		
		$("st_wan_dhcp_action").style.display = "";
		$("st_wan_ppp_action").style.display = "none";
		if(WanConnected)
		{
			$("st_wan_dhcp_renew").disabled = true;
			$("st_wan_dhcp_release").disabled = false;
		}
		else
		{
			$("st_wan_dhcp_renew").disabled = false;
			$("st_wan_dhcp_release").disabled = true;
		}			
	}
	else if(findProtocol()=="Static IP")
	{
		$("st_wan_dhcp_action").style.display = "none";
		$("st_wan_ppp_action").style.display = "none";
	}
	else
	{
		$("st_wan_dhcp_action").style.display = "none";
		$("st_wan_ppp_action").style.display = "";
		if(WanConnected)
		{
			$("st_wan_ppp_connect").disabled = true;
			$("st_wan_ppp_disconnect").disabled = false;
		}
		else
		{
			$("st_wan_ppp_connect").disabled = false;
			$("st_wan_ppp_disconnect").disabled = true;
		}	
	}
	var wan_uptime_sec = "0";
	var wan_uptime_min = "0";
	var wan_uptime_hour = "0";
	var wan_uptime_day = "0";
	if(WanConnected && G_WANConn['ExternalIPAddress'] != "10.64.64.64" && G_WANConn['ConnectionTime'] != "-1")
	{
		wan_conn_run_time = G_DeviceUpTime - G_WANConn['ConnectionTime'];
		wan_uptime_sec = wan_conn_run_time%60;
		wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
		wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
		wan_uptime_day = Math.floor(wan_conn_run_time/86400);
	}
	var wan_gw = G_WANConn['DefaultGateway'];
	var wan_mask = G_WANConn['SubnetMask']; 
	if(findProtocol()=="PPTP"||findProtocol()=="L2TP" || findProtocol()=="PPPoE")
	{
		$("lang_wan_ip").innerHTML=data_languages.Hom_status.innerHTML.lang_local_address;
		$("lang_wan_gw").innerHTML=data_languages.Hom_status.innerHTML.lang_peer_address;
		wan_gw = G_WANConn['RemoteIPAddress'];
		wan_mask = G_WANConn['LocalNetMask'];
	}
	else
	{
		$("lang_wan_ip").innerHTML=data_languages.Hom_status.innerHTML.lang_wan_ip;
		$("lang_wan_gw").innerHTML=data_languages.Hom_status.innerHTML.lang_wan_gw;
	}
	if(findProtocol() == "PPPoE" && G_WANConn['StaticIPAddress'] != "")
	G_WANConn['ExternalIPAddress'] = G_WANConn['StaticIPAddress'];
	setJSONValue({
				'st_networkstatus'	: WanConnected ? SEcode[8004] : SEcode[8005],
				'st_wancable' 		: G_cable_status=="Up" ? SEcode[8004] : SEcode[8005],
				'st_wantype' 		: findProtocol(),
				'st_connection_uptime'	: wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026,
				'st_wan_mac' 		: G_wan_clone_mac == "" ? G_wan_mac : G_wan_clone_mac,
				'st_wanipaddr' 		    : (WanConnected ? G_WANConn['ExternalIPAddress'] : "0.0.0.0") || "0.0.0.0",
				'st_wannetmask' 		: (WanConnected ? wan_mask : "0.0.0.0") || "0.0.0.0",
				'st_wangateway' 	: (WanConnected ? wan_gw : "0.0.0.0") || "0.0.0.0",
				'st_wanDNSserver' 		: (WanConnected ? (G_WANConn['DNSOverrideAllowed'] == "0" ? G_WANConn['DNSServers'].split(",")[0] :G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(",")[0]) : "0.0.0.0") || "0.0.0.0",
				'st_wanDNSserver2' 		: (WanConnected ? (G_WANConn['DNSOverrideAllowed'] == "0" ? G_WANConn['DNSServers'].split(",")[1] :G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(",")[1]) : "0.0.0.0") || "0.0.0.0"
	});

	if(findProtocol() == "PPPoE" && G_SecondWan['X_TWSZ-COM_DualAccessEnable'] == "1")
	{	
		$("wan_secondwan_block").style.display = "";
		var WanConnected2 = (G_SecondWan['X_TWSZ-COM_DualAccessStatus'] == "Connected" ? true : false);		
		setJSONValue({
					'st_secondwan_networkstatus'	: WanConnected2 ? SEcode[8004] : SEcode[8005],
					'st_secondwan_wantype' 		: G_SecondWan['X_TWSZ-COM_DualAccessAddressType'] == "DHCP"? "Dynamic":"Static",
					'st_secondwan_wanipaddr' 		    : (WanConnected2 ? G_SecondWan['X_TWSZ-COM_DualAccessIPAddress'] : "0.0.0.0") || "0.0.0.0",
					'st_secondwan_wannetmask' 		: (WanConnected2 ? G_SecondWan['X_TWSZ-COM_DualAccessSubnetMask'] : "0.0.0.0") || "0.0.0.0"
		});
	}	
	
}

//WLAN
function initial_WLAN(){
	for(var i=0; i<2; i+=1)
	{
		var str_Aband=(i==0)?"":"_Aband";
		$("st_wireless_radio"+str_Aband).innerHTML = (Radio_list[i][1]=="1" && WlanEnable[i]=="1")?data_languages.Public.innerHTML.Public001:data_languages.Public.innerHTML.Public002;
		var configred= G_Wireless[i][27]=="Configured"?data_languages.Public.innerHTML.Public003:data_languages.Public.innerHTML.Public004;
		$("st_WPS_status"+str_Aband).innerHTML = (G_Wireless[i][24]=="1")?(data_languages.Public.innerHTML.Public001+"/"+configred):(data_languages.Public.innerHTML.Public002+"/"+configred);
		$('st_SSID'+str_Aband).innerHTML = G_Wireless[i][2];
		$('st_wireless_mac'+str_Aband).innerHTML = G_Wireless[i][26];
		switch (Radio_list[i][4])
		{
		   case "bgn":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11n, 802.11g "+data_languages.Public.innerHTML.Public028+" 802.11b";
				break;
		   case "bg":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11g "+data_languages.Public.innerHTML.Public028+" 802.11b";
				break;
		   case "ng":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11n "+data_languages.Public.innerHTML.Public028+" 802.11g";
				break;
		   case "na":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11n "+data_languages.Public.innerHTML.Public028+" 802.11a";
				break;
		   case "aanac":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11ac, 802.11n "+data_languages.Public.innerHTML.Public028+" 802.11a";
		   break;
		   case "anac":
		   		$("st_80211mode"+str_Aband).innerHTML  = data_languages.Public.innerHTML.Public027 +" 802.11ac "+data_languages.Public.innerHTML.Public028+" 802.11n";
		   break;
		   case "ac":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11ac "+data_languages.Public.innerHTML.Public029;
				break;
		   case "n":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11n "+data_languages.Public.innerHTML.Public029;
				break;
		   case "5n":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11n "+data_languages.Public.innerHTML.Public029;
				break;
		   case "b":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11b "+data_languages.Public.innerHTML.Public029;
				break;
		   case "g":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11g "+data_languages.Public.innerHTML.Public029;
				break;
		   case "a":
		   		$("st_80211mode"+str_Aband).innerHTML  = "802.11a "+data_languages.Public.innerHTML.Public029;
				break;			
		}
		if((Radio_list[i][1]=="1") && (WlanEnable[i]=="1") && (G_Wireless[i][25]=="1"))
			$("st_Channel"+str_Aband).innerHTML = Radio_list[i][2]=="0"?Radio_list[i][6]:Radio_list[i][2];
		else
			$("st_Channel"+str_Aband).innerHTML = data_languages.Public.innerHTML.Public030;
		if(Radio_list[i][5]=="20O40O80")
		{
			$("st_Channel_Width"+str_Aband).innerHTML = "20/40/80MHz";
		}
		else if(Radio_list[i][5]=="20O40")
		{
			$("st_Channel_Width"+str_Aband).innerHTML = "20/40MHz";	
		}
		else
		{
			$("st_Channel_Width"+str_Aband).innerHTML = "20MHz";
		}
		switch(G_Wireless[i][4]){
			case 'None' : {
				$("st_security"+str_Aband).innerHTML = data_languages.Public.innerHTML.Public022;
				break;
			}
			case 'Basic' : {
				$("st_security"+str_Aband).innerHTML = "WEP";				
				break;
			}
			case 'WPA' : {
				if(G_Wireless[i][9]=="PSKAuthentication")
				$("st_security"+str_Aband).innerHTML = "WPA-PSK";
				else
				$("st_security"+str_Aband).innerHTML = "WPA-EAP";
				break;
			}
			case '11i' : {
				if(G_Wireless[i][11]=="PSKAuthentication")
				$("st_security"+str_Aband).innerHTML = "WPA2-PSK";
				else
				$("st_security"+str_Aband).innerHTML = "WPA2-EAP";
				break;
			}
			case 'WPAand11i' : {			
				if(G_Wireless[i][11]=="PSKAuthentication")
				$("st_security"+str_Aband).innerHTML = "WPA/WPA2-PSK";
				else
				$("st_security"+str_Aband).innerHTML = "WPA/WPA2-EAP";				
				break;
			}  
		}
		$("gz_st_wireless_radio"+str_Aband).innerHTML = (Radio_list[i][1]=="1"&&G_Wireless[i+2][25]=="1")?data_languages.Public.innerHTML.Public001:data_languages.Public.innerHTML.Public002;
		$('gz_st_SSID'+str_Aband).innerHTML = G_Wireless[i+2][2];
		switch(G_Wireless[i+2][4]){
			case 'None' : {
				$("gz_st_security"+str_Aband).innerHTML = data_languages.Public.innerHTML.Public022;
				break;
			}
			case 'Basic' : {
				$("gz_st_security"+str_Aband).innerHTML = "WEP";				
				break;
			}
			case 'WPA' : {
				if(G_Wireless[i+2][9]=="PSKAuthentication")
				$("gz_st_security"+str_Aband).innerHTML = "WPA-PSK";
				else
				$("gz_st_security"+str_Aband).innerHTML = "WPA-EAP";
				break;
			}
			case '11i' : {
				if(G_Wireless[i+2][11]=="PSKAuthentication")
				$("gz_st_security"+str_Aband).innerHTML = "WPA2-PSK";
				else
				$("gz_st_security"+str_Aband).innerHTML = "WPA2-EAP";
				break;
			}
			case 'WPAand11i' : {			
				if(G_Wireless[i+2][11]=="PSKAuthentication")
				$("gz_st_security"+str_Aband).innerHTML = "WPA/WPA2-PSK";
				else
				$("gz_st_security"+str_Aband).innerHTML = "WPA/WPA2-EAP";				
				break;
			}  
		}
	}
}

function null_errorfunc()
{
	
	return;
}

function Ajax_handler(_text)
{
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		return;
	}
	var wan_uptime_sec = "0";
	var wan_uptime_min = "0";
	var wan_uptime_hour = "0";
	var wan_uptime_day = "0";
	var WanConnected ;
	
	if (G_WANConn['Enable'] == "0")
	{
		if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] == "4in6")
		{
			$("wan_ethernet_dslite_block").style.display = "";
			$("wan_ethernet_block").style.display = "none";	

			WanConnected = (G_TunConn['ConnStatus'] == "Connected" ? true : false);
			if(WanConnected && G_TunConn['ConnectionTime'] != "-1")
			{
				wan_conn_run_time = G_DeviceUpTime - G_TunConn['ConnectionTime'];
				wan_uptime_sec = wan_conn_run_time%60;
				wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
				wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
				wan_uptime_day = Math.floor(wan_conn_run_time/86400);
			}
		
			setJSONValue({
						'st_dslite_networkstatus'	: WanConnected ? SEcode[8004] : SEcode[8005],
						'st_dslite_wancable' 		: G_cable_status=="Up" ? SEcode[8004] : SEcode[8005],
						'st_dslite_wantype' 		: "DS-Lite",
						'st_dslite_connection_uptime'	: wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026,
						'st_dslite_wan_mac' 		: G_wan_clone_mac == "" ? G_wan_mac : G_wan_clone_mac,
						'st_aftrserver' 			: G_TunConn['RemoteIpv6Address'],
						'st_dslite_dhcp6opt' 		    : G_TunConn['Dynamic'] = "1" ? data_languages.Public.innerHTML.Public001 : data_languages.Public.innerHTML.Public002
			});
			setTimeout('get_wan_conn_time()', 5*1000); 
		}
		return ;
	}
	WanConnected = (G_WANConn['ConnectionStatus'] == "Connected" ? true : false);
	if(G_WANConn['ConnectionTrigger'] == "OnDemand" && (G_WANConn['ExternalIPAddress'] == "10.64.64.64"||G_WANConn['ExternalIPAddress'] == ""))
	//if(G_WANConn['ConnectionTrigger'] == "OnDemand" && G_WANConn['ExternalIPAddress'] == "10.64.64.64")
		WanConnected = false; //demand show disconnect
	if(findProtocol()=="DHCP Client")
	{		
		$("st_wan_dhcp_action").style.display = "";
		$("st_wan_ppp_action").style.display = "none";
		if(WanConnected)
		{
			$("st_wan_dhcp_renew").disabled = true;
			$("st_wan_dhcp_release").disabled = false;
		}
		else
		{
			$("st_wan_dhcp_renew").disabled = false;
			$("st_wan_dhcp_release").disabled = true;
		}			
	}
	else if(findProtocol()=="Static IP")
	{
		$("st_wan_dhcp_action").style.display = "none";
		$("st_wan_ppp_action").style.display = "none";
	}
	else
	{
		$("st_wan_dhcp_action").style.display = "none";
		$("st_wan_ppp_action").style.display = "";
		if(WanConnected)
		{
			$("st_wan_ppp_connect").disabled = true;
			$("st_wan_ppp_disconnect").disabled = false;
		}
		else
		{
			$("st_wan_ppp_connect").disabled = false;
			$("st_wan_ppp_disconnect").disabled = true;
		}	
	}

	if(WanConnected && G_WANConn['ExternalIPAddress'] != "10.64.64.64" && G_WANConn['ConnectionTime'] != "-1")
	{
		wan_conn_run_time = G_DeviceUpTime - G_WANConn['ConnectionTime'];
		wan_uptime_sec = wan_conn_run_time%60;
		wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
		wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
		wan_uptime_day = Math.floor(wan_conn_run_time/86400);
	}
	var wan_gw = G_WANConn['DefaultGateway'];
	var wan_mask = G_WANConn['SubnetMask']; 
	if(findProtocol()=="PPTP"||findProtocol()=="L2TP" || findProtocol()=="PPPoE")
	{
		$("lang_wan_ip").innerHTML=data_languages.Hom_status.innerHTML.lang_local_address;
		$("lang_wan_gw").innerHTML=data_languages.Hom_status.innerHTML.lang_peer_address;
		wan_gw = G_WANConn['RemoteIPAddress'];
		wan_mask = G_WANConn['LocalNetMask'];
	}
	else
	{
		$("lang_wan_ip").innerHTML=data_languages.Hom_status.innerHTML.lang_wan_ip;
		$("lang_wan_gw").innerHTML=data_languages.Hom_status.innerHTML.lang_wan_gw;
	}
	if(findProtocol() == "PPPoE" && G_WANConn['StaticIPAddress'] != "")
	G_WANConn['ExternalIPAddress'] = G_WANConn['StaticIPAddress'];
	setJSONValue({
				'st_networkstatus'	: WanConnected ? SEcode[8004] : SEcode[8005],
				'st_wancable' 		: G_cable_status=="Up" ? SEcode[8004] : SEcode[8005],
				'st_wantype' 		: findProtocol(),
				'st_connection_uptime'	: wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026,
				'st_wan_mac' 		: G_wan_clone_mac == "" ? G_wan_mac : G_wan_clone_mac,
				'st_wanipaddr' 		    : (WanConnected ? G_WANConn['ExternalIPAddress'] : "0.0.0.0") || "0.0.0.0",
				'st_wannetmask' 		: (WanConnected ? wan_mask : "0.0.0.0") || "0.0.0.0",
				'st_wangateway' 	: (WanConnected ? wan_gw : "0.0.0.0") || "0.0.0.0",
				'st_wanDNSserver' 		: (WanConnected ? (G_WANConn['DNSOverrideAllowed'] == "0" ? G_WANConn['DNSServers'].split(",")[0] :G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(",")[0]) : "0.0.0.0") || "0.0.0.0",
				'st_wanDNSserver2' 		: (WanConnected ? (G_WANConn['DNSOverrideAllowed'] == "0" ? G_WANConn['DNSServers'].split(",")[1] :G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(",")[1]) : "0.0.0.0") || "0.0.0.0"
	});
	setTimeout('get_wan_conn_time()', 5*1000); 
}
function get_wan_conn_time()
{
	var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js&var:page=*";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.get();
}
function uiOnload(){
	
	setJSONValue({
		'st_time' 		: G_CurrentLocalTime,
		'st_fw' 		: G_FirmwareVersion,
		'st_lan_mac' 		: G_lan_mac,
		'st_ip_address' 		: G_lan_ip_addr,
		'st_netmask' 		: G_lan_mask,
		'st_dhcpserver_enable' 	: G_lan_dhcp_enable == "1" ? SEcode[8000] : SEcode[8001]
	});
	initial_WLAN();
	initial_DSLITE();
	initial_WAN();
	
	setTimeout('get_wan_conn_time()', 0);
	
	$T('client_list',LanHosts);
	$T('igmp_groups',G_ProxyTables);
}
addListeners(uiOnload);

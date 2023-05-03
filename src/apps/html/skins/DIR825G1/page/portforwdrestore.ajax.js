G_Error	    = "<?error found?>"; 
G_Status	= "<?error status?>"; 
G_Error_Msg	= "<?error message?>";
G_ViewState	= "<?echo $var:CacheLastData?>";
G_NewObjIndex = "<?echo $var:newobjindex?>";

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

var G_Wan_Link_status="<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANEthernetLinkConfig.EthernetLinkStatus?>";
var G_CheckWanConnection 	= "<?get :InternetGatewayDevice.X_TWSZ-COM_Diagnostics.CheckWanConnection?>";
var G_WANConnectionType 	= "<?get :InternetGatewayDevice.X_TWSZ-COM_Diagnostics.Status.WANConnectionType?>";
G_IPAddress = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.IPInterfaceIPAddress ?>";
G_AjaxToken  = "<?echo $var:sys_Token?>";

var G_DeviceUpTime = "<?get :InternetGatewayDevice.DeviceInfo.UpTime?>";
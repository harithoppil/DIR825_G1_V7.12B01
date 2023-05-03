/* JavaScript Document */
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<? if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "Enable Name NATEnabled ExternalIPAddress SubnetMask DefaultGateway DNSOverrideAllowed DNSServers AddressingType ConnectionType X_TWSZ-COM_NATType X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection X_TWSZ-COM_DomainName MaxMTUSize X_TWSZ-COM_Hostname X_TWSZ-COM_UsrDNSServers X_TWSZ-COM_DhcpUseUnicast X_TWSZ-COM_DMZHost X_TWSZ-COM_DMZEnabled"
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
			G_WANConn['X_TWSZ-COM_DhcpUseUnicast']  = "$0k";	//X_TWSZ-COM_DhcpUseUnicast
			G_WANConn['X_TWSZ-COM_DMZHost']  = "$0l";	//X_TWSZ-COM_UsrDNSServers
			G_WANConn['X_TWSZ-COM_DMZEnabled']  = "$0m";	//X_TWSZ-COM_DhcpUseUnicast
			
		`?>
	`?>

	<? if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "Enable NATEnabled Username PPPAuthenticationProtocol ConnectionTrigger IdleDisconnectTime MaxMRUSize MaxMTUSize PPPLCPEchoRetry X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_NATType PPPLCPEcho X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection ConnectionType DNSServers X_TWSZ-COM_UsrDNSServers X_TWSZ-COM_VPN_CLIENT X_TWSZ-COM_VPN_NETMASK X_TWSZ-COM_VPN_GATEWAY X_TWSZ-COM_VPN_SERVER X_TWSZ-COM_VPN_INTERFACE_REF PPPoEServiceName DNSOverrideAllowed X_TWSZ-COM_VPN_ADDR_MODE X_TWSZ-COM_ScheduleListName LocalNetMask X_TWSZ-COM_DMZHost X_TWSZ-COM_DMZEnabled"
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
			G_WANConn['X_TWSZ-COM_ScheduleListName'] 	= "$0s";	//X_TWSZ-COM_ScheduleListName
			G_WANConn['LocalNetMask'] 			= "$0t";	// LocalNetMask
			G_WANConn['X_TWSZ-COM_DMZHost']  = "$0u";	//X_TWSZ-COM_DMZHost
			G_WANConn['X_TWSZ-COM_DMZEnabled']  = "$0v";	//X_TWSZ-COM_DhcpUseUnicast
		`?>
	`?>
`?>



var G_WAN = [];
<?mget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1. "X_TWSZ-COM_CloneMACAddress X_TWSZ-COM_VLANID X_TWSZ-COM_VLANPriority X_TWSZ-COM_MACAddress"
`
	G_WAN['Path'] 		= "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.";
	G_WAN['CloneMACAddress']   = "$01";	//X_TWSZ-COM_CloneMACAddress
	G_WAN['VLANID'] 		= "$02";	//VLANID
	G_WAN['VLANPriority']   = "$03";	//VLANPriority
	G_WAN['MACAddress']   = "$04";	//X_TWSZ-COM_MACAddress	
`?>

//linsd add for dual access
var G_SecondWAN = [];
<?mget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1. "X_TWSZ-COM_DualAccessEnable X_TWSZ-COM_DualAccessAddressType X_TWSZ-COM_DualAccessIPAddress X_TWSZ-COM_DualAccessSubnetMask"
`
	G_SecondWAN['X_TWSZ-COM_DualAccessEnable']  = "$01";	//X_TWSZ-COM_DualAccessEnable
	G_SecondWAN['X_TWSZ-COM_DualAccessAddressType']  = "$02";	
	G_SecondWAN['X_TWSZ-COM_DualAccessIPAddress']  = "$03";
	G_SecondWAN['X_TWSZ-COM_DualAccessSubnetMask']  = "$04";
	
`?>


var G_TunConn = [];
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel. "Activated Mode Dynamic RemoteIpv6Address BorderRelayAddress"
`
	G_TunConn['Path'] 		= ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.$00.";
	G_TunConn['Activated']   = "$01";	//Activated
	G_TunConn['Mode'] 		= "$02";	//Mode
	G_TunConn['Dynamic']   = "$03";	//Dynamic
	G_TunConn['RemoteIpv6Address']   = "$04";	//RemoteIpv6Address
	G_TunConn['BorderRelayAddress']   = "$05";	//BorderRelayAddress
`?>

var G_Wanipv6Conn = [];

<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 2
`	<? if gt $21 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection. "Enable "
	`	G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection.$00.";

		`?>
	`?>

	<? if gt $22 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable "
		`	G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection.$00";	//Path		
		`?>
	`?>
`?>
`?>

//默认网关
var G_DefaultRouter = "<?get :InternetGatewayDevice.Layer3Forwarding.DefaultConnectionService?>";
var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= ":InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>

<? if lt 0 `<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterfaceNumberOfEntries?>`
`	<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
	`	var G_LanIPAddress  = "$01";
		var G_LanSubAddress = "$02";
	`?>
`?>

//MAC Address Clone
var LanHosts = [];
var n = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "MACAddress IPAddress"
		`	LanHosts[n] = ["$01","$02"];
			n++;
		`?>
	`?>
`?>

var LanHostsv6 = [];
n = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntriesV6"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.HostV6. "MACAddress IPAddress"
		`	LanHostsv6[n] = ["$01","$02"];
			n++;
		`?>
	`?>
`?>


var G_CurrentIP = "<?echo $var:sys_RemoteAddr ?>";
var G_CurrentMAC = GetMACByIP(G_CurrentIP).toUpperCase();
//MAC Address Clone
function GetMACByIP(ip){
	for (var i = 0; i < LanHosts.length; i++){
		if (LanHosts[i][1] == ip){
			return LanHosts[i][0];
		}
	}
	
	for (var i = 0; i < LanHostsv6.length; i++){
		if (LanHostsv6[i][1] == ip){
			return LanHostsv6[i][0];
		}
	}

	return "";
}


function alertError()
{
	var code=arguments[0];
    alert(code);   
}


function uiOnload(){	
	createSchedule();
	setJSONValue({
		"wan_ip_mode" 		: findProtocol()
	});
	switch($("wan_ip_mode").value)
	{
		case "static":
			setJSONValue({
				"st_ipaddr"		: G_WANConn['ExternalIPAddress'] || "",
				"st_mask"		: G_WANConn['SubnetMask'] || "",
				"st_gw"			: G_WANConn['DefaultGateway'] || "",
				"ipv4_dns1"		: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
				"ipv4_dns2"		: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
				"ipv4_mtu"		: G_WANConn['MaxMTUSize']|| "1500",
				"ipv4_macaddr"	: G_WAN['CloneMACAddress']
			});
			break;
		case "dhcp":
			setJSONValue({
				"dhcp_host_name"	: G_WANConn['Hostname'],
				"dhcpc_unicast"		: G_WANConn['X_TWSZ-COM_DhcpUseUnicast'] == "1" ? true : false || false,
				"ipv4_dns1"			: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
				"ipv4_dns2"			: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
				"ipv4_mtu"			: G_WANConn['MaxMTUSize']|| "1500",
				"ipv4_macaddr"		: G_WAN['CloneMACAddress']
			});
			break;
		case "pppoe":
		Form.Radio("pppoe_reconnect_radio", G_WANConn['ConnectionTrigger'] || "AlwaysOn");
		Form.Radio("pppoe_addr_type",  (G_WANConn['StaticIPAddress'] == undefined || G_WANConn['StaticIPAddress'] == "") ? "dynamic" : "static");
		Form.Radio("dns_mode",  G_WANConn['DNSOverrideAllowed']=="1" ? "manual" : "auto");
		setJSONValue({
			"pppoe_schedule"				: G_WANConn['ConnectionTrigger'] == "Manual" ? "AlwaysOn" : (G_WANConn['ConnectionTrigger'] == "OnDemand") ? "AlwaysOn" : G_WANConn['ConnectionTrigger'],
			"pppoe_ipaddr" 					: G_WANConn['StaticIPAddress'] || "",
			"pppoe_username" 				: G_WANConn['Username'] || "",
			"pppoe_password" 				: G_WANConn['Password'],
			"confirm_pppoe_password" 		: G_WANConn['Password'],
			"pppoe_service_name"			: G_WANConn['PPPoEServiceName'],
			"pppoe_max_idle_time" 			: (G_WANConn['IdleDisconnectTime'] == undefined ? " " : G_WANConn['IdleDisconnectTime']/60),
			"pppoe_dns1" 					: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
			"pppoe_dns2" 					: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
			"ppp4_macaddr" 					: G_WAN['CloneMACAddress'],
			"ppp4_mtu" 						: G_WANConn['MaxMTUSize'] || "1492"
		});
		break;
		case "rupppoe":
		Form.Radio("pppoe_reconnect_radio", G_WANConn['ConnectionTrigger'] || "AlwaysOn");
		Form.Radio("pppoe_addr_type",  (G_WANConn['StaticIPAddress'] == undefined || G_WANConn['StaticIPAddress'] == "") ? "dynamic" : "static");
		Form.Radio("dns_mode",  G_WANConn['DNSOverrideAllowed']=="1" ? "manual" : "auto");
		Form.Radio("secondwan_type",  (G_SecondWAN['X_TWSZ-COM_DualAccessAddressType'] == "DHCP") ? "dynamic" : "static");

		setJSONValue({
			"pppoe_schedule"				: G_WANConn['ConnectionTrigger'] == "Manual" ? "AlwaysOn" : (G_WANConn['ConnectionTrigger'] == "OnDemand") ? "AlwaysOn" : G_WANConn['ConnectionTrigger'],
			"pppoe_ipaddr" 					: G_WANConn['StaticIPAddress'] || "",
			"pppoe_username" 				: G_WANConn['Username'] || "",
			"pppoe_password" 				: G_WANConn['Password'],
			"confirm_pppoe_password" 		: G_WANConn['Password'],
			"pppoe_service_name"			: G_WANConn['PPPoEServiceName'],
			"pppoe_max_idle_time" 			: (G_WANConn['IdleDisconnectTime'] == undefined ? " " : G_WANConn['IdleDisconnectTime']/60),
			"pppoe_dns1" 					: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
			"pppoe_dns2" 					: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
			"ppp4_macaddr" 					: G_WAN['CloneMACAddress'],
			"ppp4_mtu" 						: G_WANConn['MaxMTUSize'] || "1492",
			"secondwan_ipaddr" 				: (G_SecondWAN['X_TWSZ-COM_DualAccessAddressType'] == "Static" ? G_SecondWAN['X_TWSZ-COM_DualAccessIPAddress']:"" ) || "",
			"secondwan_mask" 				: (G_SecondWAN['X_TWSZ-COM_DualAccessAddressType'] == "Static" ? G_SecondWAN['X_TWSZ-COM_DualAccessSubnetMask']:"" )  || ""			
		});
		break;
		case "pptp":
		Form.Radio("pptp_reconnect_radio", G_WANConn['ConnectionTrigger'] || "AlwaysOn");
		Form.Radio("pptp_addr_type",  G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"? "static" : "dynamic");

		setJSONValue({
			"pptp_schedule"				: G_WANConn['ConnectionTrigger'] == "Manual" ? "AlwaysOn" : (G_WANConn['ConnectionTrigger'] == "OnDemand") ? "AlwaysOn" : G_WANConn['ConnectionTrigger'],
			"pptp_ipaddr" 				: G_WANConn['X_TWSZ-COM_VPN_CLIENT'] || "",
			"pptp_mask" 				: G_WANConn['X_TWSZ-COM_VPN_NETMASK'] || "",
			"pptp_gw" 					: G_WANConn['X_TWSZ-COM_VPN_GATEWAY'] || "",
			"pptp_server" 				: G_WANConn['X_TWSZ-COM_VPN_SERVER'] || "",
			"pptp_username" 			: G_WANConn['Username'] || "",
			"pptp_password" 			: G_WANConn['Password'],
			"confirm_pptp_password" 	: G_WANConn['Password'],
			"pptp_max_idle_time" 		: (G_WANConn['IdleDisconnectTime'] == undefined ? " " : G_WANConn['IdleDisconnectTime']/60),
			"pptp_dns1" 				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
			"pptp_dns2" 				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
			"ppp4_macaddr" 				: G_WAN['CloneMACAddress'],
			"ppp4_mtu" 					: G_WANConn['MaxMTUSize'] || "1400"
		});
		break;
		case "l2tp":
		Form.Radio("l2tp_reconnect_radio", G_WANConn['ConnectionTrigger'] || "AlwaysOn");
		Form.Radio("l2tp_addr_type",  G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"? "static" : "dynamic");
		setJSONValue({
			"l2tp_schedule"				: G_WANConn['ConnectionTrigger'] == "Manual" ? "AlwaysOn" : (G_WANConn['ConnectionTrigger'] == "OnDemand") ? "AlwaysOn" : G_WANConn['ConnectionTrigger'],
			"l2tp_ipaddr" 				: G_WANConn['X_TWSZ-COM_VPN_CLIENT'] || "",
			"l2tp_mask" 				: G_WANConn['X_TWSZ-COM_VPN_NETMASK'] || "",
			"l2tp_gw" 					: G_WANConn['X_TWSZ-COM_VPN_GATEWAY'] || "",
			"l2tp_server" 				: G_WANConn['X_TWSZ-COM_VPN_SERVER'] || "",
			"l2tp_username" 			: G_WANConn['Username'] || "",
			"l2tp_password" 			: G_WANConn['Password'],
			"confirm_l2tp_password" 	: G_WANConn['Password'],
			"l2tp_max_idle_time" 		: (G_WANConn['IdleDisconnectTime'] == undefined ? " " : G_WANConn['IdleDisconnectTime']/60),
			"l2tp_dns1" 				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || "",
			"l2tp_dns2" 				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || "",
			"ppp4_macaddr" 				: G_WAN['CloneMACAddress'],
			"ppp4_mtu" 					: G_WANConn['MaxMTUSize'] || "1400"
		});
		break;
		case "dslite":
		Form.Radio("dslite_addr_type", G_TunConn['Dynamic'] == "1" ? "dynamic":"manual");
		setJSONValue({
			"aftr_ipaddr6" 	: G_TunConn['RemoteIpv6Address'] || "",
			"dslite_wan_ipaddr6" 	: "",
			"dslite_gw_ipaddr6" 	: ""
		});

		if(G_TunConn['BorderRelayAddress'] != '')
		{
			var ipv4addr =  G_TunConn['BorderRelayAddress'].split(".");
			$('b4_ipaddr_2').value = ipv4addr[3];
		}
		else
		{
			$('b4_ipaddr_2').value = "";
		}

		break;
	}	
	OnChangeWanIpMode();
	enable_save_button();
}
function findProtocol(){
    var _protocol = 'dhcp';
	var Conn_Type = G_WANConn['Path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";

	if (G_WANConn['Enable'] == "0" && G_TunConn['Activated'] == "1")
	{
			_protocol = 'dslite';
			return _protocol;
	}

	if(Conn_Type == 'PPP')
	{
		if(G_WANConn['ConnectionType']=="PPTP_Relay"){
			_protocol = 'pptp';
		}else if(G_WANConn['ConnectionType']=="L2TP_Relay"){
			_protocol = 'l2tp';
		}else{
			_protocol = 'pppoe';
			if(G_SecondWAN['X_TWSZ-COM_DualAccessEnable'] == "1")		
				_protocol = 'rupppoe';				
		}
	}else
	{
		if(G_WANConn['ConnectionType'] != 'IP_Bridged')
		{
			if(G_WANConn['AddressingType'] == 'Static')
			{
				_protocol = 'static';
			}else{
				_protocol = 'dhcp';
			}
		}
	}
	return _protocol;
}
function OnClickMacButton(name)
{
	$(name).value = G_CurrentMAC;;
	if($(name).value == "")
	alert(SEcode["lang_not_find_mac"]);
}
function OnClickDsliteAddrType()
{
	$("aftr_ipaddr6").disabled = $("dslite_dynamic").checked ? true: false;
}
function OnClickPppoeAddrType()
{
	$("pppoe_ipaddr").disabled = $("pppoe_dynamic").checked ? true: false;
}
function OnClickSecondWanType()
{
	$("secondwan_ipaddr").disabled = $("secondwan_dynamic").checked ? true: false;
	$("secondwan_mask").disabled = $("secondwan_dynamic").checked ? true: false;
}
function OnClickPppoeReconnect()
{
	if($("pppoe_alwayson").checked)
	{
		$("pppoe_schedule").disabled = false;
		$("BWAN048").disabled = false;
		$("pppoe_max_idle_time").disabled = true;
	}
	else if($("pppoe_ondemand").checked)
	{
		$("pppoe_schedule").disabled = true;
		$("BWAN048").disabled = true;
		$("pppoe_max_idle_time").disabled = false;
	}
	else
	{
		$("pppoe_schedule").disabled = true;
		$("BWAN048").disabled = true;
		$("pppoe_max_idle_time").disabled = true;
	}
}
function OnClickPptpAddrType()
{
	var dis = $("pptp_dynamic").checked ? true: false;
	$("pptp_ipaddr").disabled	= dis;
	$("pptp_mask").disabled	= dis;
	$("pptp_gw").disabled		= dis;
}
function OnClickPptpReconnect()
{
	if($("pptp_alwayson").checked)
	{
		$("pptp_schedule").disabled = false;
		$("BWAN070").disabled = false;
		$("pptp_max_idle_time").disabled = true;
	}
	else if($("pptp_ondemand").checked)
	{
		$("pptp_schedule").disabled = true;
		$("BWAN070").disabled = true;
		$("pptp_max_idle_time").disabled = false;
	}
	else
	{
		$("pptp_schedule").disabled = true;
		$("BWAN070").disabled = true;
		$("pptp_max_idle_time").disabled = true;
	}
}
function OnClickL2tpAddrType()
{
	var dis = $("l2tp_dynamic").checked ? true: false;
	$("l2tp_ipaddr").disabled	= dis;
	$("l2tp_mask").disabled	= dis;
	$("l2tp_gw").disabled		= dis;
}
function OnClickL2tpReconnect()
{
	if($("l2tp_alwayson").checked)
	{
		$("l2tp_schedule").disabled = false;
		$("BWAN070").disabled = false;
		$("l2tp_max_idle_time").disabled = true;
	}
	else if($("l2tp_ondemand").checked)
	{
		$("l2tp_schedule").disabled = true;
		$("BWAN070").disabled = true;
		$("l2tp_max_idle_time").disabled = false;
	}
	else
	{
		$("l2tp_schedule").disabled = true;
		$("BWAN070").disabled = true;
		$("l2tp_max_idle_time").disabled = true;
	}
}
function OnClickDnsMode()
{
	var dis = $("pppoe_dns_isp").checked;
	$("pppoe_dns1").disabled = dis;
	$("pppoe_dns2").disabled = dis;
}
function OnChangeWanIpMode()
{
	$("ipv4_setting").style.display		= "none";
	$("ppp4_setting").style.display		= "none";

	$("box_wan_static").style.display		= "none";
	$("box_wan_dhcp").style.display		= "none";
	$("box_wan_dhcpplus").style.display	= "none";
	$("box_wan_static_body").style.display= "none";
	$("box_wan_dhcp_body").style.display	= "none";
	$("box_wan_ipv4_common_body").style.display = "none";

	$("box_wan_pppoe").style.display		= "none";
	$("box_wan_pptp").style.display		= "none";
	$("box_wan_l2tp").style.display		= "none";
	$("box_wan_ru_pppoe").style.display	= "none";
	$("box_wan_ru_pptp").style.display	= "none";
	$("box_wan_ru_l2tp").style.display	= "none";
	$("show_pppoe_mppe").style.display	= "none";
	$("show_pptp_mppe").style.display		= "none";
	$("box_wan_pppoe_body").style.display	= "none";
	$("box_wan_pptp_body").style.display	= "none";
	$("box_wan_l2tp_body").style.display	= "none";
	$("box_wan_ppp4_comm_body").style.display = "none";
	$("box_wan_dslite").style.display	= "none";
	$("box_wan_dslite_body").style.display	= "none";
	$("show_netsniper").style.display = "none";
	$("show_starspeed").style.display= "none";
	$("show_pppoeplus").style.display = "none";

	$("secondwan_block").style.display = "none";
	
	switch($("wan_ip_mode").value)
	{
	case "static":
		$("ipv4_setting").style.display				= "block";
		$("box_wan_static").style.display				= "block"; 
		$("box_wan_static_body").style.display		= "block";
		$("box_wan_ipv4_common_body").style.display	= "block";
		if(findProtocol()!="static")			
			$("ipv4_mtu").value = "1500";
		else
			$("ipv4_mtu").value = G_WANConn['MaxMTUSize'];
		break;
	case "dhcp":
		$("ipv4_setting").style.display				= "block";
		$("box_wan_dhcp").style.display				= ($("wan_ip_mode").value == "dhcpplus") ? "none"  : "block";
		$("box_wan_dhcpplus").style.display			= ($("wan_ip_mode").value == "dhcpplus") ? "block" : "none";
		$("box_wan_dhcp_body").style.display			= "block";
		$("dhcpplus").style.display					= ($("wan_ip_mode").value == "dhcpplus") ? "block" : "none";
		$("box_wan_ipv4_common_body").style.display	= "block";
		if(findProtocol()!="dhcp")
		{
			$("ipv4_mtu").value = "1500";
			$("dhcp_host_name").value = "dlinkrouter";
		}
		else
		{
			$("ipv4_mtu").value = G_WANConn['MaxMTUSize'];
			$("dhcp_host_name").value = G_WANConn['Hostname'];
		}
		break;	
	case "pppoe":
		$("ppp4_setting").style.display				= "block";
		$("box_wan_pppoe_body").style.display			= "block";
		$("box_wan_pppoe").style.display				= "block";
		$("box_wan_ppp4_comm_body").style.display		= "block";
		if(findProtocol()!="pppoe")
			$("ppp4_mtu").value = "1492";
		else
			$("ppp4_mtu").value = G_WANConn['MaxMTUSize'];
		OnClickPppoeAddrType();
		OnClickPppoeReconnect();
		OnClickDnsMode();
		break;
	case "rupppoe":
		$("ppp4_setting").style.display				= "block";
		$("box_wan_pppoe_body").style.display			= "block";
		$("box_wan_ru_pppoe").style.display				= "block";
		$("box_wan_ppp4_comm_body").style.display		= "block";
		$("secondwan_block").style.display = "block";
		if(findProtocol()!="rupppoe")
			$("ppp4_mtu").value = "1492";
		else
			$("ppp4_mtu").value = G_WANConn['MaxMTUSize'];
		OnClickPppoeAddrType();
		OnClickPppoeReconnect();
		OnClickDnsMode();
		OnClickSecondWanType();

		break;
	case "pptp":
		$("ppp4_setting").style.display				= "block";
		$("box_wan_pptp").style.display				= "block";
		$("box_wan_pptp_body").style.display			= "block";
		$("box_wan_ppp4_comm_body").style.display 	= "block";
		if(findProtocol()!="pptp")		
		   $("ppp4_mtu").value = "1400";
	    else
			$("ppp4_mtu").value = G_WANConn['MaxMTUSize'];
		OnClickPptpAddrType();
		OnClickPptpReconnect();
		break;
	case "l2tp":		
		$("ppp4_setting").style.display				= "block";
		$("box_wan_l2tp").style.display				= "block";
		$("box_wan_l2tp_body").style.display			= "block";
		$("box_wan_ppp4_comm_body").style.display		= "block";
		if(findProtocol()!="l2tp")			
			$("ppp4_mtu").value = "1400";
		else
			$("ppp4_mtu").value = G_WANConn['MaxMTUSize'];
		OnClickL2tpAddrType();
		OnClickL2tpReconnect();
		break;
	case "dslite":
		$("box_wan_dslite").style.display = "block";
		$("box_wan_dslite_body").style.display = "block";
		OnClickDsliteAddrType();
		break;
	}
}
function createSchedule(){
		var array_value = [],array_options=[];
		array_value[0]="AlwaysOn";
		array_options[0]=data_languages.Public.innerHTML.Public017;
		for(var k = 0; k < schedule_list.length; k++){
			array_value[k+1]=schedule_list[k][0];
			array_options[k+1]=schedule_list[k][0];
		}
		$S('pppoe_schedule', array_options, array_value);
		$S('pptp_schedule', array_options, array_value);
		$S('l2tp_schedule', array_options, array_value);
}


function getProtocol()
{
	switch($("wan_ip_mode").value){
		case 'dhcp' : {
			return ['DHCP','IP'];
		}
		case 'static' : {
			return ['Static','IP'];
		}
		case 'pppoe' : {
			return ['PPPoE','PPP'];
		}
		case 'rupppoe' : {
			return ['RUPPPoE','PPP'];
		}
		case 'pptp' : {
			return ['PPTP','PPP'];
		}
		case 'l2tp' : {
			return ['L2TP','PPP'];
		}
		case 'dslite' : {
			return ['DSLITE','IPV6'];
		}
	}
}
function checkMACaddr(mac){
    var clonmac=$(mac).value;
    var pattern=/^([0-9A-Fa-f]{2})(-[0-9A-Fa-f]{2}){5}|([0-9A-Fa-f]{2})(:[0-9A-Fa-f]{2}){5}/;
	var result = pattern.test(clonmac);
   
    if(result == false)
    {
		alert(SEcode["lang_invalid_mac"]);
    	return false; 
    }
    return true;
}
 /*检测是否全部是数字*/
function validateKey(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ) )
			continue;
	return 0;
  }
  return 1;
}

function getDigit(str, num)
{
  i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '.' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '.' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  var d = parseInt(str, 10);
  return d;
}


/*判断地址是否在范围内*/
function checkDigitRange(str, num, min, max)
{
  var d = getDigit(str,num);
  if ( d > max || d < min )
      	return false;
  return true;
}

function checkDNSValue(_value)
{
	if (validateKey(_value) == 0) {
		return false;
	}
	if (!checkDigitRange(_value,1,1,223)) {
		return false;
	}
	if (getDigit(_value,1) == 127)
	{
		return false;
	}
	if ( !checkDigitRange(_value,2,0,255) ) {
		return false;
	}
	if ( !checkDigitRange(_value,3,0,255) ) {
		return false;
	}
	if ( !checkDigitRange(_value,4,1,254) ) {
		return false;
	}
   
	return true;
}

function checkDNS(DNS){
	var tmpPriorityDNS=$(DNS+'1').value;
	var tmpSecondaryDNS=$(DNS+'2').value;
	
	if((tmpPriorityDNS==''))
	{
		alertError(SEcode['lang_pri_dns_invalid']);
		return false;
	}

	if ((tmpPriorityDNS!='') && !checkDNSValue(tmpPriorityDNS)) 
	{
		alertError(SEcode['lang_pri_dns_invalid']);
		return false;
	}

	/*Secondary DNS Address is option*/
	/*  
	if ((tmpSecondaryDNS!='') && !checkDNSValue(tmpSecondaryDNS))
	{
		alertError('Invalid Second DNS address');
		return false;
	}
	*/
	
	if(tmpPriorityDNS&&tmpPriorityDNS==tmpSecondaryDNS)
	{
		alertError(SEcode['lang_sec_dns_invalid']);
		return false;
	}
    return true;
}
function checkIdletime(name)
{
	if(Form.Radio(name+'_reconnect_radio')=='OnDemand' && $(name+'_max_idle_time').value.length<=0)
	{		
		alertError(SEcode['lang_blank_idletime'],1);
		return false;
	}
	return true;
}
function checkPWD(name)
{
	if($(name+'_username').value == "")
	{
		alert(SEcode["lang_user_empty"]);
		return false;
	}
	
	if($(name+'_password').value == "")
	{
		alert(SEcode["lang_passwd_empty"]);
		return false;
	}
	
	if($(name+'_password').value != $('confirm_'+name+'_password').value){
		alert(SEcode["lang_passwd_not_match"]);
		return false;
	}
	return true;
}
function checkMTU(name,type)
{
	var wanMtu = $(name).value;
	if(wanMtu.length != 0 && wanMtu.charAt(0) == 0)
	{
		$('ipv4_mtu').value = parseInt(wanMtu, 10);

	}
	wanMtu=Number(wanMtu);
	if(isNaN(wanMtu))
	{
		alert(SEcode['lang_mtu_invalid']); 
		return false;
	}
	if((type=='DHCP')||(type=='Static'))
	{
		if(wanMtu<1280)
		{
			alert(SEcode['lang_mtu_dhcp_small']);
			return false;
		}
		if(wanMtu>1500)
		{
			alert(SEcode['lang_mtu_dhcp_large']); 
			return false;
		}
	}
	else if((type=='PPPoE')||(type=='RUPPPoE'))
	{
		if(wanMtu<1280)
		{ 
			alert(SEcode['lang_mtu_pppoe_small']);  
			return false;
		}
		if(wanMtu>1492)
		{ 
			alert(SEcode['lang_mtu_pppoe_large']); 
			return false;
		}
	}
	else
	{
		if(wanMtu<1280)
		{ 
			alert(SEcode['lang_mtu_small']); 
			return false;
		}
		if(wanMtu>1460)
		{ 
			alert(SEcode['lang_mtu_largel']); 
			return false;
		}
	}	
	return true;
}

function null_errorfunc()
{

	return true;
}


function Ajax_Submit(_text)
{
	
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		
		return;
	}
	G_SysToken = G_AjaxToken;
	if(G_Error == '1')
	{		
		$("menu").style.display="";
		$("content").style.display="";
		$("mbox").style.display="none";
		dealWithError();		
		document.getElementById("BWAN005").disabled = false;
		document.getElementById("BWAN110").disabled = false;
	}	
	else
	{
		$H({
			"obj-action"           : "set",
			"getpage"              : "html/index.html",
			"errorpage"            : "html/index.html",
			"var:menu"             : "basic",
			"var:page"             : "Bas_wan",
			'var:sys_Token' : G_SysToken,
			"var:errorpage"        : "Bas_wan",
			"var:CacheLastData" 	: ViewState.Save()
		}, true);
		
		var _protocol = getProtocol();
		var _tunnelpath = ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.1." ; //for tunnel
		if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] == "4in6") //tunnel enable 
		{
			if (_protocol[0] != "DSLITE")
				$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
		}
		$('uiPostForm').submit();
	}
}
function enable_save_button()
{
	document.getElementById("BWAN005").disabled = false;
    document.getElementById("BWAN110").disabled = false;
}
//提交配置
function uiSubmit(){
	
	//路径
	var ConnPath;
	var _addrtype;
	var _protocol = getProtocol();
	var _tunnelpath = ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.1." ; //for tunnel

	
	if(_protocol[0] == "DHCP"){
		_addrtype = "DHCP";
	}else if(_protocol[0] == "Static"){
		_addrtype = "Static";
	}else{
		_addrtype = "undefined";
	}

	$H({
		"obj-action"           : "set",
		"getpage"              : "html/page/portforwd.ajax.js",
		"errorpage"            : "html/page/portforwd.ajax.js",
		"var:menu"             : "basic",
		"var:page"             : "Bas_wan",
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		"var:errorpage"        : "Bas_wan",
		"var:CacheLastData" 	: ViewState.Save()
	}, true);

	ConnPath = G_WANConn['Path'];

	if(_protocol[0] != "DSLITE")
	{
		if(G_WANConn['Path'] && G_WANConn['Path'].indexOf(_protocol[1]) > -1){
			ConnPath = G_WANConn['Path'];
		}else{
			ConnPath = G_WANConn['Path'].replace(_protocol[1] == 'PPP' ? 'IP' : 'PPP', _protocol[1]);
			$F('mid','0438');
			$F(":" + ConnPath + "X_TWSZ-COM_ConnectionMode", G_WANConn['Path'].indexOf('IP') > 0 ? 'IP' : 'PPP');
			$F(":" + ConnPath + 'X_TWSZ-COM_DMZEnabled',	G_WANConn['X_TWSZ-COM_DMZEnabled']);
			$F(":" + ConnPath + 'X_TWSZ-COM_DMZHost', G_WANConn['X_TWSZ-COM_DMZHost']);	
		}

		//dslite disable 情况		
		$F(":" + ConnPath + "Enable", 						'1');
		$F(":" + ConnPath + "Name", 						_protocol[0] + '_1');	

		$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 		G_WANConn['ProtocolType'] == "IPv4_6" ?  "IPv4_6" :"IPv4"); 		//pppoe_share	
		$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 		'Internet');
		$F(":" + ConnPath + "NATEnabled", 					'1');
		$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");	
		$F(":" + ConnPath + "RouteProtocolRx", 		"Off");
		$F(":" + ConnPath + "RipDirection", 		"Both");	
		$F(":" + ConnPath + "AddressingType", 		_addrtype == "undefined" ? undefined : _addrtype);

		var DevicePath = G_WAN['Path'];
		//vlan
		if(_protocol[0] != "PPTP" && _protocol[0] != "L2TP")
		{
			$F(":" + DevicePath + "X_TWSZ-COM_VLANID", 				'0');
		}
		$F(":" + DevicePath + "X_TWSZ-COM_VLANPriority", 		'0');
		

	}


	//判断连接类型是否更改
	/*
	if(G_WANConn['Path'] && G_WANConn['Path'].indexOf(_protocol[1]) > -1){
		ConnPath = G_WANConn['Path'];
	}else{
		ConnPath = G_WANConn['Path'].replace(_protocol[1] == 'PPP' ? 'IP' : 'PPP', _protocol[1]);
		$F('mid','0438');
		$F(":" + ConnPath + "X_TWSZ-COM_ConnectionMode", G_WANConn['Path'].indexOf('IP') > 0 ? 'IP' : 'PPP');
	}
	*/
	//VLAN 节点的路径
	//IP common
	
	

	switch(_protocol[0]){
		case "DHCP" :
			/*
				if(!checkMACaddr('ipv4_macaddr'))
					return false;
				if(!checkDNS('ipv4_dns'))
					return false;
				
				if($('ipv4_dns1').value == G_WANConn['ExternalIPAddress'] || $('ipv4_dns1').value == G_LanIPAddress)
				{
					alertError('Invalid Primary DNS address');
					return false;
				}
				if($('ipv4_dns2').value == G_LanIPAddress)	
				{
					alertError('Invalid Second DNS address');
					return false;
				}
			*/
				if(!checkMTU('ipv4_mtu',_protocol[0]))
					return false;
				
				var _dnsservers = $('ipv4_dns1').value+','+$('ipv4_dns2').value;
				$F(":" + ConnPath + 'X_TWSZ-COM_Hostname',  		$('dhcp_host_name').value);
				//$F(':InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.DomainName',  	$('dhcp_host_name').value);	
				$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
				$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 		_dnsservers.delcomma());
				$F(":" + ConnPath + "MaxMTUSize", 		$('ipv4_mtu').value);
				$F(":" + ConnPath + "ConnectionType", 		 "IP_Routed");
				$F(":" + ConnPath + "X_TWSZ-COM_DhcpUseUnicast", 		    $("dhcpc_unicast").checked ? "1":"0");
				$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ipv4_macaddr').value);
				
			break;
		case "Static" :
				
				//if(!checkMACaddr('ipv4_macaddr'))
				//	return false;
				
				if(!checkDNS('ipv4_dns'))
					return false;

				
				if($('ipv4_dns1').value == $('st_ipaddr').value || $('ipv4_dns1').value == G_LanIPAddress)
				{
					alertError(SEcode['lang_pri_dns_invalid']);
					return false;
				}

				if($('ipv4_dns2').value == G_LanIPAddress)	
				{
					alertError(SEcode['lang_sec_dns_invalid']);
					return false;
				}

				if (!CheckValidity.IP("st_ipaddr",SEcode["lang_invalid_ip"]))
					return false;

				if($('st_ipaddr').value == G_LanIPAddress)	
				{
					alertError(SEcode['lang_ip_not_same']);
					return false;
				}
				if(isSameSubNet($("st_ipaddr").value,$('st_mask').value,G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("st_ipaddr").focus();
					return false;
				}
				if ($("st_mask").value == "" || $("st_mask").value=="0.0.0.0") 
				{
					alert(SEcode["lang_mask_invalid"]);	
					return false;
				}
				if (!CheckValidity.Mask("st_mask",SEcode["lang_mask_invalid"]))
					return false;
				
				if ($("st_gw").value == "" || $("st_gw").value=="0.0.0.0") 
				{
					alert(SEcode["lang_gate_invalid"]);	
					return false;
				}
				if (!CheckValidity.IP("st_gw",SEcode["lang_gate_invalid"]))
					return false;

				if($('st_gw').value == G_LanIPAddress)	
				{
					alertError(SEcode['lang_gw_not_same']);
					return false;
				}
				if(!isSameSubNet($("st_gw").value,$('st_mask').value,$('st_ipaddr').value,$('st_mask').value))
				{
					alert(SEcode["lang_gate_same_ip"]);	
					$("st_gw").focus();
					return false;
				}
					
				if(!checkMTU('ipv4_mtu',_protocol[0]))
									return false;

				var _dnsservers = $('ipv4_dns1').value+','+$('ipv4_dns2').value;
				$F(":" + ConnPath + "ExternalIPAddress", 	$('st_ipaddr').value);
				$F(":" + ConnPath + "SubnetMask", 		$('st_mask').value);
				$F(":" + ConnPath + "DefaultGateway", 		$('st_gw').value);
				$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 		_dnsservers.delcomma());
				$F(":" + ConnPath + "MaxMTUSize", 		$('ipv4_mtu').value);
				$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
				$F(":" + ConnPath + "ConnectionType", 		 "IP_Routed");
				$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ipv4_macaddr').value);
			break;
		case "PPPoE" :
			//if(!checkMACaddr('ppp4_macaddr'))
			//	return false;

			if(!checkIdletime('pppoe'))
				return false;

			if(!checkPWD('pppoe'))
				return false;

			if(!checkMTU('ppp4_mtu',_protocol[0]))
				return false;

			if(Form.Radio('dns_mode') == "manual"){ //手动设置DNS
				if(!checkDNS('pppoe_dns'))	
					return false;

				if($('pppoe_dns1').value == $('pppoe_ipaddr').value || $('pppoe_dns1').value == G_LanIPAddress)
				{
					alertError(SEcode['lang_pri_dns_invalid']);
					return false;
				}
				if($('pppoe_dns2').value == G_LanIPAddress)	
				{
					alertError(SEcode['lang_sec_dns_invalid']);
					return false;
				}
				
				var _dnsservers = $('pppoe_dns1').value+','+$('pppoe_dns2').value;
				$F(":" + ConnPath + "DNSOverrideAllowed", 	"1");
				$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 	_dnsservers.delcomma());
			}else{

				$F(":" + ConnPath + "DNSOverrideAllowed", 	"0");
			}

			if(Form.Radio('pppoe_addr_type') == "static")
			{

				if((G_WANConn['LocalNetMask'] == "") || (G_WANConn['LocalNetMask'] == undefined))
					G_WANConn['LocalNetMask'] = "255.255.255.255";
				
				if(isSameSubNet($("pppoe_ipaddr").value,G_WANConn['LocalNetMask'],G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("pppoe_ipaddr").focus();
					return false;
				}
			}
			var pppoe_passwd = Base64.Encode($('pppoe_password').value);
			$F(":" + ConnPath + "Username", 		$('pppoe_username').value);
			$F(":" + ConnPath + "Password", 		$('pppoe_password').value != '**********' ? pppoe_passwd : undefined);
			$F(":" + ConnPath + "ConnectionType", 		 "IP_Routed");
			$F(":" + ConnPath + "PPPAuthenticationProtocol", "Auto");
			$F(":" + ConnPath + 'X_TWSZ-COM_ScheduleListName',  Form.Radio('pppoe_reconnect_radio') == "Manual" ? "AlwaysOn" : (Form.Radio('pppoe_reconnect_radio') == "OnDemand") ? "AlwaysOn" : $('pppoe_schedule').value);
			$F(":" + ConnPath + "ConnectionTrigger", 	Form.Radio('pppoe_reconnect_radio') == "AlwaysOn" ? $('pppoe_schedule').value : Form.Radio('pppoe_reconnect_radio'));
			$F(":" + ConnPath + "IdleDisconnectTime", 	Form.Radio('pppoe_reconnect_radio') == "OnDemand" ? $('pppoe_max_idle_time').value*60 : undefined);
			$F(":" + ConnPath + 'PPPoEServiceName',          $('pppoe_service_name').value);
			$F(":" + ConnPath + "MaxMRUSize", 		$('ppp4_mtu').value);
			$F(":" + ConnPath + "MaxMTUSize", 		$('ppp4_mtu').value);
			$F(":" + ConnPath + "PPPLCPEcho", 		"15");
			$F(":" + ConnPath + "PPPLCPEchoRetry", 		"15");
			$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", Form.Radio('pppoe_addr_type') == "dynamic" ? "" : $('pppoe_ipaddr').value);
			$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ppp4_macaddr').value);

			break;
		case "RUPPPoE" :
			//if(!checkMACaddr('ppp4_macaddr'))
			//	return false;

			if(!checkIdletime('pppoe'))
				return false;

			if(!checkPWD('pppoe'))
				return false;

			if(!checkMTU('ppp4_mtu',_protocol[0]))
				return false;

			if(Form.Radio('dns_mode') == "manual"){ //手动设置DNS
				if(!checkDNS('pppoe_dns'))	
					return false;

				if($('pppoe_dns1').value == $('pppoe_ipaddr').value || $('pppoe_dns1').value == G_LanIPAddress)
				{
					alertError(SEcode['lang_pri_dns_invalid']);
					return false;
				}
				if($('pppoe_dns2').value == G_LanIPAddress)	
				{
					alertError(SEcode['lang_sec_dns_invalid']);
					return false;
				}
				
				var _dnsservers = $('pppoe_dns1').value+','+$('pppoe_dns2').value;
				$F(":" + ConnPath + "DNSOverrideAllowed", 	"1");
				$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 	_dnsservers.delcomma());
			}else{

				$F(":" + ConnPath + "DNSOverrideAllowed", 	"0");
			}

			if(Form.Radio('pppoe_addr_type') == "static")
			{

				if((G_WANConn['LocalNetMask'] == "") || (G_WANConn['LocalNetMask'] == undefined))
					G_WANConn['LocalNetMask'] = "255.255.255.255";
				
				if(isSameSubNet($("pppoe_ipaddr").value,G_WANConn['LocalNetMask'],G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("pppoe_ipaddr").focus();
					return false;
				}
			}
			var pppoe_passwd = Base64.Encode($('pppoe_password').value);
			$F(":" + ConnPath + "Username", 		$('pppoe_username').value);
			$F(":" + ConnPath + "Password", 		$('pppoe_password').value != '**********' ? pppoe_passwd : undefined);
			$F(":" + ConnPath + "ConnectionType", 		 "IP_Routed");
			$F(":" + ConnPath + "PPPAuthenticationProtocol", "Auto");
			$F(":" + ConnPath + 'X_TWSZ-COM_ScheduleListName',  Form.Radio('pppoe_reconnect_radio') == "Manual" ? "AlwaysOn" : (Form.Radio('pppoe_reconnect_radio') == "OnDemand") ? "AlwaysOn" : $('pppoe_schedule').value);
			$F(":" + ConnPath + "ConnectionTrigger", 	Form.Radio('pppoe_reconnect_radio') == "AlwaysOn" ? $('pppoe_schedule').value : Form.Radio('pppoe_reconnect_radio'));
			$F(":" + ConnPath + "IdleDisconnectTime", 	Form.Radio('pppoe_reconnect_radio') == "OnDemand" ? $('pppoe_max_idle_time').value*60 : undefined);
			$F(":" + ConnPath + 'PPPoEServiceName',          $('pppoe_service_name').value);
			$F(":" + ConnPath + "MaxMRUSize", 		$('ppp4_mtu').value);
			$F(":" + ConnPath + "MaxMTUSize", 		$('ppp4_mtu').value);
			$F(":" + ConnPath + "PPPLCPEcho", 		"15");
			$F(":" + ConnPath + "PPPLCPEchoRetry", 		"15");
			$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", Form.Radio('pppoe_addr_type') == "dynamic" ? "" : $('pppoe_ipaddr').value);
			$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ppp4_macaddr').value);

			$F(":" + DevicePath + "X_TWSZ-COM_DualAccessEnable", "1");
			$F(":" + DevicePath + "X_TWSZ-COM_DualAccessConnectionType", "PPPOE");
			$F(":" + DevicePath + "X_TWSZ-COM_DualAccessAddressType", Form.Radio('secondwan_type') == "static" ? "Static":"DHCP");
			
			if(Form.Radio('secondwan_type') == "static")
			{
				
				if(isSameSubNet($("secondwan_ipaddr").value,$("secondwan_mask").value,G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("secondwan_ipaddr").focus();
					return false;
				}

				$F(":" + DevicePath + "X_TWSZ-COM_DualAccessIPAddress", $("secondwan_ipaddr").value);
				$F(":" + DevicePath + "X_TWSZ-COM_DualAccessSubnetMask", $("secondwan_mask").value);

			}			
			break;			
		case "PPTP" :
			var x = (Form.Radio("pptp_addr_type") == 'dynamic') ? '1' : '0';
			if(x == "1"){
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"DHCP");  //address mode
			} else {
				if($('pptp_ipaddr').value == "")
				{
					alertError(SEcode['lang_pptp_ip_empty']);
					return false;
				}
				if (!CheckValidity.IP("pptp_ipaddr",SEcode["lang_invalid_ip"]))
					return false;
					
				if($('pptp_mask').value == "")
				{
					alertError(SEcode['lang_pptp_mask_empty']);
					return false;
				}
				if (!CheckValidity.Mask("pptp_mask",SEcode["lang_invalid_mask"]))
					return false;
				if(isSameSubNet($("pptp_ipaddr").value,$('pptp_mask').value,G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("pptp_ipaddr").focus();
					return false;
				}
				if($('pptp_gw').value == "")
				{
					alertError(SEcode['lang_pptp_gw_empty']);
					return false;
				}
				if (!CheckValidity.IP("pptp_gw",SEcode["lang_invalid_def_gate"]))
					return false;
				
				if($('pptp_ipaddr').value == $('pptp_gw').value)
				{
					alertError(SEcode['lang_pptp_gate_same_ip']);
					return false;
				}

				if(!isSameSubNet($("pptp_gw").value,$('pptp_mask').value,$('pptp_ipaddr').value,$('pptp_mask').value))
				{
					alert(SEcode["lang_gate_same_net_ip"]);
					$("pptp_gw").focus();
					return false;
				}

				if($('pptp_ipaddr').value == $('pptp_server').value)
				{
					alertError(SEcode['lang_pptp_server_same_ip']);
					return false;
				}
				
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_CLIENT',  	(Form.Radio("pptp_addr_type") == 'dynamic') ? '' :$('pptp_ipaddr').value);
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_NETMASK', 	(Form.Radio("pptp_addr_type") == 'dynamic') ? '' :$('pptp_mask').value);
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_GATEWAY', 	(Form.Radio("pptp_addr_type") == 'dynamic') ? '' :$('pptp_gw').value);
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"Static"); 


				if(!checkDNS('pptp_dns'))	
					return false;
				
			}
			
			if($('pptp_server').value == "")
			{
				alertError(SEcode['lang_pptp_server_empty']);
				return false;
			}
			
			//if (!CheckValidity.IP("pptp_server",SEcode["lang_invalid_pptp_server"]))
			//	return false;
			
			if($("pptp_server").value == G_LanIPAddress)
			{
				alert(SEcode["lang_lan_pptp_server_conflict"]);//
				$("pptp_server").focus();
				return false;
			}
			if(!checkIdletime('pptp'))
				return false;
			if(!checkPWD('pptp'))
				return false;
			if(!checkMTU('ppp4_mtu',_protocol[0]))
				return false;
			
			/*if(!checkMACaddr('ppp4_macaddr'))
				return false;*/	
			
			var _dnsservers = $('pptp_dns1').value+','+$('pptp_dns2').value;	
			$F(":" + ConnPath + 'X_TWSZ-COM_ScheduleListName',  	Form.Radio('pptp_reconnect_radio') == "Manual" ? "AlwaysOn" : (Form.Radio('pptp_reconnect_radio') == "OnDemand") ? "AlwaysOn" : $('pptp_schedule').value);
			$F(":" + ConnPath + "ConnectionTrigger", 	Form.Radio('pptp_reconnect_radio') == "AlwaysOn" ? $('pptp_schedule').value : Form.Radio('pptp_reconnect_radio'));
			$F(":" + ConnPath + "IdleDisconnectTime", 	Form.Radio('pptp_reconnect_radio') == "OnDemand" ? $('pptp_max_idle_time').value*60 : undefined);
			$F(":" + ConnPath + "MaxMTUSize", 		$('ppp4_mtu').value);			
			$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
			$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 		_dnsservers.delcomma());
			
			$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ppp4_macaddr').value);
		
			$F(":" + ConnPath + 'X_TWSZ-COM_VPN_INTERFACE_REF',  	"");
			
			$F(":" + ConnPath + 'X_TWSZ-COM_VPN_SERVER',  	$('pptp_server').value);
			$F(":" + ConnPath + 'Username', 					$('pptp_username').value);
			if ($('pptp_password').value != '**********')
			{
				var pptp_passwd = Base64.Encode($('pptp_password').value);
				$F(":" + ConnPath + 'Password', 				pptp_passwd);
			}
			$F(":" + ConnPath + 'ConnectionType', 			'PPTP_Relay');
			$F(":" + ConnPath + 'PPPAuthenticationProtocol', 'Auto');	
			
			break;
		case "L2TP" :
			var x = (Form.Radio("l2tp_addr_type") == 'dynamic') ? '1' : '0';
			if(x == "1"){
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"DHCP");  
			} else {
				if($('l2tp_ipaddr').value == "")
				{
					alertError(SEcode['lang_l2tp_ip_empty']);//
					return false;
				}
				if (!CheckValidity.IP("l2tp_ipaddr",SEcode["lang_invalid_ip"]))
					return false;
					
				if($('l2tp_mask').value == "")
				{
					alertError(SEcode['lang_l2tp_mask_empty']);
					return false;
				}
				if (!CheckValidity.Mask("l2tp_mask",SEcode["lang_invalid_mask"]))
					return false;
				if(isSameSubNet($("l2tp_ipaddr").value,$('l2tp_mask').value,G_LanIPAddress,G_LanSubAddress))
				{
					alert(SEcode["lang_lan_wan_conflict"]);	
					$("l2tp_ipaddr").focus();
					return false;
				}
				if($('l2tp_gw').value == "")
				{
					alertError(SEcode['lang_l2tp_gate_empty']);
					return false;
				}
				if (!CheckValidity.IP("l2tp_gw",SEcode["lang_invalid_def_gate"]))
					return false;
				if($('l2tp_ipaddr').value == $('l2tp_gw').value)
				{
					alertError(SEcode['lang_l2tp_gate_same_ip']);
					return false;
				}

				if(!isSameSubNet($("l2tp_gw").value,$('l2tp_mask').value,$('l2tp_ipaddr').value,$('l2tp_mask').value))
				{
					alert(SEcode["lang_gate_same_net_ip"]);
					$("l2tp_gw").focus();
					return false;
				}

				if($('l2tp_ipaddr').value == $('l2tp_server').value)
				{
					alertError(SEcode['lang_l2tp_server_same_ip']);
					return false;
				}
				
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_CLIENT',  	(Form.Radio("l2tp_addr_type") == 'dynamic') ? '' :$('l2tp_ipaddr').value);
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_NETMASK', 	(Form.Radio("l2tp_addr_type") == 'dynamic') ? '' :$('l2tp_mask').value);
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_GATEWAY', 	(Form.Radio("l2tp_addr_type") == 'dynamic') ? '' :$('l2tp_gw').value);
					
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"Static"); 
			
				if(!checkDNS('l2tp_dns'))	
					return false;
			}

			if($('l2tp_server').value == "")
			{
				alertError(SEcode['lang_l2tp_server_empty']);
				return false;
			}
			
			//if (!CheckValidity.IP("l2tp_server",SEcode["lang_invalid_l2tp_server"]))
			//	return false;
			
			if($("l2tp_server").value == G_LanIPAddress)
			{
				alert(SEcode["lang_lan_l2tp_server_conflict"]);
				$("l2tp_server").focus();
				return false;
			}
			if(!checkIdletime('l2tp'))
				return false;
			if(!checkPWD('l2tp'))
				return false;
			if(!checkMTU('ppp4_mtu',_protocol[0]))
				return false;
			/*
			if(!checkMACaddr('ppp4_macaddr'))
				return false;*/
							
			
			var _dnsservers = $('l2tp_dns1').value+','+$('l2tp_dns2').value;	
			$F(":" + ConnPath + 'X_TWSZ-COM_ScheduleListName',  	Form.Radio('l2tp_reconnect_radio') == "Manual" ? "AlwaysOn" : (Form.Radio('l2tp_reconnect_radio') == "OnDemand") ? "AlwaysOn" : $('l2tp_schedule').value);
			$F(":" + ConnPath + "ConnectionTrigger", 	Form.Radio('l2tp_reconnect_radio') == "AlwaysOn" ? $('l2tp_schedule').value : Form.Radio('l2tp_reconnect_radio'));
			$F(":" + ConnPath + "IdleDisconnectTime", 	Form.Radio('l2tp_reconnect_radio') == "OnDemand" ? $('l2tp_max_idle_time').value*60 : undefined);
			$F(":" + ConnPath + "MaxMTUSize", 		$('ppp4_mtu').value);		
			$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
			$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 		_dnsservers.delcomma());
			$F(":" + DevicePath + "X_TWSZ-COM_CloneMACAddress", 		$('ppp4_macaddr').value);
			
			$F(":" + ConnPath + 'X_TWSZ-COM_VPN_INTERFACE_REF',  	"");	
			$F(":" + ConnPath + 'X_TWSZ-COM_VPN_SERVER',  	$('l2tp_server').value);
			$F(":" + ConnPath + 'Username', 					$('l2tp_username').value);
			if ($('l2tp_password').value != '**********')
			{
				var l2tp_passwd = Base64.Encode($('l2tp_password').value);
				$F(":" + ConnPath + 'Password', 				l2tp_passwd);
			}
			$F(":" + ConnPath + 'ConnectionType', 			'L2TP_Relay');
			$F(":" + ConnPath + 'PPPAuthenticationProtocol', 'Auto');	
			
			break;
		case "Bridge" :
		case 'DSLITE' :
			if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] != "4in6" ) //tunnel enable 
			{
				alert(SEcode['lang_set_dslite']);
				return false;
			}

			$F(":" + ConnPath + "Enable", 						'0');

			$F(_tunnelpath + 'Activated', 				'1');
			$F(_tunnelpath + 'Mode', 				'4in6'); //dslite
			$F(_tunnelpath + 'Mechanism', 				'DualStackLite');
			$F(_tunnelpath + 'TunnelName', 				'tunldslite');
			$F(_tunnelpath + 'AssociatedLanIfName', 				'InternetGatewayDevice.LANDevice.1');//目前先锁定
			$F(_tunnelpath + 'AssociatedWanIfName', 				G_Wanipv6Conn['Path']);//目前先锁定

			if(Form.Radio('dslite_addr_type')== "manual")
			{
				if($('aftr_ipaddr6').value == "")
				{
					alert(SEcode["lang_AFTR_ipv6_addr_empty"]);
					return false;
				}
				$F(_tunnelpath + 'RemoteIpv6Address', 				$('aftr_ipaddr6').value);
				$F(_tunnelpath + 'Dynamic', 				'0');
			}
			else
			{
				$F(_tunnelpath + 'Dynamic', 				'1');
			}

			if($('b4_ipaddr_2').value == '' || $('b4_ipaddr_2').value.length <= 0)
			{
				$F(_tunnelpath + 'BorderRelayAddress', 				'');				
			}
			else
			{
				var addr  = "192.0.0." + $('b4_ipaddr_2').value;
				$F(_tunnelpath + 'BorderRelayAddress', 			addr);
			}
			

			break;
		default :
			;
	}

	if(findProtocol()=="rupppoe")			
	{
		if (_protocol[0] != "RUPPPoE")
			$F(":" + DevicePath + "X_TWSZ-COM_DualAccessEnable", "0");
	}
	

	if (_protocol[0] != "PPPoE")
	{
		if ( G_WANConn['ProtocolType'] == "IPv4_6")	 //pppoe share
		{
			alert(SEcode["lang_pppoe_sharewith_ipv4"]);	
			return false;
		}		
	}
/*
	if (G_TunConn['Activated'] == "1" && G_WANConn['Enable'] == "0") //tunnel enable 
	{
		if (_protocol[0] != "DSLITE")
			$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
	}

	$('uiPostForm').submit();
*/	

	document.getElementById("BWAN005").disabled = true;
	document.getElementById("BWAN110").disabled = true;
	setTimeout('enable_save_button()',10*1000);

	var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js&var:page=*";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_Submit, null_errorfunc);
	G_ajax.post($('uiPostForm'));
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
}

//错误处理函数
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//监听加载与错误处理函?
addListeners(uiOnload, dealWithError);

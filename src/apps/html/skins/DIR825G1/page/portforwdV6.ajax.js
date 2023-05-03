G_Error	    = "<?error found?>"; 
G_Status	= "<?error status?>"; 
G_Error_Msg	= "<?error message?>";
G_AjaxToken  = "<?echo $var:sys_Token?>";
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 2
`	<? if gt $21 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection. "Enable AddressingType X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.DefaultRouter X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType"
	`	G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection.$00.";
		G_Wanipv6Conn['uPath'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.";
		G_Wanipv6Conn['Enable'] = "$01";	//Enable test 3
		G_Wanipv6Conn['AddressingType'] = "$02"; //AddressingType
		G_Wanipv6Conn['AddressStatus'] = "$03"; //AddressStatus
		G_Wanipv6Conn['IPv6Address'] = "$04"; //IPv6Address
		G_Wanipv6Conn['PrefixLength'] = "$05"; //PrefixLength
		G_Wanipv6Conn['DefaultRouter'] = "$06"; //DefaultRouter
		G_Wanipv6Conn['IPv6DNSOverrideAllowed'] = "$07"; //IPv6DNSOverrideAllowed
		G_Wanipv6Conn['UsrIPv6DNSServers'] = "$08"; //UsrIPv6DNSServers
		G_Wanipv6Conn['llIPv6Address'] = "$09"; //linklocalipv6
		G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "$0a"; //IPv6PrefixDelegationEnabled
		G_Wanipv6Conn['IPv6AddressingType'] = "$0b"; //AddressingType
		`?>
	`?>

	<? if gt $22 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType"
		`	G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection.$00.";	//Path		
			G_Wanipv6Conn['uPath'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2."; //upath
			G_Wanipv6Conn['Enable'] 			= "$01";	//Enable	
			G_Wanipv6Conn['ConnectionTrigger'] 			= "$02";	//ConnectionTrigger	
			G_Wanipv6Conn['Username'] 			= "$03";	//Username	
			G_Wanipv6Conn['Password'] 			= "$04";	//Password	//"**********"
			G_Wanipv6Conn['PPPoEServiceName'] 			= "$05";	//PPPoEServiceName	
			G_Wanipv6Conn['IdleDelayTime'] 			= "$06";	//IdleDelayTime	
			G_Wanipv6Conn['MaxMTUSize'] 			= "$07";	//MaxMTUSize	
			G_Wanipv6Conn['IPv6DNSOverrideAllowed'] = "$08"; //IPv6DNSOverrideAllowed
			G_Wanipv6Conn['UsrIPv6DNSServers'] = "$09"; //UsrIPv6DNSServers		
			G_Wanipv6Conn['StaticIPAddress'] = "$0a"; //X_TWSZ-COM_StaticIPAddress		
			G_Wanipv6Conn['llIPv6Address'] = "$0b"; //linklocalipv6
			G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "$0c"; //IPv6PrefixDelegationEnabled
			G_Wanipv6Conn['IPv6Address'] = "$0d"; //IPv6Address
			G_Wanipv6Conn['IPv6AddressingType'] = "$0e"; //AddressingType
		`?>
	`?>
`?>
`?>


var G_Wan_Link_status="<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANEthernetLinkConfig.EthernetLinkStatus?>";

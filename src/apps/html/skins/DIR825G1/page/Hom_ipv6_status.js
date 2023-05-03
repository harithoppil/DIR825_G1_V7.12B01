/*  JavaScript Document:Hom_status  */

var G_Wanipv6Conn = [];

<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 2
`	<? if gt $21 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection. "Enable AddressingType X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.2.DefaultRouter X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address ConnectionTime X_TWSZ-COM_IPv6Config.ConnectionStatus X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6DNSServers X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.DefaultRouter "
	`	G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection.$00.";
		G_Wanipv6Conn['uPath'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.";
		G_Wanipv6Conn['Enable'] = "$01";	//Enable
		G_Wanipv6Conn['AddressingType'] = "$02"; //AddressingType
		G_Wanipv6Conn['AddressStatus'] = "$03"; //AddressStatus
		G_Wanipv6Conn['IPv6Address'] = "$04"; //IPv6Address
		G_Wanipv6Conn['PrefixLength'] = "$05"; //PrefixLength
		G_Wanipv6Conn['DefaultRouter'] = "$06"; //DefaultRouter
		G_Wanipv6Conn['IPv6DNSOverrideAllowed'] = "$07"; //IPv6DNSOverrideAllowed
		G_Wanipv6Conn['UsrIPv6DNSServers'] = "$08"; //STATIC UsrIPv6DNSServers
		G_Wanipv6Conn['llIPv6Address'] = "$09"; //linklocalipv6
		
		G_Wanipv6Conn['ConnectionTime'] = "$0a"; //pppoe ipv6 ConnectionTime
		G_Wanipv6Conn['ConnectionStatus'] = "$0b"; //pppoe ipv6 ConnectionStatus
		G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "$0c"; //IPv6PrefixDelegationEnabled
		G_Wanipv6Conn['IPv6DNSServers'] = "$0d"; //AUTO IPv6DNSServers
		G_Wanipv6Conn['StaticDefaultRouter'] = "$0e"; //StaticDefaultRouter
		`?>
	`?>

	<? if gt $22 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength X_TWSZ-COM_IPv6Config.ConnectionStatus ConnectionTime X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.2.DefaultRouter X_TWSZ-COM_IPv6Config.IPv6DNSServers "
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
			G_Wanipv6Conn['pppIPv6Address'] = "$0c"; //pppoe ipv6  addr
			G_Wanipv6Conn['pppIPv6PrefixLength'] = "$0d"; //pppoe ipv6 PrefixLength
			G_Wanipv6Conn['pppIPv6ConnectionStatus'] = "$0e"; //pppoe ipv6 ConnectionStatus
			G_Wanipv6Conn['pppIPv6ConnectionTime'] = "$0F"; //pppoe ipv6 ConnectionTime
			G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "$0g"; //IPv6PrefixDelegationEnabled
			G_Wanipv6Conn['DefaultRouter'] = "$0h"; //DefaultRouter
			G_Wanipv6Conn['IPv6DNSServers'] = "$0i"; //AUTO IPv6DNSServers
		`?>
	`?>
`?>
`?>

var G_TunConn = [];

<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel.  "Activated Mode Mechanism Dynamic Prefix  RemoteIpv4Address  IPv4MaskLen  RemoteIpv6Address BorderRelayAddress LocalIpv6Address LocalIPv6PrefixLength ConnStatus ConnectionTime IPv6DNSServers IPv6Address IPv6PrefixDelegationEnabled IPv6DNSOverrideAllowed"
`	G_TunConn['Path'] 		= ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.$00.";
	G_TunConn['Activated']   = "$01";	//Activated
	G_TunConn['Mode'] 		= "$02";	//Mode
	G_TunConn['Mechanism']   = "$03";	//Dynamic
	G_TunConn['Dynamic']   = "$04";	//RemoteIpv6Address
	G_TunConn['Prefix']   = "$05";	//BorderRelayAddress
	G_TunConn['RemoteIpv4Address']   = "$06";	//Activated
	G_TunConn['IPv4MaskLen'] 		 = "$07";	//Mode
	G_TunConn['RemoteIpv6Address']   = "$08";	//RemoteIpv6Address
	G_TunConn['BorderRelayAddress']   = "$09";	//BorderRelayAddress
	G_TunConn['LocalIpv6Address']   = "$0a";	//BorderRelayAddress
	G_TunConn['LocalIPv6PrefixLength']   = "$0b";	//BorderRelayAddress
	G_TunConn['ConnStatus']   = "$0c";	//BorderRelayAddress
	G_TunConn['ConnectionTime']   = "$0d";	//BorderRelayAddress
	G_TunConn['IPv6DNSServers']   = "$0e";	//BorderRelayAddress
	G_TunConn['IPv6Address']   = "$0f";	//BorderRelayAddress
	G_TunConn['IPv6PrefixDelegationEnabled']   = "$0g";	//BorderRelayAddress
	G_TunConn['IPv6DNSOverrideAllowed']   = "$0h";	//BorderRelayAddress	
`?>

var G_LinklocalAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddress?>";
var G_Linklocalprelen = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6InterfacePrefixLength?>";
var G_GAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress?>";
var G_DeviceUpTime = "<?get :InternetGatewayDevice.DeviceInfo.UpTime?>";

var G_StaticDelegate = "<?get :InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.IPv6SitePrefixConfigType.StaticDelegated?>";
var G_LAN_prefixaddr = "<?get :InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.RadVDConfigManagement.StaticPrefixInfo.Prefix ?>";

var wan_conn_run_time;
//lan
var G_lan_info = []
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement. "DHCPServerEnable IPInterface.1.IPInterfaceIPAddress IPInterface.1.IPInterfaceSubnetMask IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated"
`	var G_lan_dhcp_enable 		= "$01";
	var G_lan_ip_addr     		= "$02";
	var G_lan_mask        		= "$03";
	var G_AddrStaticDelegate = "$04";
`?>
var G_lan_mac = "<?get :InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1.MACAddress ?>";

//DelegatedAddress
var DelegatedAddress ="";
var DelegatedPrefix ="";
var DelegatedPrefixlen ;
<? objget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.DelegatedAddress. "IPv6InterfaceAddress IPv6PrefixAddress IPv6PrefixLength"
`   
	DelegatedAddress 		= "$01";
	DelegatedPrefix     	= "$02";
	DelegatedPrefixlen     	= "$03";
`?>
var G_GAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress?>";

var G_IPv4_ProtocolType = "";

<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 1
`	<? if gt $22 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "X_TWSZ-COM_ProtocolType ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength X_TWSZ-COM_IPv6Config.ConnectionStatus ConnectionTime X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.2.DefaultRouter X_TWSZ-COM_IPv6Config.IPv6DNSServers Enable X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus X_TWSZ-COM_IPv6Config.IPv6AddressList.3.AddressStatus X_TWSZ-COM_IPv6Config.IPv6AddressList.3.IPv6Address "
		`	<?if eq $11 `IPv4_6`
			`	G_IPv4_ProtocolType			= "<?echo $21?>";	//ConnectionTrigger	
				G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.<?echo $20?>."
				G_Wanipv6Conn['uPath'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1."; //upath
				G_Wanipv6Conn['ConnectionTrigger'] 			= "<?echo $22?>";	//ConnectionTrigger	
				G_Wanipv6Conn['Username'] 			= "<?echo $23?>";	//Username	
				G_Wanipv6Conn['Password'] 			= "<?echo $24?>";	//Password	//"**********"
				G_Wanipv6Conn['PPPoEServiceName'] 			= "<?echo $25?>";	//PPPoEServiceName	
				G_Wanipv6Conn['IdleDelayTime'] 			= "<?echo $26?>";	//IdleDelayTime	
				G_Wanipv6Conn['MaxMTUSize'] 			= "<?echo $27?>";	//MaxMTUSize	
				G_Wanipv6Conn['IPv6DNSOverrideAllowed'] = "<?echo $28?>"; //IPv6DNSOverrideAllowed
				G_Wanipv6Conn['UsrIPv6DNSServers'] = "<?echo $29?>"; //UsrIPv6DNSServers		
				G_Wanipv6Conn['StaticIPAddress'] = "<?echo $2a?>"; //X_TWSZ-COM_StaticIPAddress		
				G_Wanipv6Conn['llIPv6Address'] = "<?echo $2b?>"; //linklocalipv6

				G_Wanipv6Conn['pppIPv6Address'] = "<?echo $2c?>"; //pppoe ipv6  addr
				G_Wanipv6Conn['pppIPv6PrefixLength'] = "<?echo $2d?>"; //pppoe ipv6 PrefixLength
				G_Wanipv6Conn['pppIPv6ConnectionStatus'] = "<?echo $2e?>"; //pppoe ipv6 ConnectionStatus
				G_Wanipv6Conn['pppIPv6ConnectionTime'] = "<?echo $2f?>"; //pppoe ipv6 ConnectionTime
				G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "<?echo $2g?>"; //IPv6PrefixDelegationEnabled
				G_Wanipv6Conn['DefaultRouter'] = "<?echo $2h?>"; //DefaultRouter
				G_Wanipv6Conn['IPv6DNSServers'] = "<?echo $2i?>"; //AUTO IPv6DNSServers
				G_Wanipv6Conn['Enable'] 			= "<?echo $2j?>";	//Enable	
				G_Wanipv6Conn['pppAddressStatus'] = "<?echo $2k?>"; //pppoe ipv6  addr status
				G_Wanipv6Conn['lllpppAddressStatus'] = "<?echo $2l?>"; //pppoe ipv6  addr status
				G_Wanipv6Conn['lllpppIPv6Address'] = "<?echo $2m?>"; //pppoe ipv6  addr
			`?>

		`?>
	`?>
`?>	
`?>	


var LanHostsV6 = [];
var n = 0;

<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntriesV6"
`	<?if gt $11 -1
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.HostV6. "IPAddress HostName"
		`	LanHostsV6[n] = [];
			LanHostsV6[n][0] = "$01";
			LanHostsV6[n][1] = "$02";
			++n;
		`?>
	`?>
`?>


var WanV6_Mode = "";
function DHCP6_Renew()
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
	$F(':' + G_Wanipv6Conn['Path'] + 'X_TWSZ-COM_IPv6Config.X_TWSZ-COM_IPv6ReleaseRenew', 'Renew');
	$('uiPostForm').submit();
	$('st_wan_dhcp6_renew').disabled= true;
}

function DHCP6_Release()
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
	$F(':' + G_Wanipv6Conn['Path'] + 'X_TWSZ-COM_IPv6Config.X_TWSZ-COM_IPv6ReleaseRenew', 'Release');
	$('uiPostForm').submit();
	$('st_wan_dhcp6_release').disabled= true;
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
	var DevicePath = G_Wanipv6Conn['Path'];
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
	var DevicePath = G_Wanipv6Conn['Path'];
	$F(":" + DevicePath + "X_TWSZ-COM_ConnectionAction", 	"Disconnect");
	$('uiPostForm').submit();	
	$('st_wan_ppp_disconnect').disabled= true;
}
function wan_conn_time()
{
	wan_conn_run_time++;
	var wan_uptime_sec = wan_conn_run_time%60;
	var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
	var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
	var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
	
	$("st_connection_uptime").innerHTML  = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
	setTimeout('wan_conn_time()', 1000); 
}
function uiOnload(){

	var _dns;
	var ipv6WanAddress;
	
	if (G_TunConn['Activated'] == "1") //tunnel
	{
		if (G_TunConn['Mode'] == "6to4" )
		{
			WanV6_Mode = "6TO4"; //linklocal
		}
		else if (G_TunConn['Mode'] == "6in4" )
		{
			if( G_TunConn['Mechanism'] == "6in4")
				WanV6_Mode = "6IN4"; //linklocal
			else
				WanV6_Mode = "6RD"; //linklocal		
		}		
		
	}

	if (WanV6_Mode == "")
	{
	if(G_IPv4_ProtocolType == "IPv4_6") // PPPOE V6 shared with V4
	{
	   WanV6_Mode = "PPPOE";
       if(G_Wanipv6Conn['pppAddressStatus'] == "Valid"){
		  ipv6WanAddress = G_Wanipv6Conn['pppIPv6Address'];
	   }else{
		  ipv6WanAddress = (G_Wanipv6Conn['lllpppAddressStatus'] == "Valid"?G_Wanipv6Conn['lllpppIPv6Address'] : "NONE");
	   }   			
	}
	else
	
	if(G_Wanipv6Conn['Path'].indexOf('IP') > 0)
	{
		if(G_Wanipv6Conn['Enable'] == '0')
		{
			WanV6_Mode = "LL"; //linklocal
		}
		else
		{
			if (G_Wanipv6Conn['AddressingType'] == "Static")
			{
				WanV6_Mode = "STATIC";				
			}
			else
			{
				WanV6_Mode = "AUTO";								
			}
		}
	}
	else if(G_Wanipv6Conn['Path'].indexOf('PPP') > 0)
	{
		if(G_Wanipv6Conn['Enable'] == '0')
		{
			WanV6_Mode = "LL";
		}
		else
		{
			WanV6_Mode = "PPPOE";
			ipv6WanAddress = G_Wanipv6Conn['pppIPv6Address'];
		}
	}

	}
	
	
	switch(WanV6_Mode)
	{
		case "LL":
			$("ll_ipv6").style.display = "block";
			$("ipv6").style.display = "none";
			
			$("ll_type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS028;
			$("ll_gateway").innerHTML = data_languages.Public.innerHTML.Public022;
			$("ll_lan_ll_address").innerHTML = G_LinklocalAddr;
			_dns = G_Wanipv6Conn['IPv6DNSOverrideAllowed']=="1" ? G_Wanipv6Conn['UsrIPv6DNSServers'] :G_Wanipv6Conn['IPv6DNSServers'] ;
			
			break;
		case "AUTO":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS029;
			$("status").innerHTML = G_Wanipv6Conn['ConnectionStatus'];
			if(G_Wanipv6Conn['ConnectionTime'] > 0)
			{
				wan_conn_run_time = G_DeviceUpTime - G_Wanipv6Conn['ConnectionTime'];
				//alert(wan_conn_run_time);
				var wan_uptime_sec = wan_conn_run_time%60;
				var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
				var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
				var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
				$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
				wan_conn_time();
			}
			else
			{
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}
			
			$("st_wan_dhcp6_action").style.display = "block";
			if(G_Wanipv6Conn['ConnectionStatus'] == "GlobalConnected")
			{
				$("st_wan_dhcp6_renew").disabled = true;
				$("st_wan_dhcp6_release").disabled = false;
				$("wan_address").innerHTML = G_Wanipv6Conn['IPv6Address'] || data_languages.Public.innerHTML.Public022;
				$("wan_address_pl").innerHTML = G_Wanipv6Conn['IPv6Address']!= "" ?"/"+G_Wanipv6Conn['PrefixLength'] : "";
			} 
			else if(G_Wanipv6Conn['ConnectionStatus']== "LinkLocalConnected")
			{
				$("st_wan_dhcp6_renew").disabled = false;
				$("st_wan_dhcp6_release").disabled = true;
				//$("st_wan_dhcp6_renew").disabled = true;
				//$("st_wan_dhcp6_release").disabled = false;
				$("wan_address").innerHTML = G_Wanipv6Conn['llIPv6Address'] || data_languages.Public.innerHTML.Public022;
			}
			else
			{
				$("st_wan_dhcp6_renew").disabled = false;
				$("st_wan_dhcp6_release").disabled = true;
				$("wan_address").innerHTML = data_languages.Public.innerHTML.Public022;				
			}
			$("gateway").innerHTML = G_Wanipv6Conn['DefaultRouter'] || data_languages.Public.innerHTML.Public022;
			_dns = G_Wanipv6Conn['IPv6DNSOverrideAllowed']=="1" ? G_Wanipv6Conn['UsrIPv6DNSServers'] :G_Wanipv6Conn['IPv6DNSServers'] ;
			
			break;	
		case "STATIC":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS030;
			$("status").innerHTML = G_Wanipv6Conn['ConnectionStatus'];
			if(G_Wanipv6Conn['ConnectionTime'] > 0)
			{
				wan_conn_run_time = G_DeviceUpTime - G_Wanipv6Conn['ConnectionTime'];
				//alert(wan_conn_run_time);
				var wan_uptime_sec = wan_conn_run_time%60;
				var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
				var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
				var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
				$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
				wan_conn_time();
			}
			else
			{
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}
			$("wan_address").innerHTML = G_Wanipv6Conn['IPv6Address'] || data_languages.Public.innerHTML.Public022;
			$("wan_address_pl").innerHTML = G_Wanipv6Conn['IPv6Address']!= "" ?"/"+G_Wanipv6Conn['PrefixLength'] : "";
			$("gateway").innerHTML = G_Wanipv6Conn['StaticDefaultRouter'] || data_languages.Public.innerHTML.Public022;	
			_dns = G_Wanipv6Conn['IPv6DNSOverrideAllowed']=="1" ? G_Wanipv6Conn['UsrIPv6DNSServers'] :G_Wanipv6Conn['IPv6DNSServers'] ;
			
			break;
		case "PPPOE":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS031;
			$("status").innerHTML = G_Wanipv6Conn['pppIPv6ConnectionStatus']=="GlobalConnected" ? data_languages.Public.innerHTML.Public032 : data_languages.Public.innerHTML.Public033;
			
			if(G_Wanipv6Conn['pppIPv6ConnectionTime'] > 0)
			{
				wan_conn_run_time = G_DeviceUpTime - G_Wanipv6Conn['pppIPv6ConnectionTime'];
				var wan_uptime_sec = wan_conn_run_time%60;
				var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
				var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
				var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
				$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
				wan_conn_time();
			}
			else
			{
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}
			$("st_wan_ppp_action").style.display = "block";
			if(G_Wanipv6Conn['pppIPv6ConnectionStatus'] == "GlobalConnected")
			{
				$("st_wan_ppp_connect").disabled = true;
				$("st_wan_ppp_disconnect").disabled = false;
				$("wan_address").innerHTML = ipv6WanAddress || data_languages.Public.innerHTML.Public022;
				$("wan_address_pl").innerHTML = ipv6WanAddress!= "" ?"/"+G_Wanipv6Conn['pppIPv6PrefixLength'] : "";
			}
			else if(G_Wanipv6Conn['pppIPv6ConnectionStatus'] == "LinkLocalConnected")
			{
				$("st_wan_ppp_connect").disabled = true;
				$("st_wan_ppp_disconnect").disabled = false;
				$("wan_address").innerHTML = G_Wanipv6Conn['llIPv6Address'] || data_languages.Public.innerHTML.Public022;
				
			}
			else
			{
				$("st_wan_ppp_connect").disabled = false;
				$("st_wan_ppp_disconnect").disabled = true;
				$("wan_address").innerHTML = data_languages.Public.innerHTML.Public022;
			}
			$("gateway").innerHTML = G_Wanipv6Conn['DefaultRouter'] || data_languages.Public.innerHTML.Public022;
			_dns = G_Wanipv6Conn['IPv6DNSOverrideAllowed']=="1" ? G_Wanipv6Conn['UsrIPv6DNSServers'] :G_Wanipv6Conn['IPv6DNSServers']; 
			
			break;	
		case "6IN4":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS032;
			$("status").innerHTML = G_TunConn['ConnStatus'];

			if (G_TunConn['ConnStatus'] == "Connected")
			{
				$("wan_address").innerHTML = G_TunConn['LocalIpv6Address'] || data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = G_TunConn['RemoteIpv6Address'];
				_dns = G_TunConn['IPv6DNSServers'];
				if(G_TunConn['ConnectionTime'] > 0)
				{
					wan_conn_run_time = G_DeviceUpTime - G_TunConn['ConnectionTime'];
					//alert(wan_conn_run_time);
					var wan_uptime_sec = wan_conn_run_time%60;
					var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
					var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
					var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
					$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
					wan_conn_time();
				}
				else
				{
					$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
				}
				
			}
			else
			{				
				$("wan_address").innerHTML = data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = data_languages.Public.innerHTML.Public022;
				_dns = "";								
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}			


			break;
		case "6TO4":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS033;
			$("status").innerHTML = G_TunConn['ConnStatus'];

				
			if (G_TunConn['ConnStatus'] == "Connected")
			{
				$("wan_address").innerHTML = G_TunConn['IPv6Address'] || data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = "::" + G_TunConn['BorderRelayAddress'];
				_dns = G_TunConn['IPv6DNSServers'];				
				if(G_TunConn['ConnectionTime'] > 0)
				{
					wan_conn_run_time = G_DeviceUpTime - G_TunConn['ConnectionTime'];
					//alert(wan_conn_run_time);
					var wan_uptime_sec = wan_conn_run_time%60;
					var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
					var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
					var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
					$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
					wan_conn_time();
				}
				else
				{
					$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
				}
			}
			else
			{				
				$("wan_address").innerHTML = data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = data_languages.Public.innerHTML.Public022;
				_dns = "";								
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}
				
			G_AddrStaticDelegate = "1";

			break;
		case "6RD":
			$("ll_ipv6").style.display = "none";
			$("ipv6").style.display = "block";
			$("type").innerHTML = data_languages.Hom_ipv6_status.innerHTML.HIS034;
			$("status").innerHTML = G_TunConn['ConnStatus'];

			
			G_AddrStaticDelegate = "1";
			if (G_TunConn['ConnStatus'] == "Connected")
			{
				$("wan_address").innerHTML = G_TunConn['IPv6Address'] || data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = "::" + G_TunConn['BorderRelayAddress'];
				_dns = G_TunConn['IPv6DNSServers'];
				if(G_TunConn['ConnectionTime'] > 0)
				{
					wan_conn_run_time = G_DeviceUpTime - G_TunConn['ConnectionTime'];
					//alert(wan_conn_run_time);
					var wan_uptime_sec = wan_conn_run_time%60;
					var wan_uptime_min = Math.floor(wan_conn_run_time/60)%60;
					var wan_uptime_hour = Math.floor(wan_conn_run_time/3600)%24;
					var wan_uptime_day = Math.floor(wan_conn_run_time/86400);
					$("st_connection_uptime").innerHTML = wan_uptime_day+" "+data_languages.Public.innerHTML.Public023+" "+wan_uptime_hour+" "+data_languages.Public.innerHTML.Public024+" "+wan_uptime_min+" "+data_languages.Public.innerHTML.Public025+" "+wan_uptime_sec+" "+data_languages.Public.innerHTML.Public026;
					wan_conn_time();
				}
				else
				{
					$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
				}
			}
			else
			{				
				$("wan_address").innerHTML = data_languages.Public.innerHTML.Public022;
				$("gateway").innerHTML = data_languages.Public.innerHTML.Public022;
				_dns = "";								
				$("st_connection_uptime").innerHTML = "0 "+data_languages.Public.innerHTML.Public023+" "+"0 "+data_languages.Public.innerHTML.Public024+" "+"0 "+data_languages.Public.innerHTML.Public025+" "+"0 "+data_languages.Public.innerHTML.Public026;
			}
			
			break;	
	}
		
	var iparr = _dns.split(",");		
	supplyValue('br_dns1', iparr[0] || data_languages.Public.innerHTML.Public022);
	supplyValue('br_dns2', iparr[1] || data_languages.Public.innerHTML.Public022);

	$("lan_ll_address").innerHTML = G_LinklocalAddr;
	$("lan_ll_pl").innerHTML = "/"+G_Linklocalprelen;	
	
	var G_PD6_Enabled = "";

	if(WanV6_Mode == "6IN4" || WanV6_Mode == "6RD" || WanV6_Mode == "6TO4" )
	{
		G_PD6_Enabled = G_TunConn['IPv6PrefixDelegationEnabled'];
	}
	else
	{
		G_PD6_Enabled = G_Wanipv6Conn['IPv6PrefixDelegationEnabled'];		
	}

	$("enable_pd").innerHTML = G_PD6_Enabled=="1"? data_languages.Public.innerHTML.Public001 : data_languages.Public.innerHTML.Public002;
	
	var lan_prefix ="" ;
	if ( G_StaticDelegate == "0")
	{
		lan_prefix = DelegatedPrefix ;
	}
	else
	{
		lan_prefix = G_LAN_prefixaddr ;		
	}
	if(lan_prefix!= "")
	{
		if(lan_prefix!="")
		{
			$("pd_prefix").innerHTML = lan_prefix.split("::")[0];
			$("pd_pl").innerHTML = lan_prefix.split("::")[1];
		}
	}
	else	
	{
		$("pd_prefix").innerHTML = data_languages.Public.innerHTML.Public022;
	}

	if (G_AddrStaticDelegate == "0")
	{
		$("lan_address").innerHTML = DelegatedAddress || data_languages.Public.innerHTML.Public022;	 		 
	}
	else
	{
		$("lan_address").innerHTML = G_GAddr || data_languages.Public.innerHTML.Public022;	 		 		
	}
	
	$("lan_pl").innerHTML = G_GAddr!="" ? "/64" : "";

	/*
	for (var i=0; i<LanHostsV6.length-1; i++){
		for (var j=i+1; j<LanHostsV6.length; j++){
			if (LanHostsV6[i][1] == LanHostsV6[j][1]){	//same MACAddress
				LanHostsV6.deleteIndex(i);	//alert("LanHosts.deleteIndex("+i+")");
			}
		}
	}
	*/
	$T('client6_list',LanHostsV6);	
}


addListeners(uiOnload);

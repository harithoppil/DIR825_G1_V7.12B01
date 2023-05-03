var G_Wanipv6Conn = [];
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
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength"
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
			G_Wanipv6Conn['PrefixLength'] = "$0f"; //PrefixLength
		`?>
	`?>
`?>
`?>

var G_WanMac = '<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.X_TWSZ-COM_MACAddress?>'.toUpperCase();
var G_ULAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.UniqueLocalAddress?>";
var G_GAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress?>";
var G_LinklocalAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddress?>";
var hintsupport = "0";

<?mget :InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement. "RADVD.Enabled RADVD.ManagedFlag RADVD.OtherConfFlag AutoConfigurationAddress ServerType.StatefullDHCPv6.MinInterfaceID ServerType.StatefullDHCPv6.MaxInterfaceID RADVD.RouterLifeTime ServerType.StatefullDHCPv6.DHCPv6LeaseTime RadVDConfigManagement.StaticPrefixInfo.Prefix"
`	var G_RadvdEnable = "$01";
	var G_RadvdMFlag = "$02";
	var G_RadvdOFlag = "$03";
	var G_DhcpsEnable = "$04";
	var G_MinInterfaceID = "$05";
	var G_MaxInterfaceID = "$06";
	var G_RadvdLifeTime = "$07";
	var G_DHCPv6LeaseTime = "$08";
	var G_StaticPrefix = "$09";
`?>

//TUNNEL 
var G_TunConn = [];

<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel.  "Activated Mode Mechanism Dynamic Prefix  RemoteIpv4Address  IPv4MaskLen  RemoteIpv6Address BorderRelayAddress LocalIpv6Address LocalIPv6PrefixLength IPv6DNSServers LinkLocalAddress IPv6Address SubnetID IPv6PrefixDelegationEnabled IPv6DNSOverrideAllowed"
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
	G_TunConn['IPv6DNSServers']   = "$0c";	//BorderRelayAddress
	G_TunConn['LinkLocalAddress']   = "$0d";	//BorderRelayAddress
	G_TunConn['IPv6Address']   = "$0e";	//BorderRelayAddress
	G_TunConn['SubnetID']   = "$0f";	//BorderRelayAddress
	G_TunConn['IPv6PrefixDelegationEnabled']   = "$0g";	//BorderRelayAddress
	G_TunConn['IPv6DNSOverrideAllowed']   = "$0h";	//BorderRelayAddress
`?>

//ipv4 if eq $11 `IPv4`
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 1
`	<? if gt $21 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "X_TWSZ-COM_ProtocolType ExternalIPAddress"
		`	<?if eq $11 `IPv4`
			`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.<?echo $20?>";		
				G_WANConn['ExternalIPAddress'] = "<?echo $22?>";		
			`?>
		`?>
	`?>
	<? if gt $22 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "X_TWSZ-COM_ProtocolType ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType Enable X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength ExternalIPAddress"
		`	<?if eq $11 `IPv4`
			`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.<?echo $20?>";	//Path		
				G_WANConn['ExternalIPAddress'] = "<?echo $2h?>"; //PrefixLength
			`?>			
			<?if eq $11 `IPv4_6`
			`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.<?echo $20?>";	//Path		
				G_WANConn['X_TWSZ-COM_ProtocolType'] 			= "<?echo $21?>";	//ConnectionTrigger	
				//G_Wanipv6Conn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.<?echo $20?>."
				//G_Wanipv6Conn['uPath'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1."; //upath
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
				G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] = "<?echo $2c?>"; //IPv6PrefixDelegationEnabled
				G_Wanipv6Conn['IPv6Address'] = "<?echo $2d?>"; //IPv6Address
				G_Wanipv6Conn['IPv6AddressingType'] = "<?echo $2e?>"; //AddressingType			
				G_Wanipv6Conn['Enable'] 			= "<?echo $2f?>";	//Enable	
				G_Wanipv6Conn['PrefixLength'] = "<?echo $2g?>"; //PrefixLength
				G_WANConn['ExternalIPAddress'] = "<?echo $2h?>"; //PrefixLength
			`?>

		`?>
	`?>
`?>	
`?>	
	

/*** AUTO/AUTODETECT/STATIC/PPPOE/6IN4/6RD/6TO4/LL ***/
var Wan_Mode = "";
var Lan_Mode = ""

//IPv6自动填0
function IPv6FillZero(s) 
{
	var reCat = /^\s*((([0-9A-Fa-f]{1,4}:){7}(([0-9A-Fa-f]{1,4})|:))|(([0-9A-Fa-f]{1,4}:){6}(:|((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})|(:[0-9A-Fa-f]{1,4})))|(([0-9A-Fa-f]{1,4}:){5}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4}){0,1}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){0,3}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:)(:[0-9A-Fa-f]{1,4}){0,4}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(:(:[0-9A-Fa-f]{1,4}){0,5}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})))(%.+)?\s*$/gi;
	if(!reCat.test(s)){
		return "error";
	}
	var a = s.split("::");
	var l = s.split(":").length;
	if(l==8)	return s;
	var j;
	s = '';
	s += a[0];
	s += ":";
	if(l<8){
			for(j=0;j<9-l;j++){
				s+='0:'
			}
	}
	a[1]==""?s+="0":s+=a[1];
	return s;	
}

function COMM_SetSelectValue($, value)
{
	for (var i=0; i < $.length; i+=1)
		if ($[i].value == value)
		{
			$.selectedIndex = i;
			break;
		}
	return $.selectedIndex;
}
function InitWANStaticValue()
{
	if(Wan_Mode != "STATIC") 
		return true;

	$("l_ipaddr").disabled = false;
	//var G_usell = "1"; //linklocal static
	var G_usell = G_Wanipv6Conn['AddressStatus'];
	
	if(G_usell == "Invalid")
	{
		$("usell").checked = true;
		$("w_st_ipaddr").disabled = true;
		$("w_st_pl").disabled = true;
		$("w_st_ipaddr").value	= G_Wanipv6Conn['llIPv6Address']; // linklocal
		$("w_st_pl").value		= 64;
	}
	else
	{
		$("usell").checked = false;
		$("w_st_ipaddr").disabled = false;
		$("w_st_pl").disabled = false;
		$("w_st_ipaddr").value	= G_Wanipv6Conn['IPv6Address']; // static or dhcp
		$("w_st_pl").value	= G_Wanipv6Conn['PrefixLength']; // static or dhcp
	}						
	//$("w_st_ipaddr").value	= ""; //G_Wanipv6Conn['llIPv6Address']

	
	//$("w_st_gw").value = "";
	//$("w_st_pdns").value = "";
	//$("w_st_sdns").value = "";
	$("w_st_gw").value = G_Wanipv6Conn['DefaultRouter'];
	if(G_Wanipv6Conn['UsrIPv6DNSServers'] != "")
	{
		var iparr = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",");		
		supplyValue('w_st_pdns', iparr[0]);
		supplyValue('w_st_sdns', iparr[1]);
	}

	return true;
}
function InitWANDHCPValue()
{
	//alert(Wan_Mode);
	if(Wan_Mode != "AUTO" && Wan_Mode != "AUTODETECT") 
		return true;
	//alert(G_Wanipv6Conn['UsrIPv6DNSServers']);
	/*
	if(G_Wanipv6Conn['UsrIPv6DNSServers'] != "")
	{
		var iparr = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",");		
		supplyValue('w_dhcp_pdns', iparr[0]);
		supplyValue('w_dhcp_sdns', iparr[1]);
	}
	*/
	return true;
}
function InitWANPPPoEValue ()
{
/*	
	$("pppoe_dynamic").checked        = true;
	$("pppoe_ipaddr").value           = "";
	$("pppoe_sess_share").checked        = true;
	$("pppoe_username").value         = "";
	$("pppoe_password").value         = "";
	$("confirm_pppoe_password").value = "";
	$("ppp6_mtu").value = "1492";
	$("pppoe_service_name").value     = "";
*/	
	if(Wan_Mode != "PPPOE") 
		return true;

	$("pppoe_ipaddr").value           = G_Wanipv6Conn['IPv6Address'];//G_Wanipv6Conn['StaticIPAddress'];
	$("pppoe_sess_share").checked        = true;
	$("pppoe_username").value         = G_Wanipv6Conn['Username'];
	$("pppoe_password").value         = "**********"; //G_Wanipv6Conn['Password'];
	$("confirm_pppoe_password").value = "**********"; //G_Wanipv6Conn['Password'];
	$("ppp6_mtu").value = G_Wanipv6Conn['MaxMTUSize'];
	$("pppoe_service_name").value     = G_Wanipv6Conn['PPPoEServiceName'];

	var pppoe_sess_share = "0";
	if (G_WANConn['X_TWSZ-COM_ProtocolType'] == "IPv4_6")
	{
		pppoe_sess_share = "1";		
	}
	if(pppoe_sess_share == "1")
	{
		$("pppoe_sess_share").checked = true;
		$("pppoe_sess_new").checked = false;
	}
	else
	{              
		$("pppoe_sess_share").checked = false;
		$("pppoe_sess_new").checked = true;
		
	}

	OnClickPppoeSessType();
	
	//var pppoe_static = "0";
	  var pppoe_static = G_Wanipv6Conn['IPv6AddressingType'];
	if(pppoe_static == "Static")
	{
		$("pppoe_static").checked = true;
		$("pppoe_dynamic").checked = false;
	}
	else
	{              
		$("pppoe_dynamic").checked = true;
		$("pppoe_static").checked = false;
	}
	OnClickPppoeAddrType();
	
	//var pppoe_reconnect_mode   = "manual";
	var pppoe_reconnect_mode = G_Wanipv6Conn['ConnectionTrigger']; // AlwaysOn Manual On-Demand
	if(pppoe_reconnect_mode === "Manual")		
		$("pppoe_manual").checked = true;
	else if (pppoe_reconnect_mode === "OnDemand")		 //only share session				
		$("pppoe_ondemand").checked = true;
	else						
		$("pppoe_alwayson").checked = true;
	
	/* DNS */
	/*
	if ( G_Wanipv6Conn['IPv6DNSOverrideAllowed'] == "1" )
	{
		$("w_dhcp_dns_manual").checked = true;		
		if(G_Wanipv6Conn['UsrIPv6DNSServers'] != "")
		{
			var iparr = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",");		
			supplyValue('w_dhcp_pdns', iparr[0]);
			supplyValue('w_dhcp_sdns', iparr[1]);
		}

	}
	else
	{
		$("w_dhcp_dns_auto").checked = true;				
	}
	*/
	/*
	var dns_srv_auto = "1";
	if (dns_srv_auto == "1")
	{
		$("w_dhcp_dns_manual").checked = true;
	}
	else 
	{
		$("w_dhcp_dns_auto").checked = true;
		
	}
	$("w_dhcp_pdns").value = "";
	$("w_dhcp_sdns").value = "";
	*/

	var en_auto_dhcp_pd = G_Wanipv6Conn['IPv6PrefixDelegationEnabled'];//"1";
	if(en_auto_dhcp_pd == "1")
	{		
		$("en_dhcp_pd").checked = true;
		$("l_ipaddr").disabled = true;
		$("l_ipaddr").value 	=  G_GAddr; //"fe80::c2a0:bbff:fefd:8d2c";
	}
	else
	{
		$("en_dhcp_pd").checked = false;
		$("l_ipaddr").disabled = false;
		$("l_ipaddr").value	= G_GAddr;
	}
	
	if(hintsupport == "1")
	{
		var en_manual_IA_PD = "1";
		if(en_manual_IA_PD == "1")
		{
			$("en_dhcp_pd_hint").checked = true;
			$("dhcppd_hint_prefix").value = "";
			$("dhcppd_hint_pfxlen").value = "";
			$("dhcppd_hint_plft").value = "";
			
			$("dhcppd_hint_vlft").value = parseInt("0")/60;
			
		}
		else
		{
			$("en_dhcp_pd_hint").checked = false;
			$("dhcppd_hint_prefix").disabled = $("dhcppd_hint_pfxlen").disabled = $("dhcppd_hint_plft").disable 
			= $("dhcppd_hint_vlft").disabled = true;
		}
		OnClickpdhint();
	}
	return true;
}

function InitWAN6IN4Value()
{
	if(Wan_Mode != "6IN4") return true;

	$("w_tu_lov6_ipaddr").value	= G_TunConn['LocalIpv6Address'] ;
	$("w_tu_pl").value		= G_TunConn['LocalIPv6PrefixLength'] ;
	$("w_tu_rev6_ipaddr").value	= G_TunConn['RemoteIpv6Address'];
	$("w_tu_rev4_ipaddr").value	= G_TunConn['RemoteIpv4Address'];
	//$("w_st_pdns").value		= ""; //linsd reverse
	//$("w_st_sdns").value		= "";

	$("w_tu_lov4_ipaddr").innerHTML = G_WANConn['ExternalIPAddress'] ||"X.X.X.X";
	
	if (G_TunConn['IPv6DNSOverrideAllowed'] == "1") //if(G_TunConn['IPv6DNSServers'] != "")
	{
		$("w_dhcp_dns_manual").checked = true;				
		//COMM_SetSelectValue("w_dhcp_dns_rad", "manual");
		if(G_TunConn['IPv6DNSServers'] != "")
		{
			var iparr = G_TunConn['IPv6DNSServers'].split(",");		
			supplyValue('w_dhcp_pdns', iparr[0]);
			supplyValue('w_dhcp_sdns', iparr[1]);			
		}
	}
	else
	{
		$("w_dhcp_dns_auto").checked = true;				
		//COMM_SetSelectValue("w_dhcp_dns_rad", "auto");				
	}
	
	return true;
}

function InitWAN6RDValue()
{
	if(Wan_Mode != "6RD") return true;
	
	var str;
	
	$("w_6rd_v4addr").innerHTML	= G_WANConn['ExternalIPAddress'];
	
	dhcpv4_6rd = "1";
	if(G_TunConn['Dynamic'] == "0")
	{ 
		var prefix = G_TunConn['Prefix'].split("/");	
		$("w_6rd_prefix_1").value 	= prefix[0] ;
		$("w_6rd_prefix_2").value 	= prefix[1] ;
		$("w_6rd_v4addr_mask").value 	= G_TunConn['IPv4MaskLen'] ;
		$("w_6rd_relay").value	= G_TunConn['BorderRelayAddress'] ;
		
		$("6rd_dhcp_option").checked = false;
		$("6rd_manual").checked = true;
		//COMM_SetSelectValue("6rd_dhcp_option_rad", "6rd_manual");
	}
	else
	{

		$("w_6rd_prefix_1").disabled 	= true;
		$("w_6rd_prefix_2").disabled 	= true;
		$("w_6rd_v4addr_mask").disabled = true;
		$("w_6rd_relay").disabled = true;
		$("6rd_manual").checked = false;
		$("6rd_dhcp_option").checked = true;
		//COMM_SetSelectValue("6rd_dhcp_option_rad", "6rd_dhcp_option");
	}
			
	var w_6rd_prefix_3 = G_StaticPrefix;
	if(w_6rd_prefix_3 != "")
	{
		$("w_6rd_prefix_3").innerHTML = w_6rd_prefix_3;
	}
	else
		$("w_6rd_prefix_3").innerHTML = data_languages.Bas_ipv6_man_setup.innerHTML.BIPV6MANNONE;
	
	var w_6rd_ll_addr = G_TunConn['LinkLocalAddress'];
	if(w_6rd_ll_addr != "")
	{
		$("w_6rd_ll_addr").innerHTML = w_6rd_ll_addr;
	}
	else
		$("w_6rd_ll_addr").innerHTML = data_languages.Bas_ipv6_man_setup.innerHTML.BIPV6MANNONE;
	
	if (G_GAddr != "")
	{
		$("l_span_6rd").innerHTML = G_GAddr + "/64";		
	}
	else
		$("l_span_6rd").innerHTML = data_languages.Bas_ipv6_man_setup.innerHTML.BIPV6MANNONE;

	if(G_TunConn['IPv6DNSServers'] != "")
	{
		var iparr = G_TunConn['IPv6DNSServers'].split(",");		
		supplyValue('w_6rd_pdns', iparr[0]);
		supplyValue('w_6rd_sdns', iparr[1]);
	}
	//$("w_6rd_pdns").value		= "";
	//$("w_6rd_sdns").value		= "";
	
	var hubspokemode = "1"; 
	if(hubspokemode == "1")
	{
		$("en_hub_spoke").checked = true;
	}
	else
	{
		$("en_hub_spoke").checked = false;
	}
	

	return true;
}

function InitWAN6TO4Value()
{
	if(Wan_Mode != "6TO4") return true;
	
	
	$("l_ipaddr_6to4").value = G_TunConn['SubnetID']; //
	
	$("w_6to4_ipaddr").innerHTML = G_TunConn['IPv6Address'];
	
	$("l_prefix_6to4").innerHTML = "XXXX:XXXX:XXXX:";
	$("l_ipaddreui_6to4").innerHTML = ":1";
	
	$("w_6to4_relay").value	= G_TunConn['BorderRelayAddress'];
	if(G_TunConn['IPv6DNSServers'] != "")
	{
		var iparr = G_TunConn['IPv6DNSServers'].split(",");		

		supplyValue('w_6to4_pdns', iparr[0]);
		supplyValue('w_6to4_sdns', iparr[1]);
	}
	//$("w_6to4_pdns").value	= "";
	//$("w_6to4_sdns").value	= "";
		
	return true;
}
	
function InitWANConnMode()
{
	
	COMM_SetSelectValue($("wan_ipv6_mode"), Wan_Mode);
		
	return true;
}
function InitLANConnMode()
{
	//alert("InitLANConnMode");
	/*
	if(!dhcps6) return false;
	var rdnss = XG(rdnss+"/device/rdnss");
	var lan_type = XG(dhcps6+"/mode");
	switch(XG(dhcps6+"/mode"))
	{
		case "STATELESS":
			if(rdnss == 1) COMM_SetSelectValue($("lan_auto_type"), "STATELESSR");
			else COMM_SetSelectValue($("lan_auto_type"), "STATELESSD"); 
			break;
		case "STATEFUL": 		
			COMM_SetSelectValue($("lan_auto_type"), "STATEFUL")
			break;
	}
	*/
	if(G_RadvdEnable == "0" && G_RadvdEnable == "0")
		return true;

	if (G_RadvdMFlag == "1" )
	{
		if (G_RadvdOFlag == "1")		
			COMM_SetSelectValue($("lan_auto_type"), "STATEFUL");
	}
	else
	{
		if (G_RadvdOFlag == "0")		
			COMM_SetSelectValue($("lan_auto_type"), "STATELESSR");		
		else
			COMM_SetSelectValue($("lan_auto_type"), "STATELESSD");		
	}
	
	return true;
}
function InitWANInfo()
{
	//alert("InitWANInfo");
	if(Wan_Mode == "6IN4" || Wan_Mode == "6RD" || Wan_Mode == "6TO4" )
	{
		if (G_TunConn['IPv6PrefixDelegationEnabled'] == "1")
			$("en_dhcp_pd").checked = true;
		else
			$("en_dhcp_pd").checked = false;		
		
	}
	else
	{
		
		if (G_Wanipv6Conn['IPv6PrefixDelegationEnabled'] == "1")
			$("en_dhcp_pd").checked = true;
		else
			$("en_dhcp_pd").checked = false;
			
		if ( G_Wanipv6Conn['IPv6DNSOverrideAllowed'] == "1" )
		{
			$("w_dhcp_dns_manual").checked = true;		
			if(G_Wanipv6Conn['UsrIPv6DNSServers'] != "")
			{
				var iparr = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",");		
				//$("w_st_pdns").value  = iparr[0];
				//$("w_st_pdns").value  = iparr[1];
				supplyValue('w_dhcp_pdns', iparr[0]);
				supplyValue('w_dhcp_sdns', iparr[1]);
			}
			
		}
		else
		{
			$("w_dhcp_dns_auto").checked = true;				
		}		
		
	}


	
	$("l_ipaddr").disabled = true;
	$("pppoe_dynamic").checked = true; 
	$("pppoe_sess_new").checked = true; 
	$("6rd_dhcp_option").checked = true; 
	
	/* init value */
	if (!InitWANStaticValue()) return false;
	if (!InitWANDHCPValue()) return false;
	if (!InitWANPPPoEValue()) return false;
	if (!InitWAN6IN4Value()) return false;
	if (!InitWAN6RDValue()) return false;
	if (!InitWAN6TO4Value()) return false;
	

	return true;
}
function InitWANLLInfo()
{
	//alert("InitWANLLInfo");
	
	$("wan_ll").innerHTML    = "";
	$("wan_ll_pl").innerHTML    = "/64";

	return true;
}
function InitLANInfo()
{
	//alert("InitLANInfo");
	
	//if("STATIC" == Lan_Mode)
	if(Wan_Mode == "6IN4" )
	{
		if ("0" == G_TunConn['IPv6PrefixDelegationEnabled']) //pd disable
		{
			$("l_ipaddr").value	= G_GAddr;
			$("l_range_start_pl").innerHTML	= "/64";
			$("l_range_end_pl").innerHTML	= "/64";
		}
	}
	else 
	if ("0" == G_Wanipv6Conn['IPv6PrefixDelegationEnabled']) //pd disable
	{
		$("l_ipaddr").value	= G_GAddr;
		if(Wan_Mode == "STATIC")
		{
			$("l_range_start_pl").innerHTML	= "/64";
			$("l_range_end_pl").innerHTML	= "/64";
		}
		else //6IN4
		{
			;
		}
	}

	/* fill some fixed info */
	$("l_pl").innerHTML	= "/64";

	return true;
}
function InitLANLLInfo(addrmode)
{
	//alert("InitLANLLInfo");
	var lan_svcsname = "1";
	if( lan_svcsname != null ) /* this should not happen */
	{
		//$("lan_ll").innerHTML    = "fe80::c2a0:bbff:fefd:8d2c";
		$("lan_ll").innerHTML    = G_LinklocalAddr;
		$("lan_ll_pl").innerHTML    = "/64";
	}
	//$("lan_ul").innerHTML = "fe80::c2a0:bbff:fefd:8d2c";
	$("lan_ul").innerHTML = G_ULAddr;
	$("lan_ul_pl").innerHTML    = "/64";
	return true;
}
function InitDHCPS6()
{
	
	//var enable_dhcps6 = "STATELESS";
	var enable_dhcps6 = G_RadvdMFlag;
	switch (enable_dhcps6)
	{
		case "0":
			$("box_lan_dhcp").style.display = "none";
			$("box_lan_stless").style.display = "";
			
			$("ra_lifetime").value = parseInt(G_RadvdLifeTime)/60;
			
			break;
				 
		case "1":
			$("box_lan_dhcp").style.display = "";
			
			$("dhcps_start_ip_value").value = G_MinInterfaceID;
			$("dhcps_stop_ip_value").value = G_MaxInterfaceID;
			$("dhcps_start_ip_prefix").value = "";
			$("dhcps_stop_ip_prefix").value = "";
			
			$("ip_lifetime").value = parseInt(G_DHCPv6LeaseTime)/60;
			break;
	}
	
	return true;
}
function OnClickAuto()
{
	if($("enableAuto").checked)
	{
		$("lan_auto_type").disabled = false;
		$("en_lan_pd").disabled = false;
		$("en_lan_pd").checked = true;
		if($("lan_auto_type").value == "STATEFUL")
			$("box_lan_dhcp").style.display = "";
		else
			$("box_lan_stless").style.display = "";
	}
	else
	{
		$("lan_auto_type").disabled = true;
		$("en_lan_pd").checked = false;
		$("en_lan_pd").disabled = true;
		if($("lan_auto_type").value == "STATEFUL")
			$("box_lan_dhcp").style.display = "none";
		else
			$("box_lan_stless").style.display = "none";
	}
}
function InitLANAutoConf()
{
	//alert("InitLANAutoConf");

	//var dhcps6 = "dhcpv6";		
	if(G_RadvdEnable == "1" || G_RadvdEnable == "1")
	{
		$("enableAuto").checked = true;
	}
	else
	{
		$("enableAuto").checked = false;
	}
	OnClickAuto();
	
	return true;
}
function InitLANPdConf()
{
	//alert("InitLANPdConf");
	//var enable_lan_pd = "0";
	var enable_lan_pd = G_Wanipv6Conn['IPv6PrefixDelegationEnabled'];//"1";
	if(enable_lan_pd == "1")
	{
		$("en_lan_pd").checked = true;
	}
	else
	{
		$("en_lan_pd").checked = false;
	}
	return true;
}
	
function OnClickDHCPDNS()
{
	$("w_dhcp_pdns").disabled = $("w_dhcp_sdns").disabled = $("w_dhcp_dns_auto").checked ? true: false;
}

function OnClickLanpd()
{
}
function OnClickpd()
{
	if($("en_dhcp_pd").checked)
	{
		$("l_ipaddr").disabled = true;
		$("box_lan_auto_pd_body").style.display = "";
		$("box_lan_auto").style.display = "none";
		$("box_lan_auto_pd").style.display = "";
	}
	else
	{
		$("l_ipaddr").disabled = false;
		$("box_lan_auto_pd_body").style.display = "none";
		$("box_lan_auto").style.display = "";
		$("box_lan_auto_pd").style.display = "none";
		$("en_lan_pd").checked = false;
		
		if(hintsupport == "1")
		{
			$("en_dhcp_pd_hint").checked = false;
			$("dhcppd_hint_prefix").disabled =
			$("dhcppd_hint_pfxlen").disabled = 
			$("dhcppd_hint_plft").disabled = 
			$("dhcppd_hint_vlft").disabled = true;
		}
	}
}
function OnClickpdhint()
{
	$("dhcppd_hint_prefix").disabled =
	$("dhcppd_hint_pfxlen").disabled = 
	$("dhcppd_hint_plft").disabled = 
	$("dhcppd_hint_vlft").disabled = $("en_dhcp_pd_hint").checked? false:true;
}
function OnClickPppoeAddrType()
{
	$("pppoe_ipaddr").disabled = $("pppoe_dynamic").checked ? true: false;
	if($("pppoe_static").checked)
	{
		$("box_lan_pd_body").style.display		= "none";
		$("box_lan_auto").style.display 		= "";
		$("box_lan_auto_body").style.display 		= "";
		$("box_lan_auto_pd").style.display 		= "none";
		$("box_lan_auto_pd_body").style.display 	= "none";
		$("en_dhcp_pd").checked = false;
		OnClickpd();
		if(hintsupport == "1")
		{
			OnClickpdhint();
		}
	}
	else
	{
		$("box_lan_pd_body").style.display		= "";
		$("box_lan_auto").style.display 		= "none";
		$("box_lan_auto_body").style.display 		= "";
		$("box_lan_auto_pd").style.display 		= "";
		$("box_lan_auto_pd_body").style.display 	= "";
		
		OnClickpd();
		if(hintsupport == "1")
		{
			OnClickpdhint();
		}
	}
}
function OnClickUsell()
{
	/*
	if($('ipv6_sta_uselinklocal').checked==true)
	{
		var MACs=G_WanMac.split(":");
		MACs[0]=MACs[0]^2;
		var wanll = "FE80::"+MACs[0]+MACs[1]+":"+MACs[2]+"FF:FE"+MACs[3]+":"+MACs[4]+MACs[5];

		$('ipv6_sta_wanip').disabled=true;
		$('ipv6_sta_prefix_len').disabled=true;
		if(G_Wanipv6Conns['Ipv6Addressstatus'] != 'Valid')
		{
			supplyValue('ipv6_sta_wanip', mac);
			supplyValue('ipv6_sta_prefix_len', '64');
		}
		$('ipv6_sta_gateway').style.display="none";
	}
	else
	{
		$('ipv6_sta_wanip').disabled=false;
		$('ipv6_sta_prefix_len').disabled=false;
		if(G_Wanipv6Conns['Ipv6Addressstatus'] != 'Valid')
		{
			supplyValue('ipv6_sta_wanip', '');
			supplyValue('ipv6_sta_prefix_len', '');
		}
		$('ipv6_sta_gateway').style.display="";
		
	}
	*/
	$("w_st_ipaddr").disabled = $("w_st_pl").disabled = $("usell").checked ? true: false;
	if($("usell").checked)
	{
		//var r3ipaddr = "fe80::dacb:8aff:fe62:adf8"; //linklocal
		//var r3ipaddr = G_Wanipv6Conn['llIPv6Address']; //linklocal
		//if(r3ipaddr=="")	
			//r3ipaddr = "fe80::dacb:8aff:fe62:adf8";	
		//$("w_st_ipaddr").value	= r3ipaddr;
		//$("w_st_pl").value		= 64;
		var MACs=G_WanMac.split(":");
		MACs[0]=MACs[0]^2;
		var r3ipaddr = "FE80::"+MACs[0]+MACs[1]+":"+MACs[2]+"FF:FE"+MACs[3]+":"+MACs[4]+MACs[5];

		$("w_st_ipaddr").value	= r3ipaddr;
		$("w_st_pl").value		= 64;
	}
	else
	{
		$("w_st_ipaddr").value	= "";
		$("w_st_pl").value		= '';
	}
}
function LoadPpp4Value()   //share时加载 IPV4参数
{
	$("pppoe_username").value = "";
	$("pppoe_password").value = "";
	$("confirm_pppoe_password").value = $("pppoe_password").value;
	$("pppoe_service_name").value = "";
	$("ppp6_mtu").value = "1492";

	var dialup = "auto";
	if	(dialup === "auto")	
		$("pppoe_alwayson").checked = true;
	else if	(dialup === "manual")	
		$("pppoe_manual").checked = true;
	else				
		$("pppoe_ondemand").checked = true;
}
function OnClickPppoeSessType()
{
	if($("pppoe_sess_share").checked)
	{
		$("pppoe_alwayson").checked = true;
		//$("pppoe_alwayson").disabled = $("pppoe_manual").disabled = true;
		$("pppoe_alwayson").disabled = $("pppoe_ondemand").disabled = $("pppoe_manual").disabled = true;

		$("pppoe_username").disabled = true;
		$("pppoe_password").disabled = true;
		$("confirm_pppoe_password").disabled = true;
		$("pppoe_service_name").disabled = true;
		//LoadPpp4Value();
	}
	else
	{
		//$("pppoe_alwayson").disabled = $("pppoe_manual").disabled = false;
		$("pppoe_alwayson").disabled = $("pppoe_manual").disabled = false;
					
		$("pppoe_ondemand").disabled = true;	// Not Support ondemand in "Create a New Session" mode.

		$("pppoe_username").disabled = false;
		$("pppoe_password").disabled = false;
		$("confirm_pppoe_password").disabled = false;
		$("pppoe_service_name").disabled = false;
	}
	 $("pppoe_sess_share").setAttribute("modified", "true");
}
function OnClick6rdDHCPOPT()
{
	$("w_6rd_prefix_1").disabled = $("w_6rd_prefix_2").disabled = $("w_6rd_v4addr_mask").disabled = $("w_6rd_relay").disabled = $("6rd_dhcp_option").checked ? true: false;

	// set default IPv4 Mask Length = 0.
	if ($("6rd_manual").checked)
	{
		var w_6rd_v4addr_mask = "64"
		if (w_6rd_v4addr_mask != 0)
			$("w_6rd_v4addr_mask").value = w_6rd_v4addr_mask;
		else
			$("w_6rd_v4addr_mask").value = 0;
	}
	else
		$("w_6rd_v4addr_mask").value = "";

}
function OnChangewan_ipv6_mode()    //更换WAN类型
{
	$("box_wan_title").style.display			= "none";
	$("box_wan_static_body").style.display	= "none";
	$("box_wan_pppoe").style.display			= "none";
	$("box_wan_pppoe_body").style.display		= "none";
	$("bbox_wan_dns").style.display			= "none";
	$("box_wan_6to4_body").style.display		= "none";
	$("box_wan_tunnel").style.display			= "none";
	$("box_wan_tunnel_body").style.display	= "none";
	$("box_wan_6rd_body").style.display	= "none";
	
	$("box_lan").style.display				= "none";
	$("box_lan_ul").style.display				= "none";
	$("box_lan_body").style.display	= "none";
	$("box_lan_pd_body").style.display		= "none";
	$("box_lan_pd_hint_body").style.display		= "none";
	$("box_lan_ll_body").style.display 		= "none";
	$("box_lan_ul_body").style.display 		= "none";
	$("box_lan_auto").style.display 			= "none";
	$("box_lan_auto_body").style.display 		= "none";
	$("box_lan_auto_pd").style.display 			= "none";
	$("box_lan_auto_pd_body").style.display 		= "none";
	$("bbox_wan").style.display 	= "none";
	$("bbox_lan_auto").style.display 	= "none";
	$("sp_dli_s").innerHTML = "::00";
	$("sp_dli_e").innerHTML = "::00";
	$("ra_lifetime").disabled = true;
	$("ip_lifetime").disabled = true;
	$("w_6rd_v4addr").disabled = true;
	
	$("l_span_6rd").style.display 			= "none";
	$("l_span_6to4").style.display 		= "none";
	$("l_span").style.display 			= "none";
	
	var wan3 = "1";//20100614
	OnClickpd();
	//var hintsupport = "1";
	if(hintsupport == "1")
	{
		OnClickpdhint();
	}
	OnClickDHCPDNS();
	// alert($("wan_ipv6_mode").value);
	switch($("wan_ipv6_mode").value)
	{
		case "STATIC":
			$("box_wan_title").style.display = "";
			$("box_wan_static_body").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("box_lan_auto").style.display = "";
			$("box_lan_auto_body").style.display = "";
			$("box_lan_auto_pd").style.display = "none";
			$("box_lan_auto_pd_body").style.display = "none";
			$("bbox_wan").style.display 	= "";
			$("bbox_lan_auto").style.display 	= "";
			$("sp_dli_s").innerHTML = ":00";
			$("sp_dli_e").innerHTML = ":00";
			$("l_ipaddr").disabled = false;
			$("ra_lifetime").disabled = false;
			$("ip_lifetime").disabled = false;
			$("l_span").style.display = "";
			InitWANStaticValue();
			break;

		case "AUTO":
		case "AUTODETECT":
			$("bbox_wan_dns").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			//$("span_dsc2").style.display = "";
			$("box_lan_pd_body").style.display = "";
			if(hintsupport == "1")
			{
				$("box_lan_pd_hint_body").style.display = "";
			}
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("box_lan_auto_body").style.display = "";
			$("bbox_lan_auto").style.display 	= "";
			
			$("w_dhcp_pdns").disabled = $("w_dhcp_sdns").disabled = $("w_dhcp_dns_auto").checked ? true: false;
			$("ra_lifetime").value = "";
			$("ip_lifetime").value = "";
			$("l_span").style.display = "";
			break;

		case "PPPOE":
			$("box_wan_pppoe").style.display = "";
			$("box_wan_pppoe_body").style.display = "";
			$("bbox_wan_dns").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			
			$("box_lan_pd_body").style.display = "";
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			
			$("box_lan_auto_body").style.display = "";
			$("box_lan_auto_pd").style.display = "";
			$("box_lan_auto_pd_body").style.display = "";
			$("bbox_wan").style.display 	= "";
			$("bbox_lan_auto").style.display 	= "";

			/* rbj, if ppp10 some ipv6 settings regarding pppoe will follow ipv4 */
			if($("pppoe_sess_share").checked)
			{
				$("pppoe_username").disabled = true;
				$("pppoe_password").disabled = true;
				$("confirm_pppoe_password").disabled = true;
				$("pppoe_service_name").disabled = true;
				$("pppoe_alwayson").disabled = true;
				//$("pppoe_ondemand").disabled = true;
				$("pppoe_manual").disabled = true;
			}
			else
			{
				$("pppoe_username").disabled = false;
				$("pppoe_password").disabled = false;
				$("confirm_pppoe_password").disabled = false;
				$("pppoe_service_name").disabled = false;
				$("pppoe_alwayson").disabled = false;
				//$("pppoe_ondemand").disabled = true;
				$("pppoe_manual").disabled = false;
			}

			//+++ Jerry Kao, Added pppoe_max_idle_time icon, and disable on denamd
			//               in "Share with v4" and "Create new session" modes. 
			$("pppoe6_max_idle_time").disabled = true;
			$("pppoe_ondemand").disabled = true;

			// if(XG(wan1.inetp+"/addrtype")=="ppp10")
			// if(XG(wan1.inetp+"/ppp6/mtu")=="") $("ppp6_mtu").value = "1492";
			// else if(XG(wanllact.inetp+"/addrtype")=="ppp6")
			// if(XG(wanllact.inetp+"/ppp6/mtu")=="") $("ppp6_mtu").value = "1492";
			$("ppp6_mtu").value = "1492";
			//$("w_dhcp_dns_auto").checked	= true;
			$("w_dhcp_pdns").disabled = $("w_dhcp_sdns").disabled = $("w_dhcp_dns_auto").checked ? true: false;
			$("ra_lifetime").value = "";
			$("ip_lifetime").value = "";
			OnClickPppoeAddrType();
			$("l_span").style.display = "";
			//var over;	
			//if($("pppoe_sess_share").checked && over=="eth")
			//LoadPpp4Value();
			break;

		case "6IN4":
			$("box_wan_tunnel").style.display			= "";
			$("box_wan_tunnel_body").style.display	= "";
			$("bbox_wan_dns").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			//$("span_dsc2").style.display = "";
			//$("box_lan_pd_body").style.display = ""; 
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("box_lan_auto").style.display = "";
			$("box_lan_auto_body").style.display = "";
			$("box_lan_auto_pd").style.display = "none";
			//$("box_lan_auto_pd_body").style.display = "none";
			$("bbox_wan").style.display 	= "";
			$("bbox_lan_auto").style.display 	= "";
			//$("l_ipaddr").disabled = false;
			$("ra_lifetime").disabled = false;
			$("ip_lifetime").disabled = false;
			//$("w_dhcp_dns_auto").checked	= true;
			$("w_dhcp_pdns").disabled = $("w_dhcp_sdns").disabled = $("w_dhcp_dns_auto").checked ? true: false;
			$("l_span").style.display = "";

			$("l_ipaddr").disabled = false;
			$("box_lan_auto_pd_body").style.display = "none";
			$("box_lan_auto").style.display = "";
			$("box_lan_auto_pd").style.display = "none";
			$("en_lan_pd").checked = false;
			break;

		case "6RD":
			$("box_wan_title").style.display = "";
			$("box_wan_6rd_body").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("box_lan_auto").style.display = "";
			$("box_lan_auto_body").style.display = "";
			$("box_lan_auto_pd").style.display = "none";
			$("box_lan_auto_pd_body").style.display = "none";
			$("bbox_wan").style.display 	= "";
			$("bbox_lan_auto").style.display 	= "";
			$("ra_lifetime").value = "";
			$("ip_lifetime").value = "";
			
			
			var enabled_hub_spoke = "";
			if ( enabled_hub_spoke == "")
			{
				$("en_hub_spoke").checked = true;
			}	
			
			$("w_6rd_prefix_1").disabled = $("w_6rd_prefix_2").disabled = $("w_6rd_v4addr_mask").disabled  = $("w_6rd_relay").disabled = $("6rd_dhcp_option").checked ? true: false;
			$("l_span_6rd").style.display = "";
			break;
  
		case "6TO4":
			$("box_wan_title").style.display = "";
			$("box_wan_6to4_body").style.display = "";
			$("box_lan").style.display = "";
			$("span_dsc1").style.display = "";
			$("box_lan_body").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("box_lan_auto").style.display = "";
			$("box_lan_auto_body").style.display = "";
			$("box_lan_auto_pd").style.display = "none";
			$("box_lan_auto_pd_body").style.display = "none";
			$("bbox_wan").style.display 	= "";
			$("bbox_lan_auto").style.display 	= "";
			$("ra_lifetime").value = "";
			$("ip_lifetime").value = "";
			$("l_ipaddr_6to4").value = G_TunConn['SubnetID'] || "1";
			$("l_span_6to4").style.display = "";
			break;
  
		case "LL":
			$("box_lan").style.display = "";
			$("box_lan_ll_body").style.display = "";
			$("span_dsc1").style.display = "none";
			//$("span_dsc2").style.display = "none";
			var enabled_lan_ul = "1";
			if(enabled_lan_ul!="")
			{
				$("box_lan_ul_body").style.display = "";
			}
			break;
	}
}
function ShowDHCPS6()
{
	var str;
	//var prefix = G_Wanipv6Conns6Rd['SPIprefix'].split("/");

	
	$("dhcps_start_ip_value").value = G_MinInterfaceID;
		
	$("dhcps_stop_ip_value").value = G_MaxInterfaceID;
	
	$("dhcps_start_ip_prefix").value = "xxxx";
	$("dhcps_stop_ip_prefix").value = "xxxx";
	
	$("dhcps_start_ip_prefix").disabled = true;
	$("dhcps_stop_ip_prefix").disabled = true;
}
function OnChangelan_auto_type()
{
	$("box_lan_dhcp").style.display  = "none";
	$("box_lan_stless").style.display = "none";
	switch($("lan_auto_type").value)
	{
		case "STATELESSR":
		case "STATELESSD":
			$("box_lan_dhcp").style.display = "none";
			$("box_lan_stless").style.display = "";
			break;
			
		case "STATEFUL":
			$("box_lan_dhcp").style.display = "";
			$("dhcps_start_ip_prefix").disabled = true;
			$("dhcps_stop_ip_prefix").disabled = true;
			ShowDHCPS6();
			break;
	}		
	InitLANPdConf();
}
function InitValue()
{
	if (!InitWANConnMode()) return false;
	if (!InitLANConnMode()) return false;
	if (!InitWANInfo()) return false;
	if (!InitWANLLInfo()) return false;
	if (!InitLANInfo()) return false;
	if (!InitLANLLInfo()) return false;
	if (!InitLANAutoConf()) return false;
	if (!InitLANPdConf()) return false;
	if (!InitDHCPS6()) return false;
	OnChangewan_ipv6_mode();	
	OnChangelan_auto_type();
		
		return true;
}

function uiOnload()
{
	var ConnectionType="";
	var G_ipv66RdEnable=""; //6rd

	if (G_TunConn['Activated'] == "1") //tunnel
	{
		if (G_TunConn['Mode'] == "6to4" )
		{
			Wan_Mode = "6TO4"; //linklocal
		}
		else if (G_TunConn['Mode'] == "6in4" )
		{
			if( G_TunConn['Mechanism'] == "6in4")
				Wan_Mode = "6IN4"; //linklocal
			else
				Wan_Mode = "6RD"; //linklocal		
		}		
		
	}

	if (Wan_Mode == "")
	{
		if(G_WANConn['X_TWSZ-COM_ProtocolType'] == "IPv4_6")
		{
				Wan_Mode = "PPPOE"; //linklocal			
		}
		else if(G_Wanipv6Conn['Path'].indexOf('IP') > 0)
		{
			if(G_Wanipv6Conn['Enable'] == '0')
			{
				Wan_Mode = "LL"; //linklocal
			}
			else
			{
				if (G_Wanipv6Conn['AddressingType'] == "Static")
				{
					Wan_Mode = "STATIC";				
				}
				else
				{
					Wan_Mode = "AUTO";								
				}
			}
		}
		else if(G_Wanipv6Conn['Path'].indexOf('PPP') > 0)
		{
			if(G_Wanipv6Conn['Enable'] == '0')
			{
				Wan_Mode = "LL";
			}
			else
			{
				Wan_Mode = "PPPOE";
			}
		}		
		
	}


	//if((G_ipv66RdEnable == '1') && (G_Wanipv6Conns['Enable'] == '0'))
	//{
		//Wan_Mode = "6RD";
	//}

	//alert("Wan_Mode" + Wan_Mode);
	$("wan_ipv6_mode").value = Wan_Mode;
	InitValue();
				
	return;
}
function checkPWD(name)
{
	if($(name+'_password').value != $('confirm_'+name+'_password').value){
		alert(SEcode[1010]);
		return false;
	}
	return true;
}

function CheckData()
{
	
	return true;
}
//function OnSubmit()
function OnSubmit()
{
	if(!CheckData()) return;
	
	$H({
		"obj-action" 		: "set",
		"getpage" 		: 'html/page/portforwd.ajax.js',
		"errorpage" 		: 'html/page/portforwd.ajax.js',
		'var:finish'    	: "1",
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" : ViewState.Save()
	}, true);
	
	var _upath = G_Wanipv6Conn['uPath'];
	var ConnPath = G_Wanipv6Conn['Path'];
	var Ipv4ConnPath = G_WANConn['Path'] + ".";
	var ConnType;
	var _path_host6 = ":InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.";
	// 
	var _tunnelpath = ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.1." ; //for tunnel
	var pppoeshare = 0;

	var _protocol = [];
	switch($("wan_ipv6_mode").value){
		case "AUTODETECT" :		
			break;
		case "STATIC" :
		case "AUTO" :
			ConnType = 'IP';
			break;
		case "PPPOE" :
			ConnType = 'PPP';
			break;
		case "6IN4" :	
		case "6TO4" :		
		case "6RD" :
			if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] == "4in6" ) //tunnel enable 
			{
				alert(SEcode['lang_set_tunnel']);
				return false;

			}

			break;	
		case "LL" :
			break;	
	}	
	if( $("wan_ipv6_mode").value == "PPPOE" && $('pppoe_sess_share').checked == true)	//pppoe share
	{
		if(Ipv4ConnPath.indexOf("PPP") < 0) //not pppoe 
		{
			alert(SEcode["lang_changeV4_wan_pro"]);	
			return false;
		}
		//do something
	} else
	if(ConnType == 'IP' || ConnType == 'PPP')
	{
		if (typeof(ConnPath) == "undefined") { 
				//alert(SEcode["lang_undefined"]); 
				//_upath = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.0.";
				//ConnPath = _upath + 'WAN' + ConnType + 'Connection.0.';
				$F('mid','0438');
				$F(":" + ConnPath + "X_TWSZ-COM_ConnectionMode", ConnType);
		}

		else if(ConnPath.indexOf(ConnType) < 0)
		{ // -> webproxy
			ConnPath = G_Wanipv6Conn['Path'].replace(ConnType == 'IP' ? 'PPP' : 'IP', ConnType);
			$F('mid','0438');
			$F(":" + ConnPath + "X_TWSZ-COM_ConnectionMode", G_Wanipv6Conn['Path'].indexOf('IP') > 0 ? 'IP' : 'PPP');
			//_path = _path.substr(0, _path.length - 2) + '1.'; // -> InternetGatewayDevice.WANDevice.1.WANConnectionDevice.*.WANIPConnection.0.
		}

		$F(":" + _upath + "X_TWSZ-COM_VLANID", 	"0"); //vid
		$F(":" + _upath + "X_TWSZ-COM_VLANPriority", 	"0"); // vlanprio
		$F(":" + _upath + "X_TWSZ-COM_CloneMACAddress", 	""); // mac address

	}


	switch($("wan_ipv6_mode").value)
	{
		case "AUTODETECT" :
			
			break;
		case "STATIC" :
			var _dnservers = $('w_st_pdns').value+','+$('w_st_sdns').value;

			$F(":" + ConnPath + "Enable", 			"1");
			$F(":" + ConnPath + "Name", 			"Static_0_Internet");
			$F(":" + ConnPath + "AddressingType", 		"Static");
			$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
			$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
			
			if (Form.Checkbox("usell") == 1){ //uselinklocal
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Invalid");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
			    $F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		"");	

			}
			else
			{
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Valid");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		$('w_st_ipaddr').value);
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		$('w_st_pl').value);
			}

			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.ConfigType", 	"Static");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	"1");  //
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());
			
			
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.DefaultRouter", 	$('w_st_gw').value);
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6SitePrefixInfo.ValidLifeTime", 		"172800");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6SitePrefixInfo.PreferredLifeTime", 	"7200");
	
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 	"0"); //

			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			$F(":" + ConnPath + "MaxMTUSize", 			"1500");


		  break;
		case "AUTO" :  //动态
			$F(":" + ConnPath + "Enable", 			"1");
			$F(":" + ConnPath + "Name", 			"DHCP_0_Internet");
			$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
			$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
			$F(":" + ConnPath + "NATEnabled", 		"1");
			$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");
			
			$F(":" + ConnPath + "AddressingType", 		"DHCP");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.ConfigType", 	"OTHER");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");			
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		""); //是否需要清空，防止static切换到auto
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		""); //是否需要清空，防止static切换到auto
					

			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	Form.Radio('w_dhcp_dns_rad') == "manual" ? "1" : "0");
			if (Form.Radio('w_dhcp_dns_rad') == "manual")
			{
				var _dnservers = $('w_dhcp_pdns').value+','+$('w_dhcp_sdns').value;
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());								
			}
			else
			{
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSEnabled", 				"1");							
			}
					
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.DHCP6cForAddress", 		"0"); //support dhcp6c
			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			$F(":" + ConnPath + "MaxMTUSize", 			"1500");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 		Form.Checkbox('en_dhcp_pd') ? "1" : "0");


			break;
		case "PPPOE" :  //share session
			//$F(":" + ConnPath + "Enable", 			"0"); //disable			
			if( $('pppoe_sess_share').checked == true)	//pppoe share
			{

				pppoeshare = 1;
				$F(":" + ConnPath + "Enable", 			"0"); // ipv6 conn disable
				$F(":" + Ipv4ConnPath + "Enable", 		"1");			
								
				//$F(":" + Ipv4ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv4_6");
				if( $('pppoe_static').checked == true)		
				{			
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Valid");			
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		$('pppoe_ipaddr').value);
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		"128");
				}
				else
				{			
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");			
				}
			
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	Form.Radio('w_dhcp_dns_rad') == "manual" ? "1" : "0");
				if (Form.Radio('w_dhcp_dns_rad') == "manual") //注意这里是空间name 非id
				{
					var _dnservers = $('w_dhcp_pdns').value+','+$('w_dhcp_sdns').value;
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());								
				}
				else
				{
					$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSEnabled", 	"1"); //是否请求dns							
				}
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 		Form.Checkbox('en_dhcp_pd') ? "1" : "0");//support pd  即是否需要请求前缀
							
			}
			else
			{
				if(!checkPWD('pppoe'))
					return false;

				$F(":" + ConnPath + "Name", 			"PPPoE_0_Internet");
				$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
				$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
				$F(":" + ConnPath + "NATEnabled", 		"1");
				$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");
				$F(":" + ConnPath + "Username", 		$('pppoe_username').value); //"" $('pppoe_username').value
				var pppoe_passwd = Base64.Encode($('pppoe_password').value);
				$F(":" + ConnPath + "Password", 		$('pppoe_password').value != '**********' ? pppoe_passwd : undefined);//"" 
				$F(":" + ConnPath + "PPPAuthenticationProtocol", 	"Auto");
				//$F(":" + ConnPath + "ConnectionTrigger", 		"AlwaysOn");
				$F(":" + ConnPath + "ConnectionTrigger", 		$('pppoe_manual').checked ? "Manual" : "AlwaysOn");
				
				$F(":" + ConnPath + "MaxMRUSize", 		getDight($('ppp6_mtu').value)); //same as mtu"1492"
				$F(":" + ConnPath + "MaxMTUSize", 		getDight($('ppp6_mtu').value)); //"1492"
				$F(":" + ConnPath + "PPPLCPEcho", 		"10"); //default 
				$F(":" + ConnPath + "PPPLCPEchoRetry", 		"10"); //default
				$F(":" + ConnPath + "PPPoEServiceName", 	$('pppoe_service_name').value); //"tencent2"		
				$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", ""); //Form.Radio('pppoe_dynamic') == "DHCP" ? "" : $('pppoe_ipaddr').value
				//$F(":" + ConnPath + "AddressingType", 		"DHCP");
				$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
				$F(":" + ConnPath + "Enable", 			"1");

				if( $('pppoe_static').checked == true)		
				{			
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Valid");			
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		$('pppoe_ipaddr').value);
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		"128");
				}
				else
				{			
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");			
				}
				
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	Form.Radio('w_dhcp_dns_rad') == "manual" ? "1" : "0");
				if (Form.Radio('w_dhcp_dns_rad') == "manual") //注意这里是空间name 非id
				{
					var _dnservers = $('w_dhcp_pdns').value+','+$('w_dhcp_sdns').value;
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());								
				}
				else
				{
					$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSEnabled", 	"1"); //是否请求dns							
				}

				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 		Form.Checkbox('en_dhcp_pd') ? "1" : "0");//support pd  即是否需要请求前缀
				
			}	
			
			break;
		case "PPPOE2" :
			$F(":" + ConnPath + "Name", 			"PPPoE_0_Internet");
			$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
			$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
			$F(":" + ConnPath + "NATEnabled", 		"1");
			$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");
			$F(":" + ConnPath + "Username", 		$('pppoe_username').value); //"" $('pppoe_username').value
			var pppoe_passwd = Base64.Encode($('pppoe_password').value);
			$F(":" + ConnPath + "Password", 		$('pppoe_password').value != '**********' ? pppoe_passwd : undefined);//"" 
			$F(":" + ConnPath + "PPPAuthenticationProtocol", 	"Auto");
			//$F(":" + ConnPath + "ConnectionTrigger", 		"AlwaysOn");
			$F(":" + ConnPath + "ConnectionTrigger", 		$('pppoe_manual').checked ? "Manual" : "AlwaysOn");
			
			$F(":" + ConnPath + "MaxMRUSize", 		getDight($('ppp6_mtu').value)); //same as mtu"1492"
			$F(":" + ConnPath + "MaxMTUSize", 		getDight($('ppp6_mtu').value)); //"1492"
			$F(":" + ConnPath + "PPPLCPEcho", 		"10"); //default 
			$F(":" + ConnPath + "PPPLCPEchoRetry", 		"10"); //default
			$F(":" + ConnPath + "PPPoEServiceName", 	$('pppoe_service_name').value); //"tencent2"		
			$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", ""); //Form.Radio('pppoe_dynamic') == "DHCP" ? "" : $('pppoe_ipaddr').value
			//$F(":" + ConnPath + "AddressingType", 		"DHCP");
			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			$F(":" + ConnPath + "Enable", 			"1");

			if( $('pppoe_static').checked == true)		
			{			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Valid");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		$('pppoe_ipaddr').value);
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		"128");
			}
			else
			{			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");			
			}
			
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	Form.Radio('w_dhcp_dns_rad') == "manual" ? "1" : "0");
			if (Form.Radio('w_dhcp_dns_rad') == "manual") //注意这里是空间name 非id
			{
				var _dnservers = $('w_dhcp_pdns').value+','+$('w_dhcp_sdns').value;
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());								
			}
			else
			{
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSEnabled", 	"1"); //是否请求dns							
			}

			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 		Form.Checkbox('en_dhcp_pd') ? "1" : "0");//support pd  即是否需要请求前缀
			
			break;
			
		case "6IN4" :
			$F(":" + ConnPath + "Enable", 		"0");

			$F(_tunnelpath + 'Activated', 				'1');
			$F(_tunnelpath + 'Mode', 				'6in4');
			$F(_tunnelpath + 'TunnelName', 				'tunl4in6');
			$F(_tunnelpath + 'Mechanism', 				'6in4');

			$F(_tunnelpath + 'AssociatedLanIfName', 				'InternetGatewayDevice.LANDevice.1');//目前先锁定
			$F(_tunnelpath + 'AssociatedWanIfName', 				G_WANConn['Path']);//目前先锁定
			$F(_tunnelpath + 'RemoteIpv4Address', 				$('w_tu_rev4_ipaddr').value);

			$F(_tunnelpath + 'RemoteIpv6Address', 				$('w_tu_rev6_ipaddr').value);
			$F(_tunnelpath + 'LocalIpv6Address', 				$('w_tu_lov6_ipaddr').value);
			$F(_tunnelpath + 'LocalIPv6PrefixLength', 				$('w_tu_pl').value);

			$F(_tunnelpath + "IPv6DNSOverrideAllowed", 	Form.Radio('w_dhcp_dns_rad') == "manual" ? "1" : "0");
			if (Form.Radio('w_dhcp_dns_rad') == "manual") //注意这里是空间name 非id
			{
				var _dnservers = $('w_dhcp_pdns').value+','+$('w_dhcp_sdns').value;
				$F(_tunnelpath + 'IPv6DNSServers',  	_dnservers.delcomma());								
			}
			else
			{
				$F(_tunnelpath + 'IPv6DNSServers',  	"");								
				$F(_tunnelpath + 'IPv6DNSEnabled', 	"1"); 							
			}			
			
			$F(_tunnelpath + 'IPv6PrefixDelegationEnabled', 		"0");
			//$F(_tunnelpath + 'IPv6PrefixDelegationEnabled', 		Form.Checkbox('en_dhcp_pd') ? "1" : "0");
			
			
			break;
		case "6TO4" :
			var _dnservers = $('w_6to4_pdns').value+','+$('w_6to4_sdns').value;
			$F(":" + ConnPath + "Enable", 		"0");

			//tunnel config 
			$F(_tunnelpath + 'Activated', 				'1');
			$F(_tunnelpath + 'Mode', 				'6to4');
			$F(_tunnelpath + 'Mechanism', 				'6to4');
			$F(_tunnelpath + 'TunnelName', 				'tun6to4');
			$F(_tunnelpath + 'AssociatedLanIfName', 				'InternetGatewayDevice.LANDevice.1');//目前先锁定
			$F(_tunnelpath + 'AssociatedWanIfName', 				G_WANConn['Path']);//目前先锁定
			$F(_tunnelpath + 'BorderRelayAddress', 				$('w_6to4_relay').value);//目前先锁定
			$F(_tunnelpath + 'IPv6DNSServers',  	_dnservers.delcomma());								
			$F(_tunnelpath + 'SubnetID',  	$('l_ipaddr_6to4').value);								
			$F(_tunnelpath + "IPv6DNSOverrideAllowed", 	"0");
			$F(_tunnelpath + 'IPv6DNSEnabled', 	"0"); 									
			$F(_tunnelpath + 'IPv6PrefixDelegationEnabled', "0");
			
			break;
		case "6RD" :
			var _dnservers = $('w_6rd_pdns').value+','+$('w_6rd_sdns').value;
			$F(":" + ConnPath + "Enable", 		"0");
			//tunnel config 
			$F(_tunnelpath + 'Activated', 				'1');
			$F(_tunnelpath + 'Mode', 				'6in4');
			$F(_tunnelpath + 'TunnelName', 				'tunl6rd');
			$F(_tunnelpath + 'Mechanism', 				'Ipv6RapidDeployment');
			$F(_tunnelpath + 'AssociatedLanIfName', 				'InternetGatewayDevice.LANDevice.1');//目前先锁定
			$F(_tunnelpath + 'AssociatedWanIfName', 				G_WANConn['Path']);//目前先锁定

			if ($('6rd_manual').checked == true ) //注意这里是空间name 非id
			{
				$F(_tunnelpath + 'Dynamic', 		'0'); //manual 
				$F(_tunnelpath + 'Prefix', 		$('w_6rd_prefix_1').value+'/'+$('w_6rd_prefix_2').value); //manual 
				$F(_tunnelpath + 'IPv4MaskLen', 		$('w_6rd_v4addr_mask').value); //dynamic
				$F(_tunnelpath + 'BorderRelayAddress', 		$('w_6rd_relay').value); //dynamic
			}
			else
			{
				$F(_tunnelpath + 'Dynamic', 		'1'); //dynamic
			}
			$F(_tunnelpath + 'IPv6DNSServers',  	_dnservers.delcomma());								
			$F(_tunnelpath + "IPv6DNSOverrideAllowed", 	"0");
			$F(_tunnelpath + 'IPv6DNSEnabled', 	"0"); 										
			$F(_tunnelpath + 'IPv6PrefixDelegationEnabled', "0");
		
			break;	
		case "LL" :
			$F(":" + ConnPath + "Enable", 		"0");
			//$F(_6rdpath + 'Enable',         	'0');
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus",		"Invalid");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressScope", 		"Global");						//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		"");
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		"");	
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		"");	
			//$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.DefaultRouter", 		"");	
			$('enableAuto').checked = false;
			
			break;
		
		default :
			;
	}

	if (pppoeshare == 1) //pppoe share disable
	{
		$F(":" + Ipv4ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv4_6");
	}
	else
	{
		if ( G_WANConn['X_TWSZ-COM_ProtocolType'] == "IPv4_6") //share change to other
		{
			$F(":" + Ipv4ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv4");			
		}
	}

	{
		var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js";
		var ajax = Ajax.getInstance(_url, "", 0, OnLanSubmit, null_errorfunc);
		ajax.post(document.forms[0]);
		$("menu").style.display="none";
		$("content").style.display="none";
		$("mbox").style.display="";
		$("BIPV6MAN107").disabled = true;
		$("BIPV6MAN109").disabled = true;
	}
	//$('uiPostForm').submit();

}

function OnLanSubmit(_text)
{
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		return;
	}
	$("BIPV6MAN107").disabled = false;
	$("BIPV6MAN109").disabled = false;
	G_SysToken = G_AjaxToken;
	//alert(G_Error);
	if(G_Error == '1')
	{		
		$("menu").style.display="";
		$("content").style.display="";
		$("mbox").style.display="none";
		dealWithError();
		return false;
	}	
	
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index.html",
		'var:finish'    	: "1",
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		'var:sys_Token' : G_SysToken,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" : ViewState.Save()
	}, true);

	var _upath = G_Wanipv6Conn['uPath'];
	var ConnPath = G_Wanipv6Conn['Path'];
	var ConnType;
	var _path_host6 = ":InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.";
	var _path_host = ":InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.";
	var _tunnelpath = ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.1." ; //for tunnel
	// 
	var _protocol = [];
	switch($("wan_ipv6_mode").value){
		case "AUTODETECT" :		
			break;
		case "STATIC" :
		case "AUTO" :
			ConnType = 'IP';
			break;
		case "PPPOE" :
			ConnType = 'PPP';
			break;
		case "6IN4" :	
			break;
		case "6TO4" :		
			break;
		case "6RD" :
			break;	
		case "LL" :
			break;	
	}

	switch($("wan_ipv6_mode").value)
	{
		case "AUTODETECT" :
			
			break;
		case "STATIC" :
			if (G_TunConn['Activated'] == "1" ) //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
			var _dnservers = $('w_st_pdns').value+','+$('w_st_sdns').value;
			
			$F( _path_host6 + 'ServerType.StatefullDHCPv6.IPv6DNSServers', 		_dnservers.delcomma());										
			
			if($('l_ipaddr').value == "")   //lan IP 地址，需要根据lan ip生成前缀
			{
				alert(SEcode['lang_lanv6_empty']);
				$("menu").style.display="";
				$("content").style.display="";
				$("mbox").style.display="none";				
				return false;				
			}
			
			if($('l_ipaddr').value != "")
			{
				//这里需要检查ipv6 合法性
				//if(!CheckIPv6Addr($('ipv6_sta_lanip').value)){
					//alertError('WAN_IPv605');
					//return false;
				//}
			}

			var ipv6staip = IPv6FillZero($('l_ipaddr').value);
			var ipv6_lanip = ipv6staip.split(":");		
			var ipv6_addr = ipv6_lanip[0] + ':' + ipv6_lanip[1] + ':' + ipv6_lanip[2] + ':' + ipv6_lanip[3] + '::';

			//LAN Prefix LAN 前缀
			$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress'			, $('l_ipaddr').value); // lan global address
			$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "1"); //  address

			$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "1"); // 
			var _static_prefix = ipv6_addr + '/64';
			$F( _path_host6 + 'RadVDConfigManagement.StaticPrefixInfo.Prefix'			, _static_prefix); // lan prefix 
					
		  break;
		case "AUTO" :  //动态	
			if (G_TunConn['Activated'] == "1" ) //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
			if($('en_dhcp_pd').checked == true)		
			{
					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "0"); //1- static   0-from wan 
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "0"); //  address
			}
			else
			{
					if($('l_ipaddr').value == "")   //lan IP 地址，需要根据lan ip生成前缀
					{
						alert(SEcode['lang_lanv6_null']);						
						$("menu").style.display="";
						$("content").style.display="";
						$("mbox").style.display="none";
						return false;
					}
					
					if($('l_ipaddr').value != "")
					{
						//这里需要检查ipv6 合法性
						//if(!CheckIPv6Addr($('ipv6_sta_lanip').value)){
							//alertError('WAN_IPv605');
							//return false;
						//}
					}
					var ipv6staip = IPv6FillZero($('l_ipaddr').value);
					var ipv6_lanip = ipv6staip.split(":");
					
					var ipv6_addr = ipv6_lanip[0] + ':' + ipv6_lanip[1] + ':' + ipv6_lanip[2] + ':' + ipv6_lanip[3] + '::';

					//LAN Prefix LAN 前缀
					var _static_prefix = ipv6_addr + '/64';
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "1"); //  address
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress'			, $('l_ipaddr').value); // lan global address

					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "1"); //1- static   0-from wan 
					$F( _path_host6 + 'RadVDConfigManagement.StaticPrefixInfo.Prefix'			, _static_prefix); // lan prefix 
			}


			break;

		case "PPPOE" :
			//pppoe linsd
			
			if (G_TunConn['Activated'] == "1" ) //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
			
			if($('en_dhcp_pd').checked == true)		
			{
					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "0"); 
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "0"); //  address
			}
			else
			{

					if($('l_ipaddr').value == "")   
					{
						alert(SEcode['lang_lanv6_null']);						
						$("menu").style.display="";
						$("content").style.display="";
						$("mbox").style.display="none";
						return false;
					}
					
					if($('l_ipaddr').value != "")
					{
						//这里需要检查ipv6 合法性
						//if(!CheckIPv6Addr($('ipv6_sta_lanip').value)){
							//alertError('WAN_IPv605');
							//return false;
						//}
					}

					var ipv6staip = IPv6FillZero($('l_ipaddr').value);
					var ipv6_lanip = ipv6staip.split(":");				
					var ipv6_addr = ipv6_lanip[0] + ':' + ipv6_lanip[1] + ':' + ipv6_lanip[2] + ':' + ipv6_lanip[3] + '::';
					var _static_prefix = ipv6_addr + '/64';

					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress'			, $('l_ipaddr').value); // lan global address
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "1"); //  address

					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "1"); 
					$F( _path_host6 + 'RadVDConfigManagement.StaticPrefixInfo.Prefix'			, _static_prefix); // lan prefix 

					
			}
					
			break;	
		case "6IN4" :
			if (0) //if($('en_dhcp_pd').checked == true)		
			{
					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "0"); //1- static   0-from wan 
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "0"); //  address
			}
			else
			{
					if($('l_ipaddr').value == "")   //lan IP 地址，需要根据lan ip生成前缀
					{
						alert(SEcode['lang_lanv6_null']);						
						$("menu").style.display="";
						$("content").style.display="";
						$("mbox").style.display="none";
						return false;
					}
					
					if($('l_ipaddr').value != "")
					{
						//这里需要检查ipv6 合法性
						//if(!CheckIPv6Addr($('ipv6_sta_lanip').value)){
							//alertError('WAN_IPv605');
							//return false;
						//}
					}
					var ipv6staip = IPv6FillZero($('l_ipaddr').value);
					var ipv6_lanip = ipv6staip.split(":");
					
					var ipv6_addr = ipv6_lanip[0] + ':' + ipv6_lanip[1] + ':' + ipv6_lanip[2] + ':' + ipv6_lanip[3] + '::';

					//LAN Prefix LAN 前缀
					var _static_prefix = ipv6_addr + '/64';
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "1"); //  address
					$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress'			, $('l_ipaddr').value); // lan global address

					$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "1"); //1- static   0-from wan 
					$F( _path_host6 + 'RadVDConfigManagement.StaticPrefixInfo.Prefix'			, _static_prefix); // lan prefix 
			}
			
			break;
		case "6TO4" :
			

			break;
		case "6RD" :
					
			break;	
		case "LL" :
			if (G_TunConn['Activated'] == "1" ) //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
			$('enableAuto').checked = false;
			
			break;
		
		default :
			;
	}

	
	if ($('enableAuto').checked == true) {		
		$F(_path_host6 + 'RADVD.Enabled', 		'1'); //radvd
		if ($('lan_auto_type').value == "STATELESSR")
		{
			$F(_path_host6 + 'RADVD.ManagedFlag', 		'0');		
			$F(_path_host6 + 'RADVD.OtherConfFlag', 		'0');

			$F(_path_host6 + 'AutoConfigurationAddress', 		'0');													
			$F(_path_host6 + 'ServerType.Stateless', 		'1');
			
			if ($("wan_ipv6_mode").value == "STATIC" || $("wan_ipv6_mode").value == "6IN4" )
				$F(_path_host6 + 'RADVD.RouterLifeTime', 		$("ra_lifetime").value*60);
			else
				$F(_path_host6 + 'RADVD.RouterLifeTime', 		60);
				
		}
		else if ($('lan_auto_type').value == "STATELESSD")
		{
			$F(_path_host6 + 'RADVD.OtherConfFlag', 		'1');
			$F(_path_host6 + 'RADVD.ManagedFlag', 		'0');										
			$F(_path_host6 + 'ServerType.Stateless', 		'1');										
			$F(_path_host6 + 'AutoConfigurationAddress', 		'1');									
			$F(_path_host6 + 'RADVD.RouterLifeTime', 		$("ra_lifetime").value*60);
			if ($("wan_ipv6_mode").value == "STATIC" || $("wan_ipv6_mode").value == "6IN4" )
				$F(_path_host6 + 'RADVD.RouterLifeTime', 		$("ra_lifetime").value*60);
			else
				$F(_path_host6 + 'RADVD.RouterLifeTime', 		60);
			
		}
		else if ($('lan_auto_type').value == "STATEFUL")
		{
			$F(_path_host6 + 'RADVD.OtherConfFlag', 		'1');
			$F(_path_host6 + 'RADVD.ManagedFlag', 		'1');										

			$F(_path_host6 + 'AutoConfigurationAddress', 		'1'); //radvd

			$F(_path_host6 + 'ServerType.Stateless', 		'0');										
			$F(_path_host6 + 'ServerType.StatefullDHCPv6.MinInterfaceID', 		$('dhcps_start_ip_value').value);										
			$F(_path_host6 + 'ServerType.StatefullDHCPv6.MaxInterfaceID', 		$('dhcps_stop_ip_value').value);										
			if ($("wan_ipv6_mode").value == "STATIC" || $("wan_ipv6_mode").value == "6IN4" )
				$F(_path_host6 + 'ServerType.StatefullDHCPv6.DHCPv6LeaseTime', 		$("ip_lifetime").value*60);
			else
				$F(_path_host6 + 'ServerType.StatefullDHCPv6.DHCPv6LeaseTime', 		14400);
			
		}
		$F(_path_host6 + 'RADVD.FlagConfType', 		'Manual');			
	}
	else
	{
		$F(_path_host6 + 'RADVD.Enabled', 		'0'); //radvd
		$F(_path_host6 + 'AutoConfigurationAddress', 		'0'); //dhcpv6
		$F(_path_host6 + 'RADVD.OtherConfFlag', 	'1');
		$F(_path_host6 + 'RADVD.ManagedFlag', 		'1');													
	}
	$("BIPV6MAN107").disabled = true;
	$("BIPV6MAN109").disabled = true;
	$('uiPostForm').submit();
}

function null_errorfunc()
{
	
	return true;
}


//错误处理函数
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//监听加载与错误处理函数
addListeners(uiOnload, dealWithError);

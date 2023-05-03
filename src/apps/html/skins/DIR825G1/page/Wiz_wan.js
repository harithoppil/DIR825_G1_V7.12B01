var stages=["stage_desc", "stage_passwd", "stage_tz", "stage_ether", "stage_ether_cfg", "stage_finish"];
var wanTypes=["DHCP", "PPPoE", "PPTP", "L2TP", "STATIC"];
var currentStage= 0;	
var currentWanType= 0;	
var G_WANConn = [];

<?objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.  "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<?if gt $11 0 
	`	<?objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "Enable Name NATEnabled ExternalIPAddress SubnetMask DefaultGateway DNSOverrideAllowed DNSServers AddressingType ConnectionType X_TWSZ-COM_NATType X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection X_TWSZ-COM_DomainName X_TWSZ-COM_Hostname X_TWSZ-COM_UsrDNSServers" 
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.$00.";	//Path	
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['Name'] 			    = "$02";	//Name
			G_WANConn['NATEnabled'] 		= "$03";	//NATEnabled
			G_WANConn['ExternalIPAddress'] 	= "$04";	//ExternalIPAddress
			G_WANConn['SubnetMask'] 		= "$05";	//SubnetMask
			G_WANConn['DefaultGateway'] 	= "$06";	//DefaultGateway
			G_WANConn['DNSOverrideAllowed'] = "$07";	//DNSOverrideAllowed
			G_WANConn['DNSServers'] 		= "$08";	//DNSServers
			G_WANConn['AddressingType'] 	= "$09";	//AddressingType
			G_WANConn['ConnectionType'] 	= "$0a";	//ConnectionType
			G_WANConn['NATType'] 			= "$0b";	//X_TWSZ-COM_NATType
			G_WANConn['ServiceList'] 		= "$0c";	//X_TWSZ-COM_ServiceList
			G_WANConn['ProtocolType'] 		= "$0d";	//X_TWSZ-COM_ProtocolType
			G_WANConn['RouteProtocolRx'] 	= "$0e";	// RouteProtocolRx
			G_WANConn['RipDirection'] 		= "$0f";	// RipDirection
			G_WANConn['DomainName'] 		= "$0g";	// X_TWSZ-COM_DomainName
			G_WANConn['Hostname'] 			= "$0h";	// X_TWSZ-COM_Hostname
			G_WANConn['X_TWSZ-COM_UsrDNSServers']  = "$0i";	//X_TWSZ-COM_UsrDNSServers
		`?>
	`?>
	<?if gt $12 0
	`	<?objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "Enable NATEnabled Username PPPAuthenticationProtocol ConnectionTrigger IdleDisconnectTime MaxMRUSize MaxMTUSize PPPLCPEchoRetry X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_NATType PPPLCPEcho X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType RouteProtocolRx RipDirection ConnectionType DNSServers X_TWSZ-COM_UsrDNSServers X_TWSZ-COM_VPN_CLIENT X_TWSZ-COM_VPN_NETMASK X_TWSZ-COM_VPN_GATEWAY X_TWSZ-COM_VPN_SERVER X_TWSZ-COM_VPN_INTERFACE_REF PPPoEServiceName X_TWSZ-COM_VPN_ADDR_MODE"
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
			G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']               = "$0q"; //X_TWSZ-COM_VPN_ADDR_MODE
		`?>
	`?>	
`?>

var G_Password = "";
<?mget :InternetGatewayDevice.X_TWSZ-COM_Authentication.UserList.1. "Password"
`	G_Password   = "$01";
`?>

<?mget :InternetGatewayDevice.Time. "LocalTimeZoneName LocalTimeZone"
`	G_LocalTimeZoneName	= "$01";
	G_LocalTimeZone		= "$02";
`?>


var G_WAN = [];
<?mget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1. "X_TWSZ-COM_CloneMACAddress X_TWSZ-COM_VLANID X_TWSZ-COM_VLANPriority"
`
	G_WAN['Path'] 		= "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.";
	G_WAN['CloneMACAddress']   = "$01";	//X_TWSZ-COM_CloneMACAddress
	G_WAN['VLANID'] 		= "$02";	//VLANID
	G_WAN['VLANPriority']   = "$03";	//VLANPriority
	
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
var G_CurrentIP = "<?echo $var:sys_RemoteAddr ?>";
var G_CurrentMAC = GetMACByIP(G_CurrentIP).toUpperCase();
<? if lt 0 `<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterfaceNumberOfEntries?>`
`	<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
	`	var G_LanIPAddress  = "$01";
		var G_LanSubAddress = "$02";
	`?>
`?>
//MAC Address Clone
function GetMACByIP(ip){
	for (var i = 0; i < LanHosts.length; i++){
		if (LanHosts[i][1] == ip){
			return LanHosts[i][0];
		}
	}

	return "";
}


function searchTimeZoneName(){
	//TimeZoneName和TimeZone的对应关系图
	var timeZoneNameMap = {
		'1':'IDLE',//-12
		'2':'UTC',//-11
		'3':'HST',//-10
		'4':'AKST',//-9
		'5':'PST',//-8
		'6':'MST',//-7
		'7':'MST',//-7
		'8':'MST',//-7
		'9':'CST',//-6
		'10':'CST',//-6
		'11':'CST',//-6
		'12':'CST',//-6
		'13':'EST',//-5
		'14':'EST',//-5
		'15':'VEN',//-430
		'16':'AST',//-4
		'17':'AST',//-4
		'18':'AST',//-4
		'19':'NST',//-330
		'20':'BRT',//-3
		'21':'BRT',//-3
		'22':'BRT',//-3
		'23':'FNT',//-2
		'24':'WAT',//-1
		'25':'WAT',//-1
		'26':'GMT',//0
		'27':'GMT',//0
		'28':'MET',//+1
		'29':'MET',//+1
		'30':'MET',//+1
		'31':'MET',//+1
		'32':'MET',//+1
		'33':'EET',//+2
		'34':'EET',//+2
		'35':'EET',//+2
		'36':'EET',//+2
		'37':'EET',//+2
		'38':'EET',//+2
		'39':'EAT',//+3
		'40':'EAT',//+3
		'41':'EAT',//+3
		'42':'IRT',//+330
		'43':'MUT',//+4
		'44':'MUT',//+4
		'45':'MUT',//+4
		'46':'NZT',//+430
		'47':'TFT',//+5
		'48':'LKA',//+530
		'49':'LKA',//+530
		'50':'NPL', //+545
		'51':'ALMT',//+600
		'52':'ALMT',//+600
		'53':'MMT', //+630
		'54':'WAST',//+7
		'55':'WAST',//+7
		'56':'CCT',//+8
		'57':'CCT',//+8
		'58':'CCT',//+8
		'59':'CCT',//+8
		'60':'CCT',//+8
		'61':'CCT',//+8
		'62':'JST',//+9
		'63':'JST',//+9
		'64':'JST',//+9
		'65':'JST',//+9
		'66':'JST',//+9
		'67':'AEST',//+10
		'68':'AEST',//+10
		'69':'AEST',//+10
		'70':'AEST',//+10
		'71':'AEST',//+10
		'72':'AESST',//+11
		'73':'AESST',//+11
		'74':'AESST',//+12
		'75':'NZT',//+12
		'76':'NZT',//+12
		'77':'NZDT',//+13
		'78':'NZDT'//+13
	};
	for(var i in timeZoneNameMap){
		if(i == arguments[0]){
			return timeZoneNameMap[i];
		}
	}
	return G_LocalTimeZoneName;
}

function ShowCurrentStage()
{
	var i = 0;
	var type = "";
	for (i=0; i<wanTypes.length; i++)
	{
		type = wanTypes[i];		
		$(type).style.display = "none";
	}
	for (i=0; i<stages.length; i++)
	{
		if (i==currentStage)
		{
			$(stages[i]).style.display = "block";
			if (stages[currentStage]=="stage_ether_cfg")
			{
				type = wanTypes[currentWanType];
				$(type).style.display = "block";
			}
		}
		else
		{
			$(stages[i]).style.display = "none";
		}
	}
	if(wanTypes[currentWanType]=="STATIC" || wanTypes[currentWanType]=="DHCP" || wanTypes[currentWanType]=="PPPoE")
		$("DNS").style.display = "none";
	else	
		$("DNS").style.display = "block";		

	if (currentStage==0)
		SetButtonDisabled("b_pre", true);
	else
		SetButtonDisabled("b_pre", false);

	if (currentStage==stages.length-1)
	{
		SetButtonDisabled("b_next", true);
		SetButtonDisabled("b_send", false);
	}
	else
	{
		SetButtonDisabled("b_next", false);
		SetButtonDisabled("b_send", true);
	}
}
function OnClickCloneMAC()
{
	$("wiz_dhcp_mac").value = G_CurrentMAC;
}
function OnChangePPTPMode()
{
	var disable = document.getElementsByName("wiz_pptp_conn_mode")[0].checked ? true: false;
	$("wiz_pptp_ipaddr").disabled = disable;
	$("wiz_pptp_mask").disabled = disable;
	$("wiz_pptp_gw").disabled = disable;
}
function OnChangeL2TPMode()
{
	var disable = document.getElementsByName("wiz_l2tp_conn_mode")[0].checked ? true: false;
	$("wiz_l2tp_ipaddr").disabled = disable;
	$("wiz_l2tp_mask").disabled = disable;
	$("wiz_l2tp_gw").disabled = disable;
}
function SetStage(offset)
{
	var length = stages.length;
	switch (offset)
	{
	case 1:
		if (currentStage < length-1)
			currentStage += 1;
		break;
	case -1:
		if (currentStage > 0)
			currentStage -= 1;
		break;
	}
}
function OnClickPre()
{
	SetStage(-1);
	ShowCurrentStage();
}
function OnClickNext()
{
	var stage = stages[currentStage];

	if (stage == "stage_passwd")
	{
		if ($("wiz_passwd").value=="")
		{
			alert(SEcode["lang_admin_passwd_empty"]);
			return false;			
		}	
		if ($("wiz_passwd").value!="**********")
		{
			if(!$("wiz_passwd").value.match(/^[0-9a-zA-Z\\.@_$-]{1,15}$/))
			{
				alert(SEcode["lang_invalid_passwd"]);
				return false;			
			}
			for(var i=0;i < $("wiz_passwd").value.length;i++)
			{
				if ($("wiz_passwd").value.charCodeAt(i) > 256)
				{ 
					alert(SEcode["lang_invalid_passwd"]);
					return false;
				}
			}			
		}	

		if ($("wiz_passwd").value!=$("wiz_passwd2").value)
		{
			alert(SEcode["lang_passwd_not_match"]);
			return false;
		}
		SetStage(1);
		ShowCurrentStage();
	}
	else if (stage == "stage_ether_cfg")
	{
		var type = wanTypes[currentWanType];
		CheckWANSettings(type);
	}
	else if (stage == "stage_ether")
	{
		if(Form.Radio("wan_mode") == "DHCP")
		{
			if(findProtocol() != "DHCP")
			{
				$("wiz_dhcp_host").value = "dlinkrouter";
			}
			else
			{
				$("wiz_dhcp_host").value = G_WANConn['Hostname'];
			}
		}
		SetStage(1);
		ShowCurrentStage();
	}
	else
	{
		SetStage(1);
		ShowCurrentStage();
	}
}



function checkMACaddr(mac){
    var clonmac=$(mac).value;
    var pattern=/^([0-9A-Fa-f]{2})(-[0-9A-Fa-f]{2}){5}|([0-9A-Fa-f]{2})(:[0-9A-Fa-f]{2}){5}/;
	var result = pattern.test(clonmac);
   
    if(result == false)
    {
    	alert(SEcode['lang_invalid_mac']);
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
		alert(SEcode['lang_pri_dns_invalid']);
		return false;
	}

	if ((tmpPriorityDNS!='') && !checkDNSValue(tmpPriorityDNS)) 
	{
		alert(SEcode['lang_pri_dns_invalid']);
		return false;
	}
	/*Secondary DNS Address is option*/
	  
	if ((tmpSecondaryDNS!='') && !checkDNSValue(tmpSecondaryDNS))
	{
		alert(SEcode['lang_sec_dns_invalid']);
		return false;
	}
	
	if(tmpPriorityDNS&&tmpPriorityDNS==tmpSecondaryDNS)
	{
		alert(SEcode['lang_sec_dns_invalid']);
		return false;
	}
    return true;
}


function CheckWANSettings(type)
{
	switch (type)
	{
	case "DHCP":
		if($("wiz_dhcp_mac").value !="")
			if(!checkMACaddr('wiz_dhcp_mac'))
				return false;
		if($("wiz_dhcp_dns1").value !="" || $("wiz_dhcp_dns2").value !="")
			if(!checkDNS('wiz_dhcp_dns'))
				return false;
        break;
	case "PPPoE":
		if($("wiz_pppoe_usr").value=="")
		{
			alert(SEcode["lang_user_empty"]);
			return false;
		}
		if($("wiz_pppoe_passwd").value=="")
		{
			alert(SEcode["lang_passwd_empty"]);
			return false;
		}
		break;
	case "PPTP":
		if ($("wiz_"+type.toLowerCase()+"_passwd").value!=
			$("wiz_"+type.toLowerCase()+"_passwd2").value)
		{
			alert(SEcode["lang_passwd_not_match"]);
			return false;
		}
		
		if ($("wiz_pptp_usr").value == "") 
		{
			alert(SEcode["lang_user_empty"]);
			return false;
		}
		if ($("wiz_pptp_passwd").value == "") 
		{
			alert(SEcode["lang_passwd_empty"]);
			return false;
		}
		if($("wiz_pptp_svr").value == G_LanIPAddress)
		{
			alert(SEcode["lang_lan_pptp_server_conflict"]);//
			$("wiz_pptp_svr").focus();
			return false;
		}
		if ($("wiz_pptp_svr").value == "" || $("wiz_pptp_svr").value=="0.0.0.0") 
		{
			alert(SEcode["lang_invalid_pptp_server"]);
			return false;
		}
		//if (!CheckValidity.IP("wiz_pptp_svr",SEcode["lang_invalid_pptp_server"]))
		//	return false;
		if($("dns1").value !="" || $("dns2").value !="")
			if(!checkDNS('dns'))
				return false;
		if (document.getElementsByName("wiz_pptp_conn_mode")[1].checked == true) 
		{

			if ($("wiz_pptp_ipaddr").value == "" || $("wiz_pptp_ipaddr").value=="0.0.0.0") 
			{
				alert(SEcode["lang_invalid_ip"]);	
				return false;
			}
			if (!CheckValidity.IP("wiz_pptp_ipaddr",SEcode["lang_invalid_ip"]))
				return false;
			
			if ($("wiz_pptp_mask").value == "" || $("wiz_pptp_mask").value=="0.0.0.0") 
			{
				alert(SEcode["lang_invalid_mask"]);	
				return false;
			}
			if (!CheckValidity.Mask("wiz_pptp_mask",SEcode["lang_invalid_mask"]))
				return false;
			
			if ($("wiz_pptp_gw").value == "" || $("wiz_pptp_gw").value=="0.0.0.0") 
			{
				alert(SEcode["lang_gate_invalid"]);	
				return false;
			}
			if (!CheckValidity.IP("wiz_pptp_gw",SEcode["lang_gate_invalid"]))
				return false;
			

			if(!isSameSubNet($("wiz_pptp_gw").value,$('wiz_pptp_mask').value,$('wiz_pptp_ipaddr').value,$('wiz_pptp_mask').value))
			{
				alert(SEcode["lang_gate_same_ip"]);
				$("wiz_pptp_gw").focus();
				return false;
			}

			if(!checkDNS('dns'))
				return false;
			if(isSameSubNet($("wiz_pptp_ipaddr").value,$('wiz_pptp_mask').value,G_LanIPAddress,G_LanSubAddress))
			{
				alert(SEcode["lang_lan_wan_conflict"]);	
				$("wiz_pptp_ipaddr").focus();
				return false;
			}
			if($('wiz_pptp_svr').value == $('wiz_pptp_ipaddr').value)
			{
				alert(SEcode['lang_pptp_server_same_ip']);
				return false;
			}
		}
		
		break;
	case "L2TP":
		if ($("wiz_"+type.toLowerCase()+"_passwd").value!=
			$("wiz_"+type.toLowerCase()+"_passwd2").value)
		{
			alert(SEcode["lang_passwd_not_match"]);
			return false;
		}


		if ($("wiz_l2tp_usr").value == "") 
		{
			alert(SEcode["lang_user_empty"]);
			return false;
		}
		if ($("wiz_l2tp_passwd").value == "") 
		{
			alert(SEcode["lang_passwd_empty"]);
			return false;
		}
		if($("wiz_l2tp_svr").value == G_LanIPAddress)
		{
			alert(SEcode["lang_lan_l2tp_server_conflict"]);//
			$("wiz_l2tp_svr").focus();
			return false;
		}
		if ($("wiz_l2tp_svr").value == "" || $("wiz_l2tp_svr").value=="0.0.0.0") 
		{
			alert(SEcode["lang_invalid_l2tp_server"]);
			return false;
		}
		//if (!CheckValidity.IP("wiz_l2tp_svr",SEcode["lang_invalid_l2tp_server"]))
		//	return false;
		if($("dns1").value !="" || $("dns2").value !="")
			if(!checkDNS('dns'))
				return false;		
		if (document.getElementsByName("wiz_l2tp_conn_mode")[1].checked == true) 
		{

			if ($("wiz_l2tp_ipaddr").value == "" || $("wiz_l2tp_ipaddr").value=="0.0.0.0") 
			{
				alert(SEcode["lang_invalid_ip"]);	
				return false;
			}
			if (!CheckValidity.IP("wiz_l2tp_ipaddr",SEcode["lang_invalid_ip"]))
				return false;
			
			if ($("wiz_l2tp_mask").value == "" || $("wiz_l2tp_mask").value=="0.0.0.0") 
			{
				alert(SEcode["lang_invalid_mask"]);	
				return false;
			}
			if (!CheckValidity.Mask("wiz_l2tp_mask",SEcode["lang_invalid_mask"]))
				return false;
			
			if ($("wiz_l2tp_gw").value == "" || $("wiz_l2tp_gw").value=="0.0.0.0") 
			{
				alert(SEcode["lang_invalid_def_gate"]);	
				return false;
			}
			if (!CheckValidity.IP("wiz_l2tp_gw",SEcode["lang_invalid_def_gate"]))
				return false;
			
			if(!isSameSubNet($("wiz_l2tp_gw").value,$('wiz_l2tp_mask').value,$('wiz_l2tp_ipaddr').value,$('wiz_l2tp_mask').value))
			{
				alert(SEcode["lang_gate_same_ip"]);
				$("wiz_l2tp_gw").focus();
				return false;
			}
			if(!checkDNS('dns'))
				return false;
			if(isSameSubNet($("wiz_l2tp_ipaddr").value,$('wiz_l2tp_mask').value,G_LanIPAddress,G_LanSubAddress))
			{
				alert(SEcode["lang_lan_wan_conflict"]);	
				$("wiz_l2tp_ipaddr").focus();
				return false;
			}
			if($('wiz_l2tp_svr').value == $('wiz_l2tp_ipaddr').value)
			{
				alert(SEcode['lang_l2tp_server_same_ip']);
				return false;
			}
		}
		
		break;
	case "STATIC":

		if ($("wiz_static_ipaddr").value == "" || $("wiz_static_ipaddr").value=="0.0.0.0") 
		{
			alert(SEcode["lang_invalid_ip"]);	
			return false;
		}
		if (!CheckValidity.IP("wiz_static_ipaddr",SEcode["lang_invalid_ip"]))
			return false;
	
		if ($("wiz_static_mask").value == "" || $("wiz_static_mask").value=="0.0.0.0") 
		{
			alert(SEcode["lang_invalid_mask"]);	
			return false;
		}
		if (!CheckValidity.Mask("wiz_static_mask",SEcode["lang_invalid_mask"]))
			return false;
		
		if ($("wiz_static_gw").value == "" || $("wiz_static_gw").value=="0.0.0.0") 
		{
			alert(SEcode["lang_invalid_def_gate"]);	
			return false;
		}
		if (!CheckValidity.IP("wiz_static_gw",SEcode["lang_invalid_def_gate"]))
			return false;
	
		if(!isSameSubNet($("wiz_static_gw").value,$('wiz_static_mask').value,$('wiz_static_ipaddr').value,$('wiz_static_mask').value))
			{
				alert(SEcode["lang_gate_same_ip"]);
				$("wiz_static_gw").focus();
				return false;
			}
		if(!checkDNS('wiz_static_dns'))
			return false;
		if(isSameSubNet($("wiz_static_ipaddr").value,$('wiz_static_mask').value,G_LanIPAddress,G_LanSubAddress))
		{
			alert(SEcode["lang_lan_wan_conflict"]);	
			$("wiz_static_ipaddr").focus();
			return false;
		}
		break;
	}
	
	SetStage(1);
	ShowCurrentStage();
    
    return true;
}

function OnClickCancel()
{
	if (confirm(SEcode["lang_cancel_setting"]))
		self.location.href = "/cgi-bin/webproc?getpage=html/index.html&var:menu=basic&var:page=Bas_wansum";
}
function OnChangeWanType(type)
{
	for (var i=0; i<wanTypes.length; i++)
	{
		if (wanTypes[i]==type)
			currentWanType = i;
	}
}	
function SetButtonDisabled(name, disable)
{
	var button = document.getElementsByName(name);
	for (i=0; i<button.length; i++)
	{
		button[i].disabled = disable;
	}
}


function findProtocol( ){
    var _protocol = 'DHCP';
	var Conn_Type = G_WANConn['Path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";
	if(Conn_Type == 'PPP')
	{
		if(G_WANConn['ConnectionType']=="PPTP_Relay"){
			_protocol = 'PPTP';
		}else if(G_WANConn['ConnectionType']=="L2TP_Relay"){
			_protocol = 'L2TP';
		}else{
			_protocol = 'PPPoE';
		}
	}else
	{
		if(G_WANConn['ConnectionType'] != 'IP_Bridged')
		{
			if(G_WANConn['AddressingType'] == 'Static')
			{
				_protocol = 'STATIC';
			}else{
				_protocol = 'DHCP';
			}
		}
	}
	return _protocol;
}


function uiOnload(){

	var type = findProtocol();
	ShowCurrentStage();
	setJSONValue({
		"wan_mode" 		: type,
		'wiz_tz'        : G_LocalTimeZone || '56'
	});
	OnChangeWanType(type);
	
	switch(type) {
		case "DHCP" :
			setJSONValue({
			"wiz_dhcp_mac"      : G_WAN['CloneMACAddress'],
			"wiz_dhcp_host"     : G_WANConn['Hostname']||"dlinkrouter",
			"wiz_dhcp_dns1"     : G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || '',
			"wiz_dhcp_dns2"     : G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || ''
			});
			break;
			
		case "STATIC" :
			setJSONValue({
			"wiz_static_ipaddr"      : G_WANConn['ExternalIPAddress'] || "0.0.0.0",
			"wiz_static_mask"        : G_WANConn['SubnetMask'] || "0.0.0.0",
			"wiz_static_gw"          : G_WANConn['DefaultGateway'] || "0.0.0.0",
			"wiz_static_dns1"        : G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || '',
			"wiz_static_dns2"        : G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || ''
			});
			break;
			
		case "PPPoE" :
			setJSONValue({
			"wiz_pppoe_usr"           : G_WANConn['Username'] || "",
			"wiz_pppoe_passwd"        : G_WANConn['Password']
			});
			break;
			
		case "PPTP" :
			setJSONValue({
			"wiz_pptp_ipaddr"           : G_WANConn['X_TWSZ-COM_VPN_CLIENT'] || '0.0.0.0',
			"wiz_pptp_mask"        		: G_WANConn['X_TWSZ-COM_VPN_NETMASK'] || '0.0.0.0',
			"wiz_pptp_gw"        		: G_WANConn['X_TWSZ-COM_VPN_GATEWAY'] || '0.0.0.0',
			"wiz_pptp_svr"        		: G_WANConn['X_TWSZ-COM_VPN_SERVER'] || '0.0.0.0',
			"wiz_pptp_usr"        		: G_WANConn['Username'] || '',
			"wiz_pptp_passwd"       	: G_WANConn['Password'] || '',
			"wiz_pptp_passwd2"        	: G_WANConn['Password'] || '',
			"dns1"        				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || '',
			"dns2"        				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || ''
			});
			Form.Radio("wiz_pptp_conn_mode",  G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"? "static" : "dynamic");
			
			break;
		case "L2TP" :
			setJSONValue({
			"wiz_l2tp_ipaddr"           : G_WANConn['X_TWSZ-COM_VPN_CLIENT'] || '0.0.0.0',
			"wiz_l2tp_mask"        		: G_WANConn['X_TWSZ-COM_VPN_NETMASK'] || '0.0.0.0',
			"wiz_l2tp_gw"        		: G_WANConn['X_TWSZ-COM_VPN_GATEWAY'] || '0.0.0.0',
			"wiz_l2tp_svr"        		: G_WANConn['X_TWSZ-COM_VPN_SERVER'] || '0.0.0.0',
			"wiz_l2tp_usr"        		: G_WANConn['Username'] || '',
			"wiz_l2tp_passwd"        	: G_WANConn['Password'] || '',
			"wiz_l2tp_passwd2"        	: G_WANConn['Password'] || '',
			"dns1"       				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[0] || '',
			"dns2"      				: G_WANConn['X_TWSZ-COM_UsrDNSServers'].split(',')[1] || ''
			});
			Form.Radio("wiz_l2tp_conn_mode",  G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"? "static" : "dynamic");
			
			break;
		default:;
	}
	OnChangePPTPMode();
	OnChangeL2TPMode();
	
	if ("$1$TW$W4EX5n8uMLE15bpd62uqD." == G_Password)
	{
		$("wiz_passwd").value         = ""; 
		$("wiz_passwd2").value        = ""; 
	}
	else
	{
		$("wiz_passwd").value         = "**********"; 
		$("wiz_passwd2").value        = "**********"; 	
	}
}


function getProtocol()
{
	var type = wanTypes[currentWanType];
	switch(type){
		case 'DHCP' : {
			return ['DHCP','IP'];
		}
		case 'STATIC' : {
			return ['Static','IP'];
		}
		case 'PPPoE' : {
			return ['PPPoE','PPP'];
		}
		case 'PPTP' : {
			return ['PPTP','PPP'];
		}
		case 'L2TP' : {
			return ['L2TP','PPP'];
		}
	}
}
function null_errorfunc()
{

	return true;
}
function Ajax_handler(_text)
{
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		dealWithError();
		return;
	}
	var Basepsw= $('wiz_passwd').value;

	if($('wiz_passwd').value != '' && $('wiz_passwd').value != '**********' )
	{		
		var pass = $("wiz_passwd").value;
		Basepsw= Base64.Encode(pass);
	}
	G_SysToken = G_AjaxToken;
	if(G_Error == '1')
	{
		$("content").style.display="";
		$("mbox").style.display="none";
		dealWithError();
		document.getElementById("WIZ197").disabled = false;
	}	
	else
	{
		$H({
			"obj-action"           : "set",
			"getpage"              : "html/index.html",
			"errorpage"            : "html/index.html",
			"var:menu"             : "basic",
			"var:page"             : "Bas_wansum",
			"var:errorpage"        : "Bas_wansum",
			'var:sys_Token' : G_SysToken,
			"var:CacheLastData" 	: ViewState.Save()
		}, true);
	
		//TimeZone
		$F(':InternetGatewayDevice.Time.LocalTimeZone', document.getElementById("wiz_tz").value);
		$F(':InternetGatewayDevice.Time.LocalTimeZoneName', searchTimeZoneName(document.getElementById("wiz_tz").value) );
		$F(':InternetGatewayDevice.Time.DaylightSavingsUsed', '1');	

		
		//password
		if($('wiz_passwd').value != '**********' )
			$F(':InternetGatewayDevice.X_TWSZ-COM_Authentication.UserList.1.Password', Basepsw);
	
		$('uiPostForm').submit();
	}
}
function OnSubmit(){
	
	var ConnPath;
	var _addrtype;
	var _protocol = getProtocol();

	
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
		"var:page"             : "Bas_wansum",
		"var:errorpage"        : "Bas_wansum",
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	
	
	if(G_WANConn['Path'] && G_WANConn['Path'].indexOf(_protocol[1]) > -1){
		ConnPath = G_WANConn['Path'];
	}else{
		ConnPath = G_WANConn['Path'].replace(_protocol[1] == 'PPP' ? 'IP' : 'PPP', _protocol[1]);
		
		$F('mid','0438');
		$F(":" + ConnPath + "X_TWSZ-COM_ConnectionMode", G_WANConn['Path'].indexOf('IP') > 0 ? 'IP' : 'PPP');
	}
	
	
	//VLAN 节点的路径
	var DevicePath = G_WAN['Path'];
	
	//vlan
	if(_protocol[0] != "PPTP" && _protocol[0] != "L2TP")
	{
		$F(":" + DevicePath + "X_TWSZ-COM_VLANID", 				'0');
	}
	$F(":" + DevicePath + "X_TWSZ-COM_VLANPriority", 		'0');
	
	//IP common
	$F(":" + ConnPath + "Enable", 						'1');
	$F(":" + ConnPath + "Name", 						_protocol[0] + '_1');	
	$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 		'IPv4');
	$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 		'Internet');
	$F(":" + ConnPath + "NATEnabled", 					'1');
	$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");	
	$F(":" + ConnPath + "RouteProtocolRx", 		"Off");
	$F(":" + ConnPath + "RipDirection", 		"Both");	
	$F(":" + ConnPath + "AddressingType", 		_addrtype == "undefined" ? undefined : _addrtype);
	
	///*
	switch(_protocol[0]){
		case "DHCP" :
			var _dnsservers = $('wiz_dhcp_dns1').value+','+$('wiz_dhcp_dns2').value;
			$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
			$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 					_dnsservers.delcomma());
			$F(":" + ConnPath + 'X_TWSZ-COM_Hostname',  		$('wiz_dhcp_host').value);		
			$F(':InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.X_TWSZ-COM_CloneMACAddress', $('wiz_dhcp_mac').value);
			$F(":" + ConnPath + "MaxMTUSize", 			"1500");	
			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");	
			$F(":" + ConnPath + "X_TWSZ-COM_DhcpUseUnicast",     "0");
		  	break;
		case "Static" :
			var _dnsservers = $('wiz_static_dns1').value+','+$('wiz_static_dns2').value;
			$F(":" + ConnPath + "ExternalIPAddress", 	$('wiz_static_ipaddr').value);
			$F(":" + ConnPath + "SubnetMask", 			$('wiz_static_mask').value);
			$F(":" + ConnPath + "DefaultGateway", 		$('wiz_static_gw').value);
			$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
			$F(":" + ConnPath + "X_TWSZ-COM_UsrDNSServers", 			_dnsservers.delcomma());
			$F(":" + ConnPath + "MaxMTUSize", 			"1500");				
			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			break;
		case "PPPoE" :
			var pppoe_passwd = Base64.Encode($('wiz_pppoe_passwd').value);
			$F(":" + ConnPath + "Username", 		$('wiz_pppoe_usr').value);
			$F(":" + ConnPath + "Password", 		$('wiz_pppoe_passwd').value != '**********' ? pppoe_passwd : undefined);
			$F(":" + ConnPath + "PPPAuthenticationProtocol", "Auto");
			$F(":" + ConnPath + "ConnectionTrigger", 	"AlwaysOn" );
			$F(":" + ConnPath + "MaxMRUSize", 		"1492");
			$F(":" + ConnPath + "MaxMTUSize", 		"1492");
			$F(":" + ConnPath + "PPPLCPEcho", 		"15");
			$F(":" + ConnPath + "PPPLCPEchoRetry", 		"15");
			$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", "");	
			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			$F(":" + ConnPath + "DNSOverrideAllowed", 	"0");	
			
			break;
		case "PPTP" :
		case "L2TP" :	
			var _dnsservers = $('dns1').value+','+$('dns2').value;
			var x;
			if (_protocol[0] == "PPTP")
				x = (Form.Radio("wiz_pptp_conn_mode") == 'dynamic') ? '1' : '0';
			else 
				x = (Form.Radio("wiz_l2tp_conn_mode") == 'dynamic') ? '1' : '0';
			
			if(x == "1"){//Dynamic
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"DHCP");  //address mode
			} else {//static
				if (_protocol[0] == "PPTP") {
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_CLIENT',  	(Form.Radio("wiz_pptp_conn_mode") == 'dynamic') ? '' :$('wiz_pptp_ipaddr').value);
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_NETMASK', 	(Form.Radio("wiz_pptp_conn_mode") == 'dynamic') ? '' :$('wiz_pptp_mask').value);
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_GATEWAY', 	(Form.Radio("wiz_pptp_conn_mode") == 'dynamic') ? '' :$('wiz_pptp_gw').value);
					
					
				} else {
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_CLIENT',  	(Form.Radio("wiz_l2tp_conn_mode") == 'dynamic') ? '' :$('wiz_l2tp_ipaddr').value);
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_NETMASK', 	(Form.Radio("wiz_l2tp_conn_mode") == 'dynamic') ? '' :$('wiz_l2tp_mask').value);
					$F(":" + ConnPath + 'X_TWSZ-COM_VPN_GATEWAY', 	(Form.Radio("wiz_l2tp_conn_mode") == 'dynamic') ? '' :$('wiz_l2tp_gw').value);
					
					
				}
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_ADDR_MODE',  	"Static"); 
							
			}
			
			$F(":" + ConnPath + 'X_TWSZ-COM_VPN_INTERFACE_REF',  	"");
			if (_protocol[0] == "PPTP") {
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_SERVER',  	$('wiz_pptp_svr').value);
				$F(":" + ConnPath + 'Username', 					$('wiz_pptp_usr').value);
				if ($('wiz_pptp_passwd').value != '**********')
				{
					var pptp_passwd = Base64.Encode($('wiz_pptp_passwd').value);
					$F(":" + ConnPath + 'Password', 				pptp_passwd);
				}
				
			} else {
				$F(":" + ConnPath + 'X_TWSZ-COM_VPN_SERVER',  	$('wiz_l2tp_svr').value);
				$F(":" + ConnPath + 'Username', 					$('wiz_l2tp_usr').value);
				if ($('wiz_l2tp_passwd').value != '**********')
				{
					var l2tp_passwd = Base64.Encode($('wiz_l2tp_passwd').value);
					$F(":" + ConnPath + 'Password', 				l2tp_passwd);
				}
				
			}
			
			
			/* VPN只设置MTU, MRU随MRU保持一致 */
			$F(":" + ConnPath + "MaxMRUSize", 				'1460');
			$F(":" + ConnPath + 'MaxMTUSize', 				'1460');
			
			$F(":" + ConnPath + 'ConnectionType', 			(_protocol[0]=="PPTP"?"PPTP_Relay":"L2TP_Relay"));
			$F(":" + ConnPath + 'ConnectionTrigger',         'AlwaysOn');
			$F(":" + ConnPath + 'PPPAuthenticationProtocol', 'Auto');	
			$F(":" + ConnPath + "DNSOverrideAllowed", 	_dnsservers==","?"0":"1");
			$F(":" + ConnPath + 'X_TWSZ-COM_UsrDNSServers', _dnsservers.delcomma());
			break;	
		default :
			;
	}
	//*/

	document.getElementById("WIZ197").disabled = true;
	var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js&var:page=*";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.post($('uiPostForm'));
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
//监听加载与错误处理函擿
addListeners(uiOnload, dealWithError);

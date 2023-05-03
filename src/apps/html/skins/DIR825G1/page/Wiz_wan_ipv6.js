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

//ipv4
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 1
`	<? if gt $21 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "X_TWSZ-COM_ProtocolType"
		`	<?if eq $11 `IPv4`
			`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.<?echo $20?>";		
			`?>
		`?>
	`?>
	<? if gt $22 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "X_TWSZ-COM_ProtocolType ConnectionTrigger Username Password PPPoEServiceName IdleDelayTime MaxMTUSize X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers  X_TWSZ-COM_StaticIPAddress X_TWSZ-COM_IPv6Config.IPv6AddressList.2.IPv6Address X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType Enable"
		`	<?if eq $11 `IPv4`
			`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.<?echo $20?>";	//Path		
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
			`?>

		`?>
	`?>
`?>	
`?>	

//TUNNEL 
var G_TunConn = [];

<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel.  "Activated Mode Mechanism Dynamic Prefix  RemoteIpv4Address  IPv4MaskLen  RemoteIpv6Address BorderRelayAddress LocalIpv6Address LocalIPv6PrefixLength IPv6DNSServers"
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
	G_TunConn['IPv6DNSServers']   = "$0c";	//IPv6DNSServers
`?>


var G_WanMac = '<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.X_TWSZ-COM_MACAddress?>'.toUpperCase();
var G_GAddr = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress?>";

var stages = new Array ("stage_descv6", "stage_wan_detectv6", "stage_etherv6", "stage_ether_cfgv6", "stage_finishv6");
var wanTypes = new Array ("PPPoE", "AUTO", "STATIC", "6RD", "DSLITE");
var wanDetectProcess = new Array ("wan_detectv6", "wantypev6_unkown");
var wanDetectNum = 0;
var currentStage = 0;	// 0 ~ stages.length
var currentWanType = 0;	// 0 ~ wanTypes.length

var wanDetectCheckNum = 0;
var wanDetectCheckTimer;
function SetButtonDisabled(name, disable)
{
	var button = document.getElementsByName(name);
	for (i=0; i<button.length; i++)	button[i].disabled = disable;
}
function GetRadioValue(name)
{
	var radio = document.getElementsByName(name);
	var value = null;
	for (i=0; i<radio.length; i++)
	{
		if (radio[i].checked)	return radio[i].value;
	}
}

function SetRadioValue(name, value)
{
	var radio = document.getElementsByName(name);
	for (i=0; i<radio.length; i++)
	{
		if (radio[i].value==value)	radio[i].checked = true;
	}
}
function findProtocol()
{
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
	return Wan_Mode;
}
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
function OnClickUsell()
{
	if ($("usell").checked)
	{
		$("wiz_static_wan_v6addr").disabled 	= true;
		$("wiz_static_pfxlen").disabled 		= true;
		var MACs=G_WanMac.split(":");
		MACs[0]=MACs[0]^2;
		var r3ipaddr = "FE80::"+MACs[0]+MACs[1]+":"+MACs[2]+"FF:FE"+MACs[3]+":"+MACs[4]+MACs[5];

		$("wiz_static_wan_v6addr").value	= r3ipaddr;
		$("wiz_static_pfxlen").value		= 64;
	}
	else
	{
		$("wiz_static_wan_v6addr").disabled 	= false;
		$("wiz_static_pfxlen").disabled 		= false;
	}		
}
var Wan_Mode = "";
function InitValue()
{
	findProtocol();
	OnChangeWanv6Type(Wan_Mode);
	if(Wan_Mode == "STATIC")
	{
		$("wiz_static_wan_v6addr").value = G_Wanipv6Conn['IPv6Address'];
		$("wiz_static_pfxlen").value = G_Wanipv6Conn['PrefixLength'];
		$("wiz_static_gw").value = G_Wanipv6Conn['DefaultRouter'];
		$("wiz_static_lan_v6addr").value = G_GAddr;
		$("wiz_static_pridns6").value = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",")[0];
		if(G_Wanipv6Conn['UsrIPv6DNSServers'].split(",")[1]!=undefined)
			$("wiz_static_secdns6").value = G_Wanipv6Conn['UsrIPv6DNSServers'].split(",")[1];
		
		var G_usell = G_Wanipv6Conn['AddressStatus'];
	
		if(G_usell == "Invalid")
		{
			$("usell").checked = true;
		}
		else
		{
			$("usell").checked = false;
		}			
		OnClickUsell();
	}
	else if(Wan_Mode == "PPPOE")
	{
		if(G_WANConn['X_TWSZ-COM_ProtocolType'] == "IPv4_6")
		{
			document.getElementsByName("wiz_pppoe_sess_type")[0].checked = true;
		}
		else
		{
			document.getElementsByName("wiz_pppoe_sess_type")[1].checked = true;
		}
		$("wiz_pppoe_usr").value = G_Wanipv6Conn['Username'];
		$("wiz_pppoe_passwd").value = G_Wanipv6Conn['Password'];
		$("wiz_pppoe_svc").value = G_Wanipv6Conn['PPPoEServiceName'];
		$("wiz_pppoe_passwd2").value = $("wiz_pppoe_passwd").value;
		
		// OnChangePPPoESessType();
	}
	else if(Wan_Mode == "6RD")
	{
		$("wiz_6rd_v4addr").disabled = true;
		var temp = G_TunConn['Prefix'].split('/');
		
		$("wiz_6rd_prefix").value = temp[0];
		if(temp[1]!=undefined)	
			$("wiz_6rd_pfxlen").value = temp[1];
		$("wiz_6rd_v4addr_mask").value = G_TunConn['IPv4MaskLen'];
		$("wiz_6rd_v4addr").value = G_TunConn['RemoteIpv4Address'];
		$("wiz_6rd_v6addr").innerHTML =G_TunConn['RemoteIpv6Address'];
		$("wiz_6rd_relay").value = G_TunConn['BorderRelayAddress'];
		$("wiz_6rd_pridns6").value = G_TunConn['IPv6DNSServers'];
		
	}
	OnChangePPPoESessType();
	
}
//加载初始势
function uiOnload()
{
	InitValue();
	ShowCurrentStage();
}
function OnClickCancel()
{
	if (confirm(SEcode["lang_cancel_setting"]))
		self.location.href = "/cgi-bin/webproc?getpage=html/index.html&var:menu=basic&var:page=Bas_ipv6_setup";	
}
function OnChangeWanv6Type(type)
{
	for (var i=0; i<wanTypes.length; i++)
	{			
		if (wanTypes[i]==type)
			currentWanType = i;
	}
}
function LoadPpp4Value()
{
	if(Wan_Mode == "PPPOE")
	{
		$("wiz_pppoe_usr").value = G_Wanipv6Conn['Username'];
		$("wiz_pppoe_passwd").value = G_Wanipv6Conn['Password'];
		$("wiz_pppoe_svc").value = G_Wanipv6Conn['PPPoEServiceName'];
		$("wiz_pppoe_passwd2").value = $("wiz_pppoe_passwd").value;
	}
	
}
function OnChangePPPoESessType()
{
	if(document.getElementsByName("wiz_pppoe_sess_type")[0].checked)
	{
		$("wiz_pppoe_usr").disabled = true;
		$("wiz_pppoe_passwd").disabled = true;
		$("wiz_pppoe_passwd2").disabled = true;
		$("wiz_pppoe_svc").disabled = true;
		LoadPpp4Value();
	}
	else
	{
		$("wiz_pppoe_usr").disabled = false;
		$("wiz_pppoe_passwd").disabled = false;
		$("wiz_pppoe_passwd2").disabled = false;
		$("wiz_pppoe_svc").disabled = false;
	}
	
}
function WanDetectCallback(type)
{
	if(stages[currentStage]!=="stage_wan_detectv6")	return;
	switch (type)
	{
		case "PPPOE":
			for(var i=0; i < stages.length; i++)	if(stages[i]==="stage_finishv6")	currentStage=i;
			for(var i=0; i < wanTypes.length; i++)	if(wanTypes[i]==="PPPoE")	currentWanType=i;
			SetRadioValue("wanv6_mode", "PPPoE");	
			$("uiPostForm").setAttribute("modified", "true");	
			ShowCurrentStage();	
			break;
		case "STATIC":
			for(var i=0; i < stages.length; i++)	if(stages[i]==="stage_finishv6")	currentStage=i;
			for(var i=0; i < wanTypes.length; i++)	if(wanTypes[i]==="STATIC")	currentWanType=i;
			SetRadioValue("wanv6_mode", "STATIC");	
			$("uiPostForm").setAttribute("modified", "true");	
			
			ShowCurrentStage();	
			break;
		case "6RD":
			for(var i=0; i < stages.length; i++)	if(stages[i]==="stage_finishv6")	currentStage=i;
			for(var i=0; i < wanTypes.length; i++)	if(wanTypes[i]==="6RD")	currentWanType=i;
			SetRadioValue("wanv6_mode", "6RD");	
			$("uiPostForm").setAttribute("modified", "true");	
			
			ShowCurrentStage();	
			break;
		default:
			//for(var i=0; i < stages.length; i++)	if(stages[i]==="stage_etherv6")	currentStage=i;
			//ShowCurrentStage();	
			$("wan_detectv6").style.display = "none";
			$("stage_wan_detectv6").style.display = "block";
			$("wantypev6_unkown").style.display = "block";
			break;
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
		return;
	}
	
	if(G_Wan_Link_status == "Down")
	{
		if(wanDetectCheckNum < 6)//扫描6*10s
		{
			$("wan_detectv6").style.display = "block";
			$("wantypev6_unkown").style.display = "none";	
			wanDetectCheckTimer = setTimeout('WanDetectv6()', 10*1000);
		}
		else//检测超时
		{
			$("wan_detectv6").style.display = "none";
			$("stage_wan_detectv6").style.display = "block";
			$("wantypev6_unkown").style.display = "block";
			return;
		}					
	}
	else
	{
		var wan_mode = findProtocol();
		//alert(wan_mode);
		if(wan_mode == "STATIC")
		{
			WanDetectCallback(wan_mode);	
		}
		else if(wan_mode === "PPPOE")
		{
			WanDetectCallback(wan_mode);
		}
		else if(wan_mode === "6RD")
		{
			WanDetectCallback(wan_mode);	
		}
		else//这三种模式之外
		{
			WanDetectCallback("");
		}	
	}

}
function WanDetectv6()
{
	wanDetectCheckNum++;
	var _url = "/cgi-bin/webproc?getpage=html/page/portforwdV6.ajax.js&var:page=*";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.get();
}
function WanDetectPre()
{
	$("wantypev6_unkown").style.display = "none";
	$("wan_detectv6").style.display = "block";
	wanDetectCheckNum = 0;
	
	//WanDetectv6();
	$H({
		"obj-action"           : "set",
		"getpage"              : "html/page/portforwdV6.ajax.js",
		"errorpage"            : "html/page/portforwdV6.ajax.js",
		"var:menu"             : "basic",
		"var:page"             : "Bas_ipv6_setup",
		"var:errorpage"        : "Bas_ipv6_setup",
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	
	var _url = "/cgi-bin/webproc?getpage=html/page/portforwdV6.ajax.js&var:page=*";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.post($("uiPostForm"));
	
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

function OnClickNext()
{
	var stage = stages[currentStage];		
	//alert(stage);
	if (stage == "stage_etherv6")
	{
		var v4mode = "auto";
		var v6mode = GetRadioValue("wanv6_mode");			
		if(v4mode == "dslite" && v6mode=="6RD")
		{
			alert(SEcode["lang_invalid_mode"]);
			return;
		}
	}
	
	// if (stage == "stage_wan_detectv6")
	// {
		// clearTimeout(wanDetectCheckTimer);
		// SetStage(1);
	// }
	if (stage == "stage_descv6")//跳过检测类型的一步
	{
		SetStage(1);
	}
			
	SetStage(1);
	ShowCurrentStage();													
}
function OnClickPre()
{
	var stage = stages[currentStage];	
	// if (stage == "stage_wan_detectv6")
	// {
		// clearTimeout(wanDetectCheckTimer);
		// SetStage(-1);
	// }
	if (stage == "stage_etherv6")//跳过检测类型的一步
	{
		SetStage(-1);
	}
	SetStage(-1);
	ShowCurrentStage();
}
function ShowCurrentStage()
{
	var i = 0;
	var type = "";
	//alert(stages[currentStage]);		
	$("wiz_6rd_v4addr").disabled = true;
	if(stages[currentStage] == "stage_etherv6")
	{			
		if(wanTypes[currentWanType]=="AUTO")
		{				
			currentWanType = 0;
			SetRadioValue("wanv6_mode", wanTypes[currentWanType]);
		}
		SetRadioValue("wanv6_mode", wanTypes[currentWanType]);
	}
	
	// Scan all WAN types (including: PPPoE, Auto, Static, and 6RD).
	for (i=0; i<wanTypes.length; i++)
	{			
		type = wanTypes[i];
					
		if(type!="AUTO" && type!="DSLITE")
			$(type).style.display = "none";
	}
	for (i=0; i<stages.length; i++)
	{
		if (i==currentStage)
		{
			$(stages[i]).style.display = "block";
			if (stages[currentStage]=="stage_ether_cfgv6")
			{
				type = wanTypes[currentWanType];
				$(type).style.display = "block";
				
			}
		}
		else	
			$(stages[i]).style.display = "none";
	}
	
	// if (stages[currentStage]=="stage_wan_detectv6")	
	// {
		// WanDetectPre();
	// }
	// else	
		// for (var j=0; j<wanDetectProcess.length; j++)	
			// $(wanDetectProcess[j]).style.display = "none";

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

function checkPWD(name)
{
	if($(name+'_passwd').value != $(name+'_passwd2').value){
		alert(SEcode[1010]);
		return false;
	}
	return true;
}


function CheckData()
{
	var type = GetRadioValue("wanv6_mode");

	switch (type)
	{
		case "PPPoE":
			var share_pppoe = document.getElementsByName("wiz_pppoe_sess_type")[0].checked ? true: false;
			if(share_pppoe)//share with ipv4
			{
			}
			else//creat a new session
			{
				if(!checkPWD('wiz_pppoe'))
					return false;			
			}
			break;

		case "STATIC":
			
			if ($("wiz_static_pridns6").value == "")	
			{
				alert(SEcode["lang_pri_dns_invalid"]);
				$("wiz_static_pridns6").focus();
				return false;
			}
				
			
			break;
		case "6RD":

			break;
	}
	
	return true;
}

function OnSubmit()
{
	
	if(!CheckData()) return;
	
	$H({
		"obj-action" 		: "set",
		"getpage"           : "html/page/portforwdV6.ajax.js",
		"errorpage"         : "html/page/portforwdV6.ajax.js",
		'var:finish'    	: "1",
		"var:menu" 			: "basic",
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		"var:page" 			: "Bas_wansum",
		"var:errorpage" 	: "Bas_wansum",
		"var:CacheLastData" : ViewState.Save()
	}, true);
	var _upath = G_Wanipv6Conn['uPath'];
	var ConnPath = G_Wanipv6Conn['Path'];
	var ConnType;
	var _path_host6 = ":InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.";
	var Ipv4ConnPath = G_WANConn['Path'] + ".";
	var _tunnelpath = ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.1." ; //for tunnel
	var pppoeshare = 0;	// 
	var _protocol = [];
	switch(GetRadioValue("wanv6_mode")){
		case "STATIC" :
		case "AUTO" :
			ConnType = 'IP';
			break;
		case "PPPoE" :
			ConnType = 'PPP';
			break;
		case "6RD" :
			if (G_TunConn['Activated'] == "1" && G_TunConn['Mode'] == "4in6" ) //tunnel enable 
			{
				alert(SEcode['lang_set_tunnel']);
				return false;

			}		
		break;	
		
	}
	if( ConnType == "PPP" && document.getElementsByName("wiz_pppoe_sess_type")[0].checked ==true)	//pppoe share
	{
		if(Ipv4ConnPath.indexOf("PPP") < 0) //not pppoe 
		{
			alert(SEcode["lang_set_pppoev4"]);	
			return false;
		}
		//do something
	} 
	else
	{	
		if(ConnType == 'IP' || ConnType == 'PPP')
		{
			if (typeof(ConnPath) == "undefined") { 
					alert(SEcode["lang_undefined"]); 
					_upath = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.0.";
					ConnPath = _upath + 'WAN' + ConnType + 'Connection.0.';
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
		}

		$F(":" + _upath + "X_TWSZ-COM_VLANID", 	"0"); //vid
		$F(":" + _upath + "X_TWSZ-COM_VLANPriority", 	"0"); // vlanprio
		$F(":" + _upath + "X_TWSZ-COM_CloneMACAddress", 	""); // mac address
		
	}


	switch(GetRadioValue("wanv6_mode"))
	{
		case "STATIC" :
			var _dnservers = $('wiz_static_pridns6').value+','+$('wiz_static_secdns6').value;
			$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
			$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
			$F(":" + ConnPath + "Name",                     "Static_0_Internet");
			$F(":" + ConnPath + "Enable", 			"1");
			$F(":" + ConnPath + "AddressingType", 		"Static");

			if (Form.Checkbox("usell") == 1){ //uselinklocal
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Invalid");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
			}
			else
			{
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"Valid");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"Static");			
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.IPv6Address", 		$('wiz_static_wan_v6addr').value);
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.PrefixLength", 		$('wiz_static_pfxlen').value);
			}

			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.ConfigType", 	"Static");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	"1");  //
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.UsrIPv6DNSServers", 	_dnservers.delcomma());
			
			
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DefaultRouterList.1.DefaultRouter", 	$('wiz_static_gw').value);
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6SitePrefixInfo.ValidLifeTime", 		"172800");
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6SitePrefixInfo.PreferredLifeTime", 	"7200");
	
			$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 	"0"); //

			$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
			$F(":" + ConnPath + "MaxMTUSize", 			"1500");
			
		  break;
		
		case "PPPoE" :		
			if( document.getElementsByName("wiz_pppoe_sess_type")[0].checked == true)	//pppoe share
			{			
				pppoeshare = 1;
				$F(":" + ConnPath + "Enable", 			"0"); // ipv6 conn disable
				$F(":" + Ipv4ConnPath + "Enable", 		"1");			
								
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");			
			
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSOverrideAllowed", 	"0");
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6DNSEnabled", 	"1"); //是否请求dns							
				$F(":" + Ipv4ConnPath + "X_TWSZ-COM_IPv6Config.IPv6PrefixDelegationEnabled", 		"1" );//support pd  即是否需要请求前缀
				
			}
			else
			{
				$F(":" + ConnPath + "Name", 			"PPPoE_0_Internet");
				$F(":" + ConnPath + "X_TWSZ-COM_ProtocolType", 	"IPv6");
				$F(":" + ConnPath + "X_TWSZ-COM_ServiceList", 	"Internet");
				$F(":" + ConnPath + "NATEnabled", 		"1");
				$F(":" + ConnPath + "X_TWSZ-COM_NATType", 	"symmetric");
				$F(":" + ConnPath + "Username", 		$('wiz_pppoe_usr').value); //"" $('wiz_pppoe_usr').value
				var pppoe_passwd = Base64.Encode($('wiz_pppoe_passwd').value);
				$F(":" + ConnPath + "Password", 		$('wiz_pppoe_passwd').value != '**********' ? pppoe_passwd : undefined);//"" 
				$F(":" + ConnPath + "PPPAuthenticationProtocol", 	"Auto");
				$F(":" + ConnPath + "ConnectionTrigger", 		"AlwaysOn");

				//$F(":" + ConnPath + "MaxMRUSize", 		getDight($('ppp6_mtu').value)); //same as mtu"1492"
				//$F(":" + ConnPath + "MaxMTUSize", 		getDight($('ppp6_mtu').value)); //"1492"
				$F(":" + ConnPath + "PPPLCPEcho", 		"10"); //default 
				$F(":" + ConnPath + "PPPLCPEchoRetry", 		"10"); //default
				$F(":" + ConnPath + "PPPoEServiceName", 	$('wiz_pppoe_svc').value); //"tencent2"		
				$F(":" + ConnPath + "X_TWSZ-COM_StaticIPAddress", ""); //Form.Radio('pppoe_dynamic') == "DHCP" ? "" : $('pppoe_ipaddr').value
				//$F(":" + ConnPath + "AddressingType", 		"DHCP");
				$F(":" + ConnPath + "ConnectionType", 		"IP_Routed");
				$F(":" + ConnPath + "Enable", 			"1");
		
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressingType", 		"DHCP");
				$F(":" + ConnPath + "X_TWSZ-COM_IPv6Config.IPv6AddressList.1.AddressStatus", 		"InValid");
								
				
			}

			break;
			
			break;
		case "6RD" :

			$F(":" + ConnPath + "Enable", 		"0");
			//tunnel config 
			$F(_tunnelpath + 'Activated', 				'1');
			$F(_tunnelpath + 'Mode', 				'6in4');
			$F(_tunnelpath + 'TunnelName', 				'tunl6rd');
			$F(_tunnelpath + 'Mechanism', 				'Ipv6RapidDeployment');
			$F(_tunnelpath + 'AssociatedLanIfName', 				'InternetGatewayDevice.LANDevice.1');//目前先锁定
			$F(_tunnelpath + 'AssociatedWanIfName', 				G_WANConn['Path']);//目前先锁定

			{
				$F(_tunnelpath + 'Dynamic', 		'0'); //manual 
				$F(_tunnelpath + 'Prefix', 		$('wiz_6rd_prefix').value+'/'+$('wiz_6rd_pfxlen').value); //manual 
				$F(_tunnelpath + 'IPv4MaskLen', 		$('wiz_6rd_v4addr_mask').value); //dynamic
				$F(_tunnelpath + 'BorderRelayAddress', 		$('wiz_6rd_relay').value); //dynamic
		
				$F(_tunnelpath + 'IPv6DNSServers',  $('wiz_6rd_pridns6').value);								

			}
			break;	
		
		default :
			break;
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

	var _url = "/cgi-bin/webproc";
	var ajax = Ajax.getInstance(_url, "", 0, OnLanSubmit, null_errorfunc);
	ajax.post(document.forms[0]);
	$("content").style.display="none";
	$("mbox").style.display="";	
	$("WIZIPV6077").disabled = true;
}

function OnLanSubmit(_text)
{
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		return;
	}
	G_SysToken = G_AjaxToken;
	if(G_Error == 1)
	{
		$("content").style.display="";
		$("mbox").style.display="none";	
		dealWithError();
		$("WIZIPV6077").disabled = false;
		return;
	}
	$H({
		"obj-action" 		: "set",
		"getpage" 			: "html/index.html",
		"errorpage" 		: "html/index_wiz.html",
		'var:finish'    	: "1",
		'var:sys_Token' : G_SysToken,
		"var:menu" 			: "basic",
		"var:page" 			: "Bas_ipv6_setup",
		"var:errorpage" 	: "Wiz_wan_ipv6",
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
	switch(GetRadioValue("wanv6_mode")){
		case "AUTODETECT" :		
			break;
		case "STATIC" :
		case "AUTO" :
			ConnType = 'IP';
			break;
		case "PPPoE" :
			ConnType = 'PPP';
			break;
		case "6RD" :
			break;
	}

	switch(GetRadioValue("wanv6_mode"))
	{
		case "STATIC" :
			var _dnservers = $('wiz_static_pridns6').value+','+$('wiz_static_secdns6').value;
			if (G_TunConn['Activated'] == "1") //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
			$F( _path_host6 + 'ServerType.StatefullDHCPv6.IPv6DNSServers', 		_dnservers.delcomma());										
			
			if($('wiz_static_lan_v6addr').value == "")   //lan IP 地址，需要根据lan ip生成前缀
			{
				alert(SEcode['lang_lanv6_empty']);
				return false;
			}
			
			if($('wiz_static_lan_v6addr').value != "")
			{
				//这里需要检查ipv6 合法性
				//if(!CheckIPv6Addr($('ipv6_sta_lanip').value)){
					//alertError('WAN_IPv605');
					//return false;
				//}
			}

			var ipv6staip = IPv6FillZero($('wiz_static_lan_v6addr').value);
			var ipv6_lanip = ipv6staip.split(":");		
			var ipv6_addr = ipv6_lanip[0] + ':' + ipv6_lanip[1] + ':' + ipv6_lanip[2] + ':' + ipv6_lanip[3] + '::';

			//LAN Prefix LAN 前缀
			$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.GlobalAddress'			, $('wiz_static_lan_v6addr').value); // lan global address
			$F( _path_host + 'IPInterface.1.X_TWSZ-COM_IPv6InterfaceAddressingType.StaticDelegated'			, "1"); //  address

			$F( _path_host6 + 'IPv6SitePrefixConfigType.StaticDelegated' 				,  "1"); // 
			var _static_prefix = ipv6_addr + '/64';
			$F( _path_host6 + 'RadVDConfigManagement.StaticPrefixInfo.Prefix'			, _static_prefix); // lan prefix 
					
		  break;
		
		case "PPPoE" :
			if (G_TunConn['Activated'] == "1") //tunnel enable 
			{
				if (G_TunConn['Mode'] == "6to4" || G_TunConn['Mode'] == "6in4" )
					$F(_tunnelpath + 'Activated', 				'0'); //避免重复提交关闭
			}
					
			break;	
	
		case "6RD" :
					
			break;
		
		default :
			break;
	}

	
	$('uiPostForm').submit();
	
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
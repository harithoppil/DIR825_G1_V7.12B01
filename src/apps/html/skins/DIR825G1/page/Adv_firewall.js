var LanHosts = [];
var t = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "HostName MACAddress IPAddress LeaseTimeRemaining VendorClassID AddressSource"
		`	LanHosts[t] = [];
			LanHosts[t][0] = t+1;
			LanHosts[t][1] = "<?echo $11?>"=="ZFc1cmJtOTNiZz09"?"unknown":strAnsi2Unicode((Base64.Decode("<?echo $11?>")));
			LanHosts[t][2] = "<?echo $12?>";
			LanHosts[t][3] = "<?echo $13?>";
			LanHosts[t][4] = "<?echo $14?>";
			++t;
		`?>
	`?>
`?>
var G_WANConn=[];
var m=0;
<?objget :InternetGatewayDevice.WANDevice. ""
`
<?objget :InternetGatewayDevice.WANDevice.$10.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<?if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANIPConnection. "Enable X_TWSZ-COM_DMZEnabled X_TWSZ-COM_DMZHost"
		`	G_WANConn[m] = [];
			G_WANConn[m][0] = ":InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANIPConnection.$00.";
			G_WANConn[m][1]="$02";
			G_WANConn[m][2]="$03";
			m++;
		`?>
	`?>
	<?if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANPPPConnection. "Enable X_TWSZ-COM_DMZEnabled X_TWSZ-COM_DMZHost"
		`	G_WANConn[m] = [];
			G_WANConn[m][0] = ":InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANPPPConnection.$00.";
			G_WANConn[m][1]="$02";
			G_WANConn[m][2]="$03";
			m++;
		`?>
	`?>
`?>
`?>
var G_SPIEnable = "<?get :InternetGatewayDevice.X_TWSZ-COM_Firewall.SPIEnable?>";
var G_AntiDosEnable = "<?get :InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDosEnable?>";
<?mget :InternetGatewayDevice.Services.X_TWSZ-COM_ALGAbility. "PPTPEnabled IPSECEnabled RTSPEnabled SIPEnabled"
`	var	G_PPTPEnabled = "<?echo $11?>";
	var G_IPSECEnabled = "<?echo $12?>";
	var G_RTSPEnabled = "<?echo $13?>";
	var G_SIPEnabled = "<?echo $14?>";
`?>
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>	
function OnClickDMZAdd()
{
	if($("hostlist").value == "")
	{
		alert(SEcode['lang_select_machine']);
		return false;
	}
	else
	{
		$("dmzhost").value = $("hostlist").options[$("hostlist").selectedIndex].value;
		/*for(var k = 0; k < LanHosts.length; k++){
			if($("hostlist").value==LanHosts[k][1])
			{
				$("dmzhost").value = LanHosts[k][3];
				break;
			}
		}*/		
	}
}
function OnClickDMZEnable()
{
	if ($("dmzenable").checked)
	{
		$("dmzhost").setAttribute("modified", "false");
		$("dmzhost").disabled = false;
		$("dmzadd").disabled = false;
		$("hostlist").disabled = false;
	}
	else
	{
		$("dmzhost").setAttribute("modified", "ignore");
		$("dmzhost").disabled = true;
		$("dmzadd").disabled = true;
		$("hostlist").disabled = true;
	}
}
function uiOnload()
{
   createHost();
   setJSONValue({
				"spi"		: G_SPIEnable,
				"anti_spoof_enable"		: G_AntiDosEnable,
				"dmzenable"		: G_WANConn[0][1],
				"dmzhost"		: G_WANConn[0][2],
				"pptp"		: G_PPTPEnabled,
				"ipsec"		: G_IPSECEnabled,
				"rtsp"		: G_RTSPEnabled,
				"sip"		: G_SIPEnabled
			});
	OnClickDMZEnable();
}
function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;
		for(var k = 0; k < LanHosts.length; k++){
			array_value[k+1]=LanHosts[k][3];
			array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
		}
		$S('hostlist', array_options, array_value);
}

function uiSubmit()
{	
	if ($("dmzenable").checked)
	{		
		if ($("dmzhost").value == "" || !checkipaddr($("dmzhost").value) || !isSameSubNet($("dmzhost").value,G_SubMask,G_IPAddress,G_SubMask))
		{
			alert(SEcode['lang_dmzip_be_lan']);
			$("dmzhost").focus();
			return false;
		}
		if ($("dmzhost").value == G_IPAddress)
		{
			alert(SEcode['lang_dmzip_invalid']);
			$("dmzhost").focus();
			return false;
		}
	}
	$F(':InternetGatewayDevice.X_TWSZ-COM_Firewall.SPIEnable' , $('spi').checked?'1':'0');
	$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDosEnable', $('anti_spoof_enable').checked?'1':'0');
	if($('anti_spoof_enable').checked)
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.SynCookieEnable' , '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.SynMaxConnections', '50');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiFraggleEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiEchoCargenEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiIPLandEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.IgnrPortScanEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetSynFinEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetSynRstEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetFinRstEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetFinEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetPshEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetUrgEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnsetAllEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetAllEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetSynRstAckFinUrgEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetFinEnable', '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetFinUrgPshEnable', '1');
	}
	else
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.SynCookieEnable' , '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiFraggleEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiEchoCargenEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.AntiIPLandEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiDos.IgnrPortScanEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetSynFinEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetSynRstEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetFinRstEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetFinEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetPshEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnAckSetUrgEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanUnsetAllEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanSetAllEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetSynRstAckFinUrgEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetFinEnable', '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_AntiAttack.AntiScan.AntiScanForAllSetFinUrgPshEnable', '0');
	}

	if($('dmzenable').checked)
	{
		$F(G_WANConn[0][0]+'X_TWSZ-COM_DMZEnabled' , '1');
		$F(G_WANConn[0][0]+'X_TWSZ-COM_DMZHost' , $('dmzhost').value);
	}
	else
	{
		$F(G_WANConn[0][0]+'X_TWSZ-COM_DMZEnabled' , '0');
	}
		

	$F(':InternetGatewayDevice.Services.X_TWSZ-COM_ALGAbility.PPTPEnabled', $('pptp').checked ? '1' : '0');
	$F(':InternetGatewayDevice.Services.X_TWSZ-COM_ALGAbility.IPSECEnabled', $('ipsec').checked ? '1' : '0');
	$F(':InternetGatewayDevice.Services.X_TWSZ-COM_ALGAbility.RTSPEnabled', $('rtsp').checked ? '1' : '0');
	$F(':InternetGatewayDevice.Services.X_TWSZ-COM_ALGAbility.SIPEnabled', $('sip').checked ? '1' : '0');

	$F('obj-action','set');	
	
	$H({
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:errorpage': G_Page,
		'var:CacheLastData': ViewState.Save()
	});
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_fw002').disabled= true;
	$('Adv_fw019').disabled= true;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload);

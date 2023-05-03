var G_UpnpEnable = "<?get :InternetGatewayDevice.X_TWSZ-COM_UPNP.Enable?>";
var G_WanPing = "<?get :InternetGatewayDevice.X_TWSZ-COM_ACL.ACLWanPingEnable?>";
var G_McastEnable = "<?get :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.X_TWSZ-COM_IGMPEnabled?>";
var G_McastEnableV6 = "<?get :InternetGatewayDevice.X_TWSZ-COM_MLD.Proxy.Enable?>";
var G_WanSpeed = "<?get :InternetGatewayDevice.Layer2Bridging.X_TWSZ-COM_WAN_SPEED?>";

var G_WANConn=[];
var m=0;
<?objget :InternetGatewayDevice.WANDevice. ""
`
<?objget :InternetGatewayDevice.WANDevice.$10.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<?if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANIPConnection. "Enable"
		`	G_WANConn[m] = [];
			G_WANConn[m][0] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANIPConnection.$00";
			m++;
		`?>
	`?>
	<?if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANPPPConnection. "Enable"
		`	G_WANConn[m] = [];
			G_WANConn[m][0] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.$20.WANPPPConnection.$00";
			m++;
		`?>
	`?>
`?>
`?>

function uiOnload(){
	setJSONValue({
			"upnp"		: G_UpnpEnable,
			"icmprsp"	: G_WanPing,
			"wanspeed"	: G_WanSpeed,
			"mcast"		: G_McastEnable,
			"mcast6"	: G_McastEnableV6
		});
}

function uiSubmit(){
	
	$F(':InternetGatewayDevice.X_TWSZ-COM_UPNP.Enable', $('upnp').checked? '1':'0');
	if($('upnp').checked)
		$F(':InternetGatewayDevice.X_TWSZ-COM_UPNP.WANPathName', G_WANConn[0][0]);

	$F(':InternetGatewayDevice.X_TWSZ-COM_ACL.ACLWanPingEnable', $('icmprsp').checked? '1':'0');
	$F(':InternetGatewayDevice.Layer2Bridging.X_TWSZ-COM_WAN_SPEED', $('wanspeed').value);
	$F(':InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.X_TWSZ-COM_IGMPEnabled' , $('mcast').checked? '1' : '0');
	$F(':InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.IGMPVersion' , '0'); //Aoto
	$F(':InternetGatewayDevice.X_TWSZ-COM_MLD.Proxy.Enable' , $('mcast6').checked? '1' : '0');
	$F(':InternetGatewayDevice.X_TWSZ-COM_MLD.Snooping.Enable', $('mcast6').checked? '1' : '0');

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
	$('Adv_network002').disabled= true;
	$('Adv_network019').disabled= true;
}

function dealWithError(){
	if(G_Error != 1){
		return false;
	}
	
	var arrayHint = [];	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);


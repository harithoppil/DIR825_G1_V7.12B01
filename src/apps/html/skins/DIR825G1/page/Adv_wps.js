<?mget :InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS. "Enable DevicePassword X_TWSZ-COM_PeerPassword X_TWSZ-COM_GeneratePinEnable X_TWSZ-COM_WpsSessionStatus X_TWSZ-COM_SetupMethod ConfigurationState X_TWSZ-COM_DevicePassword SetupLock"
	   `var W_WPSEnable="$01"; // WPS enable
		var W_WPSDevicePassword="$02";//default PIN
		var W_WPSPeerPassword="$03"; // Client Pin
		var W_WPSGeneratePinEnable="$04";
		var W_WPSWpsSessionStatus="$05";
		var W_WPSSetupMethod="$06";    // 1 for PBC; 2 for PIN
		var W_WPSConfigurationState="$07";//
		var W_WPSDevicePasswordOrg="$08";//
		var W_WPSSetupLock="$09";//
`?>
var W_Enable2="<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable?>";
var W_Enable5="<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable?>";
var G_MacFilterEnable="<?get :InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.Enable?>";
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "X_TWSZ-COM_ScheduleListName SSID SSIDAdvertisementEnabled BeaconType WEPEncryptionLevel WEPKeyIndex BasicAuthenticationMode BasicEncryptionModes WPAAuthenticationMode WPAEncryptionModes IEEE11iAuthenticationMode IEEE11iEncryptionModes X_TWSZ-COM_PSKExpression PreSharedKey.1.KeyPassphrase PreSharedKey.1.PreSharedKey X_TWSZ-COM_WPAGroupRekey WEPKey.1.WEPKey X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey"
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
					 '$0n' //X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey
                     ];
    n++;
`?>
function OnClickEnWPS()
{
	
	if(W_Enable2 == 0 && W_Enable5 == 0)
	{
		$("en_wps").checked 		= false;
		$("en_wps").disabled		= true;
	}
	if ($("en_wps").checked)
	{		
		$("reset_pin").disabled	= false;
		$("gen_pin").disabled		= false;
		$("go_wps").disabled		= false;
		$("lock_wifi_security").disabled		= false;
	}
	else
	{
		$("reset_pin").disabled	= true;
		$("gen_pin").disabled		= true;
		$("go_wps").disabled		= true;
		$("lock_wifi_security").disabled		= true;
	}
}
function OnClickResetPIN()
{
	$("pin").innerHTML = W_WPSDevicePasswordOrg;
}
function OnClickGenPIN()
{
	var pin = "";
	var sum = 0;
	var check_sum = 0;
	var r = 0;
	for(var i=0; i<7; i++)
	{
		r = (Math.floor(Math.random()*9));
		pin += r;
		sum += parseInt(r, [10]) * (((i%2)==0) ? 3:1);
	}
	check_sum = (10-(sum%10))%10;
	pin += check_sum;
	$("pin").innerHTML = pin;
}

function Is_SecuritySupportedByWps(wifi)
{
	var issupported = true;
	
	switch(G_Wireless[wifi][4])
	{
		case "WPA":
		case "Basic":
			issupported = false;
			break;
		case "11i":
		case "WPAand11i":
			if(G_Wireless[wifi][11]=="PSKAuthentication"&&G_Wireless[wifi][12]!="TKIPEncryption")
			issupported = true;
			else
			issupported = false;	
			break;
		default : 
			issupported = true;
			break;
	}
	
	return issupported;
}	

function uiSubmit()
{
	if($("en_wps").checked)
	{
		if((W_Enable2 == 1 && !Is_SecuritySupportedByWps(0)) || (W_Enable5 == 1 && !Is_SecuritySupportedByWps(1)))
		{
			$("en_wps").checked	= false;
			alert(SEcode['lang_wps1']);
			return false;
		}
		if(G_Wireless[0][3]=="0" || G_Wireless[1][3]=="0")
		{
			$("en_wps").checked	= false;
			alert(SEcode['lang_wps2']);
			return false;
		}
		/*if(G_MacFilterEnable=="1")
		{			
			$("en_wps").checked	= false;
			alert(SEcode['lang_wps3']);
			return false;
		}*/
	}
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.Enable',Form.Checkbox('en_wps'));
    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.Enable',Form.Checkbox('en_wps'));
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.SetupLock',Form.Checkbox('lock_wifi_security'));
    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.SetupLock',Form.Checkbox('lock_wifi_security'));
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.DevicePassword',$("pin").innerHTML);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.DevicePassword',$("pin").innerHTML);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.SetupLock',Form.Checkbox('lock_wifi_security'));
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.SetupLock',Form.Checkbox('lock_wifi_security'));
	
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
	$('adv_wps003').disabled= true;
	$('adv_wps005').disabled= true;
}


function uiOnload()
{
	var str="";	
	if(W_WPSEnable=="1")
		str=data_languages.Public.innerHTML.Public001;
	else
		str=data_languages.Public.innerHTML.Public002;
	var str1="";
	if(W_WPSConfigurationState=="Configured")
		str1=data_languages.Public.innerHTML.Public003;
	else
		str1=data_languages.Public.innerHTML.Public004;
	setJSONValue({
			"en_wps"		: W_WPSEnable,
			"wifi_info_str"	: str+"/"+str1,
			"lock_wifi_security"	: W_WPSSetupLock
		});
	$("pin").innerHTML = W_WPSDevicePassword;
	OnClickEnWPS();
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

var stages=["wiz_stage_1", "wiz_stage_2_auto", "wiz_stage_2_manu", "wiz_stage_2_msg"];
var currentStage= 0;	
var en_wps=1;
var start_count_down=0;
var m_prefix=data_languages.Wiz_wps.innerHTML.BWPSPREFIX;
var m_success= data_languages.Wiz_wps.innerHTML.BWPSSUCCEED;
var m_timeout= data_languages.Wiz_wps.innerHTML.BWPSTIMEOUT;
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "X_TWSZ-COM_ScheduleListName SSID SSIDAdvertisementEnabled BeaconType WEPEncryptionLevel WEPKeyIndex BasicAuthenticationMode BasicEncryptionModes WPAAuthenticationMode WPAEncryptionModes IEEE11iAuthenticationMode IEEE11iEncryptionModes X_TWSZ-COM_PSKExpression PreSharedKey.1.KeyPassphrase PreSharedKey.1.PreSharedKey X_TWSZ-COM_WPAGroupRekey WEPKey.1.WEPKey X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey WPS.Enable Enable BSSID"
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
					 '$0n', //X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey
					 '$0o', //WPS.Enable
					 '$0p', //Enable
					 '$0q' //BSSID
                     ];
    n++;
`?>
function ShowCurrentStage()
{

	for (var i=0; i<stages.length; i++)
	{
		if (i==currentStage)
			$(stages[i]).style.display = "block";
		else
			$(stages[i]).style.display = "none";
	}

	if (currentStage==0)
	{
		SetButtonDisabled("b_pre",	true);
		SetButtonDisabled("b_next",	false);
		SetButtonDisabled("b_send",	true);
		SetButtonDisabled("b_stat",	true);
	}
	else if (currentStage==1||currentStage==2)
	{
		if (en_wps)
			SetButtonDisabled("b_send", false);
		SetButtonDisabled("b_pre",	false);
		SetButtonDisabled("b_next",	true);
		SetButtonDisabled("b_stat",	false);
	}
	else
	{
		SetButtonDisabled("b_pre",	true);
		SetButtonDisabled("b_next",	true);
	}
}
function ShowWPSMessage(state)
{

	switch (state)
	{
	case "WPS_NONE":
		$("msg").innerHTML = m_prefix + data_languages.Wiz_wps.innerHTML.BWPSTIMEOUT;
		SetButtonDisabled("b_exit",	false);
		break;
	case "WPS_ERROR":
		$("msg").innerHTML = m_prefix + data_languages.Wiz_wps.innerHTML.BWPSERROR;
		SetButtonDisabled("b_exit",	false);
		break;
	case "WPS_OVERLAP":
		$("msg").innerHTML = m_prefix + data_languages.Wiz_wps.innerHTML.BWPSOVERLAP;
		SetButtonDisabled("b_exit",	false);
		break;
	case "WPS_IN_PROGRESS":
		//SetButtonDisabled("b_exit",	true);
		SetButtonDisabled("b_send",	true);
		SetButtonDisabled("b_stat",	true);
		break;
	case "WPS_SUCCESS":
		$("msg").innerHTML = m_prefix + data_languages.Wiz_wps.innerHTML.BWPSSUCCEED;
		SetButtonDisabled("b_exit",	false);
		SetButtonDisabled("b_stat",	false);
		SetButtonDisplayStyle("b_send",	"none");
		SetButtonDisplayStyle("b_stat",	"inline");
		break;
	}
	currentStage = 3;
	ShowCurrentStage();
}
function OnClickMode()
{
	if ($("auto").checked)
	{
		SetButtonDisplayStyle("b_send",	"inline");
		SetButtonDisplayStyle("b_stat",	"none");
	}
	else
	{
		SetButtonDisplayStyle("b_send",	"none");
		SetButtonDisplayStyle("b_stat",	"inline");
	}
}
function OnClickPINCode()
{
	$("pin").checked = true;
}
function OnClickPre()
{
	currentStage = 0;
	ShowCurrentStage();
}
function OnClickNext()
{
	if ($("auto").checked)
		currentStage = 1;
	else
		currentStage = 2;
	ShowCurrentStage();
}
function OnClickCancel()
{

	/*if (currentStage==3)
	{
		self.location.href = "/cgi-bin/webproc?getpage=html/index_wiz.html&var:menu=basic&var:page=Wiz_wps";
	}*/
	if (confirm(SEcode["lang_cancel_setting"]))
	{
		self.location.href = "/cgi-bin/webproc?getpage=html/index.html&var:menu=advanced&var:page=Adv_wps";
	}
	if (currentStage==3)
	{
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_SetupMethod','Stop');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_StartWpsSession','0');

	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_SetupMethod','Stop');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_StartWpsSession','0');

		$H({
	    'getpage' : 'html/index_wiz.html',
		'var:page'     : 'Wiz_wps',
		'var:menu': 'advanced',
		'var:sys_Token' : G_SysToken,
		'obj-action' : 'set',
		'var:CacheLastData': ViewState.Save()
		},true);	
		$('uiPostForm').submit();
	}
}

var time = 120;
function WPSInProgress(_text)
{	
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		return;
	}
	G_SysToken = G_AjaxToken;
	var str = "";
	$("BWPS039").disabled=false;
	if ($("pin").checked)
	{
		str = data_languages.Wiz_wps.innerHTML.BWPSPINMSG;
	}
	else
	{
		str = data_languages.Wiz_wps.innerHTML.BWPSPBCMSG;
	}
	str += data_languages.Wiz_wps.innerHTML.BWPSTIME;
	str += m_prefix + data_languages.Wiz_wps.innerHTML.BWPSSTART;
	$("msg").innerHTML = str;	
	$("ct").innerHTML = time;
	if (time > 0)
	{
		time--;		
		if(G_WPS_StartWPSSession[0]=='3'|| G_WPS_StartWPSSession[1]=='3')
			ShowWPSMessage("WPS_SUCCESS");
		else if(G_WPS_StartWPSSession[0]=='1' || G_WPS_StartWPSSession[1]=='1')
			ShowWPSMessage("WPS_OVERLAP");
		else if(G_WPS_StartWPSSession[0]=='0' || G_WPS_StartWPSSession[1]=='0')
		{
			ShowWPSMessage("WPS_IN_PROGRESS");	
			setTimeout('ajaxGetWpsStatus()',900);
		}
		
		else
			ShowWPSMessage("WPS_NONE");
	}
	else
		ShowWPSMessage("WPS_NONE");	
}
function ShowWpsDisabled()
{
	for (var i=0; i<stages.length; i++)
	{
		$(stages[i]).style.display = "none";
	}
	$("wiz_stage_wps_disabled").style.display = "block";
}
function Initial(freq)
{		
	if(freq == "1") 		str_Aband = "_Aband";
	else					str_Aband = "";
	if(G_Wireless[freq][24]=="0")
	{
		ShowWpsDisabled();
		return true;
	}
	
	var en_wlan = 1;
	
	if(str_Aband != "")
		$("frequency"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSFREQ5;
	else 
		$("frequency"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSFREQ2;	
	
	var ssid = G_Wireless[freq][2];
	if(typeof($("ssid"+str_Aband).innerText) !== "undefined") $("ssid"+str_Aband).innerText = ssid;
	else if(typeof($("ssid"+str_Aband).textContent) !== "undefined") $("ssid"+str_Aband).textContent = ssid;
	else $("ssid"+str_Aband).innerHTML = ssid;
	switch (G_Wireless[freq][4])
	{
	case "WPAand11i":		
		$("st_pskkey"+str_Aband).style.display = "block";					
		$("st_cipher"+str_Aband).style.display = "block";
		if(G_Wireless[freq][11]=="PSKAuthentication")
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSAUTOPER;	
			if(G_Wireless[freq][13]=="KeyPassphrase")
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][14]);
			else
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][15]);
		}
		else
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSAUTOENT;
			$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][20]);
		}
		if(G_Wireless[freq][12]=="TKIPEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIP;
		else if(G_Wireless[freq][12]=="AESEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSAES;
		else
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIPAES;
		break;	
	case "11i":		
		$("st_pskkey"+str_Aband).style.display = "block";				
		$("st_cipher"+str_Aband).style.display = "block";
		if(G_Wireless[freq][11]=="PSKAuthentication")
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWPA2PSK;
			if(G_Wireless[freq][13]=="KeyPassphrase")
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][14]);
			else
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][15]);
		}
		else
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWPA2EAP;	
			$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][20]);
		}
		if(G_Wireless[freq][12]=="TKIPEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIP;
		else if(G_Wireless[freq][12]=="AESEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSAES;
		else
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIPAES;
		break;
	case "WPA":		
		$("st_pskkey"+str_Aband).style.display = "block";
		$("st_cipher"+str_Aband).style.display = "block";
		if(G_Wireless[freq][9]=="PSKAuthentication"){
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWPA2PSK;	
			if(G_Wireless[freq][13]=="KeyPassphrase")
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][14]);
			else
				$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][15]);
		}
		else
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWPA2EAP;		
			$("pskkey"+str_Aband).innerHTML = Base64.Decode(G_Wireless[freq][20]);
		}
		if(G_Wireless[freq][10]=="TKIPEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIP;
		else if(G_Wireless[freq][10]=="AESEncryption")
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSAES;
		else
		$("cipher"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSTKIPAES;
		break;
	case "Basic":		
		var key_no = 1;
		if(G_Wireless[freq][7]=="SharedAuthentication")
		{
			$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWEPAHERED;
			if(Radio_list[i][1]=="1")
			DisableWPS();
		}
		else
		$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSWEPAUTO;
		$("wepkey"+str_Aband).innerHTML = key_no + ": " +Base64.Decode(G_Wireless[freq][17]);
		$("st_wep"+str_Aband).style.display = "block";		
		break;
	case "None":
		$("security"+str_Aband).innerHTML = data_languages.Wiz_wps.innerHTML.BWPSNONE;
		break;
	}
	return true;
}
//加载初始势
function uiOnload(){
	OnClickMode();
	ShowCurrentStage();	
	Initial(0);
	Initial(1);
}
function SetButtonDisabled(name, isDisable)
{
	var button = document.getElementsByName(name);
	for (i=0; i<button.length; i++)
		button[i].disabled = isDisable;
}
function SetButtonDisplayStyle(name, style)
{
	var button = document.getElementsByName(name);
	for (i=0; i<button.length; i++)
		button[i].style.display = style;
}
function checkPIN( pin ) 
{
	var accum = 0;
    var pinNum = Number(pin);
    
    if(pinNum == NaN)
    	return false;

	if(pin.toString().length == 4)
    	return true;	
			
	accum += 3 * (Math.floor(pinNum / 10000000) % 10); 
	accum += 1 * (Math.floor(pinNum / 1000000) % 10); 
	accum += 3 * (Math.floor(pinNum / 100000) % 10); 
	accum += 1 * (Math.floor(pinNum / 10000) % 10); 
	accum += 3 * (Math.floor(pinNum / 1000) % 10); 
	accum += 1 * (Math.floor(pinNum / 100) % 10); 
	accum += 3 * (Math.floor(pinNum / 10) % 10); 
	accum += 1 * (Math.floor(pinNum / 1) % 10); 
	
    return (0 == (accum % 10));
}
function OnSubmit(){	

	if ($("pin").checked)
	{
		var devicePIN = $('pincode').value;
		if(devicePIN == '')
		{
			alert(SEcode['lang_invalid_pin'])
			return false;
		}
		else if(devicePIN.match(/^([0-9]{4}|[0-9]{8})$/) == null)
		{
			alert(SEcode['lang_invalid_pin'])
			return false;
		}
		else if(!checkPIN(devicePIN))
		{
			alert(SEcode['lang_invalid_pin'])
			return false;
		}
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_SetupMethod','StationPIN');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_StartWpsSession','0');
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_PeerPassword',$('pincode').value);

		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_SetupMethod','StationPIN');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_StartWpsSession','0');
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_PeerPassword',$('pincode').value);
	}
	else
	{

	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_SetupMethod','PBC');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPS.X_TWSZ-COM_StartWpsSession','0');

	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_SetupMethod','PBC');
	    $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPS.X_TWSZ-COM_StartWpsSession','0');

	}
	 $H({
        'getpage' : 'html/page/wps.ajax.js',
		'var:page'     : 'Wiz_wps',
		'var:errorpage': 'Wiz_wps',
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		'obj-action' : 'set'
	});
	var _url = "/cgi-bin/webproc";
	ajax = Ajax.getInstance(_url, "", 0, WPSInProgress,null_errorfunc);
	ajax.post(document.forms[0]);
	$("BWPS039").disabled = true;
}
function ajaxGetWpsStatus()
{
	//注意这个地方的page字段很重要，否则获取wps_setup.ajax.js会认证失败。
	var _url = "/cgi-bin/webproc?getpage=html/page/wps.ajax.js&var:page=*";
	ajax = Ajax.getInstance(_url, "", 0, WPSInProgress,null_errorfunc);
	ajax.get();
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
//监听加载与错误处理函擿
addListeners(uiOnload, dealWithError);

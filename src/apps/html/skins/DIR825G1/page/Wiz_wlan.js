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
var stages = ["wiz_stage_1", "wiz_stage_2", "wiz_stage_3"];
var currentStage = 0;
var dual_band = 1;
//加载初始势
function uiOnload(){
	$('wiz_ssid').value = G_Wireless[0][2];
	$('wiz_ssid_Aband').value = G_Wireless[1][2];	
	ShowCurrentStage();
}
function getProtocol()
{
	switch(Form.Radio("wan_ip_mode")){
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

function SetButtonDisabled(name, disable)
{
	var button = document.getElementsByName(name);
	for (i=0; i<button.length; i++)
		button[i].disabled = disable;
}

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
		SetButtonDisabled("b_pre", true);
	else
		SetButtonDisabled("b_pre", false);

	if (currentStage==stages.length-1)
	{
		SetButtonDisabled("b_next", true);
		SetButtonDisabled("b_send", false);
		//$("mainform").setAttribute("modified", "true");
		UpdateCFG();
	}
	else
	{
		SetButtonDisabled("b_next", false);
		SetButtonDisabled("b_send", true);
	}
}

function UpdateCFG()
{	
	for(i=0;i<2;i++)
	{
		if(i==0)	var str_Aband = "";
		else		var str_Aband = "_Aband";
		
		var ssid = $("wiz_ssid"+str_Aband).value;
		
		if(typeof($("ssid"+str_Aband).innerText) !== "undefined") $("ssid"+str_Aband).innerText = ssid;
		else if(typeof($("ssid"+str_Aband).textContent) !== "undefined") $("ssid"+str_Aband).textContent = ssid;
		else $("ssid"+str_Aband).innerHTML = ssid;
						
		if ($("autokey").checked)
		{
			$("s_key"+str_Aband).style.display = "none";
			$("l_key"+str_Aband).style.display = "block";
			$("l_key"+str_Aband).innerHTML = RandomHex(10);
		}
		else if ($("wiz_key"+str_Aband).value.length > 50)
		{
			$("s_key"+str_Aband).style.display = "none";
			$("l_key"+str_Aband).style.display = "block";
			$("l_key"+str_Aband).innerHTML = COMM_AddBR2Str($("wiz_key"+str_Aband).value,32);
		}
		else
		{
			$("l_key"+str_Aband).style.display = "none";
			$("s_key"+str_Aband).style.display = "block";
			$("s_key"+str_Aband).innerHTML = $("wiz_key"+str_Aband).value;
		}
	}
}
function COMM_AddBR2Str(str,len)
{
	var tmp = "";
	for(var i=0; i < str.length; i++)
	{
		if(i!=0 && (i%len)==0)
		{
			tmp+="<br \>";
		}
		tmp+=str.charAt(i);
	}
	return tmp;
}
function RandomHex(len)
{
	var c = "0123456789abcdef";
	var str = '';
	for (var i = 0; i < len; i+=1)
	{
		var rand_char = Math.floor(Math.random() * c.length);
		str += c.substring(rand_char, rand_char + 1);
	}
	return str;
}
function SetStage(offset)
{
	var length = stages.length;
	currentStage += offset;
}

function checkSpace(id) {
	if (id.indexOf(" ",0) == 0 || id.lastIndexOf(" ") == id.length -1) {
		return true;
	}
	else
		return false;
} 
function OnClickPre()
{
	switch (currentStage)
	{
	case 2:
		if ($("autokey").checked)
			SetStage(-2);
		else
			SetStage(-1);
		ShowCurrentStage();
		break;
	default:
		SetStage(-1);
		ShowCurrentStage();
	}
}
function set_5g_security(value)
{
    if (value) {
		$("WIZWLAN016").innerHTML = data_languages.Wiz_wlan.innerHTML.WIZWLAN016;
		$("wl_sec_Aband_div").style.display = "none";
    } else {
    	$("wl_sec_Aband_div").style.display = "block";
	   	$("WIZWLAN016").innerHTML = "2.4Ghz"+data_languages.Wiz_wlan.innerHTML.WIZWLAN016;
    	$("WIZWLAN017").innerHTML = "5Ghz"+data_languages.Wiz_wlan.innerHTML.WIZWLAN016;
    }
}
function OnClickNext()
{
	switch (currentStage)
	{
	case 0:
		if($("wiz_ssid").value.charAt(0)==" "|| $("wiz_ssid").value.charAt($("wiz_ssid").value.length-1)==" ")
		{
			alert(SEcode["lang_prefix_blank"]);
			return ;
		}
		if(dual_band && ($("wiz_ssid_Aband").value.charAt(0)==" "|| $("wiz_ssid_Aband").value.charAt($("wiz_ssid_Aband").value.length-1)==" "))
		{
			alert(SEcode["lang_prefix_blank"]);
			return ;
		}			
		if ($("wiz_ssid").value=="")
		{
			alert(SEcode["lang_ssid_empty"]);
			return;
		}
		if ($("wiz_ssid").value.match(/[^\x00-\xff]/))//汉字和中文字符
		{
			alert(SEcode["lang_ssid_printable"]);
			return;
		}	
		if (dual_band && $("wiz_ssid_Aband").value=="")
		{
			alert(SEcode["lang_ssid_empty"]);
			return;
		}	

		if (dual_band && $("wiz_ssid_Aband").value.match(/[^\x00-\xff]/))
		{
			alert(SEcode["lang_ssid_printable"]);
			return;
		}	
		if ($("autokey").checked)
			SetStage(1);
		break;
	case 1:
		if($("set_5g_security_id").checked)	$("wiz_key_Aband").value = $("wiz_key").value;
		
		if ($("wiz_key").value.length < 8 || $("wiz_key_Aband").value.length < 8)
		{
			alert(SEcode["lang_psk_printable"]);
			return;
		}
		if ( ($("wiz_key").value.length == 64 && $("wiz_key").value.match(/[^0-9a-fA-F]/)) || ($("wiz_key_Aband").value.length == 64 && $("wiz_key_Aband").value.match(/[^0-9a-fA-F]/)) )
		{
			alert(SEcode["lang_psk_64hex"]);
			return;
		}

		if ((checkSpace($("wiz_key").value) == true) ||(checkSpace($("wiz_key_Aband").value) == true))
		{
			alert(SEcode["lang_passwd_blank"]);
			return;
		}			
		break;
	default:
	}
	SetStage(1);
	ShowCurrentStage();
}
	
	
function OnClickCancel()
{
	if (confirm(SEcode["lang_cancel_setting"]))
		self.location.href = "/cgi-bin/webproc?getpage=html/index.html&var:menu=basic&var:page=Bas_wlan";
}
function OnSubmit(){
	$H({
		"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index_wiz.html",
		"var:menu" 		: "basic",
		"var:page" 		: "Bas_wlansum",
		'var:sys_Token' : G_SysToken,
		"var:errorpage" 	: "Wiz_wlan"
	}, true);
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable',  1);
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable',  1);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSID',                        $('wiz_ssid').value);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.SSID',                        $('wiz_ssid_Aband').value);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.BeaconType', 					'WPAand11i');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.WPAAuthenticationMode', 		'PSKAuthentication');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.BeaconType', 					'WPAand11i');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.WPAAuthenticationMode', 		'PSKAuthentication');
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');	
	if(Form.Radio('key')=="0"){
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('l_key').innerHTML));
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');	
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('l_key_Aband').innerHTML));
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
	}
	else{
		if(Form.Checkbox('set_5g_security_id')){
			if($('wiz_key').value.length < 64){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
			} else {
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
			}
		}
		else{
			if($('wiz_key').value.length < 64){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1'  + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1'  + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
			} else {
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wiz_key').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1' + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
			}
			if($('wiz_key_Aband').value.length < 64){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2'  + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wiz_key_Aband').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2'  + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
			} else {
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wiz_key_Aband').value));
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2' + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
			}
		}
	}		
	$('uiPostForm').submit();
	$("content").style.display="none";
	$("mbox").style.display="";	
	$("WIZWLAN048").disabled = true;
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

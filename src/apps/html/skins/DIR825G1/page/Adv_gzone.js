var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>
var sec_type = "";//加密模式
var sec_type_Aband = "";//5G加密模式
var Enabled_5G = 1;//是否启用5G
var radius_adv_flag = 0;
var radius_adv_flag_Aband = 0;
var box_wpa_enterprise_Aband = 0;
var wps = false;//是否正在进行wps
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "X_TWSZ-COM_ScheduleListName SSID SSIDAdvertisementEnabled BeaconType WEPEncryptionLevel WEPKeyIndex BasicAuthenticationMode BasicEncryptionModes WPAAuthenticationMode WPAEncryptionModes IEEE11iAuthenticationMode IEEE11iEncryptionModes X_TWSZ-COM_PSKExpression PreSharedKey.1.KeyPassphrase PreSharedKey.1.PreSharedKey X_TWSZ-COM_WPAGroupRekey WEPKey.1.WEPKey X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey Enable X_TWSZ-COM_BakRadiusEnable"
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
					 '$0o', //Enable
					 '$0p' //X_TWSZ-COM_BakRadiusEnable
                     ];
    n++;
`?>
var Radio_list=[];
var m=0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_Radio. "Enable Channel AutoChannelEnable Standard  OperatingChannelBandwidth ChannelsInUse"
`	Radio_list[m]=[];
	Radio_list[m][0] 			 = "InternetGatewayDevice.X_TWSZ-COM_Radio.$00.";//path
	Radio_list[m][1]             = "$01";//Enable
	Radio_list[m][2]             = "$02";//Channel
	Radio_list[m][3]             = "$03";//AutoChannelEnable
	Radio_list[m][4]             = "$04";//Standard
	Radio_list[m][5]	         = "$05";//OperatingChannelBandwidth
	Radio_list[m][6]	         = "$06";//ChannelsInUse
	m++;
`?>

var G_APIsolate = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.1.X_TWSZ-COM_APIsolate?>";

var en_Wlan = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable?>";
var en_Wlan_5G = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable?>";
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
function SetDisplayStyle(tag, name, style)
{
	if (tag)
		var obj = GetElementsByName_iefix(tag, name);
	else		
		var obj = document.getElementsByName(name);
	for (var i=0; i<obj.length; i++)
	{
		obj[i].style.display = style;
	}
}

function OnClickRadiusAdvanced(str_Aband)
{
	if(str_Aband=="")
	{
		if (radius_adv_flag) {
			$("div_second_radius"+str_Aband).style.display = "none";
			$("radius_adv"+str_Aband).value = data_languages.Adv_gzone.value.radius_adv;
			radius_adv_flag = 0;
		}
		else {
			$("div_second_radius"+str_Aband).style.display = "block";
			$("radius_adv"+str_Aband).value = data_languages.Adv_gzone.value.radius_adv1;
			radius_adv_flag = 1;
		}
	}
	else
	{
		if (radius_adv_flag_Aband) {
			$("div_second_radius"+str_Aband).style.display = "none";
			$("radius_adv"+str_Aband).value = data_languages.Adv_gzone.value.radius_adv_Aband;
			radius_adv_flag_Aband = 0;
		}
		else {
			$("div_second_radius"+str_Aband).style.display = "block";
			$("radius_adv"+str_Aband).value = data_languages.Adv_gzone.value.radius_adv_Aband1;
			radius_adv_flag_Aband = 1;
		}
	}
}
function OnChangeWEPAuth(str_Aband)
{
	if($("auth_type"+str_Aband).value == "SHARED" && wps==true)
	{
		alert("Can\'t choose shared key when wps is enable !!");
		$("auth_type"+str_Aband).value = "WEPAUTO";
	}
}
function OnChangeWEPKey(str_Aband)
{
	var no = parseInt($("wep_def_key"+str_Aband).value,10) - 1;
	
	switch ($("wep_key_len"+str_Aband).value)
	{
		case "40-bit":
			$("wep_64"+str_Aband).style.display = "block";
			$("wep_128"+str_Aband).style.display = "none";
			SetDisplayStyle(null, "wepkey_64"+str_Aband, "none");
			document.getElementsByName("wepkey_64"+str_Aband)[no].style.display = "inline";
			break;
		case "104-bit":
			$("wep_64"+str_Aband).style.display = "none";
			$("wep_128"+str_Aband).style.display = "block";
			SetDisplayStyle(null, "wepkey_128"+str_Aband, "none");
			document.getElementsByName("wepkey_128"+str_Aband)[no].style.display = "inline";
			break;
	}
}
function OnChangeWPAMode(str_Aband)
{
	switch ($("wpa_mode"+str_Aband).value)
	{
		case "WPA":
			$("cipher_type"+str_Aband).value = "TKIP";
			break;
		case "WPA2":
			$("cipher_type"+str_Aband).value = "AES";
			break;	
		default :
			$("cipher_type"+str_Aband).value = "TKIP+AES";
	}
}
function OnChangeSecurityType(str_Aband)
{
	switch ($("security_type"+str_Aband).value)
	{
		case "none":
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			break;
		case "wep":
			$("wep"+str_Aband).style.display = "block";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";				
			break;
		case "wpa_personal":
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "block";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			break;
		case "wpa_enterprise":
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "block";

			if(str_Aband == "")
            {
			    $("radius_srv_port"+str_Aband).value = G_Wireless[2][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[2][22];
            }
			else
			{
			    $("radius_srv_port"+str_Aband).value = G_Wireless[3][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[3][22];
            }
			break;
	}
	
}
function OnClickEnGzone(str_Aband)
{
	if ($("en_gzone"+str_Aband).checked)
	{
		$("sch_gz"+str_Aband).disabled			= false;
		$("go2sch_gz"+str_Aband).disabled	= false;
		$("ssid"+str_Aband).disabled			= false;
		$("security_type"+str_Aband).disabled	= false;
		$("auth_type"+str_Aband).disabled			= false;
		$("wep_key_len"+str_Aband).disabled	= false;
		$("wep_def_key"+str_Aband).disabled	= false;
		$("wep_64_1"+str_Aband).disabled			= false;
		$("wep_128_1"+str_Aband).disabled	= false;
		$("wpa_mode"+str_Aband).disabled			= false;
		$("cipher_type"+str_Aband).disabled	= false;
		$("wpa_grp_key_intrv"+str_Aband).disabled			= false;
		$("wpa_psk_key"+str_Aband).disabled	= false;
		$("radius_srv_ip"+str_Aband).disabled			= false;
		$("radius_srv_port"+str_Aband).disabled	= false;
		$("radius_srv_sec"+str_Aband).disabled			= false;
		$("radius_adv"+str_Aband).disabled	= false;
		$("radius_srv_ip_second"+str_Aband).disabled			= false;
		$("radius_srv_port_second"+str_Aband).disabled	= false;
		$("radius_srv_sec_second"+str_Aband).disabled			= false;
		$("security_type"+str_Aband).value	= str_Aband==""?sec_type:sec_type_Aband;
	}
	else
	{
		$("sch_gz"+str_Aband).disabled			= true;
		$("go2sch_gz"+str_Aband).disabled	= true;
		$("ssid"+str_Aband).disabled			= true;
		$("security_type"+str_Aband).disabled	= true;	
		$("auth_type"+str_Aband).disabled			= true;
		$("wep_key_len"+str_Aband).disabled	= true;
		$("wep_def_key"+str_Aband).disabled	= true;
		$("wep_64_1"+str_Aband).disabled			= true;
		$("wep_128_1"+str_Aband).disabled	= true;
		$("wpa_mode"+str_Aband).disabled			= true;
		$("cipher_type"+str_Aband).disabled	= true;
		$("wpa_grp_key_intrv"+str_Aband).disabled			= true;
		$("wpa_psk_key"+str_Aband).disabled	= true;
		$("radius_srv_ip"+str_Aband).disabled			= true;
		$("radius_srv_port"+str_Aband).disabled	= true;
		$("radius_srv_sec"+str_Aband).disabled			= true;
		$("radius_adv"+str_Aband).disabled	= true;
		$("radius_srv_ip_second"+str_Aband).disabled			= true;
		$("radius_srv_port_second"+str_Aband).disabled	= true;
		$("radius_srv_sec_second"+str_Aband).disabled			= true;
		$("security_type"+str_Aband).value	= "none";		
	}
	OnChangeSecurityType(str_Aband);		
	
					
	if($("en_gzone_Aband").checked == false && $("en_gzone").checked == false)
	{								
		$("en_routing_dualband").checked	= false;
		$("en_routing_dualband").disabled	= true;
	}
	else
	{												
		$("en_routing_dualband").disabled     = false;
	}
	
				
}
function DrawSecurityList(wlan_mode, str_Aband)
{
	var security_list = null;
	var cipher_list = null;
	if (wlan_mode == "n" || wlan_mode == "5n" || wlan_mode == "ac" || wlan_mode == "anac")
	{
		security_list = ['wpa_personal', data_languages.Adv_gzone.innerHTML.Adv_gzone016,
						'wpa_enterprise', data_languages.Adv_gzone.innerHTML.Adv_gzone017];
		cipher_list = ['AES'];
	}
	else
	{
		security_list = ['wep', data_languages.Adv_gzone.innerHTML.Adv_gzone015,
						 'wpa_personal', data_languages.Adv_gzone.innerHTML.Adv_gzone016,
						 'wpa_enterprise', data_languages.Adv_gzone.innerHTML.Adv_gzone017];
		cipher_list = ['TKIP+AES','TKIP','AES'];
	}
	//modify security_type
	var sec_length = $("security_type"+str_Aband).length;
	for(var idx=1; idx<sec_length; idx++)
	{
		$("security_type"+str_Aband).remove(1);
	}
	for(var idx=0; idx<security_list.length; idx++)
	{
		var item = document.createElement("option");
		item.value = security_list[idx++];
		item.text = security_list[idx];
		try		{ $("security_type"+str_Aband).add(item, null); }
		catch(e){ $("security_type"+str_Aband).add(item); }
	}
	// modify cipher_type
	var ci_length = $("cipher_type"+str_Aband).length;
	for(var idx=0; idx<ci_length; idx++)
	{
		$("cipher_type"+str_Aband).remove(0);
	}
	for(var idx=0; idx<cipher_list.length; idx++)
	{
		var item = document.createElement("option");
		item.value = cipher_list[idx];
		//if (item.value=="TKIP+AES") item.text = "TKIP and AES";
		if (item.value=="TKIP+AES") item.text = data_languages.Adv_gzone.innerHTML.Adv_gzone039;
		else						item.text = cipher_list[idx];
		try		{ $("cipher_type"+str_Aband).add(item, null); }
		catch(e){ $("cipher_type"+str_Aband).add(item); }
	}
}
var sec_type_Aband = "";
var sec_type = "";
function InitValue()
{	
	$("div_route_zone").style.display = "none";			
	for(var i=2; i<4; i+=1)
	{
		var str_Aband=(i==2)?"":"_Aband";
		$("en_gzone"+str_Aband).checked = (G_Wireless[i][24]=="1")?true:false;
		// OnClickEnGzone(str_Aband);
		$('ssid'+str_Aband).value = G_Wireless[i][2];	
		$('sch_gz'+str_Aband).value = G_Wireless[i][1];
		DrawSecurityList(Radio_list[i-2][4], str_Aband)
		
		switch(G_Wireless[i][4]){
			case 'None' : {
				$("security_type"+str_Aband).value = "none";
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];
				if(str_Aband == "")
					sec_type = "none";
				else
					sec_type_Aband = "none";
				break;
			}
			case 'Basic' : {
				$("security_type"+str_Aband).value = "wep";
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];
				$("auth_type"+str_Aband).value = G_Wireless[i][7];
				$("wep_key_len"+str_Aband).value = G_Wireless[i][5];
				OnChangeWEPKey(str_Aband);
				if(G_Wireless[i][5]=="40-bit")
					$("wep_64_1"+str_Aband).value = Base64.Decode(G_Wireless[i][17]);
				else
					$("wep_128_1"+str_Aband).value = Base64.Decode(G_Wireless[i][17]);
				if(str_Aband == "")
					sec_type = "wep";
				else
					sec_type_Aband = "wep";
				break;
			}
			case 'WPA' : {
				$("wpa_mode"+str_Aband).value = "WPA";
				if(G_Wireless[i][10]=="TKIPEncryption")
				$("cipher_type"+str_Aband).value = "TKIP";
				else if(G_Wireless[i][10]=="AESEncryption")
				$("cipher_type"+str_Aband).value = "AES";
				else
				$("cipher_type"+str_Aband).value = "TKIP+AES";
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];
				if(G_Wireless[i][9]=="PSKAuthentication")
				{
					if(str_Aband == "")
						sec_type = "wpa_personal";
					else
						sec_type_Aband = "wpa_personal";
					$("security_type"+str_Aband).value = "wpa_personal";
				}
				else
				{
					if(str_Aband == "")
						sec_type = "wpa_enterprise";
					else
						sec_type_Aband = "wpa_enterprise";
					$("security_type"+str_Aband).value = "wpa_enterprise";
				}
				break;
			}
			case '11i' : {
				$("wpa_mode"+str_Aband).value = "WPA2";
				if(G_Wireless[i][12]=="TKIPEncryption")
				$("cipher_type"+str_Aband).value = "TKIP";
				else if(G_Wireless[i][12]=="AESEncryption")
				$("cipher_type"+str_Aband).value = "AES";
				else
				$("cipher_type"+str_Aband).value = "TKIP+AES";
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];
				if(G_Wireless[i][11]=="PSKAuthentication")
				{
					if(str_Aband == "")
						sec_type = "wpa_personal";
					else
						sec_type_Aband = "wpa_personal";
					$("security_type"+str_Aband).value = "wpa_personal";
				}
				else
				{
					if(str_Aband == "")
						sec_type = "wpa_enterprise";
					else
						sec_type_Aband = "wpa_enterprise";
					$("security_type"+str_Aband).value = "wpa_enterprise";
				}
				break;
			}
			case 'WPAand11i' : {
				$("wpa_mode"+str_Aband).value = "WPA+2";
				if(G_Wireless[i][12]=="TKIPEncryption")
				$("cipher_type"+str_Aband).value = "TKIP";
				else if(G_Wireless[i][12]=="AESEncryption")
				$("cipher_type"+str_Aband).value = "AES";
				else
				$("cipher_type"+str_Aband).value = "TKIP+AES";
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];				
				if(G_Wireless[i][11]=="PSKAuthentication")
				{
					if(str_Aband == "")
						sec_type = "wpa_personal";
					else
						sec_type_Aband = "wpa_personal";
					$("security_type"+str_Aband).value = "wpa_personal";
				}
				else
				{
					if(str_Aband == "")
						sec_type = "wpa_enterprise";
					else
						sec_type_Aband = "wpa_enterprise";
					$("security_type"+str_Aband).value = "wpa_enterprise";
				}			
				break;
			}  
		}
		if($("security_type"+str_Aband).value == "wpa_personal")
		{
			if(G_Wireless[i][13]=="KeyPassphrase")
				$("wpa_psk_key"+str_Aband).value = Base64.Decode(G_Wireless[i][14]);
			else
				$("wpa_psk_key"+str_Aband).value = Base64.Decode(G_Wireless[i][15]);			
		}
		else
		{
			$("radius_srv_ip"+str_Aband).value = G_Wireless[i][18];
			$("radius_srv_port"+str_Aband).value = G_Wireless[i][19];
			$("radius_srv_sec"+str_Aband).value = Base64.Decode(G_Wireless[i][20]);
			if('1' == G_Wireless[i][25])
			{
				$("radius_srv_ip_second"+str_Aband).value = G_Wireless[i][21];
				$("radius_srv_port_second"+str_Aband).value = G_Wireless[i][22];
				$("radius_srv_sec_second"+str_Aband).value = Base64.Decode(G_Wireless[i][23]);	
			}
			else
			{
				//$("radius_srv_ip_second"+str_Aband).value = G_Wireless[i][21];
				if("" == $("radius_srv_port_second"+str_Aband).value)
					$("radius_srv_port_second"+str_Aband).value='1812';
			    else
				    $("radius_srv_port_second"+str_Aband).value = G_Wireless[i][22];
				//$("radius_srv_sec_second"+str_Aband).value = Base64.Decode(G_Wireless[i][23]);	
			}
		}
		//OnChangeSecurityType(str_Aband);
		OnClickEnGzone(str_Aband);
	}
}

function uiOnload(){	
	createSchedule();
	InitValue();
	if('1' == G_APIsolate)
	    $("en_routing_dualband").checked	= false;
	else
	{
		
		if(G_Wireless[2][24]=="0" && G_Wireless[3][24]=="0")
			$("en_routing_dualband").checked	= false;		
		else 
			$("en_routing_dualband").checked	= true;		
	}
	if(en_Wlan=="0")
	{
		$("en_gzone").disabled = true;
		$("sch_gz").disabled = true;
		$("go2sch_gz").disabled = true;
		$("ssid").disabled = true;
		$("security_type").disabled = true;
		$("security_type").value = "none";
		OnChangeSecurityType("");
	}
	if(en_Wlan_5G=="0")
	{
		$("en_gzone_Aband").disabled = true;
		$("sch_gz_Aband").disabled = true;
		$("go2sch_gz_Aband").disabled = true;
		$("ssid_Aband").disabled = true;
		$("security_type_Aband").disabled = true;
		$("security_type_Aband").value = "none";
		OnChangeSecurityType("_Aband");
	}
	if(en_Wlan=="0" && en_Wlan_5G=="0")
	{
		$("en_routing_dualband").disabled = true;
	}
} 
function createSchedule(){
		var array_value = [],array_options=[];
		array_value[0]="Always";
		array_options[0]=data_languages.Public.innerHTML.Public005;		
		for(var k = 0; k < schedule_list.length; k++){
			array_value[k+1]=schedule_list[k][0];
			array_options[k+1]=schedule_list[k][0];
		}
		$S('sch_gz', array_options, array_value);
		$S('sch_gz_Aband', array_options, array_value);
}
function checkSpace(id) {
	if (id.indexOf(" ",0) == 0 || id.lastIndexOf(" ") == id.length -1) {
		return true;
	}
	else
		return false;
} 
function ValidityCheck(str_Aband)
{	
	
	var obj_ssid 	= $("ssid"+str_Aband).value;
	if (obj_ssid == "")
    {
		alert(SEcode['lang_ssid_not_empty']);
        return false;
    }
    if (obj_ssid.match( /[^\x20-\x7E]/ ))
    {
		alert(SEcode['lang_ssid_printable']);
		return false;
	}
	if(obj_ssid.charAt(0)==" "|| obj_ssid.charAt(obj_ssid.length-1)==" ")
	{
		alert(SEcode['lang_ssid_not_blank']);
		return false;
	}
	
	if($("security_type"+str_Aband).value=="wep") //wep_64_1_Aband
	{
		var _wepKeyFt = $('wep_key_len'+str_Aband).value;
		var temp_node = _wepKeyFt=="40-bit"?$('wep_64_1'+str_Aband).value:$('wep_128_1'+str_Aband).value;
		var _HEX = _wepKeyFt=="40-bit"?/^[0-9a-fA-F]{10}$/:/^[0-9a-fA-F]{26}$/;	
		if(_wepKeyFt=="40-bit")
		{
			if(temp_node.length==5)
			{			
				if (checkSpace(temp_node) == true) 
				{
					alert(SEcode['lang_wepkey_not_blank']);
					return false;
				}
				if (temp_node.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_wepkey_printable']);
					return false;
				}
			}
			else if(temp_node.length==10)
			{
				if(temp_node.match(_HEX) == null){
					alert(SEcode['lang_wepkey_not_hex']);
					return false;				
				}
			}
			else
			{
				alert(SEcode['lang_wepkey_length']);
				return false;
			}
		}
		else	
		{
			if(temp_node.length==13)
			{			
				if (checkSpace(temp_node) == true) 
				{
					alert(SEcode['lang_wepkey_not_blank']);
					return false;
				}
				if (temp_node.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_wepkey_printable']);
					return false;
				}
			}
			else if(temp_node.length==26)
			{
				if(temp_node.match(_HEX) == null){
					alert(SEcode['lang_wepkey_not_hex']);
					return false;				
				}
			}
			else
			{
				alert(SEcode['lang_wepkey_length']);
				return false;
			}
		}
	}
	else if($("security_type"+str_Aband).value=="wpa_personal")
	{ 
		var key_intrv= $('wpa_grp_key_intrv'+str_Aband).value;
		if (Number(key_intrv) < 30 || Number(key_intrv) > 65535 || isNaN(key_intrv)) 
		{
			alert(SEcode['lang_wpa_interval']);
			return false;
		}
		var wpa_key= $('wpa_psk_key'+str_Aband).value;
		if(wpa_key.length < 8){
			alert(SEcode['lang_wpakey_length']);
			return false;
		}
		else if(wpa_key.length == 64 && wpa_key.match(/^[0-9a-fA-F]{64}$/) == null)
		{
			alert(SEcode['lang_wpakey_64_hex']);
			return false;
		}
		else
		{
			if (wpa_key.match( /[^\x20-\x7E]/ ))
			{
				alert(SEcode['lang_wpakey_printable']);
				return false;
			}
		}
		if (checkSpace(wpa_key) == true) 
		{
			alert(SEcode['lang_wpakey_not_blank']);
			return false;
		}		
	}
	else if($("security_type"+str_Aband).value=="wpa_enterprise")
	{	
		var radius_key= $('radius_srv_sec'+str_Aband).value;
		var radius_ip= $('radius_srv_ip'+str_Aband).value;
		var radius_key_second= $('radius_srv_sec_second'+str_Aband).value;
		var radius_ip_second= $('radius_srv_ip_second'+str_Aband).value;
		var key_intrv= $('wpa_grp_key_intrv'+str_Aband).value;
		if (Number(key_intrv) < 30 || Number(key_intrv) > 65535 || isNaN(key_intrv)) 
		{
			alert(SEcode['lang_wpa_interval']);
			return false;
		}
		if (!CheckValidity.IP('radius_srv_ip'+str_Aband,SEcode['lang_radius_ip_invalid']))
			return false;
		if (radius_key.length < 1) 
		{
			alert(SEcode['lang_radiuskey_length']);
			return false;
		}
		else
		{
			if (radius_key.match( /[^\x20-\x7E]/ ))
			{
				alert(SEcode['lang_radiuskey_printable']);
				return false;
			}
		}
		if (checkSpace(radius_key) == true) 
		{
			alert(SEcode['lang_radiuskey_not_blank']);
			return false;
		}	
		if (radius_ip_second!="") 
		{
			if (!CheckValidity.IP('radius_srv_ip_second'+str_Aband,SEcode['lang_radius_ip_invalid']))
			return false;
			
			if (radius_key_second.length < 1) 
			{
				alert(SEcode['lang_radiuskey_length']);
				return false;
			}
			else
			{
				if (radius_key_second.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_radiuskey_printable']);
					return false;
				}
			}
			if(checkSpace(radius_key_second) == true)
			{
				alert(SEcode['lang_radiuskey_not_blank']);
				return false;
			}
		}		
	}
	return true;
}
function uiSubmit()
{
	if(Form.Checkbox('en_gzone'))
	if(!ValidityCheck(""))return false;
	if(Form.Checkbox('en_gzone_Aband'))
	if(!ValidityCheck("_Aband"))return false;
    var _path;
	//2.4G ���߲����ύ����
	if(Form.Checkbox('en_gzone'))
	{
		$H({
		
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.X_TWSZ-COM_ScheduleListName' : $('sch_gz').value,
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.SSID'                        : $('ssid').value,
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.Enable'                      : Form.Checkbox('en_gzone')
		},true);		

		_path = ":InternetGatewayDevice.LANDevice.1.WLANConfiguration.3";
		switch($('security_type').value){
			case 'none' : {
				$F(_path + '.BeaconType'             , 'None');
				$F(_path + '.BasicEncryptionModes'   , 'None');
				$F(_path + '.BasicAuthenticationMode', 'None');
				break;
			}
			case 'wep' : {
				$F(_path + '.BeaconType'             , 'Basic');
				$F(_path + '.BasicEncryptionModes'   , 'WEPEncryption');
				$F(_path + '.BasicAuthenticationMode', $('auth_type').value);
				$F(_path + '.WEPEncryptionLevel'     , $('wep_key_len').value);
				$F(_path + '.WEPKeyIndex'            , '1');
				if('40-bit' == $('wep_key_len').value)
				{
					$F(_path + '.WEPKey.1.WEPKey'        , Base64.Encode($('wep_64_1').value));//GetWepKey($('INPUT_WEPKey1').value));
					$F(_path + '.WEPKey.2.WEPKey'        , Base64.Encode($('wep_64_1').value));//GetWepKey($('INPUT_WEPKey2').value));
					$F(_path + '.WEPKey.3.WEPKey'        , Base64.Encode($('wep_64_1').value));//GetWepKey($('INPUT_WEPKey3').value));
					$F(_path + '.WEPKey.4.WEPKey'        , Base64.Encode($('wep_64_1').value));//GetWepKey($('INPUT_WEPKey4').value));
				}
				else if('104-bit' == $('wep_key_len').value)
				{
					$F(_path + '.WEPKey.1.WEPKey'        , Base64.Encode($('wep_128_1').value));//GetWepKey($('INPUT_WEPKey1').value));
					$F(_path + '.WEPKey.2.WEPKey'        , Base64.Encode($('wep_128_1').value));//GetWepKey($('INPUT_WEPKey2').value));
					$F(_path + '.WEPKey.3.WEPKey'        , Base64.Encode($('wep_128_1').value));//GetWepKey($('INPUT_WEPKey3').value));
					$F(_path + '.WEPKey.4.WEPKey'        , Base64.Encode($('wep_128_1').value));//GetWepKey($('INPUT_WEPKey4').value));
				}	
					
				break;
			}
			case 'wpa_personal':
			case 'wpa_enterprise':
				{
					if( $('wpa_mode').value == 'WPA+2')
					{
						$F(_path + '.BeaconType', 					'WPAand11i');
						
						if('TKIP' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv').value);
						if($('security_type').value == 'wpa_personal'){
							$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec').value));		
								if($('radius_srv_ip_second').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second').value));		
						        } 
								else
								    $F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
							}
					}
					
					if( $('wpa_mode').value == 'WPA2')
					{
						$F(_path + '.BeaconType', 					'11i');
						
						if('TKIP' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv').value);
						if($('security_type').value == 'wpa_personal'){
							//$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								//$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec').value));		
								if($('radius_srv_ip_second').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second').value));	
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
					}
					
					if( $('wpa_mode').value == 'WPA')
					{
						$F(_path + '.BeaconType', 					'WPA');
						
						if('TKIP' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							//$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							//$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							//$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv').value);
						if($('security_type').value == 'wpa_personal'){
							$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							//$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								//$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec').value));		
								if($('radius_srv_ip_second').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second').value));
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
				   }	
			  }
		}
	}
	else
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.Enable', '0');

    //5G ���߲����ύ����

	if(Form.Checkbox('en_gzone_Aband'))
	{
		 $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.X_TWSZ-COM_ScheduleListName',  $('sch_gz_Aband').value);
		 $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.SSID',                         $('ssid_Aband').value);
		 $F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.Enable',                       Form.Checkbox('en_gzone_Aband'));
		
		_path = ":InternetGatewayDevice.LANDevice.1.WLANConfiguration.4";
		switch($('security_type_Aband').value){
			case 'none' : {
				$F(_path + '.BeaconType'             , 'None');
				$F(_path + '.BasicEncryptionModes'   , 'None');
				$F(_path + '.BasicAuthenticationMode', 'None');
				break;
			}
			case 'wep' : {
				$F(_path + '.BeaconType'             , 'Basic');
				$F(_path + '.BasicEncryptionModes'   , 'WEPEncryption');
				$F(_path + '.BasicAuthenticationMode', $('auth_type_Aband').value);
				$F(_path + '.WEPEncryptionLevel'     , $('wep_key_len_Aband').value);
				$F(_path + '.WEPKeyIndex'            , '1');
				if('40-bit' == $('wep_key_len_Aband').value)
				{
					$F(_path + '.WEPKey.1.WEPKey'        , Base64.Encode($('wep_64_1_Aband').value));//GetWepKey($('INPUT_WEPKey1').value));
					$F(_path + '.WEPKey.2.WEPKey'        , Base64.Encode($('wep_64_1_Aband').value));//GetWepKey($('INPUT_WEPKey2').value));
					$F(_path + '.WEPKey.3.WEPKey'        , Base64.Encode($('wep_64_1_Aband').value));//GetWepKey($('INPUT_WEPKey3').value));
					$F(_path + '.WEPKey.4.WEPKey'        , Base64.Encode($('wep_64_1_Aband').value));//GetWepKey($('INPUT_WEPKey4').value));
				}
				else if('104-bit' == $('wep_key_len_Aband').value)
				{
					$F(_path + '.WEPKey.1.WEPKey'        , Base64.Encode($('wep_128_1_Aband').value));//GetWepKey($('INPUT_WEPKey1').value));
					$F(_path + '.WEPKey.2.WEPKey'        , Base64.Encode($('wep_128_1_Aband').value));//GetWepKey($('INPUT_WEPKey2').value));
					$F(_path + '.WEPKey.3.WEPKey'        , Base64.Encode($('wep_128_1_Aband').value));//GetWepKey($('INPUT_WEPKey3').value));
					$F(_path + '.WEPKey.4.WEPKey'        , Base64.Encode($('wep_128_1_Aband').value));//GetWepKey($('INPUT_WEPKey4').value));
				}	
					
				break;
			}
			case 'wpa_personal':
			case 'wpa_enterprise':
				{
					if( $('wpa_mode_Aband').value == 'WPA+2')
					{
						$F(_path + '.BeaconType', 					'WPAand11i');
						
						if('TKIP' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv_Aband').value);
						if($('security_type_Aband').value == 'wpa_personal'){
							$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key_Aband').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second_Aband').value));		
						        }
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
							}
					}
					
					if( $('wpa_mode_Aband').value == 'WPA2')
					{
						$F(_path + '.BeaconType', 					'11i');
						
						if('TKIP' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv_Aband').value);
						if($('security_type_Aband').value == 'wpa_personal'){
							//$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key_Aband').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								//$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second_Aband').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second_Aband').value));	
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
					}
					
					if( $('wpa_mode_Aband').value == 'WPA')
					{
						$F(_path + '.BeaconType', 					'WPA');
						
						if('TKIP' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							//$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
						}
						else if('AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							//$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
						}
						else if('TKIP+AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							//$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
						} 
						
						
						$F(_path + '.X_TWSZ-COM_WPAGroupRekey', 	$('wpa_grp_key_intrv_Aband').value);
						if($('security_type_Aband').value == 'wpa_personal'){
							$F(_path + '.WPAAuthenticationMode', 		'PSKAuthentication');
							//$F(_path + '.IEEE11iAuthenticationMode', 	'PSKAuthentication');
							//Pre-Shared Key
							if($('wpa_psk_key_Aband').value.length < 64){
								$F(_path + '.PreSharedKey.1.KeyPassphrase', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'KeyPassphrase');
							} else {
								$F(_path + '.PreSharedKey.1.PreSharedKey', Base64.Encode($('wpa_psk_key_Aband').value));
								$F(_path + '.X_TWSZ-COM_PSKExpression', 'PreSharedKey');
							}
						} else {
								$F(_path + '.WPAAuthenticationMode', 		'EAPAuthentication');
								//$F(_path + '.IEEE11iAuthenticationMode', 	'EAPAuthentication');
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port'      , $('radius_srv_port_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second_Aband').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  ,  Base64.Encode($('radius_srv_sec_second_Aband').value));
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
				   }	
			  }
		}
	}
	else
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.Enable', '0');
	if ((Form.Checkbox('en_gzone_Aband') == false) && (Form.Checkbox('en_gzone') == false))
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.X_TWSZ-COM_APIsolate',          '0');
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.X_TWSZ-COM_APIsolate',          '0');		
	}
	else
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.X_TWSZ-COM_APIsolate',          (Form.Checkbox('en_routing_dualband')=='1')?'0':'1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.X_TWSZ-COM_APIsolate',          (Form.Checkbox('en_routing_dualband')=='1')?'0':'1');
		
	}
	  $H({
        'var:menu'     : G_Menu,
		'var:page'     : 'Adv_gzone',
		'var:sys_Token' : G_SysToken,
		'var:subpage'  : 'Adv_gzone',
		'var:errorpage': 'Adv_gzone',
		'obj-action'   :'set',
		'getpage'      :'html/index.html',
		'errorpage'    :'html/index.html',
		'var:CacheLastData': ViewState.Save()
	});
    $('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_gzone002').disabled= true;
	$('Adv_gzone101').disabled= true;
	//$F('obj-action', 'set');
    //$('uiPostForm').submit();
	//return;
}

function dealWithError(){
	if(G_Error != 1){
		return false;
	}
	
	var arrayHint = [];	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);


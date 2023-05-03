var Radio_list=[];
var WlanEnable=[];
WlanEnable[0] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.Enable?>";
WlanEnable[1] = "<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.Enable?>";

var m=0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_Radio. "Enable Channel AutoChannelEnable Standard  OperatingChannelBandwidth ChannelsInUse RegulatoryDomain"
`	Radio_list[m]=[];
	Radio_list[m][0] 			 = "InternetGatewayDevice.X_TWSZ-COM_Radio.$00.";//path
	Radio_list[m][1]             = "$01";//Enable
	Radio_list[m][2]             = "$02";//Channel
	Radio_list[m][3]             = "$03";//AutoChannelEnable
	Radio_list[m][4]             = "$04";//Standard
	Radio_list[m][5]	         = "$05";//OperatingChannelBandwidth
	Radio_list[m][6]	         = "$06";//ChannelsInUse
	Radio_list[m][7]	         = "$07";//RegulatoryDomain
	m++;
`?>
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "X_TWSZ-COM_ScheduleListName SSID SSIDAdvertisementEnabled BeaconType WEPEncryptionLevel WEPKeyIndex BasicAuthenticationMode BasicEncryptionModes WPAAuthenticationMode WPAEncryptionModes IEEE11iAuthenticationMode IEEE11iEncryptionModes X_TWSZ-COM_PSKExpression PreSharedKey.1.KeyPassphrase PreSharedKey.1.PreSharedKey X_TWSZ-COM_WPAGroupRekey WEPKey.1.WEPKey X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey WPS.Enable FixRate X_TWSZ-COM_BakRadiusEnable"
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
					 '$0p', //FixRate
					 '$0q' //X_TWSZ-COM_BakRadiusEnable

                     ];
    n++;
`?>
var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= ":InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>
//var _index = -1;
//var _ssidIndex = '<?echo $var:ssid_idx?>';

var sec_type_Aband = "";
var sec_type = "";

function createChannel_24G()
{
	var new_options,new_values;
	switch(Radio_list[0][7])
	{	
		case 'RU' :
		case 'KR' :
		case 'BR' :
		case 'LA' :
		case 'IN' :
		case 'SG' :
		case 'EU' :
		case 'BD' :
		{
				new_options = ['01','02','03','04','05','06','07','08','09','10','11','12','13'];
				new_values  = ['1','2','3','4','5','6','7','8','9','10','11','12','13'];
				break;
		}
			
        case 'TW' :
	    case 'NA' :
		{
				new_options = ['01','02','03','04','05','06','07','08','09','10','11'];
				new_values  = ['1','2','3','4','5','6','7','8','9','10','11'];
				break;
		}

	}	
	$S('channel', new_options, new_values);
}
function createChannel_24G_40M()
{
	var new_options,new_values;
	switch(Radio_list[0][7])
	{	
		case 'RU' :
		case 'KR' :
		case 'BR' :
		case 'LA' :
		case 'IN' :
		case 'SG' :
		case 'EU' :
		case 'BD' :
		{
				new_options = ['01','02','03','04','05','06','07','08','09','10','11','12','13'];
				new_values  = ['1','2','3','4','5','6','7','8','9','10','11','12','13'];
				break;
		}
			
        case 'TW' :
	    case 'NA' :
		{
				new_options = ['01','02','03','04','05','06','07','08','09','10','11'];
				new_values  = ['1','2','3','4','5','6','7','8','9','10','11'];
				break;
		}
	}
	$S('channel', new_options, new_values);
}

function createChannel_5G()
{
	var new_options,new_values;
	switch(Radio_list[1][7])
	{
		case 'RU' :	
		{
				new_options = ['36','40','44','48','52','56','60','64','136','140','144','149','153','157','161','165'];
				new_values  = ['36','40','44','48','52','56','60','64','136','140','144','149','153','157','161','165'];
				break;
		}
		case 'KR' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}
		case 'BR' :	
		case 'LA' :	
		case 'NA' :	
		case 'SG' :	
		{
				new_options = ['36','40','44','48','149','153','157','161','165'];
				new_values  = ['36','40','44','48','149','153','157','161','165'];
				break;
		}

		case 'TW' :	
		{
				new_options = ['36','40','44','48','149','153','157','161','165'];
				new_values  = ['36','40','44','48','149','153','157','161','165'];
				break;
		}

		case 'BD' :	
		{
				new_options = ['149','153','157','161','165','169','173'];
				new_values  = ['149','153','157','161','165','169','173'];
				break;
		}

		case 'IN' :	
		{
				new_options = ['36','40','44','48','149','153','157','161','165','169','173'];
				new_values  = ['36','40','44','48','149','153','157','161','165','169','173'];
				break;
		}
		case 'EU' :	
		{
				new_options = ['36','40','44','48'];
				new_values  = ['36','40','44','48'];
				break;
		}
		
	}	
	$S('channel_Aband', new_options, new_values);
}
function createChannel_5G_40M()
{
	var new_options,new_values;
	switch(Radio_list[1][7])
	{
			
		case 'RU' :	
		{
				new_options = ['36','40','44','48','52','56','60','64','136','144','149','153','157','161'];
				new_values  = ['36','40','44','48','52','56','60','64','136','144','149','153','157','161'];
				break;
		}
		case 'KR' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}
		case 'BR' :	
		case 'LA' :	
		case 'NA' :	
		case 'SG' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}

		case 'TW' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}

		case 'BD' :	
		{
				new_options = ['149','153','157','161','165','169'];
				new_values  = ['149','153','157','161','165','169'];
				break;
		}

		case 'IN' :	
		{
				new_options = ['36','40','44','48','149','153','157','161','165','169'];
				new_values  = ['36','40','44','48','149','153','157','161','165','169'];
				break;
		}
		case 'EU' :	
		{
				new_options = ['36','40','44','48'];
				new_values  = ['36','40','44','48'];
				break;
		}
	}	
	$S('channel_Aband', new_options, new_values);
}

function createChannel_5G_80M()
{
	var new_options,new_values;
	switch(Radio_list[1][7])
	{
			
		case 'RU' :	
		{
				new_options = ['36','40','44','48','52','56','60','64','149','153','157','161'];
				new_values  = ['36','40','44','48','52','56','60','64','149','153','157','161'];
				break;
		}
		case 'KR' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}
		case 'BR' :	
		case 'LA' :	
		case 'NA' :	
		case 'SG' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}

		case 'TW' :	
		{
				new_options = ['36','40','44','48','149','153','157','161'];
				new_values  = ['36','40','44','48','149','153','157','161'];
				break;
		}

		case 'BD' :	
		{
				new_options = ['149','153','157','161','165'];
				new_values  = ['149','153','157','161','165'];
				break;
		}

		case 'IN' :	
		{
				new_options = ['36','40','44','48','149','153','157','161','165'];
				new_values  = ['36','40','44','48','149','153','157','161','165'];
				break;
		}
		case 'EU' :	
		{
				new_options = ['36','40','44','48'];
				new_values  = ['36','40','44','48'];
				break;
		}
	}	
	$S('channel_Aband', new_options, new_values);
}

function uiOnload(){	
	createSchedule();
	initial();
}
function initial(){
	for(var i=0; i<2; i+=1)
	{
		var str_Aband=(i==0)?"":"_Aband";
		$("en_wifi"+str_Aband).checked = (Radio_list[i][1]=="1")?true:false;
		$('ssid'+str_Aband).value = G_Wireless[i][2];	
		$('wlan_mode'+str_Aband).value = Radio_list[i][4];	
		DrawSecurityList($('wlan_mode'+str_Aband).value, str_Aband)
		$('sch'+str_Aband).value = G_Wireless[i][1];
		$("auto_ch"+str_Aband).checked = (Radio_list[i][3]=="1")?true:false;
		OnClickEnAutoChannel(str_Aband);
		var channel_use=Radio_list[i][6]==""?(str_Aband==""?"1":"36"):Radio_list[i][6];
		$("channel"+str_Aband).value = Radio_list[i][3]=="1"?channel_use:Radio_list[i][2];
		$("txrate"+str_Aband).value = Radio_list[i][2];
		$("bw"+str_Aband).value = Radio_list[i][5];
		OnChangeBandwidth(str_Aband);
		$("txrate"+str_Aband).value = G_Wireless[i][25];
		Form.Radio("invisible_type"+str_Aband,G_Wireless[i][3]);

        if(i==0)
        {
           if($("bw"+str_Aband).value=="20")
		       createChannel_24G();
		   else//20O40
		   	   createChannel_24G_40M();
		}
		else
	    {
            if($("bw"+str_Aband).value=="20")//20
		        createChannel_5G();
		    else if($("bw"+str_Aband).value=="20O40")//20O40
		        createChannel_5G_40M();
		    else //20O40O80
		        createChannel_5G_80M();
		}
	   
		$("channel"+str_Aband).value = Radio_list[i][3]=="1"?channel_use:Radio_list[i][2];

	
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
				$("auth_type"+str_Aband).value = G_Wireless[i][7];
				$("wep_key_len"+str_Aband).value = G_Wireless[i][5];
				$("wpa_grp_key_intrv"+str_Aband).value = G_Wireless[i][16];
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
			if('1' == G_Wireless[i][26])
			{
				$("radius_srv_ip_second"+str_Aband).value = G_Wireless[i][21];
				$("radius_srv_port_second"+str_Aband).value = G_Wireless[i][22];
				$("radius_srv_sec_second"+str_Aband).value = Base64.Decode(G_Wireless[i][23]);	
			}
			else
			{
				//$("radius_srv_ip_second"+str_Aband).value = G_Wireless[i][21];
				if("" == $("radius_srv_port_second"+str_Aband).value)
				    $("radius_srv_port_second"+str_Aband).value = "1812";
				else
					$("radius_srv_port_second"+str_Aband).value = G_Wireless[i][22];
				//$("radius_srv_sec_second"+str_Aband).value = Base64.Decode(G_Wireless[i][23]);	
			}
		}	
		OnClickEnWLAN(str_Aband);
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
		$S('sch', array_options, array_value);
		$S('sch_Aband', array_options, array_value);
}

//检测字符串开始和结尾是否包含空格
function checkSpace(id) {
	if (document.getElementById(id).value.length == 0) 
		return false;
	if (document.getElementById(id).value.indexOf(" ",0) == 0 || document.getElementById(id).value.lastIndexOf(" ") ==
	 (document.getElementById(id).value.length -1)) {
		return true;
	}
}
function OnClickEnWLAN(str_Aband)
{
	if ($("en_wifi"+str_Aband).checked)
	{		
		$("sch"+str_Aband).disabled	= false;
		$("go2sch"+str_Aband).disabled	= false;
		$("ssid"+str_Aband).disabled	= false;
		$("auto_ch"+str_Aband).disabled	= false;
		if (!$("auto_ch"+str_Aband).checked) $("channel"+str_Aband).disabled = false;
		$("txrate"+str_Aband).disabled	= false;
		$("wlan_mode"+str_Aband).disabled	= false;
		if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
		{
			$("bw"+str_Aband).disabled	= false;
			$("en_wmm"+str_Aband).disabled = true;
		}
		else
			$("en_wmm"+str_Aband).disabled = false;
		$("ssid_visible"+str_Aband).disabled = false;
		$("ssid_invisible"+str_Aband).disabled = false;		
		$("security_type"+str_Aband).disabled= false;
		$("wep_key_len"+str_Aband).disabled= false;	
		$("auth_type"+str_Aband).disabled= false;
		$("wep_64_1"+str_Aband).disabled= false;
		$("wep_128_1"+str_Aband).disabled= false;
		$("wpa_mode"+str_Aband).disabled= false;
		$("cipher_type"+str_Aband).disabled= false;
		$("wpa_grp_key_intrv"+str_Aband).disabled= false;
		$("wpa_psk_key"+str_Aband).disabled= false;
		$("radius_srv_ip"+str_Aband).disabled= false;
		$("radius_srv_port"+str_Aband).disabled= false;
		$("radius_srv_sec"+str_Aband).disabled= false;
		$("radius_adv"+str_Aband).disabled= false;
		$("radius_srv_ip_second"+str_Aband).disabled= false;
		$("radius_srv_port_second"+str_Aband).disabled= false;
		$("radius_srv_sec_second"+str_Aband).disabled= false;
		$("security_type"+str_Aband).value= str_Aband==""?sec_type:sec_type_Aband;
	}
	else
	{
		
		$("sch"+str_Aband).disabled	= true;
		$("go2sch"+str_Aband).disabled	= true;
		$("ssid"+str_Aband).disabled	= true;
		$("auto_ch"+str_Aband).disabled	= true;
		$("channel"+str_Aband).disabled	= true;
		$("txrate"+str_Aband).disabled	= true;
		$("wlan_mode"+str_Aband).disabled	= true;
		$("en_wmm"+str_Aband).disabled = true;
		$("ssid_visible"+str_Aband).disabled = true;
		$("ssid_invisible"+str_Aband).disabled = true;		
		$("security_type"+str_Aband).disabled= true;
		$("wep_key_len"+str_Aband).disabled= true;	
		$("auth_type"+str_Aband).disabled= true;
		$("wep_64_1"+str_Aband).disabled= true;
		$("wep_128_1"+str_Aband).disabled= true;
		$("wpa_mode"+str_Aband).disabled= true;
		$("cipher_type"+str_Aband).disabled= true;
		$("wpa_grp_key_intrv"+str_Aband).disabled= true;
		$("wpa_psk_key"+str_Aband).disabled= true;
		$("radius_srv_ip"+str_Aband).disabled= true;
		$("radius_srv_port"+str_Aband).disabled= true;
		$("radius_srv_sec"+str_Aband).disabled= true;
		$("radius_adv"+str_Aband).disabled= true;
		$("radius_srv_ip_second"+str_Aband).disabled= true;
		$("radius_srv_port_second"+str_Aband).disabled= true;
		$("radius_srv_sec_second"+str_Aband).disabled= true;
		$("security_type"+str_Aband).value= "none";
	}
	OnChangeSecurityType(str_Aband);
	OnChangeChannel(str_Aband);
}
function OnChangeChannel(str_Aband)
{
	if (!$("auto_ch"+str_Aband).checked && ( $("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac" ))
	{
		/* when change channel to 116 and channel list does not has channel 120, 
			 the channel width should select only 20 MHz. */
		if ($("channel"+str_Aband).value=="116")
		{
			 var select = document.getElementById ("channel"+str_Aband);
			 var has120 = false;
			 for (var i = 0; i < select.options.length; i++)
			 {
				if (select.options[i].value === "120") { has120 = true; break; }
				else has120 = false;
			 }
		}     
		
		/*if( has120==false || $("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13")*/
        if( has120==false || $("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="14" )
		{
			OnChangeSecurityType(str_Aband);
			$("bw"+str_Aband).value="20";
			$("bw"+str_Aband).disabled = true;				
		}
		else if($("channel"+str_Aband).value=="132" || $("channel"+str_Aband).value=="136")
		{
			$("bw"+str_Aband).disabled	= false;
			OnChangeSecurityType(str_Aband);
			COMM_RemoveSelectOptionIfExist($("bw"+str_Aband), "20O40O80");
		}
		else
		{
			$("bw"+str_Aband).disabled = false;
			OnChangeSecurityType(str_Aband);
			if($("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "anan" || $("wlan_mode"+str_Aband).value == "aanac")
			COMM_AddSelectOptionIfNoExist($("bw"+str_Aband), "20O40O80", data_languages.Bas_wlan.innerHTML.BWLAN119);
		}
	}
}
function OnChangeSecurityType(str_Aband)
{
    $('wlan_mode'+str_Aband).disabled = false;
	switch ($("security_type"+str_Aband).value)
	{
		case "none":
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
					$("bw"+str_Aband).disabled = false;
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			$("pad").style.display = "block";
			break;
		case "wep":
			if(str_Aband == "")
			    $('wlan_mode'+str_Aband).value = "bg";
			else
				$('wlan_mode'+str_Aband).value = "a";

			    $('wlan_mode'+str_Aband).disabled = true;
				
			$("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "block";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";				
			$("pad").style.display = "none";
			break;
		case "wpa_personal":
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
					OnChangeCipherType(str_Aband);
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "block";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			$("pad").style.display = "none";
			break;
		case "wpa_enterprise":
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
				OnChangeCipherType(str_Aband);
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "block";
			$("pad").style.display = "none";
            if(str_Aband == "")
            {
			    $("radius_srv_port"+str_Aband).value = G_Wireless[0][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[0][22];
            }
			else
			{
			    $("radius_srv_port"+str_Aband).value = G_Wireless[1][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[1][22];
            }
		    
			
			break;
	}
	if (!$("en_wifi"+str_Aband).checked) $("bw"+str_Aband).disabled = true;
}
function OnChangeCipherType(str_Aband)
{
    $('wlan_mode'+str_Aband).disabled = false;
	switch ($("cipher_type"+str_Aband).value)
	{
		case "TKIP":
			$("bw"+str_Aband).value = "20";
			$("bw"+str_Aband).disabled = true;
			if((str_Aband == ""))
			{
			    //if( $('wpa_mode').value == 'WPA')
			    {
				    $('wlan_mode'+str_Aband).value = "bg";
				    $('wlan_mode'+str_Aband).disabled = true;
			    }
			}
			else
			{
			    //if( $('wpa_mode_Aband').value == 'WPA')
			    {
				    $('wlan_mode'+str_Aband).value = "a";
				    $('wlan_mode'+str_Aband).disabled = true;
			    }
			}
				
			break;
		case "AES":
			if(str_Aband == "")
				$("bw"+str_Aband).value = "20O40";
			else
				$("bw"+str_Aband).value = "20O40O80";
			$("bw"+str_Aband).disabled = true;
			break;
		case "TKIP+AES":
			$("bw"+str_Aband).disabled = false;
			break;
	}
}
function OnChangeWLMode(str_Aband)
{	
	var phywlan = "";
	//alert($("wlan_mode"+str_Aband).value);
	if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "na")
	{		
		//if ($("bw"+str_Aband).value == "20O40O80")
			$("bw"+str_Aband).value="20O40";
		$("bw"+str_Aband).disabled	= false;
		$("en_wmm"+str_Aband).checked = true;
		$("en_wmm"+str_Aband).disabled = true;
		COMM_RemoveSelectOptionIfExist($("bw"+str_Aband), "20O40O80")
	}
	else if($("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
	{
		$("bw"+str_Aband).disabled	= false;
		COMM_AddSelectOptionIfNoExist($("bw"+str_Aband), "20O40O80", data_languages.Bas_wlan.innerHTML.BWLAN119);
		$("bw"+str_Aband).value="20O40O80";
	}		
	else
	{
	    $("bw"+str_Aband).value="20";
		$("bw"+str_Aband).disabled	= true;
		$("en_wmm"+str_Aband).disabled = false;
	}
	var shortGuard=str_Aband==""?"800":"400";
	DrawTxRateList($("bw"+str_Aband).value, shortGuard, str_Aband);
	DrawSecurityList($("wlan_mode"+str_Aband).value, str_Aband);
	OnChangeSecurityType(str_Aband);
	OnChangeChannel(str_Aband);
	if(str_Aband=="_Aband")
	createChannel();
}
function DrawTxRateList(bw, sgi, str_Aband)
{	
	var listOptions = null;
	var cond = bw+":"+sgi;
	var antenna_type = "";
	var antenna_type_24g = "2T2R";
	var antenna_type_5g = "1T1R";
	
	if(antenna_type != "") {tx_antenna = antenna_type.substr(0,2);}
	else
	{
		if(str_Aband == "") {tx_antenna = antenna_type_24g.substr(0,2);}
		else								{tx_antenna = antenna_type_5g.substr(0,2);}
	}
	switch(cond)
	{
		case "20:800":
			if (tx_antenna == "1T")
			{listOptions = new Array("0 - 6.5","1 - 13.0","2 - 19.5","3 - 26.0","4 - 39.0","5 - 52.0","6 - 58.5","7 - 65.0")}
			else if (tx_antenna == "2T")
			{listOptions = new Array("0 - 6.5","1 - 13.0","2 - 19.5","3 - 26.0","4 - 39.0","5 - 52.0","6 - 58.5","7 - 65.0","8 - 13.0","9 - 26.0","10 - 39.0","11 - 52.0","12 - 78.0","13 - 104.0","14 - 117.0","15 - 130.0")}
			else if (tx_antenna == "3T")
			{listOptions = new Array("0 - 6.5","1 - 13.0","2 - 19.5","3 - 26.0","4 - 39.0","5 - 52.0","6 - 58.5","7 - 65.0","8 - 13.0","9 - 26.0","10 - 39.0","11 - 52.0","12 - 78.0","13 - 104.0","14 - 117.0","15 - 130.0","16 - 19.5","17 - 39.0","18 - 58.5","19 - 78.0","20 - 117.0","21 - 156.0","22 - 175.5","23 - 195.0")}
			break;
		case "20:400":
			if (tx_antenna == "1T")
			{listOptions = new Array("0 - 7.2","1 - 14.4","2 - 21.7","3 - 28.9","4 - 43.3","5 - 57.8","6 - 65.0","7 - 72.2")}
			else if (tx_antenna == "2T")
			{listOptions = new Array("0 - 7.2","1 - 14.4","2 - 21.7","3 - 28.9","4 - 43.3","5 - 57.8","6 - 65.0","7 - 72.2","8 - 14.444","9 - 28.889","10 - 43.333","11 - 57.778","12 - 86.667","13 - 115.556","14 - 130.000","15 - 144.444")}
			else if (tx_antenna == "3T")
			{listOptions = new Array("0 - 7.2","1 - 14.4","2 - 21.7","3 - 28.9","4 - 43.3","5 - 57.8","6 - 65.0","7 - 72.2","8 - 14.444","9 - 28.889","10 - 43.333","11 - 57.778","12 - 86.667","13 - 115.556","14 - 130.000","15 - 144.444","16 - 21.7","17 - 43.3","18 - 65.0","19 - 86.7","20 - 130.0","21 - 173.3","22 - 195.0","23 - 216.7")}
			break;
		case "20O40:800":
			if (tx_antenna == "1T")
			{listOptions = new Array("0 - 13.5","1 - 27.0","2 - 40.5","3 - 54.0","4 - 81.0","5 - 108.0","6 - 121.5","7 - 135.0")}
			else if (tx_antenna == "2T")
			{listOptions = new Array("0 - 13.5","1 - 27.0","2 - 40.5","3 - 54.0","4 - 81.0","5 - 108.0","6 - 121.5","7 - 135.0","8 - 27.0","9 - 54.0","10 - 81.0","11 - 108.0","12 - 162.0","13 - 216.0","14 - 243.0","15 - 270.0")}
			else if (tx_antenna == "3T")
			{listOptions = new Array("0 - 13.5","1 - 27.0","2 - 40.5","3 - 54.0","4 - 81.0","5 - 108.0","6 - 121.5","7 - 135.0","8 - 27.0","9 - 54.0","10 - 81.0","11 - 108.0","12 - 162.0","13 - 216.0","14 - 243.0","15 - 270.0","16 - 40.5","17 - 81.0","18 - 121.5","19 - 162.0","20 - 243.0","21 - 324.0","22 - 364.5","23 - 405.0")}
			break;
		case "20O40:400":
			if (tx_antenna == "1T")
			{listOptions = new Array("0 - 15.0","1 - 30.0","2 - 45.0","3 - 60.0","4 - 90.0","5 - 120.0","6 - 135.0","7 - 150.0")}
			else if (tx_antenna == "2T")
			{listOptions = new Array("0 - 15.0","1 - 30.0","2 - 45.0","3 - 60.0","4 - 90.0","5 - 120.0","6 - 135.0","7 - 150.0","8 - 30.0","9 - 60.0","10 - 90.0","11 - 120.0","12 - 180.0","13 - 240.0","14 - 270.0","15 - 300.0")}
			else if (tx_antenna == "3T")
			{listOptions = new Array("0 - 15.0","1 - 30.0","2 - 45.0","3 - 60.0","4 - 90.0","5 - 120.0","6 - 135.0","7 - 150.0","8 - 30.0","9 - 60.0","10 - 90.0","11 - 120.0","12 - 180.0","13 - 240.0","14 - 270.0","15 - 300.0","16 - 45.0","17 - 90.0","18 - 135.0","19 - 180.0","20 - 270.0","21 - 360.0","22 - 405.0","23 - 450.0")}
	}

	var tr_length = $("txrate"+str_Aband).length;
	for(var idx=1; idx<tr_length; idx++)
	{
		$("txrate"+str_Aband).remove(1);
	}
	if ($("wlan_mode"+str_Aband).value == "n"||$("wlan_mode"+str_Aband).value == "5n")
	{
		for(var idx=0; idx<listOptions.length; idx++)
		{
			var item = document.createElement("option");
			item.value = idx;
			item.text = listOptions[idx];
			try		{ $("txrate"+str_Aband).add(item, null); }
			catch(e){ $("txrate"+str_Aband).add(item); }
		}
	}
}
function OnClickEnAutoChannel(str_Aband)
{
	if ($("auto_ch"+str_Aband).checked || !$("en_wifi"+str_Aband).checked)
		$("channel"+str_Aband).disabled = true;
	else
		$("channel"+str_Aband).disabled = false;		
	
	if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "na")
	{		
		if ($("bw"+str_Aband).value == "20O40O80")
			$("bw"+str_Aband).value ="20O40";
		$("bw"+str_Aband).disabled	= false;
		$("en_wmm"+str_Aband).checked = true;
		$("en_wmm"+str_Aband).disabled = true;	
		COMM_RemoveSelectOptionIfExist($("bw"+str_Aband), "20O40O80")
	}
	else if($("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
	{
		$("bw"+str_Aband).disabled	= false;
		COMM_AddSelectOptionIfNoExist($("bw"+str_Aband), "20O40O80", data_languages.Bas_wlan.innerHTML.BWLAN119);
	}		
	else
	{
		$("bw"+str_Aband).disabled	= true;
		$("en_wmm"+str_Aband).disabled = false;
	}
	OnChangeSecurityType(str_Aband);
	OnChangeChannel(str_Aband);
}
function OnChangeWPAMode(str_Aband)
{

    $('wlan_mode'+str_Aband).disabled = false;
	switch ($("wpa_mode"+str_Aband).value)
	{
		case "WPA":
			$("bw"+str_Aband).disabled = true;
			$("cipher_type"+str_Aband).value = "TKIP";
			if((str_Aband == ""))
			{

				    $('wlan_mode'+str_Aband).value = "bg";
				    $('wlan_mode'+str_Aband).disabled = true;
			}
			else
			{
				    $('wlan_mode'+str_Aband).value = "a";
				    $('wlan_mode'+str_Aband).disabled = true;
			}
			break;
		case "WPA2":
			$("bw"+str_Aband).disabled = false;
			$("cipher_type"+str_Aband).value = "AES";
			break;	
		default :
			$("bw"+str_Aband).disabled = false;
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" )
				$("cipher_type"+str_Aband).value = "AES";
			else
				$("cipher_type"+str_Aband).value = "TKIP+AES";
	}
}
function OnChangeWEPAuth(str_Aband)
{
	var wps = "0";
	if($("auth_type"+str_Aband).value == "SHARED" && wps==true)
	{
		alert(SEcode["lang_cannot_share"]);
		$("auth_type"+str_Aband).value = "WEPAUTO";
	}
}
var S2I	= function(str) {return isNaN(str)?0:parseInt(str, 10);}
function OnChangeWEPKey(str_Aband)
{
	var no = S2I($("wep_def_key"+str_Aband).value) - 1;
	switch ($("wep_key_len"+str_Aband).value)
	{
		case "40-bit":
			$("wep_64"+str_Aband).style.display = "block";
			$("wep_128"+str_Aband).style.display = "none";
			//SetDisplayStyle(null, "wepkey_64"+str_Aband, "none");
			//document.getElementsByName("wepkey_64"+str_Aband)[no].style.display = "inline";
			break;
		case "104-bit":
			$("wep_64"+str_Aband).style.display = "none";
			$("wep_128"+str_Aband).style.display = "block";
			//SetDisplayStyle(null, "wepkey_128"+str_Aband, "none");
			//document.getElementsByName("wepkey_128"+str_Aband)[no].style.display = "inline";
	}
}
var radius_adv_flag=false;
var radius_adv_flag_Aband=false;
function OnClickRadiusAdvanced(str_Aband)
{
	if(str_Aband=="")
	{
		if (radius_adv_flag) {
			$("div_second_radius"+str_Aband).style.display = "none";
			$("radius_adv"+str_Aband).value = data_languages.Bas_wlan.value.BWLAN183;
			radius_adv_flag = false;
		}
		else {
			$("div_second_radius"+str_Aband).style.display = "block";
			$("radius_adv"+str_Aband).value = data_languages.Bas_wlan.value.BWLAN184;
			radius_adv_flag = true;
		}
	}
	else
	{
		if (radius_adv_flag_Aband) {
			$("div_second_radius"+str_Aband).style.display = "none";
			$("radius_adv"+str_Aband).value = data_languages.Bas_wlan.value.BWLAN183;
			radius_adv_flag_Aband = false;
		}
		else {
			$("div_second_radius"+str_Aband).style.display = "block";
			$("radius_adv"+str_Aband).value = data_languages.Bas_wlan.value.BWLAN184;
			radius_adv_flag_Aband = true;
		}
	}
}
    
function SetDisplayStyle(tag, name, style)
{
	if (tag)	var obj = GetElementsByName_iefix(tag, name);
	else		var obj = document.getElementsByName(name);
	for (var i=0; i<obj.length; i++)
	{
		obj[i].style.display = style;
	}
}
function GetElementsByName_iefix(tag, name)
{
	var elem = document.getElementsByTagName(tag);
	var arr = new Array();
	for(i = 0,iarr = 0; i < elem.length; i++)
	{
		att = elem[i].getAttribute("name");
		if(att == name)
		{
			arr[iarr] = elem[i];
			iarr++;
		}
	}
	return arr;
}
function OnChangeBandwidth(str_Aband)
{
	var shortGuard = str_Aband==""?"800":"400";
	DrawTxRateList($("bw"+str_Aband).value, shortGuard, str_Aband);
	if(str_Aband=="_Aband")
	createChannel();	
}
function createChannel()
{
	var channel_5G=$('channel_Aband').value;
	if($('bw_Aband').value=="20O40O80")
	{	
		createChannel_5G_80M(Radio_list[1][7]);
		if(channel_5G=="165"||channel_5G=="132"||channel_5G=="136"||channel_5G=="140")
		{
			if(Radio_list[1][7]=="TW")			
				$('channel_Aband').value = '149';
			else 
				$('channel_Aband').value = '36';
		}
		else
			$('channel_Aband').value = channel_5G;
		if($('channel_Aband').value =="")
		{		
			if(Radio_list[1][7]=="TW")			
				$('channel_Aband').value = '149';
			else 
				$('channel_Aband').value = '36';
		}	
	}
	else if($('bw_Aband').value=="20O40")
	{
		createChannel_5G_40M(Radio_list[1][7]);
		if(channel_5G=="165"||channel_5G=="140")
		{
			if(Radio_list[1][7]=="TW")			
				$('channel_Aband').value = '149';
			else 
				$('channel_Aband').value = '36';
		}
		else
		$('channel_Aband').value = channel_5G;
		if($('channel_Aband').value =="")		
		{
			if(Radio_list[1][7]=="TW")			
				$('channel_Aband').value = '149';
			else 
				$('channel_Aband').value = '36';
		}
	}
	else
	{
		
		createChannel_5G(Radio_list[1][7]);
		$('channel_Aband').value = channel_5G;
		if($('channel_Aband').value =="")		
		{
			if(Radio_list[1][7]=="TW")			
				$('channel_Aband').value = '149';
			else 
				$('channel_Aband').value = '36';
		}			
	}	
}
function DrawSecurityList(wlan_mode, str_Aband)
{
	var security_list = null;
	var cipher_list = null;
	
	if (wlan_mode == "n" || wlan_mode == "5n" || wlan_mode == "ac" || wlan_mode == "anac")
	{
		security_list = ['wpa_personal', data_languages.Bas_wlan.innerHTML.BWLAN129,
						'wpa_enterprise', data_languages.Bas_wlan.innerHTML.BWLAN130];
		cipher_list = ['AES'];
	}
	else
	{
		security_list = ['wep', data_languages.Bas_wlan.innerHTML.BWLAN128,
						 'wpa_personal', data_languages.Bas_wlan.innerHTML.BWLAN129,
						 'wpa_enterprise', data_languages.Bas_wlan.innerHTML.BWLAN130];
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
		if (item.value=="TKIP+AES") item.text = data_languages.Bas_wlan.innerHTML.BWLAN160;
		else						item.text = cipher_list[idx];
		try		{ $("cipher_type"+str_Aband).add(item, null); }
		catch(e){ $("cipher_type"+str_Aband).add(item); }
	}
}
function COMM_AddSelectOptionIfNoExist(obj, value, text)
{
	var Option_Exist = false;
	for(var i=0; i < obj.length; i++)
	{ 
		if(obj.options[i].value == value)
		{
			Option_Exist = true;
			break;
		}
	}
	if(!Option_Exist)
	{
		var new_option = new Option(text, value); 
		obj.options.add(new_option);				
	}
}

function COMM_RemoveSelectOptionIfExist(obj, value)
{
	for(var i=0; i < obj.length; i++)
	{ 
		if(obj.options[i].value == value)
		{
			obj.remove(i);
			break;
		}
	}	
}	
function OnChangeSecurityType(str_Aband)
{
    $('wlan_mode'+str_Aband).disabled = false;
	switch ($("security_type"+str_Aband).value)
	{
		case "none":		
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
					$("bw"+str_Aband).disabled = false;
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			$("pad").style.display = "block";
			break;
		case "wep":
			if(str_Aband == "")
			    $('wlan_mode'+str_Aband).value = "bg";
			else
				$('wlan_mode'+str_Aband).value = "a";

			    $('wlan_mode'+str_Aband).disabled = true;

                   $("bw"+str_Aband).value = "20";
			$("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "block";
			$("box_wpa"+str_Aband).style.display = "none";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";				
			$("pad").style.display = "none";
			break;
		case "wpa_personal":
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
					OnChangeCipherType(str_Aband);
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "block";
			$("box_wpa_enterprise"+str_Aband).style.display = "none";
			$("pad").style.display = "none";
			break;
		case "wpa_enterprise":
			if ($("wlan_mode"+str_Aband).value == "n" ||$("wlan_mode"+str_Aband).value == "5n" || $("wlan_mode"+str_Aband).value == "ng" || $("wlan_mode"+str_Aband).value == "bgn" || $("wlan_mode"+str_Aband).value == "ac" || $("wlan_mode"+str_Aband).value == "na" || $("wlan_mode"+str_Aband).value == "anac" || $("wlan_mode"+str_Aband).value == "aanac")
			{
				if(!$("auto_ch"+str_Aband).checked && ($("channel"+str_Aband).value=="140" || $("channel"+str_Aband).value=="165" || $("channel"+str_Aband).value=="12" || $("channel"+str_Aband).value=="13"))
					$("bw"+str_Aband).disabled = true;
				else 
				OnChangeCipherType(str_Aband);
			}
			else $("bw"+str_Aband).disabled = true;
			$("wep"+str_Aband).style.display = "none";
			$("box_wpa"+str_Aband).style.display = "block";
			$("box_wpa_personal"+str_Aband).style.display = "none";
			$("box_wpa_enterprise"+str_Aband).style.display = "block";
			$("pad").style.display = "none";

			if(str_Aband == "")
            {
			    $("radius_srv_port"+str_Aband).value = G_Wireless[0][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[0][22];
            }
			else
			{
			    $("radius_srv_port"+str_Aband).value = G_Wireless[1][19];
			    $("radius_srv_port_second"+str_Aband).value = G_Wireless[1][22];
            }
			break;
	}
	if (!$("en_wifi"+str_Aband).checked) $("bw"+str_Aband).disabled = true;
}

function checkIllegal(str_Aband){
	var ssid = $('ssid'+str_Aband).value;
	if (ssid == "")
    {
		alert(SEcode['lang_ssid_empty']);
        return false;
    }
    if (ssid.match( /[^\x20-\x7E]/ ))
    {
		alert(SEcode['lang_ssid_printable']);
		return false;
	}
	if (checkSpace(ssid) == true) 
	{
		alert(SEcode["lang_prefix_blank"]);
		return false;
	}
	//check wep format
	if($('security_type'+str_Aband).value == 'wep')
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
					alert(SEcode["lang_pre_wep_blank"]);
					return false;
				}
				if (temp_node.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_wep_printable']);
					return false;
				}
			}
			else if(temp_node.length==10)
			{
				if(temp_node.match(_HEX) == null){
					alert(SEcode['lang_wep_print_hex']);
					return false;				
				}
			}
			else
			{
				alert(SEcode['lang_wep_length']);
				return false;
			}
		}
		else	
		{
			if(temp_node.length==13)
			{			
				if (checkSpace(temp_node) == true) 
				{
					alert(SEcode["lang_pre_wep_blank"]);
					return false;
				}
				if (temp_node.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_wep_printable']);
					return false;
				}
			}
			else if(temp_node.length==26)
			{
				if(temp_node.match(_HEX) == null){
					alert(SEcode['lang_wep_print_hex']);
					return false;				
				}
			}
			else
			{
				alert(SEcode['lang_wep_length']);
				return false;
			}
		}
	}
	else if($('security_type'+str_Aband).value == 'wpa_personal')
	{
		var wpa_key= $('wpa_psk_key'+str_Aband).value;
		if(wpa_key.length < 8){
			alert(SEcode["lang_psk_least8"]);
			return false;
		}
		else if(wpa_key.length == 64 && wpa_key.match(/^[0-9a-fA-F]{64}$/) == null)
		{
			alert(SEcode["lang_psk_64hex"]);
			return false;
		}
		else
		{
			if (wpa_key.match( /[^\x20-\x7E]/ ))
			{
				alert(SEcode['lang_psk_printable']);
				return false;
			}
		}
		if (checkSpace(wpa_key) == true) 
		{
			alert(SEcode["lang_pre_psk_blank"]);
			return false;
		}		
	}
	else if($('security_type'+str_Aband).value == 'wpa_enterprise')
	{
		var radius_key= $('radius_srv_sec'+str_Aband).value;
		var radius_ip= $('radius_srv_ip'+str_Aband).value;
		var radius_key_second= $('radius_srv_sec_second'+str_Aband).value;
		var radius_ip_second= $('radius_srv_ip_second'+str_Aband).value;
		var key_intrv= $('wpa_grp_key_intrv'+str_Aband).value;
		if (Number(key_intrv) < 30 || Number(key_intrv) > 65535 || isNaN(key_intrv)) 
		{
			alert(SEcode["lang_groupkey_length"]);
			return false;
		}
		if (!CheckValidity.IP('radius_srv_ip'+str_Aband,SEcode["lang_RADIUS_ip_invalid"]))
			return false;
		if (radius_key.length < 1) 
		{
			alert(SEcode["lang_secret_length"]);
			return false;
		}
		else
		{
			if (radius_key.match( /[^\x20-\x7E]/ ))
			{
				alert(SEcode['lang_RADIUS_printable']);
				return false;
			}
		}
		if (checkSpace(radius_key) == true) 
		{
			alert(SEcode["lang_RADIUS_blank"]);
			return false;
		}	
		if (radius_ip_second!="") 
		{
			if (!CheckValidity.IP('radius_srv_ip_second'+str_Aband,SEcode["lang_RADIUS_ip_invalid"]))
			return false;	
		
			if (radius_key_second.length < 1) 
			{
				alert(SEcode["lang_secret_length"]);
				return false;
			}
			else
			{
				if (radius_key_second.match( /[^\x20-\x7E]/ ))
				{
					alert(SEcode['lang_RADIUS_printable']);
					return false;
				}
			}
			if(checkSpace(radius_key_second) == true)
			{
				alert(SEcode["lang_RADIUS_blank"]);
				return false;
			}
		}		
	}	
	return true;
}

function checkSpace(id) {
	if (id.indexOf(" ",0) == 0 || id.lastIndexOf(" ") == id.length -1) {
		return true;
	}
	else
		return false;
} 

function uiSubmit()
{
	if(Form.Checkbox('en_wifi'))
	if(!checkIllegal('')){
		return false;
	}
	if(Form.Checkbox('en_wifi_Aband'))
	if(!checkIllegal('_Aband')){
		return false;
	}
    var _path;
	var _wps_hiddenssid1="1",_wps_wep1="1",_wps_enterprise1="1",_wps_wpa_tkip1="1",_wps_hiddenssid2="1",_wps_wep2="1",_wps_enterprise2="1",_wps_wpa_tkip2="1";
	//2.4G ���߲����ύ����
	if(Form.Checkbox('en_wifi'))
	{	
		$H({
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.X_TWSZ-COM_ScheduleListName' : $('sch').value,
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSID'                        : $('ssid').value,
			':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable'                                   : Form.Checkbox('en_wifi'),
			':InternetGatewayDevice.X_TWSZ-COM_Radio.1.OperatingChannelBandwidth'                : $('bw').value,
			':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Standard'                                 : $('wlan_mode').value,
			':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSIDAdvertisementEnabled'    : Form.Radio('invisible_type'),
			'var:menu'      : G_Menu,
			'var:page'      : G_Page,
			'var:errorpage' : G_Page,
			'var:sys_Token' : G_SysToken,
			'getpage'       : 'html/index.html',
			'errorpage'     : 'html/index.html',
			'var:CacheLastData' : ViewState.Save()
		},true);
		
		if(Form.Radio('invisible_type')=="0")
			_wps_hiddenssid1="0";
		
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.FixRate', $('txrate').value);
		
		if(Form.Checkbox('auto_ch') == 1)
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Channel', '0');
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.AutoChannelEnable', Form.Checkbox('auto_ch'));
		}
		else
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Channel', $('channel').value);
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.AutoChannelEnable', Form.Checkbox('auto_ch'));
		}	

		_path = ":InternetGatewayDevice.LANDevice.1.WLANConfiguration.1";
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
				_wps_wep1="0";	
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
							_wps_wpa_tkip1="0";
						}
						else if('AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
							_wps_enterprise1=$('security_type').value == 'wpa_personal'?"1":"0";
						}
						else if('TKIP+AES' == $('cipher_type').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
							_wps_enterprise1=$('security_type').value == 'wpa_personal'?"1":"0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second').value));		
						        } 
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
							}
					}
					
					else if( $('wpa_mode').value == 'WPA2')
					{
						$F(_path + '.BeaconType', 					'11i');
						
						if('TKIP' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
							_wps_wpa_tkip1="0";
						}
						else if('AES' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
							_wps_enterprise1=$('security_type').value == 'wpa_personal'?"1":"0";
						}
						else if('TKIP+AES' == $('cipher_type').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
							_wps_enterprise1=$('security_type').value == 'wpa_personal'?"1":"0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second').value));	
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
					}
					else
					{
						_wps_wpa_tkip1="0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec').value));		
								if($('radius_srv_ip_second').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second').value));
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
				   }	
			  }
		}
	}
	else
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable', '0');
    //5G ���߲����ύ����
	if(Form.Checkbox('en_wifi_Aband'))
	{
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.X_TWSZ-COM_ScheduleListName', $('sch_Aband').value);
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.SSID',                        $('ssid_Aband').value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable',                                   Form.Checkbox('en_wifi_Aband'));
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.OperatingChannelBandwidth',                $('bw_Aband').value==''?'20':$('bw_Aband').value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Standard',                                 $('wlan_mode_Aband').value);

		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.SSIDAdvertisementEnabled',    Form.Radio('invisible_type_Aband'));

		if(Form.Radio('invisible_type_Aband')=="0")
			_wps_hiddenssid2="0";

		if(Form.Checkbox('auto_ch_Aband') == 1)
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Channel', '0');
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.AutoChannelEnable', Form.Checkbox('auto_ch_Aband'));
		}
		else
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Channel', $('channel_Aband').value);
			//$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Channel', '48');
			$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.AutoChannelEnable', Form.Checkbox('auto_ch_Aband'));
		}	
		
		$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.FixRate', $('txrate_Aband').value);

		_path = ":InternetGatewayDevice.LANDevice.1.WLANConfiguration.2";
		switch($('security_type_Aband').value){
			case 'none' : {
				$F(_path + '.BeaconType'             , 'None');
				$F(_path + '.BasicEncryptionModes'   , 'None');
				$F(_path + '.BasicAuthenticationMode', 'None');
				break;
			}
			case 'wep' : {
				_wps_wep2="0";
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
							_wps_wpa_tkip2="0";
						}
						else if('AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
							_wps_enterprise2=$('security_type_Aband').value == 'wpa_personal'?"1":"0";
						}
						else if('TKIP+AES' == $('cipher_type_Aband').value)
						{
							$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
							_wps_enterprise2=$('security_type_Aband').value == 'wpa_personal'?"1":"0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second_Aband').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second_Aband').value));		
						        }
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
							}
					}
					
					else if( $('wpa_mode_Aband').value == 'WPA2')
					{
						$F(_path + '.BeaconType', 					'11i');
						
						if('TKIP' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPEncryption');
							_wps_wpa_tkip2="0";
						}
						else if('AES' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'AESEncryption'); //yll test
							$F(_path + '.IEEE11iEncryptionModes', 		'AESEncryption');
							_wps_enterprise2=$('security_type_Aband').value == 'wpa_personal'?"1":"0";
						}
						else if('TKIP+AES' == $('cipher_type_Aband').value)
						{
							//$F(_path + '.WPAEncryptionModes', 			'TKIPandAESEncryption'); //yll test 
							$F(_path + '.IEEE11iEncryptionModes', 		'TKIPandAESEncryption');
							_wps_enterprise2=$('security_type_Aband').value == 'wpa_personal'?"1":"0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second_Aband').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second_Aband').value));	
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
					}
					
					else
					{
						_wps_wpa_tkip2="0";
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
								$F(_path + '.X_TWSZ-COM_RadiusServer.1.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_Aband').value));		
								if($('radius_srv_ip_second_Aband').value!='')
								{
								$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '1');
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_IPAddress' , $('radius_srv_ip_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_Port'      , $('radius_srv_port_second_Aband').value);
								$F(_path + '.X_TWSZ-COM_RadiusServer.2.X_TWSZ-COM_ShareKey'  , Base64.Encode($('radius_srv_sec_second_Aband').value));
								}
								else
									$F(_path + '.X_TWSZ-COM_BakRadiusEnable' , '0');
						}
				   }	
			  }
		}
	}
	else
		$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable', '0');
	 if(G_Wireless[0][24]=="1")
	 {
		 if(_wps_wpa_tkip1=="0"&&$("en_wifi").checked==true||_wps_wpa_tkip2=="0"&&$("en_wifi_Aband").checked==true)
		 {
			if(confirm(SEcode['lang_wpa_disable_wps'])){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable' , "0");
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WPS.Enable' , "0");
			}else 
				return false;
		}
		if(_wps_enterprise1=="0"&&$("en_wifi").checked==true||_wps_enterprise2=="0"&&$("en_wifi_Aband").checked==true)
		{
			if(confirm(SEcode['lang_wpa_enter_disable_wps'])){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable' , "0");
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WPS.Enable' , "0");
			}else 
				return false;
		}
		if(_wps_wep1=="0"&&$("en_wifi").checked==true||_wps_wep2=="0"&&$("en_wifi_Aband").checked==true)
		{
			if(confirm(SEcode['lang_wep_disable_wps'])){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable' , "0");
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WPS.Enable' , "0");
			}else 
				return false;
		}
		if(_wps_hiddenssid1=="0"&&$("en_wifi").checked==true||_wps_hiddenssid2=="0"&&$("en_wifi_Aband").checked==true)
		{
			if(confirm(SEcode['lang_hidden_ssid_disable_wps'])){
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable' , "0");
				$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WPS.Enable' , "0");
			}else 
				return false;
		}
	 }
	if(Form.Checkbox('en_wifi') && $("security_type").value=="none")
	{
		if(!confirm(SEcode['lang_warn_24_wlan']))
			return false;		
	}
	if(Form.Checkbox('en_wifi_Aband') && $("security_type_Aband").value=="none")
	{		
		if(!confirm(SEcode['lang_warn_5_wlan']))
			return false;		
	}	
	  $H({
        'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:errorpage' : G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'var:CacheLastData' : ViewState.Save()
		//'$-action'    : 'set'
	});
	$F('obj-action', 'set');
    $('uiPostForm').submit();	
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("BWLAN181").disabled = true;
	$("BWLAN185").disabled = true;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];

	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

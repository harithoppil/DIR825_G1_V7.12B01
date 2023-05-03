var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>
var LanHosts = [];
var t = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "HostName MACAddress IPAddress LeaseTimeRemaining VendorClassID AddressSource"
		`	<?if eq `DHCP` `<?echo $26?>` 	
		`	LanHosts[t] = [];
			LanHosts[t][0] = t+1;
			LanHosts[t][1] = "<?echo $21?>"=="ZFc1cmJtOTNiZz09"?"unknown":strAnsi2Unicode((Base64.Decode("<?echo $21?>")));
			LanHosts[t][2] = "<?echo $22?>";
			LanHosts[t][3] = "<?echo $23?>";
			LanHosts[t][4] = "<?echo $24?>";
			++t;
			`?>
		`?>
	`?>
`?>
var G_Blacklist = [];
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.BlackList. "Enable MACAddress SchedulePath"
`	G_Blacklist[m] = [];
	G_Blacklist[m][0] = "$01";
	G_Blacklist[m][1] = "$02";
	G_Blacklist[m][2] = "$03";
	m++;
`?>

var G_Whitelist = [];
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.WhiteList. "Enable MACAddress SchedulePath"
`	G_Whitelist[n] = [];
	G_Whitelist[n][0] = "$01";
	G_Whitelist[n][1] = "$02";
	G_Whitelist[n][2] = "$03";
	n++;
`?>

<?mget :InternetGatewayDevice.X_TWSZ-COM_MAC_Filter. "Enable FilterMode"
   `var G_Enable                  = "$01";//TransmitPower
	var G_FilterMode              = "$02";//BeaconInterval	
`?>
var wps_enable="<?get :InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable?>";
function uiOnload()
{
	var rmd = 0;
	if(G_Enable=="0")
	$('mode').value = "DISABLE";
    else
	$('mode').value = G_FilterMode;	
	OnChangeMode();
	createSchedule();
	createHost(); 
	if($('mode').value == "Allow"){
		for(var i=1; i<=24; i+=1)
		{
			$('en_'+i).checked = G_Whitelist[i-1][0]=="1"?true:false;
			if(G_Whitelist[i-1][0]=="1") rmd++;
			$('mac_'+i).value = G_Whitelist[i-1][1];
			if(G_Whitelist[i-1][2]=="Always")
			$('sch_'+i).value = "Always";
			else
			{
				for(var j=0; j<schedule_num; j+=1)
				{
					if(G_Whitelist[i-1][2]==schedule_list[j][2])
					{
						$('sch_'+i).value = schedule_list[j][0];
						break;
					}
				}
			}
		}
	}
	else{
		for(var i=1; i<=24; i+=1)
		{
			$('en_'+i).checked = G_Blacklist[i-1][0]=="1"?true:false;
			if(G_Blacklist[i-1][0]=="1") rmd++;
			$('mac_'+i).value = G_Blacklist[i-1][1];
			if(G_Blacklist[i-1][2]=="Always")
			$('sch_'+i).value = "Always";
			else
			{
				for(var j=0; j<schedule_num; j+=1)
				{
					if(G_Blacklist[i-1][2]==schedule_list[j][2])
					{
						$('sch_'+i).value = schedule_list[j][0];
						break;
					}
				}
			}
		}
	}
	$('rmd').innerHTML = 24-rmd;
}
function createSchedule(){
		var array_value = [],array_options=[];
		array_value[0]="Always";
		array_options[0]=data_languages.Public.innerHTML.Public005;
		for(var i=1; i<=24; i+=1)
		{
			for(var k = 0; k < schedule_list.length; k++){
				array_value[k+1]=schedule_list[k][0];
				array_options[k+1]=schedule_list[k][0];
			}
			$S('sch_'+i, array_options, array_value);
		}
}
function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;
		for(var i=1; i<=24; i+=1)
		{
			for(var k = 0; k < LanHosts.length; k++){
				array_value[k+1]=LanHosts[k][2];
				array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
			}
			$S('client_list_'+i, array_options, array_value);
		}
}
function OnChangeMode()
{
	/* load table content */
	for(i=1; i<=24; i++)
	{		
		if($("mode").value == "DISABLE")
		{
			$("uid_"+i).disabled	= true;
			$("en_"+i).disabled	= true;
			$("mac_"+i).disabled	= true;
			$("client_list_"+i).disabled     = true;
			$("arrow_"+i).disabled           = true;
			$("sch_"+i).disabled             = true;
			$("schedule_"+i+"_btn").disabled = true;				
		}	
		else
		{
			$("uid_"+i).disabled	= false;
			$("en_"+i).disabled	= false;
			$("mac_"+i).disabled	= false;
			$("client_list_"+i).disabled     = false;
			$("arrow_"+i).disabled           = false;				
			$("sch_"+i).disabled             = false;
			$("schedule_"+i+"_btn").disabled = false;								
		}	
	}
}	
function OnClickArrowKey(index)
{
	var dhcp_client = $("client_list_"+index);

	if (dhcp_client.value == "")
	{
		alert(SEcode['lang_select_machine']);
		return false;
	}
	else
	{
		$("mac_"+index).value = $("client_list_"+index).options[$("client_list_"+index).selectedIndex].value;
		/*for(var k = 0; k < LanHosts.length; k++){
			if(dhcp_client.value==LanHosts[k][1])
			{
				$("mac_"+index).value = LanHosts[k][2];
				break;
			}
		}*/		
	}
}

function uiSubmit()
{
    for (var i=1; i<=24; i+=1)
	{
		if($("mac_"+i).value == "")
		{
			$("en_"+i).checked = false;
		}		
	}
	var i = 0;
	var count=0;
	if($("mode").value == "Deny")
	{	
		$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.Enable' , '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.FilterMode' , 'Deny');
		for(i=1; i<25; i++)
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.BlackList.'+i+'.Enable' , $('en_'+i).checked?"1":"0");
			if($('sch_'+i).value == "Always"){
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.BlackList.'+i+'.SchedulePath' , 'Always');
			}
			else
			{
				for(var k = 0; k < schedule_list.length; k++){	
					if($('sch_'+i).value==schedule_list[k][0])
					{
						$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.BlackList.'+i+'.SchedulePath' , schedule_list[k][2]);
						break;
					}
				}
			}
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.BlackList.'+i+'.MACAddress' , $('mac_'+i).value.toLowerCase());
		}
	}
	else if($("mode").value == "Allow")
	{
		 for (var i=1; i<=24; i+=1)
		{
			if($("mac_"+i).value == "")
			{
				count++;
			}		
		}
		if(count==24)
		{
			if(confirm(SEcode['lang_whitelist_warning']) == false)
			return false;
		}
		$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.Enable' , '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.FilterMode' , 'Allow');
		for(i=1; i<25; i++)
		{
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.WhiteList.'+i+	'.Enable' , $('en_'+i).checked?"1":"0");
			if($('sch_'+i).value == "Always"){
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.WhiteList.'+i+'.SchedulePath' , 'Always');
			}
			else
			{
				for(var k = 0; k < schedule_list.length; k++){	
					if($('sch_'+i).value==schedule_list[k][0])
					{
						$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.WhiteList.'+i+'.SchedulePath' , schedule_list[k][2]);
						break;
					}
				}
			}
			$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.WhiteList.'+i+'.MACAddress' , $('mac_'+i).value.toLowerCase());
		}

	}
	else
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_MAC_Filter.Enable' , '0');
	}	
	
	/*if(wps_enable == "1"&& $("mode").value != "DISABLE")
	{
		if(confirm('To open the Mac filter, WPS must be disabled. Proceed?')){
			$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WPS.Enable' , "0");
			$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WPS.Enable' , "0");
		}else 
			return false;
	}*/
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
	$('Adv_macft002').disabled= true;
	$('Adv_macft013').disabled= true;
}

function dealWithError()
{
     if (G_Error != 1)
     { return false; }

     var arrayHint = [];

     dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,dealWithError);


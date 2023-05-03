var G_PortMapping = [];
var m = 0;

<?objget :InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping. "PortMappingEnabled PortMappingDescription PortMappingProtocol ExternalPort ExternalPortEndRange InternalPort InternalClient PortMappingTime X_TWSZ-COM_InBoundFilter"
`	G_PortMapping[m] = [];
	G_PortMapping[m][0] = "$01";
	G_PortMapping[m][1] = "$02";
	G_PortMapping[m][2] = "$03";
	G_PortMapping[m][3] = "$04";
	G_PortMapping[m][4] = "$05";
	G_PortMapping[m][5] = "$06";
	G_PortMapping[m][6] = "$07";
	G_PortMapping[m][7] = "$08";
	G_PortMapping[m][8] = "$09";
	m++;
`?>
var G_PortForward = [];
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortForward.PortForward. "PortForwardEnabled TcpPortMap UdpPortMap"
`	G_PortForward[n] = [];
	G_PortForward[n][0] = "$01";
	G_PortForward[n][1] = "$02";
	G_PortForward[n][2] = "$03";
	n++;
`?>
var G_PortTrigger = [];
var p = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList. "Enable Description OpenPortMap TriggerPortMap"
`	G_PortTrigger[p] = [];
	G_PortTrigger[p][0] = "$01";
	G_PortTrigger[p][1] = "$02";
	G_PortTrigger[p][2] = "$03";
	G_PortTrigger[p][3] = "$04";
	p++;
`?>
var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>
var G_inbf = [];
var k = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList. "Name IPRange Action"
`	G_inbf[k] = [];
	G_inbf[k][0] = "InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList.$00.";
	G_inbf[k][1] = "$01";
	G_inbf[k][2] = "$02";
	G_inbf[k][3] = "$03";
	k++;
`?>
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
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>
<?mget :InternetGatewayDevice.X_TWSZ-COM_RemoteManage. "RemanageEn RePort ReHttpsEn"
`	var G_RemanageEn 		  = "$01";
	var G_RePort      		  = "$02";
	var G_ReHttpsEn 		  = "$03";
`?>
var admin_remote_port_https = "443";
var apps= [	
			["Application name","TCP", "0",   "0"],
			["TELNET",          "TCP", "23",  "23" ],
			["HTTP",			"TCP", "80",  "80" ],
			["HTTPS",			"TCP", "443", "443" ],
			["FTP",			    "TCP", "21",  "21" ],
			["DNS",			    "UDP", "53",  "53" ],
			["SMTP",			"TCP", "25",  "25" ],
			["POP3",			"TCP", "110", "110" ],
			["H.323",			"TCP", "1720","1720" ],
			["REMOTE DESKTOP",  "TCP", "3389","3389" ],
			["PPTP",			"TCP", "1723","1723" ],
			["L2TP",			"UDP", "1701","1701" ],
			["Wake-On-Lan",	    "UDP", "9",	  "9" ]
		  ];

function uiOnload(){
	var str = "";
	var rmd = 0;
	for(var i=1; i<=24; i+=1)
	{
		str = "";
		str += '<select id="app_'+i+'" class="broad">';
		apps[0][0]=data_languages.Public.innerHTML.Public009;
		for(var j=0; j<apps.length; j+=1)
			str += '<option value="'+j+'">'+apps[j][0]+'</option>';
		str += '</select>';
		$("span_app_"+i).innerHTML = str;
	}
	createSchedule();
	for(var i=1; i<=24; i+=1)
	{
		if(G_PortMapping[i-1][7]=="Always")
			$('sch_'+i).value = "Always";
		else
		{
			for(var j=0; j<schedule_num; j+=1)
			{
				if(G_PortMapping[i-1][7]==schedule_list[j][2])
				{
					$('sch_'+i).value = schedule_list[j][0];
					break;
				}
			}
		}
	}
	createInbound();
	for(var i=1; i<=24; i+=1)
	{
		if(G_PortMapping[i-1][8]=="allowall"||G_PortMapping[i-1][8]=="denyall")
			$('inbfilter_'+i).value = G_PortMapping[i-1][8];
		else
		{
			for(var j=0; j<G_inbf.length; j+=1)
			{
				if(G_PortMapping[i-1][8]==G_inbf[j][0])
				{
					$('inbfilter_'+i).value = G_inbf[j][1];
					break;
				}
			}
		}
	}
	createHost();
	for(var i=1; i<=24; i+=1)
	{
		$('en_'+i).checked = G_PortMapping[i-1][0]=="1"?true:false;
		if(G_PortMapping[i-1][0]=="1") rmd++;
		$('dsc_'+i).value = G_PortMapping[i-1][1];
		$('pro_'+i).value = G_PortMapping[i-1][2];
		$('priport_'+i).value = G_PortMapping[i-1][5];
		$('ip_'+i).value = G_PortMapping[i-1][6];
		$('pubport_'+i).value = G_PortMapping[i-1][3];
		OnClickProtocal(i);
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
function createInbound(){
		var array_value = [],array_options=[];
		array_value[0]="allowall";
		array_options[0]=data_languages.Public.innerHTML.Public006;
		array_value[1]="denyall";
		array_options[1]=data_languages.Public.innerHTML.Public007;
		for(var i=1; i<=24; i+=1)
		{
			for(var k = 0; k < G_inbf.length; k++){
				array_value[k+2]=G_inbf[k][1];
				array_options[k+2]=G_inbf[k][1];
			}
			$S('inbfilter_'+i, array_options, array_value);
		}
}
function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;
		for(var i=1; i<=24; i+=1)
		{
			for(var k = 0; k < LanHosts.length; k++){
				array_value[k+1]=LanHosts[k][3];
				array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
			}
			$S('pc_'+i, array_options, array_value);
		}
}
function OnClickAppArrow(idx)
{
	var i = $("app_"+idx).value;
	$("dsc_"+idx).value = (i==="0") ? "" : apps[i][0];
	$("pro_"+idx).value = (apps[i][1]==="") ? "TCP/UDP" : apps[i][1];
	$("pubport_"+idx).value	= (apps[i][2]==="") ? "0" : apps[i][2];
	$("priport_"+idx).value	= (apps[i][3]==="") ? "0" : apps[i][3];
	OnClickProtocal(idx);
}
function OnClickPCArrow(idx)
{
	if($("pc_"+idx).value=="")
		$("ip_"+idx).value ="";
	else
	{
		$("ip_"+idx).value = $("pc_"+idx).options[$("pc_"+idx).selectedIndex].value;	

		/*for(var k = 0; k < LanHosts.length; k++){
			if($("pc_"+idx).value==LanHosts[k][1])
			{
				$("ip_"+idx).value = LanHosts[k][3];
				break;
			}
		}	*/	
	}
}
function OnClickProtocal(idx)
{
	var pro_value = $("pro_"+idx).value;
	if(pro_value=="Other")
	{
		$("pubport_"+idx).value = "";
		$("priport_"+idx).value = "";
		$("pubport_"+idx).disabled = true;
		$("priport_"+idx).disabled = true;
		$("pronum_"+idx).disabled = false;
		$("pronum_"+idx).value = "";
	}
	else
	{
		$("pubport_"+idx).disabled = false;
		$("priport_"+idx).disabled = false;
		$("pronum_"+idx).disabled = true;
		if(pro_value=="TCP")
			$("pronum_"+idx).value = "6";
		else if(pro_value=="UDP")
			$("pronum_"+idx).value = "17";
		else if(pro_value=="TCP/UDP")
			$("pronum_"+idx).value = "256";
	}
}
function CheckPortConflict(type,index)
{
	for(var j=0;j<24;j++)
	{
		if( G_PortForward[j][0] == "1" && ($(type+index).value == G_PortForward[j][1] || $(type+index).value == G_PortForward[j][2]) )
		{
			if(type=="pubport_")
				alert(SEcode['lang_public_port_conflict_pf']);
			else if(type=="priport_")
				alert(SEcode['lang_private_port_conflict_pf']);
			return false;	
		}
		if( G_PortTrigger[j][0] == "1" && ($(type+index).value == G_PortTrigger[j][2] || $(type+index).value == G_PortTrigger[j][3]) )
		{
			if(type=="pubport_")
				alert(SEcode['lang_public_port_conflict_pt']);
			else if(type=="priport_")
				alert(SEcode['lang_private_port_conflict_pt']);
			return false;	
		}
	}
	return true;
}
function uiSubmit(){
	var i = 0;
	for (i=1; i<=24; i+=1)
	{
		if($("pubport_"+i).value == "" && $("priport_"+i).value == "" && $("ip_"+i).value == "" && $("dsc_"+i).value == "")
		{
			$("en_"+i).checked = false;
		}
		else
		{
			if ($("pubport_"+i).value == "" || isNaN($("pubport_"+i).value) || Number($("pubport_"+i).value) < 1 || Number($("pubport_"+i).value) > 65535)
			{
				alert(SEcode['lang_public_port']);
				$("pubport_"+i).focus();
				return false;
			}
			if (!CheckPortConflict("pubport_",i))
			{
				//alert(SEcode['lang_public_port_conflict_pf']);
				$("pubport_"+i).focus();
				return false;
			}
			if ($("priport_"+i).value == "" || isNaN($("priport_"+i).value) || Number($("priport_"+i).value) < 1 || Number($("priport_"+i).value) > 65535)
			{
				alert(SEcode['lang_private_port']);
				$("priport_"+i).focus();
				return false;
			}
			if (!CheckPortConflict("priport_",i))
			{
				//alert(SEcode['lang_private_port_conflict_pf']);
				$("priport_"+i).focus();
				return false;
			}
			if ($("ip_"+i).value == "" || !checkipaddr($("ip_"+i).value) || !isSameSubNet($("ip_"+i).value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_host_ip']);
				$("ip_"+i).focus();
				return false;
			}
			if ($("ip_"+i).value == G_IPAddress)
			{
				alert(SEcode['lang_ip_not_same']);
				$("ip_"+i).focus();
				return false;
			}
			if($("dsc_"+i).value == "")
			{
				alert(SEcode['lang_host_name']);
				$("dsc_"+i).focus();
				return false;
			}	
			for (var j=1; j < i; j+=1)
			{
				if($("dsc_"+i).value==$("dsc_"+j).value && $("dsc_"+i).value!="") 
				{
					alert(SEcode['lang_name_not_same']);
					$("dsc_"+j).focus();
					return false;
				}	
				if($("pubport_"+i).value==$("pubport_"+j).value && $("pubport_"+i).value!="") 
				{
					alert(SEcode['lang_public_port_not_same']);
					$("pubport_"+j).focus();
					return false;
				}																				
			}	
			if($("pubport_"+i).value!="" && ($("pro_"+i).value=="TCP" || $("pro_"+i).value=="TCP/UDP"))
			{
				if(G_RemanageEn==1)
				{				
					if($("pubport_"+i).value==G_RePort || ($("pubport_"+i).value==admin_remote_port_https && G_ReHttpsEn==1))
					{
						alert(SEcode['lang_public_port_conflict']);
						$("pubport_"+i).focus();
						return false;
					}	
				}				
			}		
		}
	}
	for(i=1; i<25; i++)
	{		
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.PortMappingEnabled' , $('en_'+i).checked?"1":"0");
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.PortMappingDescription' , $('dsc_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.PortMappingProtocol' , $('pro_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.ExternalPort' , $('pubport_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.ExternalPortEndRange' , $('pubport_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.InternalPort' , $('priport_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.InternalClient' , $('ip_'+i).value);
		if($('sch_'+i).value == "Always"){
			$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.PortMappingTime' , 'Always');
		}
		else
		{
			for(var k = 0; k < schedule_list.length; k++){	
				if($('sch_'+i).value==schedule_list[k][0])
				{
					$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.PortMappingTime' , schedule_list[k][2]);
					break;
				}
			}
		}
		if($('inbfilter_'+i).value == "allowall" || $('inbfilter_'+i).value == "denyall"){
			$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.X_TWSZ-COM_InBoundFilter' , $('inbfilter_'+i).value);
		}
		else
		{
			for(var k = 0; k < G_inbf.length; k++){
				if($('inbfilter_'+i).value==G_inbf[k][1])
				{
					$F(':InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping.'+i+'.X_TWSZ-COM_InBoundFilter' , G_inbf[k][0]);
					break;
				}
			}
		}
	}
	
	
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
	$('Adv_vt002').disabled= true;
	$('Adv_vt248').disabled= true;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,dealWithError);

var G_PortTrigger = [];
var m = 0;

<?objget :InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList. "Enable Description OpenPortMap TriggerPortMap TriggerProtocol OpenProtocol PortTriggerTime"
`	G_PortTrigger[m] = [];
	G_PortTrigger[m][0] = "$01";
	G_PortTrigger[m][1] = "$02";
	G_PortTrigger[m][2] = "$03";
	G_PortTrigger[m][3] = "$04";
	G_PortTrigger[m][4] = "$05";
	G_PortTrigger[m][5] = "$06";
	G_PortTrigger[m][6] = "$07";
	m++;
`?>
var apps = [	
			[ "Application Name",	"TCP/UDP"	,"TCP/UDP",  "",		 ""],
			[ "AIM Talk",		    "TCP"		,"TCP",      "4099",	 "5190"],
			[ "BitTorrent",		    "TCP"		,"TCP",      "6969",	 "6881-6889"],
			[ "Calista IP Phone",	"TCP"		,"UDP",      "5190",	 "3000"],
			[ "ICQ",	            "UDP"		,"TCP",      "4000",     "20000,20019,20039,20059"],
			[ "PalTalk",		    "TCP"		,"TCP/UDP",  "5001-5020","2090,2091,2095"]			
		  ];
		  
var G_PortForward = [];
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortForward.PortForward. "PortForwardEnabled TcpPortMap UdpPortMap"
`	G_PortForward[n] = [];
	G_PortForward[n][0] = "$01";
	G_PortForward[n][1] = "$02";
	G_PortForward[n][2] = "$03";
	n++;
`?>

var G_PortMapping = [];
var p = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping. "PortMappingEnabled ExternalPort InternalPort"
`	G_PortMapping[p] = [];
	G_PortMapping[p][0] = "$01";
	G_PortMapping[p][1] = "$02";
	G_PortMapping[p][2] = "$03";
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
<?mget :InternetGatewayDevice.X_TWSZ-COM_RemoteManage. "RemanageEn RePort ReHttpsEn"
`	var G_RemanageEn 		  = "$01";
	var G_RePort      		  = "$02";
	var G_ReHttpsEn 		  = "$03";
`?>
var admin_remote_port_https = "443";
function uiOnload(){
   var str = "";
   	var rmd = 0;
	for(var i=1; i<=24; i+=1)
	{
		str = "";
		str += '<select id="app_'+i+'">';
		apps[0][0]=data_languages.Public.innerHTML.Public009;
		for(var j=0; j<this.apps.length; j+=1)
			str += '<option value="'+j+'">'+this.apps[j][0]+'</option>';
		str += '</select>';
		$("span_app_"+i).innerHTML = str;
	}
	createSchedule();
	for(var i=1; i<=24; i+=1)
	{
		if(G_PortTrigger[i-1][6]=="Always")
			$('sch_'+i).value = "Always";
		else
		{
			for(var j=0; j<schedule_num; j+=1)
			{
				if(G_PortTrigger[i-1][6]==schedule_list[j][2])
				{
					$('sch_'+i).value = schedule_list[j][0];
					break;
				}
			}
		}
	}
	for(var i=1; i<=24; i+=1)
	{
		$('en_'+i).checked = G_PortTrigger[i-1][0]=="1"?true:false;
		if(G_PortTrigger[i-1][0]=="1") rmd++;
		$('name_'+i).value = G_PortTrigger[i-1][1];
		$('priport_'+i).value = G_PortTrigger[i-1][3];
		$('pripro_'+i).value = G_PortTrigger[i-1][4];
		$('pubport_'+i).value = G_PortTrigger[i-1][2];
		$('pubpro_'+i).value = G_PortTrigger[i-1][5];
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
function OnClickAppArrow(idx)
{
	var i = $("app_"+idx).value;
	
	if( i == "0" )
	{
		alert(SEcode['lang_select_name']);
		return false;
	}
	
	$("name_"+idx).value		= apps[i][0];
	$("pubpro_"+idx).value	= apps[i][1];
	$("pripro_"+idx).value	= apps[i][2];
	$("pubport_"+idx).value	= apps[i][4];
	$("priport_"+idx).value	= apps[i][3];
}
function CheckPort(port)
{
	var vals = port.toString().split("-");
	switch (vals.length)
	{
	case 1:
		if (!TEMP_IsDigit(vals))
			return false;
		break;
	case 2:
		if (!TEMP_IsDigit(vals[0])||!TEMP_IsDigit(vals[1]))
			return false;
		break;
	default:
		return false;
	}
	return true;
}
function check_trigger_port(list)
{
	var port = list.split(",");

	if (port.length > 1)  //  Trigger port can have just one port_range;
		return false;
	else
		return CheckPort(port);
}
function TEMP_IsDigit(no)
{
	if (no==""||no==null)
		return false;
	if (no.toString()!=parseInt(no, 10).toString())
		return false;

	return true;
}
function check_public_port(list)
{
	var port = list.split(",");

	if (port.length > 1)
	{
		for (var i=0; i<port.length; i++)
		{
			if (!CheckPort(port[i]))
				return false;
		}
		return true;
	}
	else
	{
		return CheckPort(port);
	}
}
function PortStringCheck(PortString1, PortString2)
{
	var PortStrArr1 = PortString1.split(",");
	var PortStrArr2 = PortString2.split(",");
	for(var i=0; i < PortStrArr1.length; i++)
	{
		for(var j=0; j < PortStrArr2.length; j++)
		{
			if(PortStrArr1[i].match("-")=="-" && PortStrArr2[j].match("-")=="-")
			{
				var PortRange1 = PortStrArr1[i].split("-");
				var PortRangeStart1	= parseInt(PortRange1[0], 10);
				var PortRangeEnd1	= parseInt(PortRange1[1], 10);
				var PortRange2 = PortStrArr2[j].split("-");
				var PortRangeStart2	= parseInt(PortRange2[0], 10);
				var PortRangeEnd2	= parseInt(PortRange2[1], 10);
				if(PortRangeStart2 <= PortRangeEnd1 &&  
					PortRangeStart1 <= PortRangeEnd2) return true;
			}	
			else if(PortStrArr1[i].match("-")=="-")
			{
				var PortRange1 = PortStrArr1[i].split("-");
				var PortRangeStart1	= parseInt(PortRange1[0], 10);
				var PortRangeEnd1	= parseInt(PortRange1[1], 10);
				if(PortRangeStart1 <= parseInt(PortStrArr2[j], 10) && 
					parseInt(PortStrArr2[j], 10) <= PortRangeEnd1) return true;
			}
			else if(PortStrArr2[j].match("-")=="-")
			{
				var PortRange2 = PortStrArr2[j].split("-");
				var PortRangeStart2	= parseInt(PortRange2[0], 10);
				var PortRangeEnd2	= parseInt(PortRange2[1], 10);
				if(PortRangeStart2 <= parseInt(PortStrArr1[i], 10) && 
					parseInt(PortStrArr1[i], 10) <= PortRangeEnd2) return true;
			}
			else
			{
				if(parseInt(PortStrArr1[i], 10)==parseInt(PortStrArr2[j], 10)) return true;
			}					
		}
	}
	return false;
}
function CheckPortConflict(type,index)
{
	for(var j=0;j<24;j++)
	{
		if( G_PortMapping[j][0] == "1" && ($(type+index).value == G_PortMapping[j][1] || $(type+index).value == G_PortMapping[j][2]) )
		{
			if(type=="priport_")
				alert(SEcode['lang_trigger_port_conflict_vs']);
			else if(type=="pubport_")
				alert(SEcode['lang_firewall_port_conflict_vs']);
			return false;	
		}
		if( G_PortForward[j][0] == "1" && ($(type+index).value == G_PortForward[j][1] || $(type+index).value == G_PortForward[j][2]) )
		{
			if(type=="priport_")
				alert(SEcode['lang_trigger_port_conflict_pf']);
			else if(type=="pubport_")
				alert(SEcode['lang_firewall_port_conflict_pf']);
			return false;	
		}
	}
	
	return true;
}
function uiSubmit(){
	
	var i = 0;
	for (var i=1; i<=24; i+=1)
	{
		if($("priport_"+i).value == "" && $("pubport_"+i).value == "" && $("name_"+i).value == "")
		{
			$("en_"+i).checked = false;
		}
		else
		{	
			if ($("priport_"+i).value=="")
			{
				alert(SEcode['lang_trigger_port']);
				$("priport_"+i).focus();
				return false;
			}
			if ($("priport_"+i).value!="" && !check_trigger_port($("priport_"+i).value))
			{
				alert(SEcode['lang_trigger_port_invalid']);
				$("priport_"+i).focus();
				return false;
			}
			if (!CheckPortConflict("priport_",i))
			{
				$("priport_"+i).focus();
				return false;
			}
			if ($("pubport_"+i).value=="")
			{
				alert(SEcode['lang_firewall_port']);
				$("pubport_"+i).focus();
				return false;
			}
			if ($("pubport_"+i).value!="" && !check_public_port($("pubport_"+i).value))
			{
				alert(SEcode['lang_firewall_port_invalid']);
				$("pubport_"+i).focus();
				return false;
			}
			if (!CheckPortConflict("pubport_",i))
			{
				$("pubport_"+i).focus();
				return false;
			}
			if($("name_"+i).value == "")
			{
				alert(SEcode['lang_host_name']);
				$("name_"+i).focus();
				return false;
			}
			for(j=1; j < i; j++)
			{
				var name = $("name_"+i).value;
				if( $("name_"+i).value == $("name_"+j).value )
				{
					alert(SEcode['lang_name']+name+SEcode['lang_already_used']);
					$("name_"+i).focus();
					return false;
				}	
				if( $("priport_"+i).value == $("priport_"+j).value &&
					$("pripro_"+i).value == $("pripro_"+j).value   &&
					$("pubport_"+i).value == $("pubport_"+j).value &&
					$("pubpro_"+i).value == $("pubpro_"+j).value &&
					$("sch_"+i).value == $("sch_"+j).value	  )
				{
					alert(SEcode['lang_rule']+name+SEcode['lang_already_used']);
					$("priport_"+i).focus();
					return false;
				}				
			}
			if($("pubport_"+i).value!="" && ($("pubpro_"+i).value=="TCP" || $("pubpro_"+i).value=="TCP/UDP"))
			{
				if(G_RemanageEn==1)
				{				
					if(PortStringCheck($("pubport_"+i).value, G_RePort) || (PortStringCheck($("pubport_"+i).value, admin_remote_port_https) && G_ReHttpsEn==1))
					{
						alert(SEcode['lang_firewall_port_conflict']);
						$("pubport_"+i).focus();
						return false;
					}	
				}				
			}	
		}
	}
	for(i=1; i<25; i++)
	{		
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.Enable' , $('en_'+i).checked?"1":"0");
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.Description' , $('name_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.OpenPortMap' , $('pubport_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.TriggerPortMap' , $('priport_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.TriggerProtocol' , $('pripro_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.OpenProtocol' , $('pubpro_'+i).value);	
		if($('sch_'+i).value == "Always"){
			$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.PortTriggerTime' , 'Always');
		}
		else
		{
			for(var k = 0; k < schedule_list.length; k++){	
				if($('sch_'+i).value==schedule_list[k][0])
				{
					$F(':InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList.'+i+'.PortTriggerTime' , schedule_list[k][2]);
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
	$('Adv_app002').disabled= true;
	$('Adv_app153').disabled= true;
}

function dealWithError(){
         if (G_Error != 1){ return false; }
         var arrayHint = [];
         dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

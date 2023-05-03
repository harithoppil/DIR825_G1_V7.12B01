var G_ACLWanEnable="<?get :InternetGatewayDevice.X_TWSZ-COM_ACL.RACLEnable?>";
var G_ACLWanGroup = []
var WanCount = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_ACL.RACL.1.Service. "Name Enable Schedule SrcIPList Method WebFilterEn WebFilter PortFilterEn"
`    <?objget :InternetGatewayDevice.X_TWSZ-COM_ACL.RACL.1.Service.$10.PortFilter. "Name Enable DesIP DesIPEnd Protocol DestPortStart DestPortEnd"
	`	G_ACLWanGroup[WanCount] = [];
		G_ACLWanGroup[WanCount][0] = "InternetGatewayDevice.X_TWSZ-COM_ACL.RACL.1.Service.$10."; //Path
		G_ACLWanGroup[WanCount][1] = "$11"; //Name
		G_ACLWanGroup[WanCount][2] = "$12"; //enable
		G_ACLWanGroup[WanCount][3] = "$13"; //Schedule
		G_ACLWanGroup[WanCount][4] = "$14"; //SrcIPList
		G_ACLWanGroup[WanCount][5] = "$15"; //Method
		G_ACLWanGroup[WanCount][6] = "$16"; //WebFilterEn
		G_ACLWanGroup[WanCount][7] = "$17"; //WebFilter
		G_ACLWanGroup[WanCount][8] = "$18"; //PortFilterEn
		G_ACLWanGroup[WanCount][9] = "$01"; //Name
		G_ACLWanGroup[WanCount][10] = "$02"; //enable
		G_ACLWanGroup[WanCount][11] = "$03"; //DesIP
		G_ACLWanGroup[WanCount][12] = "$04"; //DesIPEnd
		G_ACLWanGroup[WanCount][13] = "$05"; //Protocol
		G_ACLWanGroup[WanCount][14] = "$06"; //DestPortStart
		G_ACLWanGroup[WanCount][15] = "$07"; //DestPortEnd
		WanCount++;
	`?>
`?>	
var stages=["access_main", "access_descript", "access_name", "access_schedule", "access_machine", "access_filter_meth", "access_port_filter", "access_web_logging"];
var currentStage=0;
var max_policy_num = 120;
var max_machine_num = 8; 
var policy_num = WanCount;
var Medit="-";
var edit="-";
var Mdelete="-";
var Mtype = "";
var Mvalue = "";
var mothed 		= "";
var webfilter 	= "";
var weblog   	= "";				
var portfilter	= "";
//****************************************************
var G_IPAddress = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.IPInterfaceIPAddress?>";
var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable SelectDays StartTime EndTime"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_list[schedule_num][3] = "$03";	//SelectDays
	 schedule_list[schedule_num][4] = "$04";	//StartTime
	 schedule_list[schedule_num][5] = "$05";	//EndTime
	 schedule_num++;
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
var G_CurrentIP = "<?echo $var:sys_RemoteAddr ?>";
var G_CurrentMAC = GetMACByIP(G_CurrentIP).toUpperCase();
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>
//MAC Address Clone
function GetMACByIP(ip){
	for (var i = 0; i < LanHosts.length; i++){
		if (LanHosts[i][3] == ip){
			return LanHosts[i][2];
		}
	}

	return "";
}

function ctrlEnable(index){
    _node = $('Enable_' + index,'IP_' + index,'Mask_' + index);
    if (_node[0].checked == true){
        _node[1].disabled = false;
        _node[2].disabled = false;
    }else{
        _node[1].disabled = true;
        _node[2].disabled = true;
    }
    
}

function uiOnload()
{
		createSchedule();
		createHost();
		$('en_access').checked = G_ACLWanEnable=="1"?true:false;
		OnClickEnACCESS();		
		StageClose();
		StageMain();
}

function createHost(){
		var array_value = [],array_options=[];
		var array_value1 = [],array_options1=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;
		array_value1[0]="";
		array_options1[0]=data_languages.Public.innerHTML.Public008;
		for(var k = 0; k < LanHosts.length; k++){
			array_value[k+1]=LanHosts[k][3];
			array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
		}
		for(var k = 0; k < LanHosts.length; k++){
			array_value1[k+1]=LanHosts[k][2];
			array_options1[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
		}
		
		$S('MachineIPSelect', array_options, array_value);
		$S('MachineMACSelect', array_options1, array_value1);
}

function createSchedule(){
	var array_value = [],array_options=[];
	array_value[0]="Always";
	array_options[0]=data_languages.Public.innerHTML.Public005;
	for(var k = 0; k < schedule_list.length; k++){
		array_value[k+1]=schedule_list[k][0];
		array_options[k+1]=schedule_list[k][0];
	}	
	$S('sch_select', array_options, array_value);
}
function OnClickPrev()
{
	StageClose();
	var	stage = stages[currentStage];
	if(stage == "access_web_logging" && $("PortFilterCheck").checked != "1")	currentStage-=2;
	else	currentStage--;
	stage = stages[currentStage];	
	ShowCurrentStage(stage);
}	
function OnClickNext()
{
	if(StageCheck() == false) return;
	StageClose();
	var	stage = stages[currentStage];
	if((stage == "access_main" && edit != "-") || 
		(stage == "access_filter_meth" && $("PortFilterCheck").checked != true && $("WebFilterCheck").checked == "1"))	currentStage+=2;
	else	currentStage++;
	stage = stages[currentStage];
	ShowCurrentStage(stage);
}
function ShowCurrentStage(stage)
{
	switch (stage)
	{
	case "access_descript":
		StageDescript();
		break;
	case "access_name":
		StageName();
		break;
	case "access_schedule":
		StageSchedule();
		break;
	case "access_machine":
		Mvalue = edit=="-"?Mvalue:G_ACLWanGroup[edit][4];
		StageMachine();
		break;	
	case "access_filter_meth":
		StageFliterMeth();
		break;
	case "access_port_filter":
		StagePortFliter();
		break;	
	case "access_web_logging":
		StageWebLogging();
		break;		
	default:
	}		
}	

function OnClickCancel()
{
	uiPageRefresh();
}
function TEMP_IsDigit(no)
{
	if (no==""||no==null)
		return false;
	if (no.toString()!=parseInt(no, 10).toString())
		return false;

	return true;
}
function COMM_IPv4ADDR2INT(addr)
{
	var nums;
	var vals = new Array();
	var val;

	nums = addr.split(".");
	vals[0] = (parseInt(nums[0], [10]) % 256);
	vals[1] = (parseInt(nums[1], [10]) % 256);
	vals[2] = (parseInt(nums[2], [10]) % 256);
	vals[3] = (parseInt(nums[3], [10]) % 256);
	val = vals[0];
	val = val*256 + vals[1];
	val = val*256 + vals[2];
	val = val*256 + vals[3];
	return val;
}
function StageCheck()
{	
	var	stage = stages[currentStage];
	if(stage == "access_main")
	{
	}	
	else if(stage == "access_descript")
	{
		
	}
	else if(stage == "access_name")
	{
		if($("policyname").value == "")
		{			
			alert(SEcode['lang_policy_name_empty']);
			return false;
		}
		for(var i=0; i<WanCount/8; i++)
		{
			if(edit != "-")
			{			
				if(($("policyname").value == G_ACLWanGroup[8*i][1])&&edit/8!=i)
				{			
					alert(SEcode['lang_policy_name_same']);
					return false;
				}	
			}
			else
			{
				if(($("policyname").value == G_ACLWanGroup[8*i][1]))
				{			
					alert(SEcode['lang_policy_name_same']);
					return false;
				}	
			}
		}
	}
	else if(stage == "access_schedule")
	{
	}
	else if(stage == "access_machine")
	{
		var src=Mvalue.split(",");	
		if (src == "")
		{
			alert(SEcode['lang_enter_machine']);
			return false;
			
		}
	}	
	else if(stage == "access_filter_meth")
	{
		if($("LOGWEBONLY").checked)	
		{
			mothed 		= "LOGWEBONLY";
			webfilter 	= "0";
			weblog 	= "1";				
			portfilter	= "0";
		}	
		else if($("BLOCKALL").checked)
		{	
			mothed 		= "BLOCKALL";
			webfilter 	= "0";
			weblog 	= "0";				
			portfilter	= "0";
		}	
		else
		{		
			mothed = "BLOCKSOME";
			if($("WebFilterCheck").checked && $("PortFilterCheck").checked)			
			{
				webfilter 	= "1";
				portfilter 	= "1";
			}
			else if($("WebFilterCheck").checked && !$("PortFilterCheck").checked)	
			{
				webfilter	= "1";
				portfilter 	= "0";
			}
			else if(!$("WebFilterCheck").checked && $("PortFilterCheck").checked)	
			{
				webfilter 	= "0";
				weblog 	= "0";
				portfilter 	= "1";
			}	
			else if(!$("WebFilterCheck").checked && !$("PortFilterCheck").checked)	
			{
				alert(SEcode['lang_select_filter']);
				return false;
			}	
		}	
	}
	else if(stage == "access_port_filter")
	{
		if(	$("filter_name1").value == "" &&
			$("filter_name2").value == "" &&
			$("filter_name3").value == "" &&
			$("filter_name4").value == "" &&
			$("filter_name5").value == "" &&
			$("filter_name6").value == "" &&
			$("filter_name7").value == "" &&
			$("filter_name8").value == "")
		{
			alert(SEcode['lang_name_all_empty']);
			return false;	
		}
			
		for(var i=1; i<=8; i++)
		{
			
			if($("filter_name"+i).value != "")
			{
				for(var j=1; j<=8; j++)
				{
					if($("filter_name"+i).value == $("filter_name"+j).value && i!=j) 
					{	
						alert(SEcode['lang_name_same']);
						return false;
					}							
				}	
			
				var FSIP = $("filter_startip"+i).value;
				var FEIP = $("filter_endip"+i).value;
				var FSIP_val = FSIP.split(".");
				var FEIP_val = FEIP.split(".");
				if (FSIP == "" || !checkipaddr(FSIP))
				{
					alert(SEcode['lang_destip_start']);
					$("filter_startip"+i).focus();
					return false;
				}
				if (isSameSubNet(FSIP,G_SubMask,G_IPAddress,G_SubMask))
				{
					alert(SEcode['lang_start_ip_not_same']);
					$("filter_startip"+i).focus();
					return false;
				}
				if (FEIP == "" || !checkipaddr(FEIP))
				{
					alert(SEcode['lang_destip_end']);
					$("filter_endip"+i).focus();
					return false;
				}
				if (isSameSubNet(FEIP,G_SubMask,G_IPAddress,G_SubMask))
				{
					alert(SEcode['lang_end_ip_not_same']);
					$("filter_endip"+i).focus();
					return false;
				}				
				if(COMM_IPv4ADDR2INT(FSIP) > COMM_IPv4ADDR2INT(FEIP))
				{
					alert(SEcode['lang_endip_greater_startip']);
					$("filter_startip"+i).focus();
					return false;
				}	
					
				var FSPort = $("filter_startport"+i).value;
				var FEPort = $("filter_endport"+i).value;	
				if($("filter_protocol"+i).value == "TCP" || $("filter_protocol"+i).value == "UDP")
				{ 
					if(!TEMP_IsDigit(FSPort) || FSPort < 1 || FSPort > 65535)
					{
						alert(SEcode['lang_start_port']);
						$("filter_startport"+i).focus();
						return false;
					}
					if(!TEMP_IsDigit(FEPort) || FEPort < 1 || FEPort > 65535)
					{
						alert(SEcode['lang_end_port']);
						$("filter_endport"+i).focus();
						return false;
					}	
					if(parseInt(FSPort, 10) > parseInt(FEPort, 10))
					{
						alert(SEcode['lang_endport_greater_startport']);
						$("filter_startport"+i).focus();
						return false;
					}			
				}
			}
		}
	}
	else if(stage == "access_web_logging")
	{
		if($("WebLogEnabled").checked)	weblog = "1";
		else	weblog = "0";
	}
	return true;		
}
function StageClose()
{
	$("access_main").style.display = "none";
	$("help_hint").style.display = "none";
	$("mainbody").className = "menubody";
	$("access_descript").style.display = "none";
	$("access_name").style.display = "none";
	$("access_schedule").style.display = "none";
	$("access_machine").style.display = "none";
	$("access_filter_meth").style.display = "none";
	$("access_port_filter").style.display = "none";
	$("access_web_logging").style.display = "none";	
}
function StageMain()
{
	currentStage = 0;
	$("access_main").style.display = "block";
	$("help_hint").style.display = "block";
	$("mainbody").className = "mainbody";		
}
function StageDescript()
{
	$("access_descript").style.display = "block";
	Set4ButtonDisabled(true, false, true, false);
}
function StageName()
{
	$("access_name").style.display = "block";
	if(edit != "-")	Set4ButtonDisabled(true, false, false, false);
	else	Set4ButtonDisabled(false, false, true, false);		
	if(edit != "-")	$("policyname").value = G_ACLWanGroup[edit][1];
	else	$("policyname").value = "";
}
function StageSchedule()
{
	$("access_schedule").style.display = "block";
	if(edit != "-")	Set4ButtonDisabled(false, false, false, false);
	else	Set4ButtonDisabled(false, false, true, false);		
	if(edit != "-")
	{
		 	
		if(G_ACLWanGroup[edit][3]=="Always")
			$('sch_select').value = data_languages.Public.innerHTML.Public005;
		else
		{
			for(var j=0; j<schedule_num; j+=1)
			{
				if(G_ACLWanGroup[edit][3]==schedule_list[j][2])
				{
					$('sch_select').value = schedule_list[j][0];
					break;
				}
			}
		}		
		OnClickSchSelect($('sch_select').value);
	}
	else	$("sch_detail").value = data_languages.Public.innerHTML.Public005;
}
function StageMachine()
{
	$("access_machine").style.display = "block";
	if(edit != "-")	Set4ButtonDisabled(false, false, false, false);
	else	Set4ButtonDisabled(false, false, true, false);
	OnClickMachineType("IP");
	InsectMachineTable();
	Medit = "-";
	$("machine_submit").value = data_languages.Public.innerHTML.Public016;
}
function StageFliterMeth()
{
	$("access_filter_meth").style.display = "block";
	if(edit == "-")	OnClickFilterMethod("LOGWEBONLY");
	else	OnClickFilterMethod(G_ACLWanGroup[edit][5]);
}
function StagePortFliter()
{
				
	$("access_port_filter").style.display = "block";
	if($("PortFilterCheck").checked == true && $("WebFilterCheck").checked == true)
	{
		if(edit != "-")	Set4ButtonDisabled(false, false, false, false);
		else	Set4ButtonDisabled(false, false, true, false);
	}
	else if($("PortFilterCheck").checked == true && $("WebFilterCheck").checked == false)	Set4ButtonDisabled(false, true, false, false);
			
	for(var i=1; i<=8; i++)
	{
		$("filter_enable"+i).checked 	= false;
		$("filter_name"+i).value 		= "";
		$("filter_startip"+i).value 	= "0.0.0.0";
		$("filter_endip"+i).value 	= "255.255.255.255";
		$("filter_protocol"+i).value 	= "ALL";
		$("filter_startport"+i).value = 1;
		$("filter_endport"+i).value 	= 65535;	
		OnClickProtocol(i);	
	}
	if(edit != "-")
	{		
		var k=1;
		for(var j=0; j<8; j++)
		{			
			$("filter_enable"+k).checked 	= G_ACLWanGroup[edit+j][10]=="1"?true:false;
			$("filter_name"+k).value 		= G_ACLWanGroup[edit+j][9];
			$("filter_startip"+k).value 	= G_ACLWanGroup[edit+j][11];
			$("filter_endip"+k).value 	= G_ACLWanGroup[edit+j][12];
			$("filter_protocol"+k).value 	= G_ACLWanGroup[edit+j][13];
			$("filter_startport"+k).value = G_ACLWanGroup[edit+j][14];
			$("filter_endport"+k).value 	= G_ACLWanGroup[edit+j][15];
			OnClickProtocol(k);
			k++;
		}	
	}	
}
function StageWebLogging()
{
	$("access_web_logging").style.display = "block";
	Set4ButtonDisabled(false, true, false, false);
	if(edit == "-")
	{
		if(weblog == "1")	OnClickWebLogging("enable");
		else	OnClickWebLogging("disable");
	}
	else
	{
		if(G_ACLWanGroup[edit][7] == "1")	OnClickWebLogging("enable");
		else	OnClickWebLogging("disable");
	}
}
function OnClickEnACCESS()
{
	if($("en_access").checked)
	{	
		$("add_policy").style.display = "block";
		$("policytableframe").style.display = "block";
		InsectPolicyTable();
	}
	else
	{		
		$("policytableframe").style.display = "none";
		$("add_policy").style.display = "none";
	}
	if(parseInt(policy_num, 10) < parseInt(max_policy_num, 10)) $("add_policy").disabled = false;
	else $("add_policy").disabled = true;
}	
function InsectMachineTable()
{
	Table.Clear('machinetable');	
	var array_value = [];
	var k=0;
	var src=Mvalue.split(",");	
	if(Medit!="-")
	{		
		src.splice(Medit,1,src[src.length-1]);
		src.splice(src.length-1,1);
	}	
	if(Mvalue!="")
	{
		Mvalue="";
		for(var i=0; i<src.length; i+=1)
		{			
			array_value[i] = [src[i],
				'<img onclick="OnMachineEdit('+k+');" src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit"/>',
				'<img onclick="OnMachineDelete('+k+');" src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete"/>'
				];
			k++;	
			if(i<src.length-1)
				Mvalue=Mvalue+src[i]+",";
		    else
				Mvalue=Mvalue+src[i];
		}
		$T("machinetable", array_value);
	}
}
function InsectPolicyTable()
{
	Table.Clear("policytable");
	var array_value = [];
	for (var i=0; i<WanCount; i+=8)
	{
		var Filtering = "";
		var Logged = "";
		var Schedule = "";
		if(G_ACLWanGroup[i][5] == "LOGWEBONLY")	Filtering = data_languages.Adv_acl.innerHTML.Adv_acl051;
		else if(G_ACLWanGroup[i][5] == "BLOCKALL")	Filtering = data_languages.Adv_acl.innerHTML.Adv_acl052;
		else	Filtering = data_languages.Adv_acl.innerHTML.Adv_acl053;
		if(G_ACLWanGroup[i][7] == "1")	Logged = data_languages.Public.innerHTML.Public014;	
		else	Logged =data_languages.Public.innerHTML.Public015;	
		if(G_ACLWanGroup[i][3]=="Always") Schedule = data_languages.Public.innerHTML.Public005;
		else
		{
			for(var j=0; j<schedule_num; j+=1)
			{
				if(G_ACLWanGroup[i][3]==schedule_list[j][2])
				{
					Schedule = schedule_list[j][0];
					break;
				}
			}
		}
		array_value[i] = [G_ACLWanGroup[i][2]=="1"?'<input type="checkbox" id="Ptable'+i+'+_check_0" name="Ptable_check" checked>':'<input type="checkbox" id="Ptable'+i+'+_check_0" name="Ptable_check">', G_ACLWanGroup[i][1], G_ACLWanGroup[i][4], Filtering, Logged, Schedule,
			'<img onclick="OnPolicyEdit('+i+');" src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit"/>',
			'<img onclick="OnPolicyDelete('+i+');" src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete" id="delete_'+i+'"/>'
			];		
	}
	$T("policytable", array_value);
}
function MachineString(i)
{
	var note="", d="";
	for (var k=0; k<Mac.length; k+=1)
	{
		if (pol[i][0] === Mac[k][0]) 
		{
			note+=d+Mac[k][1];
			d=",";
		}
	}	
	return note;
}
function OnClickAddPolicy()
{		
	OnClickNext();
}	
function OnClickSchSelect(sch_uid)
{
	if(sch_uid == "Always")	$("sch_detail").value = data_languages.Public.innerHTML.Public005;
	else
	{
		for(var i=0;i<schedule_num;i++)
		{
			if(sch_uid==schedule_list[i][0])
			{
				$("sch_detail").value = schedule_list[i][3].substring(0,schedule_list[i][3].length-1)+" "+schedule_list[i][4]+'~'+schedule_list[i][5];
				break;
			}
		}
	}		
}	
function OnClickMachineType(Mtype)
{
	//alert(Mtype);
	if (Mtype == "IP")
	{
		$("MIP").checked = true;
		$("MMAC").checked = false;
		//$("MOthers").checked = false;
		$("MachineIP").disabled = false;
		$("MachineIPSelect").disabled = false;
		$("MachineMAC").disabled = true;
		$("MachineMACSelect").disabled = true;
		$("ipv4_mac_button").disabled = true;
	}
	else //if (Mtype == "MAC")
	{
		$("MIP").checked = false;
		$("MMAC").checked = true;
		//$("MOthers").checked = false;
		$("MachineIP").disabled = true;
		$("MachineIPSelect").disabled = true;
		$("MachineMAC").disabled = false;
		$("MachineMACSelect").disabled = false;
		$("ipv4_mac_button").disabled = false;
	}		
	/*else
	{	
		$("MIP").checked = false;
		$("MMAC").checked = false;
		$("MOthers").checked = true;
		$("MachineIP").disabled = true;
		$("MachineIPSelect").disabled = true;
		$("MachineMAC").disabled = true;
		$("MachineMACSelect").disabled = true;
		$("ipv4_mac_button").disabled = true;
	}*/			
}	
function OnClickMachineIPSelect(IP)
{
	if(IP != "")
	{
		$("MachineIP").value = $("MachineIPSelect").options[$("MachineIPSelect").selectedIndex].value;
		/*for(var k = 0; k < LanHosts.length; k++){
			if($("MachineIPSelect").value==LanHosts[k][1])
			{
				$("MachineIP").value = LanHosts[k][3];
				break;
			}
		}*/
	}
	else	$("MachineIP").value = "";
}	
function OnClickMachineMACSelect(MAC)
{
	if(MAC != "")
	{
		$("MachineMAC").value = $("MachineMACSelect").options[$("MachineMACSelect").selectedIndex].value;
		/*for(var k = 0; k < LanHosts.length; k++){
			if($("MachineMACSelect").value==LanHosts[k][1])
			{
				$("MachineMAC").value = LanHosts[k][2];
				break;
			}
		}*/
	}
	else	$("MachineMAC").value = "";
}	
function OnClickMACButton()
{
	$("MachineMAC").value=G_CurrentMAC;
}
function OnClickMachineSubmit()
{	
	Mtype = "";
	Mvalue = edit=="-"?Mvalue:G_ACLWanGroup[edit][4];
	var machine_num=1;	
	var src;
	var src=Mvalue.split(",")		
	for(var i=0; i<src.length; i++)
	{
		machine_num++;
		if(machine_num > parseInt(max_machine_num, 10))
		{
			alert(SEcode['lang_max_machines']);
			return false;			
		}
	}
	
	if($("MIP").checked)
	{
		
		if(G_IPAddress == $("MachineIP").value)
		{
			alert(SEcode['lang_ip_not_same']);
			return false;
		}			
		if($("MachineIP").value == "" || !checkipaddr($("MachineIP").value))
		{
			alert(SEcode['lang_invalid_ip']);
			return false;
		}
		if (!isSameSubNet($("MachineIP").value,G_SubMask,G_IPAddress,G_SubMask))
		{
			alert(SEcode['lang_ip_be_lan']);
			$("MachineIP").focus();
			return false;
		}
		for(var i=0; i<src.length; i++)
		{
			if($("MachineIP").value==src[i])
			{
				alert(SEcode['lang_machine_conflict']);
				return false;			
			}
		}
		Mtype 	= "IP";
		if(Mvalue!="")
		Mvalue 	= Mvalue+","+$("MachineIP").value;
		else
		Mvalue 	= $("MachineIP").value;
	}	
	else if($("MMAC").checked)
	{
		var mac = CheckMAC($("MachineMAC").value);

		if (mac=="")
		{
			alert(UEcode[1017]);
			return false;
		}
		for(var i=0; i<src.length; i++)
		{
			if(mac==src[i])
			{
				alert(SEcode['lang_machine_conflict']);
				return false;			
			}
		}
					
		Mtype 	= "MAC";
		if(Mvalue!="")
		Mvalue 	= Mvalue+","+mac;
		else
		Mvalue 	= mac;	
			
	}
	else
	{		
		Mtype 	= "OTHERMACHINES";
		if(Mvalue!="")
		Mvalue 	= Mvalue+","+"Other Machines";	
		else
		Mvalue 	= "Other Machines";	
	} 

	OnClickMachineCancel();
	StageMachine();
}	
function OnClickMachineCancel()
{
	OnClickMachineType("IP");
	$("MachineIP").value 	= "";
	$("MachineIPSelect").value 	= "";
	$("MachineMAC").value = "";
	$("MachineMACSelect").value = "";
}
function OnClickFilterMethod(method)
{
	if(method == "LOGWEBONLY")
	{
		$("LOGWEBONLY").checked = true;
		$("BLOCKALL").checked = false;
		$("BLOCKSOME").checked = false;
		$("WebFilter").style.display = "none";
		$("PortFilter").style.display = "none";
		Set4ButtonDisabled(false, true, false, false);
	}	
	else if(method == "BLOCKALL")
	{
		$("LOGWEBONLY").checked = false;
		$("BLOCKALL").checked = true;
		$("BLOCKSOME").checked = false;
		$("WebFilter").style.display = "none";
		$("PortFilter").style.display = "none";
		Set4ButtonDisabled(false, true, false, false);			
	}		
	else
	{
		$("LOGWEBONLY").checked = false;
		$("BLOCKALL").checked = false;
		$("BLOCKSOME").checked = true;
		$("WebFilter").style.display = "block";
		$("PortFilter").style.display = "block";
		Set4ButtonDisabled(false, false, true, false);
		if(edit == "-")
		{
			if(webfilter == "1")	$("WebFilterCheck").checked = true;	
			if(portfilter == "1")	$("PortFilterCheck").checked = true;	
		}
		else
		{
			if(G_ACLWanGroup[edit][6]=="1") $("WebFilterCheck").checked = true;
			if(G_ACLWanGroup[edit][8]=="1") $("PortFilterCheck").checked = true;			
		}								
	}			
}	
function OnClickProtocol(i)
{
	if($("filter_protocol"+i).value == "ALL" || $("filter_protocol"+i).value == "ICMP")
	{
		$("filter_startport"+i).disabled = true;
		$("filter_endport"+i).disabled = true;
	}
	else if($("filter_protocol"+i).value == "TCP" || $("filter_protocol"+i).value == "UDP")
	{
		$("filter_startport"+i).disabled = false;
		$("filter_endport"+i).disabled = false;
	}			
}	
function OnClickWebLogging(enable)
{
	if(enable == "disable")	
	{
		$("WebLogDisabled").checked = true;
		$("WebLogEnabled").checked = false;
	}	
	else
	{
		$("WebLogDisabled").checked = false;
		$("WebLogEnabled").checked = true;			
	}		
}	
function OnPolicyEdit(i)
{
	edit = i;
	OnClickNext();
}
function OnPolicyDelete(i)
{
	if(!confirm(SEcode[1001])){
		return false;
	}
	$H({
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'obj-action'    : 'del',
		'del-obj'       : G_ACLWanGroup[i][0],
		'var:CacheLastData'	: ViewState.Save()
	},true);
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_acl002').disabled= true;
	$('Adv_acl013').disabled= true;	
	for(var i=0; i<WanCount/8; i++)
	{
		$('delete_'+i).disabled= true;
	}
}
function OnMachineEdit(i)
{
	Medit = i;
	var src=Mvalue.split(",")
	for(var j=0; j<src.length; j++)
	{	
		if(i == j)
		{
			// if(src[i]=="Other Machines")			
			// OnClickMachineType("Others");
			// else 
			if(src[i].indexOf(".")>-1)
			OnClickMachineType("IP");
			else
			OnClickMachineType("MAC");	
			if(src[i].indexOf(".")>-1) $("MachineIP").value = src[i];
			else if(src[i].indexOf(":")>-1 ) $("MachineMAC").value = src[i];
			break;	
		}		
	}			
	$("machine_submit").value = data_languages.Public.innerHTML.Public011;
}
function OnMachineDelete(i)
{
	var src=Mvalue.split(",")
	for(var j=0; j<src.length; j++)
	{
		if(i == j)
		{	
			src.splice(j,1);
			break;
		}
	}
	Mvalue="";
	for(var j=0; j<src.length; j++)
	{
		if(j<src.length-1)
			Mvalue=Mvalue+src[j]+",";
		else
			Mvalue=Mvalue+src[j];
	}
	StageMachine();
	Mdelete="1";
}
function SetButtonDisabled(name, disable)
{
	var button = document.getElementsByName(name);
	for (var i=0; i<button.length; i++)
	{
		button[i].disabled = disable;
	}
}
function Set4ButtonDisabled(prev, next, save, cancel)
{
	SetButtonDisabled("b_prev", prev);
	SetButtonDisabled("b_next", next);
	SetButtonDisabled("b_save", save);
	SetButtonDisabled("b_cancel", cancel);
}
function CheckMAC(m)
{
	var myMAC="";
	if (m.search(":") != -1)	var tmp=m.split(":");
	else				var tmp=m.split("-");
	if (m == "" || tmp.length != 6)	return "";

	for (var i=0; i<tmp.length; i++)
	{
		if (tmp[i].length==1)	tmp[i]="0"+tmp[i];
		else if (tmp[i].length==0||tmp[i].length>2)	return "";
	
		tmp[i]=tmp[i].toLowerCase();
      
		for(var j=0; j<tmp[i].length; j++)
		{
			var c = "0123456789abcdef";
			var str_hex=0;		
			for(var k=0; k<c.length; k++)	if(tmp[i].substr(j,1)==c.substr(k,1))	{str_hex=1;break;}
			if(str_hex==0) return "";	
			
		}		
	}
	
	myMAC = tmp[0];
    
    if (tmp[0]%2==1)
    {
    	//marco, check for multicast address
       return "";
    }

	for (var i=1; i<tmp.length; i++)
	{
		myMAC = myMAC + ':' + tmp[i];
	}
	if(myMAC=="ff:ff:ff:ff:ff:ff" || myMAC=="01:11:11:11:11:11"  || myMAC=="00:00:00:00:00:00")	return "";	
	
	return myMAC;
}

function uiSubmit()
{   
	var action = 0;
   if(StageCheck() == false) return false;
   if(edit == "-")
   {
		var _Path = "";
		$H({	
			'obj-action'    : 'add-set',
			'add-obj'       : 'InternetGatewayDevice.X_TWSZ-COM_ACL.RACL.1.Service.'
		});		
		$F(":" + _Path + 'Enable',       '1');
		action = 1;
	}
	else
	{
		var _Path = G_ACLWanGroup[edit][0];
		$F('obj-action', 'set');	
		if(Mvalue==""&&Mdelete=="-")
			Mvalue=G_ACLWanGroup[edit][4];
		if(portfilter=="")
			portfilter=G_ACLWanGroup[edit][8];
		if(webfilter=="")
			webfilter=G_ACLWanGroup[edit][6];
		if(weblog=="")
			weblog=G_ACLWanGroup[edit][7];
		
		$F(":" + _Path + 'Enable',    $('Ptable'+edit+'+_check_0').checked?"1":"0");
		action = 0;
	}
	
	$F(":" + _Path + 'Name'         , $('policyname').value);
	$F(":" + _Path + 'SrcIPList'    , Mvalue);
	$F(":" + _Path + 'Method'       , Form.Radio("method"));
	$F(":" + _Path + 'PortFilterEn' , portfilter);
	$F(":" + _Path + 'WebFilterEn'  , webfilter);
	$F(":" + _Path + 'WebFilter'	, weblog);
	if($('sch_select').value == "Always"){
		$F(":" + _Path + 'Schedule' , 'Always');
	}
	else
	{
		for(var k = 0; k < schedule_list.length; k++){	
			if($('sch_select').value==schedule_list[k][0])
			{
				$F(":" + _Path + 'Schedule' , schedule_list[k][2]);
				break;
			}
		}
	}
	if(portfilter=="1"){
	for(var i = 1; i <= 8; i++){		
		$F(":" + _Path + 'PortFilter.'+i+'.Enable' , $('filter_enable'+i).checked?"1":"0");
		$F(":" + _Path + 'PortFilter.'+i+'.Name' , $('filter_name'+i).value);
		$F(":" + _Path + 'PortFilter.'+i+'.DesIP' , $('filter_startip'+i).value);
		$F(":" + _Path + 'PortFilter.'+i+'.DesIPEnd' , $('filter_endip'+i).value);
		$F(":" + _Path + 'PortFilter.'+i+'.Protocol' , $('filter_protocol'+i).value);
		$F(":" + _Path + 'PortFilter.'+i+'.DestPortStart' , $('filter_startport'+i).value);
		$F(":" + _Path + 'PortFilter.'+i+'.DestPortEnd' , $('filter_endport'+i).value);
	}}

	if(G_ACLWanEnable == "0" && (action == 1))	
	{
		$H({
			'var:menu'     : G_Menu,
			'var:page'     : G_Page,
			'var:sys_Token' : G_SysToken,
			'getpage'      : 'html/page/portforwd.ajax.js',
			'errorpage'    : 'html/page/portforwd.ajax.js',
			'var:errorpage': G_Page
		});
		var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js&var:page=*";
		G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
		G_ajax.post($('uiPostForm'));		
	}
	else
	{
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
	}
	
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_acl079').disabled= true;
	$('Adv_acl058').disabled= true;
	$('Adv_acl046').disabled= true;
	$('Adv_acl033').disabled= true;
	$('Adv_acl026').disabled= true;
	$('Adv_acl087').disabled= true;
}

function uiSubmitEn()
{
   $F(':InternetGatewayDevice.X_TWSZ-COM_ACL.RACLEnable', $('en_access').checked? '1':'0');
   $F('obj-action' ,'set');
	for(var i=0; i<WanCount; i+=8)
	{
		var _Path = G_ACLWanGroup[i][0];
		$F(":" + _Path + 'Enable',    $('Ptable'+i+'+_check_0').checked?"1":"0");
	}
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
	$('Adv_acl002').disabled= true;
	$('Adv_acl013').disabled= true;	
	for(var i=0; i<WanCount/8; i++)
	{
		$('delete_'+i).disabled= true;
	}
}
function null_errorfunc()
{

	return true;
}

function Ajax_handler(_text)
{
	try{
		eval(_text);
	}catch(e){
		alert("G_Error="+G_Error);
		uiPageRefresh();
		dealWithError();
		return;
	}

	G_SysToken = G_AjaxToken;
	if(G_Error == '1')
	{
		$("menu").style.display="";
		$("content").style.display="";
		$("mbox").style.display="none";
		dealWithError();
		$('Adv_acl079').disabled= false;
		$('Adv_acl058').disabled= false;
		$('Adv_acl046').disabled= false;
		$('Adv_acl033').disabled= false;
		$('Adv_acl026').disabled= false;
		$('Adv_acl087').disabled= false;
			
	}	
	else
	{
		$H({
			'obj-action'   : 'set',
			'var:menu'     : G_Menu,
			'var:page'     : G_Page,
			'var:sys_Token' : G_SysToken,
			'getpage'      : 'html/index.html',
			'errorpage'    : 'html/index.html',
			'var:errorpage': G_Page,
			'var:CacheLastData': ViewState.Save()
		},true);
	
	   $F(':InternetGatewayDevice.X_TWSZ-COM_ACL.RACLEnable', '1');

		$('uiPostForm').submit();
	}
}


function dealWithError()
{
     if (G_Error != 1)
     { return false; }

     var arrayHint = [];

     dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,dealWithError);


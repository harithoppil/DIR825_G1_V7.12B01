/*  Javascript Document:Bas_lan.js  */
/*  实例默认 3-6下行 7-10上行  */
var G_QosQueue = [];
var G_QoSRule = [];
var n = 0, m = 0;
<?objget :InternetGatewayDevice.QueueManagement.Queue. "QueueEnable SchedulerAlgorithm QueueWeight X_TWSZ-COM_EnableDSCPMark X_TWSZ-COM_EnableCOSMark ShapingRate"
`	G_QosQueue[n] = [];
	G_QosQueue[n][0] = "InternetGatewayDevice.QueueManagement.Queue.$00.";
	G_QosQueue[n][1] = "$01";
	G_QosQueue[n][2] = "$02";
	G_QosQueue[n][3] = "$03";
	G_QosQueue[n][4] = "$04";
	G_QosQueue[n][5] = "$05";
	G_QosQueue[n][6] = "$06";
	n++;
`?>
<?objget :InternetGatewayDevice.QueueManagement.Classification. "ClassificationEnable ClassInterface DestMACAddress DestIP DestMask DestPort DestPortRangeMax SourceMACAddress SourceIP SourceMask SourcePort SourcePortRangeMax Protocol VLANIDCheck DSCPCheck EthernetPriorityCheck DSCPMark EthernetPriorityMark ClassQueue ClassName DestPort DestPortRangeMax"
`	G_QoSRule[m] = [];
	G_QoSRule[m][0] = "InternetGatewayDevice.QueueManagement.Classification.$00.";
	G_QoSRule[m][1] 	= "$01";		//ClassificationEnable
	G_QoSRule[m][2] 	= "$02"          //ClassInterface
	G_QoSRule[m][3] 	= "$03"		//DestMACAddress
	G_QoSRule[m][4] 	= "$04";		//DestIP
	G_QoSRule[m][5] 	= "$05";		//DestMask
	G_QoSRule[m][6] 	= "$06";		//DestPort
	G_QoSRule[m][7] 	= "$07";		//DestPortRangeMax
	G_QoSRule[m][8] 	= "$08";		//SourceMACAddress
	G_QoSRule[m][9] 	= "$09";		//SourceIP
	G_QoSRule[m][10] 	= "$0a";		//SourceMask
	G_QoSRule[m][11] 	= "$0b";		//SourcePort
	G_QoSRule[m][12] 	= "$0c";		//SourcePortRangeMax
	G_QoSRule[m][13] 	= "$0d";		//Protocol
	G_QoSRule[m][14] 	= "$0e";		//VLANIDCheck
	G_QoSRule[m][15] 	= "$0f";		//DSCPCheck
	G_QoSRule[m][16] 	= "$0g";		//EthernetPriorityCheck
	G_QoSRule[m][17] 	= "$0h";		//DSCPMark
	G_QoSRule[m][18] 	= "$0i";		//EthernetPriorityMark
	G_QoSRule[m][19] 	= "$0j";		//ClassQueue
	G_QoSRule[m][20] 	= "$0k";		//ClassName
	G_QoSRule[m][21] 	= "$0l";		//DestPort
	G_QoSRule[m][22] 	= "$0m";		//DestPortRangeMax
	m++;
`?>

<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>	
	
var G_edit= -1;
var G_QoSEnable = "<?get :InternetGatewayDevice.QueueManagement.Enable?>";
//加载初始势
function uiOnload(){
	Form.Radio("qtype", G_QosQueue[0][2]);
	setJSONValue({
			"en_qos" 				: G_QoSEnable,
			"upstream" 				: G_QosQueue[0][6],
			"downstream" 			: G_QosQueue[1][6],
			"priority_dsc1" 		: G_QosQueue[2][3],
			"priority_dsc2" 		: G_QosQueue[3][3],
			"priority_dsc3" 		: G_QosQueue[4][3],
			"priority_dsc4" 		: G_QosQueue[5][3]			
		});
	
	createListTb();
	OnClickQOSEnable();
	OnClickQtype(G_QosQueue[0][2]);
}
function OnClickQOSEnable()
{
	var feature_china = 0;
	/*add a message to alert Traffic Control is already enabled,
	  because Traffic Control and QoS Engine could not enable in the same time.*/
	if (feature_china =="1")
	{
		if (!confirm('The Traffic Control is already enabled, and this two function are the same. Do you want to disable Traffic Control to use QoS Engine?')) 
		{	
			$("en_qos").checked = false;
			$("en_qos").disabled = true;
		}
	}
	
	if ($("en_qos").checked)
	{
		$("upstream").disabled = $("select_upstream").disabled = false;
		$("downstream").disabled = $("select_downstream").disabled = false;
		$("Qtype_SPQ").disabled = $("Qtype_WFQ").disabled =false;
		$("queue_table").disabled = false;
		if($("Qtype_WFQ").checked) $("priority_dsc1").disabled = $("priority_dsc2").disabled = $("priority_dsc3").disabled = $("priority_dsc4").disabled = false;
		$("qos_table").disabled =false;
		//$("en").disabled =	$("desc").disabled = $("select_pri").disabled = $("select_pro").disabled = $("select_pro").disabled =  $("select_classify").disabled = false;
		//$("src_startip").disabled = $("src_endip").disabled = false;
		//$("dst_startip").disabled = $("dst_endip").disabled = false;
		//$("app_port_start").disabled = $("app_port_end").disabled = false;
		//$("select_app_port").disabled = $("qossubmit").disabled = $("qoscancel").disabled = false;
		/*for (var i=1; i<=32; i+=1)
		{
			$("en_"+i).disabled =	$("dsc_"+i).disabled = $("pri_"+i).disabled = $("pro_"+i).disabled = $("select_pro_"+i).disabled = false;
			$("src_startip_"+i).disabled = $("src_endip_"+i).disabled = false;
			$("dst_startip_"+i).disabled = $("dst_endip_"+i).disabled = false;
			$("app_port_"+i).disabled = $("select_app_port_"+i).disabled = false;
		}*/	
	}
	else
	{
		$("upstream").disabled = $("select_upstream").disabled = true;
		$("downstream").disabled = $("select_downstream").disabled = true;
		$("Qtype_SPQ").disabled = $("Qtype_WFQ").disabled =true;
		$("queue_table").disabled = true;
		if($("Qtype_WFQ").checked) $("priority_dsc1").disabled = $("priority_dsc2").disabled = $("priority_dsc3").disabled = $("priority_dsc4").disabled = true;
		$("qos_table").disabled =true;
		$("en").disabled =	$("desc").disabled = $("select_pri").disabled = $("select_pro").disabled = $("select_pro").disabled = $("select_classify").disabled = true;
		$("src_startip").disabled = $("src_endip").disabled = true;
		$("dst_startip").disabled = $("dst_endip").disabled = true;
		$("app_port_start").disabled = $("app_port_end").disabled = true;
		$("select_app_port").disabled = $("qossubmit").disabled = $("qoscancel").disabled = true;
		/*for (var i=1; i<=32; i+=1)
		{
			$("en_"+i).disabled =	$("dsc_"+i).disabled = $("pri_"+i).disabled = $("pro_"+i).disabled = $("select_pro_"+i).disabled = true;	
			$("src_startip_"+i).disabled = $("src_endip_"+i).disabled = true;
			$("dst_startip_"+i).disabled = $("dst_endip_"+i).disabled = true;
			$("app_port_"+i).disabled = $("select_app_port_"+i).disabled = true;					
		}*/
	}
}
function OnChangeQOSUpstream()
{
	$("upstream").value = $("select_upstream").value;
	$("select_upstream").value=0;
}
function OnChangeQOSDownstream()
{
	$("downstream").value = $("select_downstream").value;
	$("select_downstream").value=0;
}
function OnClickQtype(Qtype)
{
	if(Qtype == "SP")
	{	
		$("priority").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos053;		
		$("priority1").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos055;
		$("priority2").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos056;
		$("priority3").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos057;
		$("priority4").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos058;
	}
	else
	{
		$("priority").innerHTML = data_languages.Adv_qos.innerHTML.Adv_qos054;
		$("priority1").innerHTML = '<input id="priority_dsc1" type="text" size="3" maxlength="3" />%';
		$("priority2").innerHTML = '<input id="priority_dsc2" type="text" size="3" maxlength="3" />%';
		$("priority3").innerHTML = '<input id="priority_dsc3" type="text" size="3" maxlength="3" />%';
		$("priority4").innerHTML = '<input id="priority_dsc4" type="text" size="3" maxlength="3" />%';				
		//var bwcqd1p="40",bwcqd2p="30",bwcqd3p="20",bwcqd4p="10";
		$("priority_dsc1").value	=	G_QosQueue[2][3];
		$("priority_dsc2").value	=	G_QosQueue[3][3];
		$("priority_dsc3").value	=	G_QosQueue[4][3];
		$("priority_dsc4").value	=	G_QosQueue[5][3];			
	}
}
function OnChangeProt(idx)
{
	$("pro_"+idx).value	=	$("select_pro_"+idx).value;
}
function OnChangeAppPort(idx)
{
	$("app_port_"+idx).value	=	$("select_app_port_"+idx).value;
	this.OnChangeAppPortInput(idx);
}

var _desPortS = _desPortE = "";
var _pro = "-1";
function onChgAPP(xValue){
	$('select_pro').disabled = false;
	switch(xValue){
		case "sip" :
			_desPortS = "5060";
			_desPortE = "5060";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "h323" :
			_desPortS = "1719";
			_desPortE = "1720";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "mgcp" :
			_desPortS = "2427";
			_desPortE = "2427";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "snmp" :
			_desPortS = "161";
			_desPortE = "161";
			_pro = "0";	
			$('select_pro').disabled = true;
			break;
		case "dns" :
			_desPortS = "53";
			_desPortE = "53";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "dhcp" :
			_desPortS = "67";
			_desPortE = "67";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "rip" :
			_desPortS = "502";
			_desPortE = "502";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "rstp" :
			_desPortS = "554";
			_desPortE = "554";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "rtcp" :
			_desPortS = "5005";
			_desPortE = "5005";
			_pro = "17";
			$('select_pro').disabled = true;
			break;
		case "rtp" :
			_desPortS = "5004";
			_desPortE = "5004";
			_pro = "17";
			$('select_pro').disabled = true;				
			break;
		default :
			break;
	}
	
	setJSONValue({
		"app_port_start" 		: _desPortS,
		"app_port_end" 		: _desPortE,
		"select_pro" 		: _pro
	});
}
function OnChangeAppPortInput(idx)
{
	if($("app_port_"+idx).value=="YOUTUBE" || $("app_port_"+idx).value=="FTP" || $("app_port_"+idx).value=="HTTP"
		|| $("app_port_"+idx).value=="HTTP_AUDIO" || $("app_port_"+idx).value=="HTTP_VIDEO" || $("app_port_"+idx).value=="HTTP_DOWNLOAD")
	{
		$("pro_"+idx).value = "TCP";
		$("pro_"+idx).disabled = true;
		$("select_pro_"+idx).disabled = true;			
	}
	else if($("app_port_"+idx).value=="P2P")
	{
		$("pro_"+idx).value = "ALL";
		$("pro_"+idx).disabled = true;
		$("select_pro_"+idx).disabled = true;
	}	
	else	 	
	{
		$("pro_"+idx).disabled = false;
		$("select_pro_"+idx).disabled = false;			
	}	
}
function createListTb(){
	var array_value = [];
	Table.Clear('qostable');	
	for(var i = 0, _len = G_QoSRule.length; i < _len; i++){
		var pri="",pro="";
		if(G_QoSRule[i][19]=="7" || G_QoSRule[i][19]=="3") pri=data_languages.Adv_qos.innerHTML.Adv_qos055;
		else if(G_QoSRule[i][19]=="8" || G_QoSRule[i][19]=="4") pri=data_languages.Adv_qos.innerHTML.Adv_qos056;
		else if(G_QoSRule[i][19]=="9" || G_QoSRule[i][19]=="5") pri=data_languages.Adv_qos.innerHTML.Adv_qos057;
		else pri=data_languages.Adv_qos.innerHTML.Adv_qos059;
		if(G_QoSRule[i][13]=="6") pro=data_languages.Adv_qos.innerHTML.Adv_qos024;
		else if(G_QoSRule[i][13]=="17") pro=data_languages.Adv_qos.innerHTML.Adv_qos025;
		else pro=data_languages.Adv_qos.innerHTML.Adv_qos023;
		array_value[i] = [G_QoSRule[i][1]=="1"?data_languages.Public.innerHTML.Public014:data_languages.Public.innerHTML.Public015,
					  G_QoSRule[i][20],
					  pri,							  
					  pro,
					  G_QoSRule[i][2]=="LAN"?data_languages.Adv_qos.innerHTML.Adv_qos027:data_languages.Adv_qos.innerHTML.Adv_qos028,
					  G_QoSRule[i][9]+"-"+G_QoSRule[i][10],							  
					  G_QoSRule[i][6]+"-"+G_QoSRule[i][7],
					  G_QoSRule[i][4]+"-"+G_QoSRule[i][5],
					  '<img id="edit_' + i + '" src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit" onclick="uiEdit(' + i + ')"/>',
					  '<img id="delete_' + i + '" src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete" onclick="uiDelete(' + i + ')"/>'				  
					  ];
	}
	$T('qostable', array_value);
}

//提交配置
function uiSubmit(){
	$H({
	   	":InternetGatewayDevice.QueueManagement.Enable" : $('en_qos').checked ? '1':'0',
	   	"obj-action" 		: "set",
		'var:menu'     : G_Menu,
		'var:sys_Token' : G_SysToken,
		'var:page'     : G_Page,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:errorpage': G_Page,
		'var:CacheLastData': ViewState.Save()
	}, true);
	
	var _scheduler   = Form.Radio('qtype');

	if($('en_qos').checked)
	{

		if($('upstream').value == "0")
		{
			alert(SEcode['lang_uplink_speed']);
			$("upstream").focus();
			return false;
		}
		if($('downstream').value == "0")
		{
			alert(SEcode['lang_downlink_speed']);
			$("downstream").focus();
			return false;
		}
	//download
	
        $F(":InternetGatewayDevice.QueueManagement.Queue.2.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.2.SchedulerAlgorithm", _scheduler);
		$F(":InternetGatewayDevice.QueueManagement.Queue.2.ShapingRate",	 	$('downstream').value);
		//$F(":InternetGatewayDevice.QueueManagement.Queue.2.X_TWSZ-COM_EnableDSCPMark",	Form.Checkbox('CHB_DSCPMark'));
		//$F(":InternetGatewayDevice.QueueManagement.Queue.2.X_TWSZ-COM_EnableCOSMark",	Form.Checkbox('CHB_8021pMark'));
		$F(":InternetGatewayDevice.QueueManagement.Queue.7.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.7.SchedulerAlgorithm", _scheduler);
		$F(":InternetGatewayDevice.QueueManagement.Queue.7.X_TWSZ-COM_EnableForceWeight", 0);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.7.QueueWeight",	 $('priority_dsc1').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.8.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.8.SchedulerAlgorithm", _scheduler);
		$F(":InternetGatewayDevice.QueueManagement.Queue.8.X_TWSZ-COM_EnableForceWeight", 0);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.8.QueueWeight",	 $('priority_dsc2').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.9.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.9.SchedulerAlgorithm", _scheduler);
		$F(":InternetGatewayDevice.QueueManagement.Queue.9.X_TWSZ-COM_EnableForceWeight", 0);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.9.QueueWeight",	 $('priority_dsc3').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.10.QueueEnable",	 '1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.10.SchedulerAlgorithm", _scheduler);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.10.QueueWeight",	 $('priority_dsc4').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.10.X_TWSZ-COM_EnableForceWeight", 0);
	//uplink
        $F(":InternetGatewayDevice.QueueManagement.Queue.1.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.1.SchedulerAlgorithm", _scheduler);
		$F(":InternetGatewayDevice.QueueManagement.Queue.1.ShapingRate", 		$('upstream').value);
		//$F(":InternetGatewayDevice.QueueManagement.Queue.1.X_TWSZ-COM_EnableDSCPMark",	Form.Checkbox('CHB_DSCPMark'));
		//$F(":InternetGatewayDevice.QueueManagement.Queue.1.X_TWSZ-COM_EnableCOSMark",	Form.Checkbox('CHB_8021pMark'));
		$F(":InternetGatewayDevice.QueueManagement.Queue.3.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.3.SchedulerAlgorithm", _scheduler);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.3.QueueWeight",	 $('priority_dsc1').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.4.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.4.SchedulerAlgorithm", _scheduler);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.4.QueueWeight",	 $('priority_dsc2').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.5.QueueEnable", 	'1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.5.SchedulerAlgorithm", _scheduler);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.5.QueueWeight",	 $('priority_dsc3').value);
		$F(":InternetGatewayDevice.QueueManagement.Queue.6.QueueEnable",	 '1');
		$F(":InternetGatewayDevice.QueueManagement.Queue.6.SchedulerAlgorithm", _scheduler);
		if(_scheduler == 'WRR')
			$F(":InternetGatewayDevice.QueueManagement.Queue.6.QueueWeight",	 $('priority_dsc4').value);
	}
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_qos002').disabled= true;
	$('Adv_qos049').disabled= true;
	$('qossubmit').disabled= true;
	for(var i=0; i<G_QoSRule.length; i++)
	{
		$('delete_'+i).disabled= true;
	}
}
function uiEdit(_i){
	if(G_QoSEnable=="0")
	return false;
	$("en").checked = G_QoSRule[_i][1]=="1"?true:false;
	$("desc").value =  G_QoSRule[_i][20];	
	$("select_pri").value = G_QoSRule[_i][2]=="LAN"?G_QoSRule[_i][19]:Number(G_QoSRule[_i][19])-4; 
	$("select_pro").value =  G_QoSRule[_i][13];
	$("src_startip").value = G_QoSRule[_i][9];
	$("src_endip").value = G_QoSRule[_i][10];
	$("dst_startip").value =  G_QoSRule[_i][4];
	$("dst_endip").value = G_QoSRule[_i][5];
	$("app_port_start").value = G_QoSRule[_i][6];
	$("app_port_end").value = G_QoSRule[_i][7];
	$("select_classify").value = G_QoSRule[_i][2];
	$("qossubmit").value = data_languages.Public.innerHTML.Public011;
	G_edit = _i;
}
//添加
function uiAdd(){
	if(G_QoSEnable=="0")
	return false;
	
	if($('desc').value == "")
	{
		alert(SEcode['lang_name_not_blank']);
		$("desc").focus();
		return false;
	}
	for(var i=0; i<G_QoSRule.length; i++)
	{
		if(G_edit != -1)
		{			
			if(($("desc").value == G_QoSRule[i][20]) && G_edit != i)
			{			
				alert(UEcode[6409]);
				$("desc").focus();
				return false;
			}
			if(G_edit != i && $("src_startip").value==G_QoSRule[i][9] && $("src_endip").value==G_QoSRule[i][10]
				&& $("dst_startip").value==G_QoSRule[i][4] && $("dst_endip").value==G_QoSRule[i][5] && $("select_classify").value==G_QoSRule[i][2])
			{
				if($("app_port_start").value==G_QoSRule[i][6] && $("app_port_end").value==G_QoSRule[i][7])
				{	
					if($("select_pro").value==G_QoSRule[i][13] || $("select_pro").value=="0" || $("select_pro").value=="0")
					{
						alert(SEcode['lang_rules_not_same']);
						$("src_startip").focus();
						return false;						
					}
				}	
			}			
		}
		else
		{
			if(($("desc").value == G_QoSRule[i][20]))
			{			
				alert(UEcode[6409]);
				$("desc").focus();
				return false;
			}
			if($("src_startip").value==G_QoSRule[i][9] && $("src_endip").value==G_QoSRule[i][10]
				&& $("dst_startip").value==G_QoSRule[i][4] && $("dst_endip").value==G_QoSRule[i][5] && $("select_classify").value==G_QoSRule[i][2])
			{
				if($("app_port_start").value==G_QoSRule[i][6] && $("app_port_end").value==G_QoSRule[i][7])
				{	
					if($("select_pro").value==G_QoSRule[i][13] || $("select_pro").value=="0" || $("select_pro").value=="0")
					{
						alert(SEcode['lang_rules_not_same']);
						$("src_startip").focus();
						return false;						
					}
				}	
			}			
		}
	}
	if($('src_startip').value != "")
	{
		if (!checkipaddr($("src_startip").value)) 
		{
			alert(SEcode['lang_localip_start']);
			$("src_startip").focus();
			return false;
		}
		if($('src_endip').value == "")
		{
			alert(SEcode['lang_localip_end_empty']);
			$("src_endip").focus();
			return false;
		}
	}
	if($('src_endip').value != "")
	{
		if (!checkipaddr($("src_endip").value)) 
		{
			alert(SEcode['lang_localip_end']);
			$("src_endip").focus();
			return false;
		}
		if($('src_startip').value == "")
		{
			alert(SEcode['lang_localip_start_empty']);
			$("src_startip").focus();
			return false;
		}
	}
	if($('dst_startip').value != "")
	{
		if (!checkipaddr($("dst_startip").value)) 
		{
			alert(SEcode['lang_remoteip_start']);
			$("dst_startip").focus();
			return false;
		}
		if($('dst_endip').value == "")
		{
			alert(SEcode['lang_remoteip_end_empty']);
			$("dst_endip").focus();
			return false;
		}
	}
	if($('dst_endip').value != "")
	{
		if (!checkipaddr($("dst_endip").value)) 
		{
			alert(SEcode['lang_remoteip_end']);
			$("dst_endip").focus();
			return false;
		}
		if($('dst_startip').value == "")
		{
			alert(SEcode['lang_remoteip_start_empty']);
			$("dst_startip").focus();
			return false;
		}
	}

	if(($('src_startip').value != "") && ($('src_endip').value != "" ))
	{
		var varIP = $("src_startip").value.split('.');
		var varIP1 = $("src_endip").value.split('.');
		var startIp = Number(varIP[3]);
		var endIp   = Number(varIP1[3]);

		if (startIp > endIp)
		{
			alert(SEcode['lang_localip_range']);
			return false;
		}
	}

	if(($('dst_startip').value != "") && ($('dst_endip').value != "" ))
	{
		var varIP2 = $("dst_startip").value.split('.');
		var varIP3 = $("dst_endip").value.split('.');
		var startIp1 = Number(varIP2[3]);
		var endIp1   = Number(varIP3[3]);

		if (startIp1 > endIp1)
		{
			alert(SEcode['lang_remoteip_range']);
			return false;
		}
	}
	
	if(Form.Select('select_classify') != "LAN" )
	{
		if($('src_startip').value != "")
		{
			if( isSameSubNet($("src_startip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_localip_start_not_lan']);
				$("src_startip").focus();
				return false;
			}
		}
		if($('src_endip').value != "")
		{
			if( isSameSubNet($("src_endip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_localip_end_not_lan']);
				$("src_endip").focus();
				return false;
			}
		}
		if($('dst_startip').value != "")
		{
			if(G_IPAddress == $("dst_startip").value)
			{
				alert(SEcode['lang_remoteip_start_conflict']);
				$("dst_startip").focus();
				return false;
			}
			if( !isSameSubNet($("dst_startip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_remoteip_start_lan']);
				$("dst_startip").focus();
				return false;
			}
		}
		if($('dst_endip').value != "")
		{
			if(G_IPAddress == $("dst_endip").value)
			{
				alert(SEcode['lang_remoteip_end_conflict']);
				$("dst_endip").focus();
				return false;
			}
			if( !isSameSubNet($("dst_endip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_remoteip_end_lan']);
				$("dst_endip").focus();
				return false;
			}
		}
	}
	else
	{	
		if($('src_startip').value != "")
		{
			if(G_IPAddress == $("src_startip").value)
			{
				alert(SEcode['lang_localip_start_conflict']);
				$("src_startip").focus();
				return false;
			}
		
			if( !isSameSubNet($("src_startip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_localip_start_lan']);
				$("src_startip").focus();
				return false;
			}
		}
		if($('src_endip').value != "")
		{
			if(G_IPAddress == $("src_endip").value)
			{
				alert(SEcode['lang_localip_end_conflict']);
				$("src_endip").focus();
				return false;
			}
			if( !isSameSubNet($("src_endip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_localip_end_lan']);
				$("src_endip").focus();
				return false;
			}
		}
		if($('dst_startip').value != "")
		{
			if( isSameSubNet($("dst_startip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_remoteip_start_not_lan']);
				$("dst_startip").focus();
				return false;
			}
		}
		if($('dst_endip').value != "")
		{
			if( isSameSubNet($("dst_endip").value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_remoteip_end_not_lan']);
				$("dst_endip").focus();
				return false;
			}
		}
	}

	$H({
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true);	
	if(G_edit==-1)
	{
		if(G_QoSRule.length==32)
		{
			alert(SEcode['lang_max_rules_32']);
			return false;
		}
		var _Path = "";
		$H({	
			'obj-action'    : 'add-set',
			'add-obj'       : 'InternetGatewayDevice.QueueManagement.Classification.'
		});
	}
	else
	{
		var _Path =G_QoSRule[G_edit][0];
		$F('obj-action', 'set');
	}
	//rule
	$F(":" + _Path + "ClassificationEnable", 	Form.Checkbox('en'));
	$F(":" + _Path + "DestIP", 					$('dst_startip').value);
	$F(":" + _Path + "DestMask", 				$('dst_endip').value);
	$F(":" + _Path + "SourceIP", 				$('src_startip').value);
	$F(":" + _Path + "SourceMask", 				$('src_endip').value);
	$F(":" + _Path + "Protocol", 				Form.Select('select_pro'));	
	$F(":" + _Path + "ClassQueue", 			    (Form.Select('select_classify') == "LAN" ? 0 :4 ) + Number(Form.Select('select_pri')));	
	$F(":" + _Path + "ClassName", 				$('desc').value);		
	$F(":" + _Path + "ClassInterface", 			$('select_classify').value);	
	$F(":" + _Path + "DestPortRangeMax", 		$('app_port_end').value);	
	$F(":" + _Path + "DestPort", 				$('app_port_start').value);	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_qos002').disabled= true;
	$('Adv_qos049').disabled= true;
	$('qossubmit').disabled= true;
	for(var i=0; i<G_QoSRule.length; i++)
	{
		$('delete_'+i).disabled= true;
	}
}
function uiDelete(_i){	
	if(G_QoSEnable=="0")
	return false;
	if(!confirm(SEcode[1001])){
		return false;
	}
   $H({
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:sys_Token' : G_SysToken,
		'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'obj-action'    : 'del',
		'del-obj'       : G_QoSRule[_i][0],
		'var:CacheLastData'	: ViewState.Save()
	},true);
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";	
	$('Adv_qos002').disabled= true;
	$('Adv_qos049').disabled= true;
	$('qossubmit').disabled= true;
	for(var i=0; i<G_QoSRule.length; i++)
	{
		$('delete_'+i).disabled= true;
	}
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

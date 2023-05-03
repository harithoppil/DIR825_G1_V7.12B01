/*  ڴJavaScript  */
//DDNS
var G_DDNS = [];
var m = 0;
<?mget :InternetGatewayDevice.X_TWSZ-COM_DDNS.1. "DDNSProvider Hostname Username Password DDNSState Enable Timeout DDNSEnableIpv6 DDNSHostNameIPv6 DDNSIPv6"
`	G_DDNS = ['$01', //DDNSProvider
				 '$02', //Hostname
				 '$03', //Username
				 '$05', //DDNSState
				 '********', //Password
				 '$06', //Enable
				 '$07', //Timeout
				 '$08', //DDNSEnableIpv6
				 '$09', //DDNSHostNameIPv6
				 '$0a' //DDNSIPv6
				 ]; 
`?>
var LanHosts = [];
var n = 0;
/*
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
*/

<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntriesV6"
`	<?if gt $11 -1
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.HostV6. "HostName MACAddress IPAddress "
		`	LanHosts[n] = [];
			LanHosts[n][0] = n+1;
			LanHosts[n][1] = "PC"+n;
			LanHosts[n][2] = "$02";
			LanHosts[n][3] = "$03";
			++n;
		`?>
	`?>
`?>
var g_edit="-1";
function AddDDNS()
{
	var add_index=0;
			
	/*if(COMM_EatAllSpace($("v6addr").value) == "")
	{			
		alert("Please input the IPv6 address.");
		return false;	
	}
	
	if(COMM_EatAllSpace($("v6host").value) == "")
	{			
		alert("Please input the IPv6 host name.");
		return false;
	}	
		
	$("v6addr").value = COMM_EatAllSpace($("v6addr").value);			
	$("v6host").value = COMM_EatAllSpace($("v6host").value);*/
	
	/*var max_cnt = 32;				
	
	if (add_index > max_cnt)
	{
		alert("The maximum number of list is"+" "+max_cnt);		
	}
	if(g_edit=="-1"){
	$H({
		'add-obj'		:'InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.',
		':DDNSEnableIpv6'   		: _nodes[0].value,
		':DDNSHostNameIPv6'   		: Form.Radio('schdayselect')=='1'?1:0,
		':DDNSIPv6'   		: selectdays,
		'obj-action' 		: 'add-set',
		'var:menu' 		: G_Menu,
		'var:page' 		: G_Page,
		'var:errorpage' 	: G_Page,
		'getpage' 		: 'html/index.html',
		'errorpage' 		: 'html/index.html',
		'var:CacheLastData'	: ViewState.Save()
	}, true);
	}else{*/
		$H({
			'obj-action' 		: 'set',
			'var:menu' 		: G_Menu,
			'var:page' 		: G_Page,
			'var:errorpage' 	: G_Page,
			'getpage' 		: 'html/index.html',
			'errorpage' 		: 'html/index.html',
			'var:CacheLastData'	: ViewState.Save()
		}, true);
		var path=':InternetGatewayDevice.X_TWSZ-COM_DDNS.1.';
		$F(path+'DDNSEnableIpv6',$('en_ddns_v6').checked?"1":"0");
		$F(path+'DDNSHostNameIPv6', $('v6host').value);
		$F(path+'DDNSIPv6', $('v6addr').value);		
	//}
	
	$('uiPostForm').submit();
}
function createDDNSTable()
{
	var add_index=0;
	var array_value = [];	
	array_value[0] = [];
	array_value[0][0]	= G_DDNS[7]=="1"?'<input type="checkbox" id="en_ddns_v6'+add_index+'" checked>':'<input type="checkbox" id="en_ddns_v6'+add_index+'">';
	array_value[0][1]	='<span id="en_ddns_v6host'+add_index+'">'+G_DDNS[8]+'</span>';
	array_value[0][2]	='<span id="en_ddns_v6addr'+add_index+'"">'+G_DDNS[9]+'</span>';
	array_value[0][3]	='<img onclick="OnEdit('+add_index+');" src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit"/>';
	array_value[0][4]	='<img onclick="OnDelete('+add_index+');" src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete"/>';		
	$T_ddns("v6ddns_list", array_value, add_index);	
}	
	
function OnEdit(i)
{		
	$("en_ddns_v6").checked=$("en_ddns_v6"+i).checked;
	$("v6addr").value=$("en_ddns_v6addr"+i).innerHTML;
	$("v6host").value=$("en_ddns_v6host"+i).innerHTML;
	g_edit=i;
}
	
function OnDelete(i)
{
	var _path = G_SchedEntry[Idx][7];
	$H({
	   	"del-obj" 		: _path,
	   	"obj-action" 	: "del",
		"getpage" 		: "html/index.html",
		"errorpage" 	: "html/index.html",
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" : G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	$('uiPostForm').submit();	
}

function ClearDDNS()
{
	$("v6addr").value = "";
	$("v6host").value = "";
	$("en_ddns_v6").checked = false;
	//$("mainform").setAttribute("modified", "true");
}

function OnClickPCArrow(idx)
{	
	if($("pc_"+idx).value=="")
		$("v6addr").value ="";
	else
	{
		for(var k = 0; k < LanHosts.length; k++){
			if($("pc_"+idx).value==LanHosts[k][1])
			{
				$("v6addr").value = LanHosts[k][3];
				break;
			}
		}		
	}
}

function OnClickAddr()
{
	var ddns_addr = $("server").value;

	if($("en_ddns").checked)
	{
		if(ddns_addr == "DLINK")	$("v4addr").value = "dlinkddns.com";
		else if(ddns_addr == "DYNDNS")	$("v4addr").value = "dyndns.com";
		//else if(ddns_addr == "DLINK.COM.CN")	$("v4addr").value = "dlinkddns.com.cn";
		else if(ddns_addr == "ORAY")	$("v4addr").value = "Oray.cn";
	}
	else
	{	
		$("v4addr").value = "";
	}
	//OnChangeServer();
}

function EnableDDNS()
{
	if($("en_ddns").checked)
		$("server").disabled = $("v4addr").disabled = $("host").disabled = $("user").disabled = $("passwd").disabled = $("passwd_verify").disabled = $("timeout").disabled = false; 
	else
		$("server").disabled = $("v4addr").disabled = $("host").disabled = $("user").disabled = $("passwd").disabled = $("passwd_verify").disabled = $("timeout").disabled = true;	
	if($("en_ddns_v6").checked)
	{
		$("v6addr").disabled = $("v6host").disabled = $("pc_").disabled = $("add_ddns_v6").disabled = $("clear_ddns_v6").disabled = false; 			
		/*for(var i = 1;i <  ; i++)
		{
			$("en_ddns_v6"+i).disabled = false;		
		}	*/		
	}
	else
	{
		$("v6addr").disabled = $("v6host").disabled = $("pc_").disabled = $("add_ddns_v6").disabled = $("clear_ddns_v6").disabled =  true;						
		/*for(var i = 1;i < g_table_index; i++)
		{
			$("en_ddns_v6"+i).disabled = true;		
		}	*/		
	}
}

function OnChangeServer()
{
	if($("v4addr").value=="Oray.cn")
	{
		$("dsc_oray").style.display = "block";
		$("report_div").style.display = "none";
		$("peanut_status_div").style.display = "block";
		$("host_div").style.display = "none";
		$("host").disabled = true;
	}
	else
	{
		$("report_div").style.display="block";
		$("dsc_oray").style.display="none";
		$("peanut_status_div").style.display="none";
		$("peanut_detail_div").style.display="none";
		$("host_div").style.display = "block";
		$("host").disabled = false;
		// When switching to servers other than the ORAY, stops refreshing.
		//if(timer_refresh) { clearTimeout(timer_refresh); timer_refresh=null; }
	}
}
function null_errorfunc()
{
	return true;
}
function DDNSProgress(_text)
{	
	try{
		eval(_text);
	}catch(e){
		alert(SEcode['lang_ddns_ajax']+e.message);
		return;
	}
	G_SysToken = G_AjaxToken;
	if(G_check_status == "IPupdatedsuccessful")
	{
		$("report").innerHTML = data_languages.Public.innerHTML.Public032;
	}
	else if(G_check_status == "DDNSprocessing")
	{
		$("report").innerHTML = data_languages.Public.innerHTML.Public033;
	}
	else
	{
		$("report").innerHTML = data_languages.Public.innerHTML.Public034;
	}
	$('save01').disabled= false;
	$('save02').disabled= false;
	$('save03').disabled= false;
	$('save04').disabled= false;
	setTimeout('ajaxGetDDNSStatus()',3000);
}
function ajaxGetDDNSStatus()
{
	//注意这个地方的page字段很重要，否则获取wps_setup.ajax.js会认证失败。
	var _url = "/cgi-bin/webproc?getpage=html/page/check_ddnsstatus.ajax.js&var:page=*";
	ajax = Ajax.getInstance(_url, "", 0, DDNSProgress,null_errorfunc);
	ajax.get();
}
function uiOnload(){
	$('save01').disabled= true;
	$('save02').disabled= true;
	$('save03').disabled= true;
	$('save04').disabled= true;
	createHost();
    setJSONValue({
			"en_ddns"		: G_DDNS[5],
			"v4addr"		: G_DDNS[0],
			"host"			: G_DDNS[1],
			"user"			: G_DDNS[2],
//			"passwd"		:  $("en_ddns").checked ? G_DDNS[4] : "",
//			"passwd_verify"	: $("en_ddns").checked ? G_DDNS[4] : "",
			"timeout"		: G_DDNS[6],
			//"report"		: G_DDNS[3],
			"en_ddns_v6"	: G_DDNS[7],
			"v6addr"		: G_DDNS[9],
			"v6host"		: G_DDNS[8]
		});
    setJSONValue({
			"passwd"		:  $("en_ddns").checked ? G_DDNS[4] : "",
			"passwd_verify"	: $("en_ddns").checked ? G_DDNS[4] : ""
		});
	EnableDDNS();
	createDDNSTable();
	
	setTimeout('ajaxGetDDNSStatus()',3000);
}

function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;		
		for(var k = 0; k < LanHosts.length; k++){
			array_value[k+1]=LanHosts[k][1];
			array_options[k+1]=LanHosts[k][1];
		}
		$S('pc_', array_options, array_value);
}
function TEMP_IsDigit(no)
{
	if (no==""||no==null)
		return false;
	if (no.toString()!=parseInt(no, 10).toString())
		return false;

	return true;
}
function IsBlank(s)
{
	var i=0;
	for(i=0;i<s.length;i++)
	{
		c=s.charAt(i);
		if(c!=' ')return false;
	}
	return true;
}
function uiSubmit()
{
	var node_ddns= $('en_ddns','v4addr','host','user','passwd','timeout','en_ddns_v6','v6addr','v6host');
	if($("en_ddns").checked)
	{
		if(IsBlank($("v4addr").value))
		{			
			alert(SEcode['lang_ddns_server_addr']);
			return false;	
		}	
		
		if(IsBlank($("host").value) && $("v4addr").value!="Oray.cn")
		{			
			alert(SEcode['lang_ddns_host_name']);
			return false;	
		}	
		
		if(IsBlank($("user").value))
		{			
			alert(SEcode['lang_ddns_user_account']);
			return false;	
		}	
		
		if(IsBlank($("passwd").value))
		{			
			alert(SEcode['lang_ddns_passwd']);
			return false;	
		}		
	
		if ($("passwd").value != $("passwd_verify").value)
		{
			alert(SEcode['lang_ddns_vfpasswd']);
			return false;
		}
		if (!TEMP_IsDigit($("timeout").value) || parseInt($("timeout").value, 10)>8670 || parseInt($("timeout").value, 10)<1)
		{
			alert(SEcode['lang_ddns_period']);
			$("timeout").focus();
			return false;
		}
		if($('passwd').value != '********')
		{
			var ddns_passwd = Base64.Encode($('passwd').value);
			$F(":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Password" , 		ddns_passwd);
		}
	
	}		
	$H({
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.DDNSProvider"       : node_ddns[0].checked ? node_ddns[1].value : undefined,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Hostname" : node_ddns[0].checked ? node_ddns[2].value : undefined,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Username" : node_ddns[0].checked ? node_ddns[3].value: undefined,
		//":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Password"     : node_ddns[0].checked ?($('passwd').value != '********' ? ddns_passwd : undefined) : undefined,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Enable"   : node_ddns[0].checked ? 1: 0,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.Timeout"   : node_ddns[0].checked ? node_ddns[5].value: undefined,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.DDNSEnableIpv6"   : node_ddns[6].checked ? 1: 0,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.DDNSHostNameIPv6"   : node_ddns[6].checked  ? node_ddns[8].value: undefined,
		":InternetGatewayDevice.X_TWSZ-COM_DDNS.1.DDNSIPv6"   : node_ddns[6].checked  ? node_ddns[7].value: undefined,
		'getpage'          :'html/index.html',
		'errorpage'        :'html/index.html',
		'var:menu'         :G_Menu,
		'var:page'         :G_Page,
		'var:sys_Token' : G_SysToken,
		'var:errorpage'    :G_Page,
		'obj-action'   : 'set',
		'var:CacheLastData': ViewState.Save()
	});	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('save01').disabled= true;
	$('save02').disabled= true;
	$('save03').disabled= true;
	$('save04').disabled= true;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);


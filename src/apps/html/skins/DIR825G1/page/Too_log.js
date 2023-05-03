/*  JavaScript Document  */
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
var G_Enable = "<?get :InternetGatewayDevice.X_TWSZ-COM_Logger.RemoteLogEnabled?>";
var G_Logger = "<?get :InternetGatewayDevice.X_TWSZ-COM_Logger.RemoteLogger?>";
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>
function OnClickSYSLOGEnable()
{
	if ($("syslogenable").checked)
	{
		$("div_logip").style.display = "block";
		$("sysloghost").setAttribute("modified", "false");
		$("sysloghost").disabled = false;
		$("syslogadd").disabled = false;
		$("hostlist").disabled = false;
	}
	else
	{
		$("sysloghost").setAttribute("modified", "ignore");
		$("sysloghost").disabled = true;
		$("syslogadd").disabled = true;
		$("hostlist").disabled = true;
		$("div_logip").style.display = "none";
	}
}
function OnClickSYSLOGAdd()
{
	if($("hostlist").value == "")
	{
		alert(SEcode['lang_syslog_select']);
		return false;
	}
	else
	{
		$("sysloghost").value = $("hostlist").options[$("hostlist").selectedIndex].value;
		/*for(var k = 0; k < LanHosts.length; k++){
			if($("hostlist").value==LanHosts[k][1])
			{
				$("sysloghost").value = LanHosts[k][3];
				break;
			}
		}*/		
	}
}
//初始化
function uiOnload(){
	Form.Checkbox("syslogenable", G_Enable);
	OnClickSYSLOGEnable();
	setJSONValue({
		'sysloghost'	 : G_Logger		
	});
	createHost();	
}
function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;		
		for(var k = 0; k < LanHosts.length; k++){
			array_value[k+1]=LanHosts[k][3];
			array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
		}
		$S('hostlist', array_options, array_value);
}
//提交数据
function OnSubmit(){
	if ($("syslogenable").checked)
	{
		if ($("sysloghost").value == "" || !checkipaddr($("sysloghost").value) || !isSameSubNet($("sysloghost").value,G_SubMask,G_IPAddress,G_SubMask))
		{
			alert(SEcode['lang_syslog_host']);
			$("sysloghost").focus();
			return false;
		}		
	}
	$H({
		':InternetGatewayDevice.X_TWSZ-COM_Logger.RemoteLogEnabled' : Form.Checkbox('syslogenable'),
		':InternetGatewayDevice.X_TWSZ-COM_Logger.RemoteLogger' : Form.Checkbox('syslogenable')?$('sysloghost').value:undefined,
		':InternetGatewayDevice.X_TWSZ-COM_Logger.DebugLogEnabled' : '1',
	   	"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index.html",
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		'var:sys_Token' : G_SysToken,
		"var:CacheLastData" 	: ViewState.Save()
	}, true, 'uiPostForm');
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('LOG003').disabled= true;
	$('LOG008').disabled= true;
}

//错误处理函数
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//监听加载与错误处理函数
addListeners(uiOnload, dealWithError);
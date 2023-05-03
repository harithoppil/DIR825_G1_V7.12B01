//Radvd
var G_RadvdEnabled = "<?get :InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.RADVD.Enabled?>";
var G_ULA = "<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.X_TWSZ-COM_IPv6LanIntfAddress.UniqueLocalAddress?>";

<?mget :InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement.RadVDConfigManagement.ULAPrefixInfo. "Prefix Enable Mode"
`	G_RadPrefix 	= "$01";
	G_RadEnable 	= "$02";
	G_PreGetMode	= "$03"
`?>


function OnClickUseDefault()
{
	if($("use_default_ula").checked)
	{
		$("ula_prefix").disabled = true;
	}
	else
	{
		$("ula_prefix").disabled = false;
	}
}
function OnClickEnableUla()
{
	$("bbox_ula").style.display = "";
	$("box_ula_title").style.display = "";
	$("box_ula_body").style.display = "";
	$("ula_span").style.display = "";
	$("box_cula_title").style.display = "";
	$("box_cula_prefix_body").style.display = "";
	$("box_cula_addr_body").style.display = "";
	
	if($("en_ula").checked)
	{
		$("use_default_ula").disabled = false;
	}
	else
	{
		$("use_default_ula").disabled = true;
		$("ula_prefix").disabled = true;
	}
}

function InitUlaVal()
{
	//var active = "1";
	if(G_RadEnable=="1")	
		$("en_ula").checked = true;
	else		
		$("en_ula").checked = false;
	
	var Pre_Prefix = G_RadPrefix.split('/')[0], Pre_Length = 64;
	var ipaddr = "0080:1234::2222";
	//var isStatic = "1";
	
	if(G_PreGetMode=="1")
	{	
		$("use_default_ula").checked = true;
		$("ula_prefix").disabled = true;
	}
	else
	{		
		$("use_default_ula").checked	= false;
		$("ula_prefix").disabled	= false;
		//$("ula_prefix").value		= ipaddr;
		$("ula_prefix").value		= Pre_Prefix;
	}
	
	/*** fill some fixed info ***/
	$("ula_prefix_pl").innerHTML	= "/64";
	$("cula_prefix").innerHTML	= Pre_Prefix;
	$("cula_prefix_pl").innerHTML	= "/64";
	$("cula_addr").innerHTML	= G_ULA;
	$("cula_addr_pl").innerHTML	= "/64";

	return true;
}

function uiOnload()
{
	InitUlaVal();
	OnClickEnableUla();
				
	return;
}

function OnSubmit()
{

	//var _path_host4 = ":InternetGatewayDevice.LANDevice.1.LANHostConfigManagement."
	var _path_host6 = ":InternetGatewayDevice.LANDevice.1.X_TWSZ-COM_IPv6LANHostConfigManagement."
	
	$H({
		"obj-action" 		: "set",
		"getpage" 			: "html/index.html",
		"errorpage" 		: "html/index.html",
		"var:menu" 			: G_Menu,
		"var:page" 			: G_Page,
		"var:errorpage" 	: G_Page,
		'var:sys_Token' : G_SysToken,
		"var:CacheLastData" : ViewState.Save()
	}, true);

	//Radvd
	//ula
	$F( _path_host6 + 'RadVDConfigManagement.ULAPrefixInfo.Enable' 				, Form.Checkbox('en_ula'));
	if(Form.Checkbox('en_ula')==true)
	{
		if(Form.Checkbox('use_default_ula') == 1)
		{
			$F( _path_host6 + 'RadVDConfigManagement.ULAPrefixInfo.Mode' 				, "1" );
		}
		else
		{
			$F( _path_host6 + 'RadVDConfigManagement.ULAPrefixInfo.Mode' 				, "0" );
			var _Prefix_length = $('ula_prefix').value + '/64';
			$F( _path_host6 + 'RadVDConfigManagement.ULAPrefixInfo.Prefix'			, _Prefix_length);
		}
	}
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("BIPV6LOCAL011").disabled = true;
	$("BIPV6LOCAL013").disabled = true; 
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
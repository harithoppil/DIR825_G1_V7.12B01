var Ipv4_Ping_Result="<?get :InternetGatewayDevice.X_TWSZ-COM_Diagnostics.IPV4PingTest.Result?>"
var Ipv6_Ping_Result="<?get :InternetGatewayDevice.X_TWSZ-COM_Diagnostics.IPV6PingTest.Result?>"
var ping='<?echo $var:ping?>';
function OnClick_Ping()
{
	ResetPing();
	$("ping").disabled = true;
	//$("dst").value= COMM_EatAllSpace($("dst").value);
	if ($("dst").value=="")
	{
		alert(SEcode['lang_ping_host']);
		$("dst").focus();
		ResetPing();
		return false;
	}
	$("report").innerHTML = data_languages.Public.innerHTML.Public035;
	$H({
		':InternetGatewayDevice.X_TWSZ-COM_Diagnostics.IPV4PingTest.IPv4' : $("dst").value,		
		'obj-action'   : 'set',
		'var:ping'	   : 'ipv4',
		'var:sys_Token' : G_SysToken,
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:errorpage': G_Page,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:CacheLastData': ViewState.Save()
	});
	$('uiPostForm').submit();
	$("ping").disabled = true;
	$("ping_ipv6").disabled = true;
}
	
function OnClick_Ping_ipv6()
{
	ResetPing();
	$("ping_ipv6").disabled = true;
	//$("dst_ipv6").value= COMM_EatAllSpace($("dst_ipv6").value);
	if ($("dst_ipv6").value=="")
	{
		alert(SEcode['lang_ping_host']);
		$("dst_ipv6").focus();
		ResetPing();
		return false;
	}
	
	$("report").innerHTML = data_languages.Public.innerHTML.Public035;
	$H({
		':InternetGatewayDevice.X_TWSZ-COM_Diagnostics.IPV6PingTest.IPv6' : $("dst_ipv6").value,		
		'obj-action'   : 'set',
		'var:ping'	   : 'ipv6',
		'var:sys_Token' : G_SysToken,
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:errorpage': G_Page,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:CacheLastData': ViewState.Save()
	});
	$('uiPostForm').submit();	
	$("ping").disabled = true;
	$("ping_ipv6").disabled = true;
}

function ResetPing()
{
	$("ping").disabled = false;
	$("ping_ipv6").disabled = false;
}

function uiOnload(){
	if(ping=="ipv4")
	{
		if(Ipv4_Ping_Result=="PASS")
			$("report").innerHTML = data_languages.Public.innerHTML.Public036;
		else
			$("report").innerHTML = data_languages.Public.innerHTML.Public037;
	}
	else if(ping=="ipv6")
	{
		if(Ipv6_Ping_Result=="PASS")
			$("report").innerHTML = data_languages.Public.innerHTML.Public036;
		else
			$("report").innerHTML = data_languages.Public.innerHTML.Public037;
	}
	else
		$("report").innerHTML = "";
	dealWithError();
}

function dealWithError(){
	if (G_Error != 1){ return false;}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,Form.Action);


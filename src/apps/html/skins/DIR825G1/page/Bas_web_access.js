
var G_SambaEnabled = "<?get :InternetGatewayDevice.X_TWSZ-COM_SAMBA.SambaEnable?>";
var G_SambaPwd = "<?get :InternetGatewayDevice.X_TWSZ-COM_SAMBA.Newpassword?>";
function OnClickEnableSabma()
{
	
	if($("en_webaccess").checked)
	{
		$("div_usercreat").style.display = "";
	}
	else
	{
		$("div_usercreat").style.display = "none";
	}
}

function uiOnload()
{
	if(G_SambaEnabled == "1")
		$("en_webaccess").checked = true;
	else
		$("en_webaccess").checked = false;
	$("pwd").value = Base64.Decode(G_SambaPwd);

	OnClickEnableSabma();
	return;
}

function OnSubmit()
{
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
	
	if($("en_webaccess").checked==true)
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_SAMBA.' + 'SambaEnable' , "1");
		$F(':InternetGatewayDevice.X_TWSZ-COM_SAMBA.' + 'Newpassword' , Base64.Encode($("pwd").value));
	}
	else
		$F(':InternetGatewayDevice.X_TWSZ-COM_SAMBA.' + 'SambaEnable' , "0");
	
	$('uiPostForm').submit();
	$("BWACCESS011").disabled = true;
	$("BWACCESS013").disabled = true; 
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
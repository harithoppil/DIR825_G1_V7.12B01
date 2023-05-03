
function OnClickReboot()
{
	$('tokenid4').value=G_SysToken;
	if (confirm(SEcode['lang_system_reboot']))
	{
		$('reboot').submit();		
		$('SYS009').disabled= true;
		$('SYS011').disabled= true;
		$('SYS005').disabled= true;
		$('SYS007').disabled= true;
		$('SYS016').disabled= true;
		setTimeout('refreshTokenid()',1000);
	}	
}

function OnClickReboot_factory_def()
{
	$('tokenid3').value=G_SysToken;
	if (confirm(SEcode['lang_system_reset']))
	{
		$('restore').submit();		
		$('SYS009').disabled= true;
		$('SYS011').disabled= true;
		$('SYS005').disabled= true;
		$('SYS007').disabled= true;
		$('SYS016').disabled= true;
		setTimeout('refreshTokenid()',1000);
	}
	
}

var configpro = 0;
function upConfigProgress() {
	$('tokenid2').value=G_SysToken;
	var _config = $("config").value;
	//文件类型
	if(_config == ""){
		alert(SEcode[2000]);
		return false;
	}else if(_config.match(/.xml$/) == null){
		alert(SEcode[2002]);
		return false;
	}
	
	if(configpro == 0){
		configpro = 1;
		return true;
	}
	alert(SEcode[2001]);
	
	return false;
	$('SYS009').disabled= true;
	$('SYS011').disabled= true;
	$('SYS005').disabled= true;
	$('SYS007').disabled= true;
	$('SYS016').disabled= true;
	setTimeout('refreshTokenid()',1000);
}

function isBakeup()
{	
	$('tokenid1').value=G_SysToken;
	$('SYS009').disabled= true;
	$('SYS011').disabled= true;
	$('SYS005').disabled= true;
	$('SYS007').disabled= true;
	$('SYS016').disabled= true;
	setTimeout('refreshTokenid()',1000);
}

function isClear()
{	
	$('tokenid5').value=G_SysToken;
	$('SYS009').disabled= true;
	$('SYS011').disabled= true;
	$('SYS005').disabled= true;
	$('SYS007').disabled= true;
	$('SYS016').disabled= true;
	setTimeout('refreshTokenid()',1000);
}

function refreshTokenid()
{
	if (checkTokenCookie()) {
		G_SysToken = Cookie.Get('token_sys');
		Cookie.Delete('token_sys','/');
		$('SYS009').disabled= false;
		$('SYS011').disabled= false;
		$('SYS005').disabled= false;
		$('SYS007').disabled= false;
		$('SYS016').disabled= false;
	}
}

function checkTokenCookie()
{
	if (Cookie.Get('token_sys')) {
		return true;
	}else {
		return false;
	}
}
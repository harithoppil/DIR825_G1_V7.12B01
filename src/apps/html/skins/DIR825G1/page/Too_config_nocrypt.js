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
	$('FW010').disabled= true;
	$('FW013').disabled= true;
	setTimeout('refreshTokenid()',1000);
}

function isBakeup()
{	
	$('tokenid1').value=G_SysToken;
	$('FW010').disabled= true;
	$('FW013').disabled= true;
	setTimeout('refreshTokenid()',1000);
}

function refreshTokenid()
{
	if (checkTokenCookie()) {
		G_SysToken = Cookie.Get('token_sys');
		Cookie.Delete('token_sys','/');
		$('FW010').disabled= false;
		$('FW013').disabled= false;
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

function uiOnload(){
}

function dealWithError(){
	if (G_Error != 1){ return false;}
	
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,dealWithError);
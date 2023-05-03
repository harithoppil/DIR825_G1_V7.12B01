/*  JavaScript Document  */
//deviceinfo
var G_FirmwareTime = "<?get :InternetGatewayDevice.X_TWSZ-COM_Lang.X_TWSZ-COM_CURFW_DATE?>";//软件编译时间暂时没有
var G_CurFirmwareVersion = "<?get :InternetGatewayDevice.DeviceInfo.ModemFirmwareVersion?>";

var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<? if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "ConnectionStatus"
		`	
			G_WANConn['ConnectionStatus'] = "$01";	// ConnectionStatus
		`?>
	`?>

	<? if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "ConnectionStatus"
		`	
			G_WANConn['ConnectionStatus'] = "$01";	// ConnectionStatus
			
		`?>
	`?>
`?>
var firmwarepro = 0;
var LangFilepro = 0;
function upFirmwareProgress() {
	var _firmware = $("firmware").value;
	$('tokenid1').value=G_SysToken;
	//var _config = $("config").value;
	//每次只能升级一个
	if(_firmware == ""){
		alert(SEcode[2000]);
		return false;
	}else if(_firmware.match(/.bin$/) == null){
		alert(SEcode[2003]);
		return false;
	}
	
	if(firmwarepro == 0){
		firmwarepro = 1;
		return true;
	}
	alert(SEcode[2001]);
	return false;
}
function upLangFileProgress() {
	var _LangFile = $("uploadLangFile").value;
	$('tokenid2').value=G_SysToken;
	//alert(_LangFile);
	//var _config = $("config").value;
	//每次只能升级一个
	if(_LangFile == ""){
		alert(SEcode[2000]);
		return false;
	}else if(_LangFile.match(/.lng$/) == null){
		alert(SEcode[2006]);
		return false;
	}
	
	if(LangFilepro == 0){
		LangFilepro = 1;
		return true;
	}
	alert(SEcode[2001]);
	return false;
}

var configpro = 0;
function upConfigProgress() {
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
}

function ajaxGetStatus()
{
	var _url = "/cgi-bin/webproc?getpage=html/page/FW_check.ajax.js&var:page=*";
	ajax = Ajax.getInstance(_url, "", 0, Ajax_handler,null_errorfunc);
	ajax.get();
}
var count=0;
function Ajax_handler(_text)
{
	try{
		eval(_text);
	}catch(e){
		alert(SEcode['lang_ddns_ajax']+e.message);
		return;
	}
	G_SysToken = G_AjaxToken;
	count++;
	if(G_Error == '1')
	{
		dealWithError();
		$("chkfw_btn").disabled = false;			
		$("FW0082").disabled = false;
		$("FW0081").disabled = false;
	}	
	else
	{
		if(G_checkfw_status == "256"&&count<300)
			setTimeout('ajaxGetStatus()',1000);
		else if(G_checkfw_status == "255"||count>=300)
		{
			$("fw_message").innerHTML = data_languages.Too_firmware.innerHTML.FW025;
			$("chkfw_btn").disabled = false;			
			$("FW0082").disabled = false;
			$("FW0081").disabled = false;
		}
		else
		{	
			$("chkfw_btn").disabled = false;			
			$("FW0082").disabled = false;
			$("FW0081").disabled = false;
			var fw_maj = parseInt(G_NEWFwMAJ, [10]);//大版本号
			var fw_min = parseInt(G_NEWFwMIN, [10]);//小版本号
			//alert("main:"+fw_maj+",min:"+fw_min);
			
			var G_CurFW = "<?get :InternetGatewayDevice.DeviceInfo.SoftwareVersion?>";
			G_CurFW="0.00";
			//我们当前的版本号首字符是字母,样机的没有字母，这里就没有比较首字符
			var curfwstr  = G_CurFW.split(".");//当前版本
			var curfw_maj = parseInt(curfwstr[0], [10]);
			var curfw_min = parseInt(curfwstr[1], [10]);
			//alert("main:"+curfw_maj+",min:"+curfw_min);
			if(curfw_maj > fw_maj || (curfw_maj ==fw_maj && curfw_min >= fw_min))
			{
				$("fw_message").innerHTML = data_languages.Too_firmware.innerHTML.FW026;
				$("div_ckfwver").style.display="none";
			}
			else
			{
				$("fw_message").style.display = "none";
				$("div_ckfwver").style.display = "block";
				setJSONValue({
					'latest_fw_ver' 		: G_NEWFwMAJ+"."+G_NEWFwMIN,
					'latest_fw_date' 		: G_LatestFwDate
				});	
				G_LatestFwURLS = strAnsi2Unicode(Base64.Decode(G_LatestFwURLS));
				var urls1=[];
				var urls=[];				
				if(G_LatestFwURLS.indexOf(";")>-1)
				{
					urls=G_LatestFwURLS.split(";");
					for(var i=0;i<urls.length;i++)
					{
						urls1[i]= urls[i].split("/")[2];
					}
				}
				else
				{
					urls1[0]=G_LatestFwURLS.split("/")[2];
					urls[0]=G_LatestFwURLS;
				}
				for(var i=0;i<urls1.length;i++)
				{
					var x=document.getElementById("fw_dl_locs");
					var option=document.createElement("option");
					option.text=urls1[i];
					option.value=urls[i];
					try
						{
						  //for IE earlier than version 8
						  x.add(option,x.options[null]);
						}
					catch (e)
						{
							x.add(option,null);
						}
				}
			}
		}
	}
}
function null_errorfunc()
{

	return true;
}
function OnClickChkFW()
{
	$("chkfw_btn").disabled = true;
	$("FW0082").disabled = true;
	$("FW0081").disabled = true;
	$("fw_message").style.display="block";
	$("fw_message").innerHTML = data_languages.Too_firmware.innerHTML.FW027;
	$("div_ckfwver").style.display="none";
	$H({
		"obj-action"           : "set",
		':InternetGatewayDevice.X_TWSZ-COM_Lang.X_TWSZ-COM_ImageUpgradeOpt' : 'AutoDetect',
		"getpage"              : "html/page/FW_check.ajax.js",
		"errorpage"            : "html/page/FW_check.ajax.js",
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		'var:menu'      	   : G_Menu,
		'var:page'      	   : G_Page,
		"var:errorpage"        : G_Page,
		"var:CacheLastData"    : ViewState.Save()
	}, true);

	var _url = "/cgi-bin/webproc";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.post($('uiPostForm'));
	
}
function OnClickDownloadFW()
{
	var downloadSelect = $("fw_dl_locs");
	var selectBox = downloadSelect.selectedIndex;
	var URL = downloadSelect.options[selectBox].value;
	
	self.location.href=URL;

}

function uiOnload(){
	
	setJSONValue({
		'fw_build_time' 		: G_FirmwareTime,
		'st_fw' 				: G_CurFirmwareVersion
	});
	
	$('tokenid1').value=G_SysToken;
	$('tokenid2').value=G_SysToken;
	if(G_WANConn['ConnectionStatus'] == "Disconnected")
	{
		$("chkfw_btn").disabled = true;
	}
}

function dealWithError(){
	if (G_Error != 1){ return false;}
	
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload,dealWithError);

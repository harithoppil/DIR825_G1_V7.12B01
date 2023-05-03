/*  JavaScript Document  */

var G_Enable = "<?get :InternetGatewayDevice.X_TWSZ-COM_Logger.LoggerEnabled?>";
var logPages=0;
var pageInx= 0;
var logItems=0;
var msgItems=10;
var _string=[];
var lang_schedule_month = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
<?mget :InternetGatewayDevice.X_TWSZ-COM_Logger. "LogTypeSys LogTypeSec LogTypeStat LogLevel.LogTypeSys LogLevel.LogTypeSec LogLevel.LogTypeStat"
`	var G_LogType1 		  = "$01";
	var G_LogType2 		  = "$02";
	var G_LogType3        = "$03";
	var G_LogLevel1 	  = "$04";
	var G_LogLevel2    	  = "$05";  
	var G_LogLevel3       = "$06";
`?>
function ReNewVars()
{
	logItems = _string.length;
	logPages = Math.floor(logItems/msgItems);
	var isint = logItems/msgItems;
	if(isint==logPages && logPages > 1)
	{
		logPages = logPages-1;
	}
	pageInx = 0;
	return true;
}

function OnClickToPage(to)
{
	if(to == "-1" && pageInx > 0)
	{
		pageInx--;
	}
	else if(to == "+1" && pageInx < logPages)
	{
		pageInx++;
	}
	else if(to == "1")
	{
		pageInx = 0;
	}
	else if(to == "0")
	{
		pageInx = logPages;
	}
	else
	{return false;}
	DrawLog();
}

function DrawLog()
{
	if (logPages == 0)
	{
		$("pp").disabled=true;
		$("np").disabled=true;
	}
	else
	{
		if(pageInx == 0)
		{
			$("pp").disabled=true;
			$("np").disabled=false;
		}
		if(pageInx == logPages)
		{
			$("pp").disabled=false;
			$("np").disabled=true;
		}
		if(pageInx > 0 && pageInx < logPages)
		{
			$("pp").disabled=false;
			$("np").disabled=false;
		}
	}
	var str = "<p>"+ (pageInx + 1) + "/" + (1 + logPages) + "</p>";
	str += "<table class=\"general\"><tr>";
	str += '<th width="128px" id="LOGS022">' + "Time" + "</th>";
	str += '<th width="396px" id="LOGS023">' + "Message" + "</th>";
	str += "</tr>";
	for(var i=(logItems-pageInx*msgItems-1); i > logItems-(pageInx+1)*msgItems-1 && i >= 0; i--)
	{
		if(_string[i]!=""&&_string[i]!=undefined)
		{	
			var _str1="";
			var _str2="";
			
			_str1=_string[i].substr(0,_string[i].indexOf("["));
			_str2=_string[i].substr(_string[i].lastIndexOf("]")+1,_string[i].length-1);
			if(_str2==""||_str2==undefined)
			_str2=_string[i].substr(_string[i].indexOf("]")+1,_string[i].length-1);
			
			if(_str1!=""&&_str1!=undefined&&_str2!=""&&_str2!=undefined)
			{   
				time=_str1.split(" ");
				time1=time[0].split("-");
				if(time1[1]=="01") month=lang_schedule_month[0];
				else if(time1[1]=="02") month=lang_schedule_month[1];
				else if(time1[1]=="03") month=lang_schedule_month[2];
				else if(time1[1]=="04") month=lang_schedule_month[3];
				else if(time1[1]=="05") month=lang_schedule_month[4];
				else if(time1[1]=="06") month=lang_schedule_month[5];
				else if(time1[1]=="07") month=lang_schedule_month[6];
				else if(time1[1]=="08") month=lang_schedule_month[7];
				else if(time1[1]=="09") month=lang_schedule_month[8];
				else if(time1[1]=="10") month=lang_schedule_month[9];
				else if(time1[1]=="11") month=lang_schedule_month[10];
				else if(time1[1]=="12") month=lang_schedule_month[11];
				if(time1[2]!="10"&&time1[2]!="20"&&time1[2]!="30")
				time1[2]=time1[2].replace("0","");
				str += "<tr>";
				str += "<td>" + month+" "+time1[2]+" "+time[1]+" "+time1[0]+" "+"</td>";
				str += "<td class=\"msg\">" + _str2 + "</td>";
				str += "</tr>";
			}
		}
	}
	str += "</table>";
	$("sLog").innerHTML = str;
	$("LOGS022").innerHTML = data_languages.Hom_logs.innerHTML.LOGS022;
	$("LOGS023").innerHTML = data_languages.Hom_logs.innerHTML.LOGS023;
}

function OnClickClear()
{
	$H({
	':InternetGatewayDevice.X_TWSZ-COM_Logger.LogClearTrigger' : "1",
	"obj-action" 		: "set",
	"getpage" 		: "html/index.html",
	"errorpage" 		: "html/index.html",
	'var:sys_Token' : G_SysToken,
	"var:menu" 		: G_Menu,
	"var:page" 		: G_Page,
	"var:errorpage" 	: G_Page,
	"var:CacheLastData" 	: ViewState.Save()
	}, true, 'uiPostForm');
	$('uiPostForm').submit();	
	$('clear').disabled= true;
	$('LOGS003').disabled= true;
	$('save').disabled= true;
	$('LOGS004').disabled= true;
}

function OnClickChangeType(type)
{		
	ReNewVars();
	DrawLog();
}
//初始化
function uiOnload()
{
	$('clear').disabled= true;
	$('LOGS003').disabled= true;
	$('save').disabled= true;
	$('LOGS004').disabled= true;
	var type="0";
	if(G_LogType1=="1") type="1";
	else if(G_LogType2=="2") type="2";
	else if(G_LogType3=="3") type="3";
	Form.Radio("Type",type);
	var level="0";
	if(G_LogLevel1=="1") level="1";
	else if(G_LogLevel2=="2") level="2";
	else if(G_LogLevel3=="3") level="3";
	Form.Radio("Level",level);
	ajax_GetLogText();
}

function refreshTokenid()
{
	if (checkTokenCookie()) {
		G_SysToken = Cookie.Get('token_sys');
		Cookie.Delete('token_sys','/');		
		$('clear').disabled= false;
		$('LOGS003').disabled= false;
		$('save').disabled= false;
		$('LOGS004').disabled= false;
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

function ajax_GetLogText()
{
	var _url = "/cgi-bin/webupg";
	$('tokenid').value="tk59651649";
	ajax = Ajax.getInstance(_url, "", 0, processResult);
	ajax.post($('uiShowLog'));
}
//Ajax
function processResult(responseText){
	responseText=responseText.replace(responseText.substr(0,responseText.indexOf("#")),"");
	responseText=responseText.replace("###","");
	_string = responseText.split("###");
	ReNewVars();
	DrawLog();
	refreshTokenid();
}

//提交数据
function uiSubmit(){
	$H({
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogTypeSys' : Form.Radio("Type")=='1'?"1":"0",
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogTypeSec' : Form.Radio("Type")=='2'?"2":"0",
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogTypeStat' : Form.Radio("Type")=='3'?"3":"0",
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogLevel.LogTypeSys' : Form.Radio("Level")=='1'?"1":"0",
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogLevel.LogTypeSec' : Form.Radio("Level")=='2'?"2":"0",
		':InternetGatewayDevice.X_TWSZ-COM_Logger.LogLevel.LogTypeStat' : Form.Radio("Level")=='3'?"3":"0",
	   	"obj-action" 		: "set",
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true, 'uiPostForm');
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('clear').disabled= true;
	$('LOGS003').disabled= true;
	$('save').disabled= true;
	$('LOGS004').disabled= true;
}

function Refresh()
{
	$('clear').disabled= true;
	$('LOGS003').disabled= true;
	$('save').disabled= true;
	$('LOGS004').disabled= true;
	uiPageRefresh();
}
//保存
function uiSave(){
	$('tokenid1').value=G_SysToken;
	$('uiDownloadLog').submit();
	$('clear').disabled= true;
	$('LOGS003').disabled= true;
	$('save').disabled= true;
	$('LOGS004').disabled= true;
	setTimeout('refreshTokenid()',1000);
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
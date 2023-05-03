

function uiOnload(){
	$('HSS003').disabled= true;
	var _url = "/cgi-bin/webupg";
	ajax = Ajax.getInstance(_url, "", 0, processResult);
	$('tokenid').value="tk59651649";
	ajax.post($('uiShowSession'));
}
//Ajax
function refreshTokenid()
{
	if (checkTokenCookie()) {
		G_SysToken = Cookie.Get('token_sys');
		Cookie.Delete('token_sys','/');
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

var data1=new Array();
function processResult(responseText){
	var data=responseText.split(/,|=|\n/);  
	for(var i=1;i<=data.length/2;i++)
	{
		data1[i-1]=data[2*i-1];
	}
	var str = "";					
		str = "<table class=general><tr><th width='20%' id='HSS007'>IP</th><th width='10%' id='HSS008'>Protocol</th><th width='10%' id='HSS009'>NAT</th><th width='20%' id='HSS010'>Internet</th>"
		+"<th width='10%' id='HSS011'>State</th><th width='10%' id='HSS012'>Dir</th><th width='10%' id='HSS013'>Time Out</th></tr>";	
	for(var j=0;j<(data1.length)/7;j++)
	{	
		str = str +"<tr>"
		+"<td>"+data1[j*7+0]+"</td>"
		+"<td>"+data1[j*7+1].toUpperCase()+"</td><td>"+data1[j*7+2]+"</td>"
		+"<td>"+data1[j*7+3]+"</td><td>"+data1[j*7+4]+"</td>"
		+"<td>"+data1[j*7+5]+"</td><td>"+data1[j*7+6]+"</td>"	
		+"</tr>";
	}
	str = str + "</table>";
	$("sess_list").innerHTML = str;	
	$("INPUT_SessionText").innerHTML = responseText;
	$("HSS007").innerHTML = data_languages.Hom_sessions.innerHTML.HSS007;
	$("HSS008").innerHTML = data_languages.Hom_sessions.innerHTML.HSS008;
	$("HSS009").innerHTML = data_languages.Hom_sessions.innerHTML.HSS009;
	$("HSS010").innerHTML = data_languages.Hom_sessions.innerHTML.HSS010;
	$("HSS011").innerHTML = data_languages.Hom_sessions.innerHTML.HSS011;
	$("HSS012").innerHTML = data_languages.Hom_sessions.innerHTML.HSS012;
	$("HSS013").innerHTML = data_languages.Hom_sessions.innerHTML.HSS013;
	refreshTokenid();
	$('HSS003').disabled= false;	
}

function Refresh()
{
	$('HSS003').disabled= true;
	uiPageRefresh();
}		
addListeners(uiOnload);

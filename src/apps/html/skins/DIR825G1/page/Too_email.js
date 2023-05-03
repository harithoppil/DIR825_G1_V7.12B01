//MAIL
<?mget :InternetGatewayDevice.X_TWSZ-COM_Mail. "Enable SenderMail SmtpUsername SmtpPassword SmtpHost TosendMail Fullenable Authrq SmtpPort MailSubjectName ScheduleListName ScheduleEnable"
`	var G_Enable 		      = "$01";
	var G_SenderMail 		  = "$02";
	var G_SmtpUsername        = "$03";
	var G_SmtpPassword 		  = "$04";
	var G_SmtpHost    		  = "$05";  
	var G_TosendMail          = "$06";
	var G_Fullenable 		  = "$07";
	var G_Authrq			  = "$08";
	var G_SmtpPort			  = "$09";
	var G_MailSubjectName 	  = "$0a";
	var G_ScheduleListName	  = "$0b";
	var G_ScheduleEnable	  = "$0c";
`?>
var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable SelectDays StartTime EndTime"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_list[schedule_num][3] = "$03";	//SelectDays
	 schedule_list[schedule_num][4] = "$04";	//StartTime
	 schedule_list[schedule_num][5] = "$05";	//EndTime
	 schedule_num++;
`?>
function OnChangeSchedule()
{
	if($("log_sch").value!="never")
	{
		for(var i=0;i<schedule_num;i++)
		{
			if($("log_sch").value==schedule_list[i][0])
			{
				$("log_detail").value = schedule_list[i][3].substring(0,schedule_list[i][3].length-1)+" "+schedule_list[i][4]+'~'+schedule_list[i][5];
				break;
			}
		}
	}
	else
		$("log_detail").value = data_languages.Public.innerHTML.Public021;
}
function OnClickEnableSchedule()
{
	if($("en_log_sch").checked && $("en_mail").checked)
		$("log_sch").disabled			= false;
	else
		$("log_sch").disabled			= true;
}

function null_errorfunc()
{
	return true;
}

function OnClickSendMail()
{	
	if(G_Enable=="0")
		return false;
	$H({
		":InternetGatewayDevice.X_TWSZ-COM_Mail.SendCMD"       : 1,		
		'obj-action'   : 'set',
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:errorpage': G_Page,
		'var:sys_Token' : G_SysToken,
		'ajax'          : 'ok',
		'getpage'      : 'html/page/email.ajax.js',
		'errorpage'    : 'html/index.html',		
		'var:CacheLastData': ViewState.Save()
	});

	var _url = "/cgi-bin/webproc";
	G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
	G_ajax.post($('uiPostForm'));
	$('sendmail').disabled= true;
	$('email003').disabled= true;
	$('email022').disabled= true;	
	$("send_msg").style.display="block";
	$("email017").innerHTML = data_languages.Too_email.innerHTML.email028;
}

function ajaxGetStatus()
{
	var _url = "/cgi-bin/webproc?getpage=html/page/email.ajax.js&var:page=*";
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
		$('sendmail').disabled= false;
		$('email003').disabled= false;
		$('email022').disabled= false;
		$("send_msg").style.display="none";
	}	
	else
	{
		if(G_email_status == "sending"&&count<180)
			setTimeout('ajaxGetStatus()',1000);
		else if(G_email_status == "failed"||count>=180)
		{
			$("email017").innerHTML = data_languages.Too_email.innerHTML.email030;
			$("sendmail").disabled = false;			
			$("email003").disabled = false;
			$("email022").disabled = false;
			setTimeout('$("send_msg").style.display="none";', 6000); 
		}
		else
		{	
			$("email017").innerHTML = data_languages.Too_email.innerHTML.email029;
			$("sendmail").disabled = false;			
			$("email003").disabled = false;
			$("email022").disabled = false;
			setTimeout('$("send_msg").style.display="none";', 6000); 
		}
	}
}

function OnClickAuthEnable()
{
	if ($("authenable").checked && $("en_mail").checked)
	{
		$("account_name").setAttribute("modified", "false");
		$("account_name").disabled = false;
		$("passwd").setAttribute("modified", "false");
		$("passwd").disabled = false;
		$("verify_passwd").setAttribute("modified", "false");
		$("verify_passwd").disabled = false;
	}
	else
	{
		$("account_name").setAttribute("modified", "ignore");
		$("account_name").disabled = true;
		$("passwd").setAttribute("modified", "ignore");
		$("passwd").disabled = true;
		$("verify_passwd").setAttribute("modified", "ignore");
		$("verify_passwd").disabled = true;
	}
}
function OnClickEnable()
{
	if($("en_mail").checked)
	{
		$("from_addr").disabled			= false;
		$("to_addr").disabled				= false;
		$("email_subject").disabled		= false;
		$("smtp_server_addr").disabled	= false;
		$("smtp_server_port").disabled	= false;
		$("authenable").disabled			= false;
		OnClickAuthEnable();
		$("sendmail").disabled			= false;
		$("en_logfull").disabled			= false;
		$("en_log_sch").disabled			= false;
		$("log_sch").disabled				= false;
	}   
	else
	{
		$("from_addr").disabled			= true;
		$("to_addr").disabled				= true;
		$("email_subject").disabled		= true;
		$("smtp_server_addr").disabled	= true;
		$("smtp_server_port").disabled	= true;
		$("authenable").disabled			= true;
		OnClickAuthEnable();
		$("sendmail").disabled			= true;
		$("en_logfull").disabled			= true;
		$("en_log_sch").disabled			= true;
		$("log_sch").disabled				= true;
	}   
}


function uiOnload(){
	createSchedule();
	setJSONValue({
		'en_mail'       : G_Enable,
		'account_name' : G_SmtpUsername,
		'passwd' : G_SmtpPassword,
		'verify_passwd' : G_SmtpPassword,
		'smtp_server_addr'     : G_SmtpHost,
		'from_addr'     : G_SenderMail,
		'email_subject' : G_MailSubjectName,
		'smtp_server_port':G_SmtpPort,
		'to_addr'   : G_TosendMail,
		'en_logfull'	 : G_Fullenable,
		'en_log_sch'	 : G_ScheduleEnable,
		'log_sch'	 	 : G_ScheduleListName,
		'authenable'	 : G_Authrq		
	});
	OnClickEnable();
	OnClickEnableSchedule();
	OnChangeSchedule();
	dealWithError();
}
function createSchedule(){
		var array_value = [],array_options=[];
		array_value[0]="never";
		array_options[0]=data_languages.Public.innerHTML.Public021;		
		for(var k = 0; k < schedule_list.length; k++){
			array_value[k+1]=schedule_list[k][0];
			array_options[k+1]=schedule_list[k][0];
		}
		$S('log_sch', array_options, array_value);
}
function IsVaildPasswd(pwd)
{
	for(var i=0; i<pwd.length; i++) if ( (pwd.charCodeAt(i) >=1 && pwd.charCodeAt(i) <= 31) || pwd.charCodeAt(i) >= 127 ) return false;
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
function IsVaildEmail(email)
{	
	var strReg=/^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/i;
	if(email.search(strReg)==-1) return false;
	else return true;
}
function IsVaildPort(port)
{		
	if (!TEMP_IsDigit(port) || parseInt(port, 10)>65535 || parseInt(port, 10)<1) return false;
	return true;
}	
function TEMP_IsDigit(no)
{
	if (no==""||no==null)
		return false;
	if (no.toString()!=parseInt(no, 10).toString())
		return false;

	return true;
}
function checkData()
{
    if($(en_mail).checked)    
    { 
		var fro = $("from_addr").value;
		var to = $("to_addr").value;
		var sub = $("email_subject").value;
		var server = $("smtp_server_addr").value;
		var SMTP_port = $("smtp_server_port").value;
		var user = $("account_name").value;
		var passwd = $("passwd").value;
		var verpasswd = $("verify_passwd").value;
		if(IsBlank(fro))
		{
			alert(SEcode['lang_email_addr']);
			$("from_addr").focus();
			return false;
		}
		if(IsBlank(to))
		{
			alert(SEcode['lang_email_addr']);
			$("to_addr").focus();
			return false;
		}
		if(!IsVaildEmail(fro))
		{
			alert(SEcode['lang_email_addr']);
			$("from_addr").focus();
			return false;
		}
		if(!IsVaildEmail(to))
		{
			alert(SEcode['lang_email_addr']);
			$("to_addr").focus();
			return false;
		}
		if(IsBlank(server))
		{
			alert(SEcode['lang_email_server']);
			$("smtp_server_addr").focus();
			return false;
		}	
		if(!IsVaildPort(SMTP_port))
		{
			alert(SEcode['lang_email_smtpport']);
			$("smtp_server_port").focus();
			return false;
		}
		if($("authenable").checked)
		{
			if(IsBlank(user))
			{
				alert(SEcode['lang_email_user']);
				$("account_name").focus();
				return false;
			}
			if(IsBlank(passwd))
			{
				alert(SEcode['lang_email_passwd']);
				$("passwd").focus();
				return false;
			}
			if(IsBlank(verpasswd))
			{	
				alert(SEcode['lang_email_passwd']);
				$("verify_passwd").focus();
				return false;
			}
			if(!IsVaildPasswd(passwd))
			{
				alert(SEcode['lang_email_passwd']);
				$("passwd").focus();
				return false;
			}
			if(!IsVaildPasswd(verpasswd))
			{
				alert(SEcode['lang_email_passwd']);
				$("verify_passwd").focus();
				return false;
			}
			if(passwd != verpasswd)
			{
				alert(SEcode['lang_email_vfpasswd']);
				$("passwd").focus();
				return false;
			}
		}
	}	
    return true;
}
function uiSubmit(){
	var node_mail= $('en_mail','account_name','passwd','smtp_server_addr','from_addr','email_subject','smtp_server_port','to_addr','en_logfull','en_log_sch','log_sch','authenable');
	if(checkData() == false)
	{return false;}	
	
	if($("en_mail").checked)
	{
		$H({
			":InternetGatewayDevice.X_TWSZ-COM_Mail.Enable"       : node_mail[0].checked ? 1 : 0,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.SmtpUsername" : node_mail[11].checked ? node_mail[1].value : "",
			":InternetGatewayDevice.X_TWSZ-COM_Mail.SmtpPassword" : node_mail[11].checked ? node_mail[2].value: "",
			":InternetGatewayDevice.X_TWSZ-COM_Mail.SmtpHost"     : node_mail[3].value,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.TosendMail"   : node_mail[7].value,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.SenderMail"   : node_mail[4].value,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.SmtpPort"     : node_mail[6].value,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.Fullenable"   : node_mail[8].checked ? 1 : 0,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.Authrq" 	  : node_mail[11].checked ? 1 : 0,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.ScheduleListName"   : node_mail[10].value,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.ScheduleEnable"     : node_mail[9].checked ? 1 : 0,
			":InternetGatewayDevice.X_TWSZ-COM_Mail.MailSubjectName" 	: node_mail[5].value,
			'obj-action'   : 'set',
			'var:menu'     : G_Menu,
			'var:page'     : G_Page,
			'var:errorpage': G_Page,
			'var:sys_Token' : G_SysToken,
			'getpage'      : 'html/index.html',
			'errorpage'    : 'html/index.html',
			'var:CacheLastData': ViewState.Save()
		});
	}
	else
	{
		$H({
			":InternetGatewayDevice.X_TWSZ-COM_Mail.Enable"       : node_mail[0].checked ? 1 : 0,
			'obj-action'   : 'set',
			'var:menu'     : G_Menu,
			'var:page'     : G_Page,
			'var:errorpage': G_Page,
			'var:sys_Token' : G_SysToken,
			'getpage'      : 'html/index.html',
			'errorpage'    : 'html/index.html',
			'var:CacheLastData': ViewState.Save()
		});
	}
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('sendmail').disabled= true;
	$('email003').disabled= true;
	$('email022').disabled= true;
}

function dealWithError(){
	if (G_Error != 1){ return false;}
	
	var arrayHint = [];
	arrayHint['Host']          	= 'INPUT_Host';
	arrayHint['MaxHopCount'] 		= 'INPUT_MaxHopCount';
	arrayHint['Timeout']       = 'INPUT_Timeout';
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload);


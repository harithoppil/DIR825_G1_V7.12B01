<?mget :InternetGatewayDevice.X_TWSZ-COM_RemoteManage. "HttpsEn RemanageEn RePort ReHttpsEn InFilter"
`	var G_HttpsEn 		      = "$01";
	var G_RemanageEn 		  = "$02";
	var G_RePort      		  = "$03";
	var G_ReHttpsEn 		  = "$04";
	var G_InFilter    		  = "$05";  
`?>
var G_inbf = [];
var k = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList. "Name IPRange Action"
`	G_inbf[k] = [];
	G_inbf[k][0] = "InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList.$00.";
	G_inbf[k][1] = "$01";
	G_inbf[k][2] = "$02";
	G_inbf[k][3] = "$03";
	k++;
`?>
var G_PortMapping = [];
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortMapping.PortMapping. "PortMappingProtocol ExternalPort"
`	G_PortMapping[m] = [];
	G_PortMapping[m][0] = "$01";
	G_PortMapping[m][1] = "$02";
	m++;
`?>
var G_PortForward = [];
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortForward.PortForward. "TcpPortMap"
`	G_PortForward[n] = [];
	G_PortForward[n][0] = "$01";
	n++;
`?>
var G_PortTrigger = [];
var t = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_PortTrigger.TriggerList. "OpenPortMap OpenProtocol"
`	G_PortTrigger[t] = [];
	G_PortTrigger[t][0] = "$01";
	G_PortTrigger[t][1] = "$02";
	t++;
`?>
var admin_remote_port_https = "443";
var G_Enable = "<?get :InternetGatewayDevice.X_TWSZ-COM_Authentication.UserList.1.RandomEnable?>";
function uiOnload(){

	setJSONValue({
		'en_captcha' 		: G_Enable,
		'stunnel' 			: G_HttpsEn,
		'en_remote'			: G_RemanageEn,
		'enable_https'		: G_ReHttpsEn
	});
	createInbound();	
	if(G_InFilter=="allowall"||G_InFilter=="denyall")
		$('remote_inb_filter').value = G_InFilter;
	else
	{
		for(var j=0; j<G_inbf.length; j+=1)
		{
			if(G_InFilter==G_inbf[j][0])
			{
				$('remote_inb_filter').value = G_inbf[j][1];
				break;
			}
		}
	}	
	OnClickEnRemote();
	OnClickRemoteInbFilter($('remote_inb_filter').value);
	setJSONValue({
		'remote_port' 		: G_RePort
	});
}

function createInbound(){
	var array_value = [],array_options=[];
	array_value[0]="allowall";
	array_options[0]=data_languages.Public.innerHTML.Public006;
	array_value[1]="denyall";
	array_options[1]=data_languages.Public.innerHTML.Public007;		
	for(var k = 0; k < G_inbf.length; k++){
		array_value[k+2]=G_inbf[k][1];
		array_options[k+2]=G_inbf[k][1];
	}
	$S('remote_inb_filter', array_options, array_value);
}

function OnClickStunnel()
{
	if ($("stunnel").checked && $("en_remote").checked)	
	{
		$("enable_https").disabled = false;
	}
	else							
	{
		$("enable_https").disabled = true;
		$("enable_https").checked = false;
	}
	OnClickEnableHttps();
}
	
function OnClickEnableHttps()
{
	if ($("enable_https").checked)	
	{
		$("remote_port").value="8090";
	}
	else
	{
		$("remote_port").value="8080";
	}
}

function OnClickRemoteInbFilter(inbf_uid)
{
	var str = "";
	if (inbf_uid == "allowall")	str = data_languages.Public.innerHTML.Public006;
	else if (inbf_uid == "denyall") str = data_languages.Public.innerHTML.Public007;
	else
	{				
		for(var i=0;i<G_inbf.length;i++)
		{
			if(inbf_uid==G_inbf[i][1])
			{				
				str = G_inbf[i][3]+" "+G_inbf[i][2];
				break;
			}
		}		
	}		
	$("inb_filter_detail").value = str;
	$("inb_filter_detail").disabled = true;
}

function OnClickEnRemote()
{
	OnClickStunnel();
	if ($("en_remote").checked)	
		$("remote_port").disabled = $("remote_inb_filter").disabled = false;
	else							
		$("remote_port").disabled = $("remote_inb_filter").disabled = $("inb_filter_detail").disabled = true;
}
function PortStringCheck(PortString1, PortString2)
{
	var PortStrArr1 = PortString1.split(",");
	var PortStrArr2 = PortString2.split(",");
	for(var i=0; i < PortStrArr1.length; i++)
	{
		for(var j=0; j < PortStrArr2.length; j++)
		{
			if(PortStrArr1[i].match("-")=="-" && PortStrArr2[j].match("-")=="-")
			{
				var PortRange1 = PortStrArr1[i].split("-");
				var PortRangeStart1	= parseInt(PortRange1[0], 10);
				var PortRangeEnd1	= parseInt(PortRange1[1], 10);
				var PortRange2 = PortStrArr2[j].split("-");
				var PortRangeStart2	= parseInt(PortRange2[0], 10);
				var PortRangeEnd2	= parseInt(PortRange2[1], 10);
				if(PortRangeStart2 <= PortRangeEnd1 &&  
					PortRangeStart1 <= PortRangeEnd2) return true;
			}	
			else if(PortStrArr1[i].match("-")=="-")
			{
				var PortRange1 = PortStrArr1[i].split("-");
				var PortRangeStart1	= parseInt(PortRange1[0], 10);
				var PortRangeEnd1	= parseInt(PortRange1[1], 10);
				if(PortRangeStart1 <= parseInt(PortStrArr2[j], 10) && 
					parseInt(PortStrArr2[j], 10) <= PortRangeEnd1) return true;
			}
			else if(PortStrArr2[j].match("-")=="-")
			{
				var PortRange2 = PortStrArr2[j].split("-");
				var PortRangeStart2	= parseInt(PortRange2[0], 10);
				var PortRangeEnd2	= parseInt(PortRange2[1], 10);
				if(PortRangeStart2 <= parseInt(PortStrArr1[i], 10) && 
					parseInt(PortStrArr1[i], 10) <= PortRangeEnd2) return true;
			}
			else
			{
				if(parseInt(PortStrArr1[i], 10)==parseInt(PortStrArr2[j], 10)) return true;
			}					
		}
	}
	return false;
}
//鎻愪氦鏁版嵁
function uiSubmit(){
	//check password
	if($('en_remote').checked)
	{
		if ($("remote_port").value == "" || isNaN($("remote_port").value) || Number($("remote_port").value) < 1 || Number($("remote_port").value) > 65535)
		{
			alert(SEcode['lang_pass_port']);
			$("remote_port").focus();
			return false;
		}
		for(var i=0;i<G_PortMapping.length;i++)
		{
			if(G_PortMapping[i][0]=="TCP" || G_PortMapping[i][0]=="TCP/UDP")
			{
				if(G_PortMapping[i][1]==$("remote_port").value || (G_PortMapping[i][1]==admin_remote_port_https && $('enable_https').checked==true))
				{	
					alert(SEcode['lang_pass_pvused']);
					$("remote_port").focus();
					return false;
				}
			}
		}	
		for(var i=0;i<G_PortForward.length;i++)
		{			
			if(PortStringCheck(G_PortForward[i][0],$("remote_port").value) || (PortStringCheck(G_PortForward[i][0],admin_remote_port_https) && $('enable_https').checked==true))
			{
				alert(SEcode["lang_pass_psused"]);
				$("remote_port").focus();
				return false;
			}
		}
		for(var i=0;i<G_PortTrigger.length;i++)
		{
			if(G_PortTrigger[i][1]=="TCP" || G_PortTrigger[i][1]=="TCP/UDP")
			{
				if(PortStringCheck(G_PortTrigger[i][0], $("remote_port").value) || (PortStringCheck(G_PortTrigger[i][0], admin_remote_port_https) && $('enable_https').checked==true))
				{
					alert(SEcode['lang_pass_paused']);
					$("remote_port").focus();
					return false;
				}
			}
		}			
	}
	if($('admin_p1').value != $('admin_p2').value){
		alert(SEcode[1010]);
		return false;
	}

	$H({
		'obj-action'   : 'set',
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:sys_Token' : G_SysToken,
		'var:errorpage': G_Page,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:CacheLastData': ViewState.Save()
	}, true);	
	$F(':InternetGatewayDevice.X_TWSZ-COM_Authentication.UserList.1.RandomEnable', $('en_captcha').checked?'1':'0');
	$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.HttpsEn', $('stunnel').checked?'1':'0');
	$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.RemanageEn', $('en_remote').checked?'1':'0');
	if($('en_remote').checked)
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.RePort', $('remote_port').value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.ReHttpsEn', $('enable_https').checked?'1':'0');
		if($('remote_inb_filter').value=="allowall"||$('remote_inb_filter').value=="denyall")
			$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.InFilter', $('remote_inb_filter').value);
		else
		{
			for(var k = 0; k < G_inbf.length; k++){
				if($('remote_inb_filter').value==G_inbf[k][1])
				{
					$F(':InternetGatewayDevice.X_TWSZ-COM_RemoteManage.InFilter', G_inbf[k][0]);
					break;
				}
			}
		}	
	}
	if($('admin_p1').value != '')
	{
		var pass = $("admin_p1").value;
		var Basepsw= Base64.Encode(pass);
		$F(':InternetGatewayDevice.X_TWSZ-COM_Authentication.UserList.1.Password', Basepsw);
	}
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('PWD019').disabled= true;
	$('PWD004').disabled= true;
}

//閿欒澶勭悊鍑芥暟
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//鐩戝惉鍔犺浇涓庨敊璇鐞嗗嚱鏁?
addListeners(uiOnload, dealWithError);

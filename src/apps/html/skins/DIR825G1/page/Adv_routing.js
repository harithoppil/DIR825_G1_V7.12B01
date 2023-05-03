
//wan
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<? if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "Enable ExternalIPAddress"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.$00";
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['ExternalIPAddress']		= "$02";	//ExternalIPAddress
		`?>
	`?>

	<? if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "Enable ExternalIPAddress"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.$00";	//Path		
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['ExternalIPAddress']			= "$02";	// ExternalIPAddress
		`?>
	`?>
`?>

<? if lt 0 `<?get :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterfaceNumberOfEntries?>`
`	<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
	`	var G_LanIPAddress  = "$01";
		var G_LanSubAddress = "$02";
	`?>
`?>

var G_Forwarding = [];
var m = 0;
<?objget :InternetGatewayDevice.Layer3Forwarding.Forwarding. "Enable Name DestIPAddress DestSubnetMask ForwardingMetric Interface GatewayIPAddress"
`	G_Forwarding[m] = [];
	G_Forwarding[m][0] = "$01";
	G_Forwarding[m][1] = "$02";
	G_Forwarding[m][2] = "$03";
	G_Forwarding[m][3] = "$04";
	G_Forwarding[m][4] = "$05";
	G_Forwarding[m][5] = "$06";
	G_Forwarding[m][6] = "$07";
	m++;
`?>

function alertError()
{
	var code=arguments[0];
    alert(code);   
}
function uiSubmit()
{
	var i = 0;

	for(i=1; i<33; i++)
	{		
		if($("name_"+i).value == "" && $("dstip_"+i).value == "" && $("netmask_"+i).value == "" && $("gateway_"+i).value == "")
		{
			$("enable_"+i).checked = false;
		}
		else
		{
		
			if(($('name_'+i).value == "") || ($('name_'+i).value == " "))
			{
				alert(SEcode['lang_name_not_blank']);
				return false;
			}

			if(($('dstip_'+i).value == "") || ($('dstip_'+i).value == " "))
			{
				alert(SEcode['lang_destip_invalid']);
				return false;
			}
			if (!CheckValidity.IP('dstip_'+i,SEcode['lang_destip_invalid']))
				return false;
			
			if(($('netmask_'+i).value == "") || ($('netmask_'+i).value == " "))
			{
				alert(SEcode['lang_mask_invalid']);
				return false;
			}

			if ($('netmask_'+i).value != "255.255.255.255")
			{
				if (!CheckValidity.Mask('netmask_'+i,SEcode['lang_mask_invalid']))
				return false;
			}	
			if(($('gateway_'+i).value == "") || ($('gateway_'+i).value == " "))
			{
				alert(SEcode['lang_gateway_invalid']);
				return false;
			}

			if (!CheckValidity.IP('gateway_'+i,SEcode['lang_gateway_invalid']))
				return false;

			if($('dstip_'+i).value == G_LanIPAddress)
			{
				alertError(SEcode['lang_ip_not_same']);
				return false;
			}

			if($('gateway_'+i).value == G_LanIPAddress)	
			{
				alertError(SEcode['lang_gw_not_same']);
				return false;
			}
			if(isSameSubNet($("gateway_"+i).value,G_LanSubAddress,G_LanIPAddress,G_LanSubAddress))
			{
				alert(SEcode["lang_gw_lan_conflict"]);	
				$("gateway_"+i).focus();
				return false;
			}
			for (var j=1; j < i; j+=1)
			{
				if($("name_"+i).value==$("name_"+j).value && $("name_"+i).value!="") 
				{
					alert(SEcode['lang_name_not_same']);
					$("name_"+j).focus();
					return false;
				}	
			}
		}
	}	
	for(i=1; i<33; i++)
	{
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.Enable' , $('enable_'+i).checked?"1":"0");
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.Name' , $('name_'+i).value);
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.DestIPAddress' , $('dstip_'+i).value);
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.DestSubnetMask' , $('netmask_'+i).value);
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.ForwardingMetric' , $('metric_'+i).value);
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.Interface' , G_WANConn['Path']);//or InternetGatewayDevice.LANDevice.1
		$F(':InternetGatewayDevice.Layer3Forwarding.Forwarding.'+i+'.GatewayIPAddress' , $('gateway_'+i).value);
	}
	
	$F('obj-action','set');	
	
	$H({
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:errorpage': G_Page,
		'var:CacheLastData': ViewState.Save()
	});
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_rt002').disabled= true;
	$('Adv_rt136').disabled= true;
}

function uiOnload()
{
	var rmd = 0;
	for(var i=1; i<=32; i+=1)
	{
		$('enable_'+i).checked = G_Forwarding[i-1][0]=="1"?true:false;
		if(G_Forwarding[i-1][0]=="1") rmd++;
		$('name_'+i).value = G_Forwarding[i-1][1];
		$('dstip_'+i).value = G_Forwarding[i-1][2];
		$('netmask_'+i).value = G_Forwarding[i-1][3];
		$('gateway_'+i).value = G_Forwarding[i-1][6];
		$('metric_'+i).value = G_Forwarding[i-1][4];

		var inf_val = document.getElementById('inf_'+i);
		inf_val.options[0].text = data_languages.Adv_routing.innerHTML.Adv_rt140+"("+G_WANConn['ExternalIPAddress']+")";
		
	}
	$('rmd').innerHTML = 32-rmd;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

var G_Wanipv6Conn; //wan 
var G_Lanipv6Conn; //lan

<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 2
`	<? if gt $21 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection. "Enable"
		`	G_Wanipv6Conn = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection.$00.";
	`?>
	`?>

	<? if gt $22 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable"
		`	G_Wanipv6Conn = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection.$00.";	//Path		
	`?>
	`?>
`?>
`?>


<?objget :InternetGatewayDevice.LANDevice. "X_TWSZ-COM_DeviceName"
`	G_Lanipv6Conn = "InternetGatewayDevice.LANDevice.$00";//path
`?>

var G_Forwarding = [];
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding. "Enable Name Interface ForwardingMetric GatewayIPv6Address DestIPv6Address DestPrefixLength"
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



function uiSubmit()
{
	var i = 0;
	
	for(i=1; i<=10; i++)
	{		
		if(!CheckData(i))return false;
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Enable' , $('enable_'+i).checked?"1":"0");
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Name' , $('dsc_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.DestIPv6Address' , $('dest1_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.DestPrefixLength' , $('dest2_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.ForwardingMetric' , $('metric_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.GatewayIPv6Address' , $('gateway_'+i).value);

		switch ($('inf_'+i).value)
		{
			case "NULL" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Interface' , "");
				break;
			case "LAN-4" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Interface' , G_Lanipv6Conn);//
				break;
			case "WAN-4" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Interface' , G_Wanipv6Conn);//
				break;
			case "PD" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.'+i+'.Interface' , G_Lanipv6Conn);// use lan
				break;
			default :
				break;
					
		}
		
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
	$('Adv_routing6002').disabled= true;
	$('Adv_routing6085').disabled= true;
}


function OnChangeinf(index)
{
	var inf	= $("inf_"+index).value;
	if(inf==="PD")
	{
		$("gateway_"+index).disabled = true;
		$("metric_"+index).disabled = true;
	}
	else
	{
		$("gateway_"+index).disabled = false;
		$("metric_"+index).disabled = false;
	}
}
function CheckData(index)
{
	if($("enable_"+index).checked)
	{
		if($("dsc_"+index).value == "" )
		{
			alert(SEcode['lang_name_empty']);
			$("dsc_"+index).focus();
			return false;
		}
		else if($("dest1_"+index).value == "" )
		{
			alert(SEcode['lang_destip_ipv6_empty']);
			$("dest1_"+index).focus();
			return false;
		}
		else if($("dest2_"+index).value == "")
		{
			alert(SEcode['lang_prefix_empty']);
			$("dest2_"+index).focus();
			return false;
		}
		else if($("metric_"+index).value == "")
		{
			alert(SEcode['lang_metric_empty']);
			$("metric_"+index).focus();
			return false;
		}
		else if($("gateway_"+index).value == "" )
		{
			alert(SEcode['lang_gateway_empty']);
			$("gateway_"+index).focus();
			return false;
		}
		for(var j=1; j < index ; j++)
		{
			var dsc = $("dsc_"+index).value;
			if($("dsc_"+index).value == $("dsc_"+j).value)
			{
				alert(SEcode['lang_name']+dsc+SEcode['lang_already_used']);
				$("dsc_"+index).focus();
				return false;
			}
		}
	}
	
	return true;
}

function InitValue()
{
	var index =0;
	for(var i=0;i<G_Forwarding.length;i++)
	{
		index = i+1;
		$('enable_'+index).checked = G_Forwarding[i][0]==1? true : false;
		$('dsc_'+index).value = G_Forwarding[i][1];
		switch(G_Forwarding[i][2])
		{
			case "" :
				break;
			case G_Lanipv6Conn :
					$("inf_"+index).value = "LAN-4";
				break;
			case G_Wanipv6Conn :
					$("inf_"+index).value = "WAN-4";
				break;
			case G_Lanipv6Conn ://DHCP-PD
			
				break;
			default :
				break;
		}
		OnChangeinf(index);
		$('metric_'+index).value = G_Forwarding[i][3];
		$('gateway_'+index).value = G_Forwarding[i][4];
		$('dest1_'+index).value = G_Forwarding[i][5];
		$('dest2_'+index).value = G_Forwarding[i][6];
	}
}

function uiOnload()
{
	InitValue();
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

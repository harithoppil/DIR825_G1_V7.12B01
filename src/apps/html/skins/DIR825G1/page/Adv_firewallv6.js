var schedule_num=0;
var schedule_list=[];
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName Enable"
	`schedule_list[schedule_num] = [];
	 schedule_list[schedule_num][0]= "$01";
	 schedule_list[schedule_num][1]= "$02";
	 schedule_list[schedule_num][2]= "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";
	 schedule_num++;
`?>
var G_Wanipv6Conn; //wan 
var G_Lanipv6Conn; //lan

<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`   <? if eq $10 2
`	<? if gt $21 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection. "Enable"
		`	G_Wanipv6Conn = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANIPConnection.$00";
	`?>
	`?>

	<? if gt $22 0	
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection. "Enable"
		`	G_Wanipv6Conn = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.2.WANPPPConnection.$00";	//Path		
	`?>
	`?>
`?>
`?>


<?objget :InternetGatewayDevice.LANDevice. "X_TWSZ-COM_DeviceName"
`	G_Lanipv6Conn = "InternetGatewayDevice.LANDevice.$00";//path
`?>

var G_Ipv6Filter = [];
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter. "Enable Description SrcIP SrcPrefixLen DestIP DstPrefixLen Protocol DestPort DevPath SchedulePath"
`	G_Ipv6Filter[m] = [];
	G_Ipv6Filter[m][0] = "$01"; //name 
	G_Ipv6Filter[m][1] = "$02";	//desc
	G_Ipv6Filter[m][2] = "$03"; //srcip
	G_Ipv6Filter[m][3] = "$04"; //src prefixlen
	G_Ipv6Filter[m][4] = "$05";	//dst ip 
	G_Ipv6Filter[m][5] = "$06"; //dst prefixlen
	G_Ipv6Filter[m][6] = "$07";	//protocol
	G_Ipv6Filter[m][7] = "$08"; //dst port
	G_Ipv6Filter[m][8] = "$09"; //dev path
	G_Ipv6Filter[m][9] = "$0a"; //SchedulePath	
	m++;
`?>


//ipv6 filter
var G_SPIEnable = "<?get :InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SPIEnable?>"; // spi enable
var G_IPFilterEnable = "<?get :InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPFilterEnable?>"; // filter enable
var G_SecurityLevel  = "<?get :InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SecurityLevel?>";  // security level

var g_max = 20;
function OnChangeProt(index)
{
	var prot = $("pro_"+index).value;

	if (prot==="TCP" || prot==="UDP")
	{
		$("dst_startport_"+index).disabled = false;
		$("dst_endport_"+index).disabled = false;
	}
	else
	{
		$("dst_startport_"+index).disabled = true;
		$("dst_endport_"+index).disabled = true;
	}
}

function uiOnload()
{
	var index =0;
	createSchedule();
	
	if (G_SPIEnable == "1") 	
	{
		$('smp_security').checked = true;		
	}
	else
	{
		$('smp_security').checked = false;				
	}				
	//$('smp_security').value = G_SPIEnable;		

	if (G_IPFilterEnable == "0") 	
	{
		$('mode').value = "DISABLE";		
	}
	else
	{
		if (G_SecurityLevel == "1")	
			$('mode').value = "ACCEPT";		
		else if (G_SecurityLevel == "3") 
			$('mode').value = "DROP";		
	}

	for(var i=0;i< G_Ipv6Filter.length;i++)
	{
		index = i+1;
		$('en_'+index).checked = G_Ipv6Filter[i][0]==1? true : false;
		$('dsc_'+index).value = G_Ipv6Filter[i][1];
		$('src_startip_'+index).value = G_Ipv6Filter[i][2];
		$('src_prelen_'+index).value = G_Ipv6Filter[i][3];
		$('dst_startip_'+index).value = G_Ipv6Filter[i][4];
		$('dst_prelen_'+index).value = G_Ipv6Filter[i][5];
		$('pro_'+index).value = G_Ipv6Filter[i][6];
		if(G_Ipv6Filter[i][6] == "TCP" || G_Ipv6Filter[i][6] == "UDP")
		{
			$("dst_startport_"+index).value = G_Ipv6Filter[i][7].split(":")[0];
			if(G_Ipv6Filter[i][7].split(":")[1]!=undefined)
				$("dst_endport_"+index).value = G_Ipv6Filter[i][7].split(":")[1];
		}
		if(G_Ipv6Filter[i][1]!= "")
			g_max--;
		switch(G_Ipv6Filter[i][8]) //devpath
		{
			case "" :
				break;
			case G_Lanipv6Conn :
					$("src_inf_"+index).value = "LAN-4";
					$("dst_inf_"+index).value = "WAN-4";
				break;
			case G_Wanipv6Conn :
					$("src_inf_"+index).value = "WAN-4";
					$("dst_inf_"+index).value = "LAN-4";
				break;
			default :
				break;
		}

		if(G_Ipv6Filter[i][9]=="Always")
			$('sch_'+index).value = "Always";
		else
		{
			for(var j=0; j<schedule_num; j+=1)
			{
				if(G_Ipv6Filter[i][9]==schedule_list[j][2])
				{
					$('sch_'+index).value = schedule_list[j][0];
					break;
				}
			}
		}
	}

	for(var i=1;i<21;i++)
		OnChangeProt(i);
	$("rmd").innerHTML = g_max;

}
function createSchedule(){
		var array_value = [],array_options=[];
		array_value[0]="Always";
		array_options[0]=data_languages.Public.innerHTML.Public005;
	
		for(var i=1; i<=20; i+=1)
		{
			
			for(var k = 0; k < schedule_list.length; k++){
				array_value[k+1]=schedule_list[k][0];
				array_options[k+1]=schedule_list[k][0];
			}
			
			$S('sch_'+i, array_options, array_value);
			//暂时支持Always
			$('sch_'+i).disabled=true;
		}
		
}

var   r   =   /^[0-9]*[1-9][0-9]*$/　　//正整数   
function CheckData(index)
{
	if($('en_'+index).checked==true && $('dsc_'+index).value=="")
	{
		alert(SEcode['lang_fw_name_empty']);
		$("dsc_"+index).focus();
		return false;
	}
	if($('en_'+index).checked==true || $('dsc_'+index).value!=="")//enabled or has name 
	{
		if($('src_startip_'+index).value=="")
		{
			alert(SEcode['lang_startip_source_empty']);
			$('src_startip_'+index).focus();
			return false;
		}
		if($('src_prelen_'+index).value=="")
		{
			alert(SEcode['lang_prefix_source_empty']);
			$('src_prelen_'+index).focus();
			return false;
		}
		if(!r.test($('src_prelen_'+index).value) )
		{
			alert(SEcode['lang_prefix_source_invalid']);
			$('src_prelen_'+index).focus();
			return false;
		}
		if($('dst_inf_'+index).value == $('src_inf_'+index).value)
		{
			alert(SEcode['lang_interface_not_same']);
			return false;
		}
		if($('dst_startip_'+index).value=="")
		{
			alert(SEcode['lang_startip_dest_empty']);
			$('dst_startip_'+index).focus();
			return false;
		}
		if($('dst_prelen_'+index).value=="")
		{
			alert(SEcode['lang_prefix_dest_empty']);
			$('dst_prelen_'+index).focus();
			return false;
		}
		if(!r.test($('dst_prelen_'+index).value) )
		{
			alert(SEcode['lang_prefix_dest_invalid']);
			$('dst_prelen_'+index).focus();
			return false;
		}
		if($('pro_'+index).value=="TCP" || $('pro_'+index).value=="UDP")
		{
			if($('dst_startport_'+index).value=="")
			{
				alert(SEcode['lang_start_port_empty']);
				$('dst_startport_'+index).focus();
				return false;
			}
			if(!r.test($('dst_startport_'+index).value))
			{
				alert(SEcode['lang_start_port']);
				$('dst_startport_'+index).focus();
				return false;
			}
			if($('dst_endport_'+index).value=="")
			{
				alert(SEcode['lang_end_port_empty']);
				$('dst_endport_'+index).focus();
				return false;
			}
			if(!r.test($('dst_endport_'+index).value))
			{
				alert(SEcode['lang_end_port']);
				$('dst_endport_'+index).focus();
				return false;
			}
		}
		for(var j=1; j < index ; j++)
		{
			var dsc = $("dsc_"+index).value;
			if($("dsc_"+index).value === $("dsc_"+j).value)
			{
				alert(SEcode['lang_name']+dsc+SEcode['lang_already_used']);
				$("dsc_"+index).focus();
				return false;
			}
		}
		//check same rule exist or not
		for(j=1; j < index ; j++)
		{
			var dsc = $("dsc_"+index).value;
			if(	$("src_inf_"+j).value === $("src_inf_"+index).value
				&& $("src_startip_"+j).value === $("src_startip_"+index).value
				&& $("src_prelen_"+j).value === $("src_prelen_"+index).value
				&& $("pro_"+j).value === $("pro_"+index).value
				//&& $("dst_inf_"+j).value === $("dst_inf_"+index).value
				&& $("dst_startip_"+j).value === $("dst_startip_"+index).value
				&& $("dst_prelen_"+j).value === $("dst_prelen_"+index).value
				&& $("sch_"+j).value === $("sch_"+index).value
				&& (
					($("pro_"+j).value !== "TCP" && $("pro_"+j).value !== "UDP")
					||($("dst_startport_"+j).value === $("dst_startport_"+index).value
					&& $("dst_endport_"+j).value === $("dst_endport_"+index).value)
				)
			)
			{
				alert(SEcode['lang_rule']+dsc+SEcode['lang_already_used']);
				return false;
			}
		}
		
	}
	else
	{
		$("dsc_"+index).value = "";
		$("sch_"+index).value = "";
		$("src_inf_"+index).value = "LAN-4";
		$("src_startip_"+index).value = "";
		$("src_prelen_"+index).value = "";
		$("dst_startip_"+index).value = "";
		$("dst_prelen_"+index).value = "";
		$("pro_"+index).value = "ALL";
		
	}
	
	return true;
}
function uiSubmit()
{
	$H({
		'var:menu'     : G_Menu,
		'var:page'     : G_Page,
		'var:errorpage': G_Page,
		'var:sys_Token' : G_SysToken,
		'obj-action'   : 'set',
		'getpage'      : 'html/index.html',
		'errorpage'    : 'html/index.html',
		'var:CacheLastData': ViewState.Save()
	},true);

		$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SPIEnable", 			$('smp_security').checked? "1":"0");

	switch($("mode").value){
		case "DISABLE" :		
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPFilterEnable", 			"0");
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SecurityLevel", 			"1");
			break;
		case "ACCEPT" :
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPFilterEnable", 			"1");
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SecurityLevel", 			"1");
			break;
		case "DROP" :		
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPFilterEnable", 			"1");
			$F(":InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.SecurityLevel", 			"3");
			break;
	}

	for(i=1; i<=20; i++)
	{		
		if(!CheckData(i))return false;
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.Enable' , $('en_'+i).checked? "1":"0");
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.Description' , $('dsc_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.SrcIP' , $('src_startip_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.SrcPrefixLen' , $('src_prelen_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DestIP' , $('dst_startip_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DstPrefixLen' , $('dst_prelen_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.Protocol' , $('pro_'+i).value);
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DevPath' , $('src_inf_'+i).value =="LAN-4"? G_Lanipv6Conn : G_Wanipv6Conn);

		switch ($('pro_'+i).value)
		{
			case "ALL" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DestPort' , "");
				break;
			case "TCP" :
			case "UDP" :
				//$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DestPort' , $('dst_startport_'+i).value + ':' + $('dst_endport_'+i).value);//
				$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DestPort' , (($('dst_startport_'+i).value + ':' + $('dst_endport_'+i).value).replace(/(^:*)|(:*$)/g,'')));//
				
				break;
			case "ICMPv6" :
				$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.DestPort' , "");
				break;
			default :
				break;
					
		}

		if($('sch_'+i).value == "Always"){
		$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.SchedulePath' , 'Always');
		}
		else
		{
			for(var k = 0; k < schedule_list.length; k++){	
				if($('sch_'+i).value==schedule_list[k][0])
				{
					$F(':InternetGatewayDevice.X_TWSZ-COM_IP6Firewall.IPV6Filter.'+i+'.SchedulePath' , schedule_list[k][2]);
					break;
				}
			}
		}		
	}	

	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_fw6002').disabled= true;
	$('Adv_fw6310').disabled= true;
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

//addListeners(uiOnload);
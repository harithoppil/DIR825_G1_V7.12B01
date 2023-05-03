/*  Javascript Document:Bas_lan.js  */
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>

<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement. "DHCPServerEnable DHCPRelay MinAddress MaxAddress DHCPLeaseTime X_TWSZ-COM_DNSRelayEnable DNSServers DomainName X_TWSZ-COM_DHCPRelayAddress X_TWSZ-COM_UseIPRoutersAsDNSServer HostName DHCPServerAlwaysBroadcastEnable"
`	G_DSEnable 		= "$01";
	G_DHCPRelay 	= "$02";
	G_MinAddress 	= "$03";
	G_MaxAddress 	= "$04";
	G_DHCPLeaseTime = "$05";
	G_DNSRelayEnable= "$06";
	G_DNSServers 	= "$07";
	G_DomainName 	= "$08";
	G_DHCPRelayAddr = "$09";
    G_UseIPRoutersAsDNSServer = "$0a";
    G_Hostname = "$0b";
	G_Brd = "$0c";
`?>

//预留IP
var G_DHCPResAddr = [];
var n = 0;
<?objget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.X_TWSZ-COM_DHCPFilter.WhiteList. "FilterIP FilterMAC PerEnabled FilterPCName"
`	G_DHCPResAddr[n] = [];
	G_DHCPResAddr[n][0] = "$00";
	G_DHCPResAddr[n][1] = "$01";
	G_DHCPResAddr[n][2] = "$02";
	G_DHCPResAddr[n][3] = "$03";
	G_DHCPResAddr[n][4] = "$04";
	G_DHCPResAddr[n][5] = "InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.X_TWSZ-COM_DHCPFilter.WhiteList.$00.";
	n++;
`?>

var LanHosts = [];
var m = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "HostName MACAddress IPAddress LeaseTimeRemaining VendorClassID AddressSource"
		`	LanHosts[m] = [];
			LanHosts[m][0] = m+1;
			LanHosts[m][1] = "<?echo $11?>"=="ZFc1cmJtOTNiZz09"?"unknown":strAnsi2Unicode((Base64.Decode("<?echo $11?>")));
			LanHosts[m][2] = "<?echo $12?>";
			LanHosts[m][3] = "<?echo $13?>";
			LanHosts[m][4] = "<?echo $14?>";
			LanHosts[m][5] = "<?echo $16?>";
			++m;
		`?>
	`?>
`?>

var G_CurrentIP = "<?echo $var:sys_RemoteAddr ?>";
var G_CurrentMAC = GetMACByIP(G_CurrentIP).toUpperCase();
var G_WANConn = [];
<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<? if gt $11 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection. "Enable ExternalIPAddress SubnetMask AddressingType ConnectionType"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.$00.";
			G_WANConn['Enable'] 			= "$01";	//Enable			
			G_WANConn['ExternalIPAddress'] 		= "$02";	//ExternalIPAddress
			G_WANConn['SubnetMask'] 		= "$03";	//SubnetMask			
			G_WANConn['AddressingType'] 		= "$04";	//AddressingType
			G_WANConn['ConnectionType'] 		= "$05";	//ConnectionType			
		`?>
	`?>

	<? if gt $12 0
	`	<? objget :InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection. "Enable X_TWSZ-COM_StaticIPAddress ConnectionType X_TWSZ-COM_VPN_CLIENT X_TWSZ-COM_VPN_NETMASK X_TWSZ-COM_VPN_ADDR_MODE  LocalNetMask"
		`	G_WANConn['Path'] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.$00.";	//Path		
			G_WANConn['Enable'] 			= "$01";	//Enable
			G_WANConn['StaticIPAddress'] 		= "$02";	//X_TWSZ-COM_StaticIPAddress			
			G_WANConn['ConnectionType'] 		= "$03";	//ConnectionType
			G_WANConn['X_TWSZ-COM_VPN_CLIENT']     = "$04";	//
			G_WANConn['X_TWSZ-COM_VPN_NETMASK']    = "$05";	//			
			G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE'] 	= "$06";	//X_TWSZ-COM_VPN_ADDR_MODE			
			G_WANConn['LocalNetMask'] 			= "$07";	// LocalNetMask
		`?>
	`?>
`?>
var G_TunConn = [];
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPTunnel. "Activated"
`
	G_TunConn['Path'] 		= ":InternetGatewayDevice.X_TWSZ-COM_IPTunnel.$00.";
	G_TunConn['Activated']   = "$01";	//Activated
`?>
var _conflict="0";
function findProtocol(){
    var _protocol = 'dhcp';
	var Conn_Type = G_WANConn['Path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";

	if (G_WANConn['Enable'] == "0" && G_TunConn['Activated'] == "1")
	{
			_protocol = 'dslite';
			return _protocol;
	}

	if(Conn_Type == 'PPP')
	{
		if(G_WANConn['ConnectionType']=="PPTP_Relay"){
			_protocol = 'pptp';
			_conflict = G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"?"1":"0";
		}else if(G_WANConn['ConnectionType']=="L2TP_Relay"){
			_protocol = 'l2tp';
			_conflict = G_WANConn['X_TWSZ-COM_VPN_ADDR_MODE']=="Static"?"1":"0";
		}else{
			_protocol = 'pppoe';
			_conflict = G_WANConn['StaticIPAddress']==""?"0":"1";
		}
	}else
	{
		if(G_WANConn['ConnectionType'] != 'IP_Bridged')
		{
			if(G_WANConn['AddressingType'] == 'Static')
			{
				_protocol = 'static';
				_conflict = "1";
			}else{
				_protocol = 'dhcp';
			}
		}
	}
	return _protocol;
}
//MAC Address Clone
function GetMACByIP(ip){
	for (var i = 0; i < LanHosts.length; i++){
		if (LanHosts[i][3] == ip){
			return LanHosts[i][2];
		}
	}

	return "";
}
//全局变量
var action = 0;		//0 ==> 新增；1 ==> 编辑
var cur_index;		//当前编辑实例


//加载初始化
function uiOnload(){
	
	var Pir_DNS = G_DNSServers.split(',')[0], Sec_DNS = G_DNSServers.split(',')[1];
	
	setJSONValue({
		
		"ipaddr" 			: G_IPAddress,
		"netmask" 			: G_SubMask,
		"domain" 			: G_Hostname,
		"dnsr" 				: G_DNSRelayEnable,
		"device" 			: G_DomainName,
		
		"startip" 			: G_MinAddress.split('.')[3],
		"endip" 			: G_MaxAddress.split('.')[3],
		"leasetime" 		: G_DHCPLeaseTime,
		"primarywins" 		: Pir_DNS || "",
		"secondarywins" 	: Sec_DNS || ""
		
	});
	
	//Form.Radio('RAD_dnsrelay', G_DNSRelayEnable || '1');
	
	if(G_DSEnable == '1')
	{
		$('dhcpsvr').checked = true;
	}
	
	if(G_Brd == '1')
	{
		$('broadcast').checked = true;
	}

	OnClickDHCPSvr();
	
	on_check_netbios();
	
	for(var i=0; i<LanHosts.length; i++)
	{
		
		var x=document.getElementById("pc");
		var option=document.createElement("option");
		//option.text=strAnsi2Unicode(Base64.Decode( LanHosts[i][0] ));
		option.text=LanHosts[i][1];
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
	//created reservation table
	createResTb();
	createHost();
	if(G_DSEnable == '1')
		createDHCPClientTb();
}
function createHost(){
		var array_value = [],array_options=[];
		array_value[0]="";
		array_options[0]=data_languages.Public.innerHTML.Public008;
		for(var k = 0; k < LanHosts.length; k++){
			array_value[k+1]=LanHosts[k][3];
			array_options[k+1]=LanHosts[k][1]=="unknown"?data_languages.Public.innerHTML.Public010:LanHosts[k][1];
		}
		$S('pc', array_options, array_value);
}
function InjectTable(tblID, uid, data, type)
{
	var rows = $(tblID).getElementsByTagName("tr");
	var tagTR = null;
	var tagTD = null;
	var i;
	var str;
	var found = false;
	
	/* Search the rule by UID. */
	for (i=0; !found && i<rows.length; i++) if (rows[i].id == uid) found = true;
	if (found)
	{
		for (i=0; i<data.length; i++)
		{
			tagTD = $(uid+"_"+i);
			switch (type[i])
			{
				case "checkbox":
					str = "<input type='checkbox'";
					str += " id="+uid+"_check_"+i;
					if (COMM_ToBOOL(data[i])) str += " checked";
					str += " disabled>";
					tagTD.innerHTML = str;
					break;
				case "text":
					str = data[i];
					if(typeof(tagTD.innerText) !== "undefined")	tagTD.innerText = str;
					else if(typeof(tagTD.textContent) !== "undefined")	tagTD.textContent = str;
					else	tagTD.innerHTML = str;
					break;	
				default:
					str = data[i];
					tagTD.innerHTML = str;
					break;
			}
		}
		return;
	}

	/* Add a new row for this entry */
	tagTR = $(tblID).insertRow(rows.length);
	tagTR.id = uid;
	/* save the rule in the table */
	for (i=0; i<data.length; i++)
	{
		tagTD = tagTR.insertCell(i);
		tagTD.id = uid+"_"+i;
		tagTD.className = "content";
		switch (type[i])
		{
		case "checkbox":
			str = "<input type='checkbox'";
			str += " id="+uid+"_check_"+i;
			if (COMM_ToBOOL(data[i])) str += " checked";
			str += " disabled>";
			tagTD.innerHTML = str;
			break;
		case "text":
			str = data[i];
			if(typeof(tagTD.innerText) !== "undefined")	tagTD.innerText = str;
			else if(typeof(tagTD.textContent) !== "undefined")	tagTD.textContent = str;
			else	tagTD.innerHTML = str;
			break;
		default:
			str = data[i];
			tagTD.innerHTML = str; 
			break;
		}
	}
}
function OnClickMacButton($name)
{
	$($name).value = G_CurrentMAC;
}
function OnChangeGetClient()
{
	if($("pc").value=="")
	{
		return;
	}
	else
	{
		for(var k = 0; k < LanHosts.length; k++){
			if($("pc").options[$("pc").selectedIndex].value==LanHosts[k][3])
			{
				$("en_dhcp_reserv").checked= true;
				$("reserv_host").value = LanHosts[k][1];
				$("reserv_ipaddr").value = LanHosts[k][3];
				$("reserv_macaddr").value = LanHosts[k][2];
				$("pc").value = "";
				break;
			}
		}		
	}
}

//把要删除的数据和列表数据比对，得出列表的路径序号
function find_num(index)
{
	for(var i=0;i<G_DHCPResAddr.length;i++)
	{
		if($("en_dhcp_ipaddr_"+index).innerHTML == G_DHCPResAddr[i][1] 
		&& $("en_dhcp_macaddr_"+index).innerHTML == G_DHCPResAddr[i][2])
			return i;
	}
	
	return -1;
}

function Add_Handle(index)
{
	$H({
		'add-obj'			:'InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.X_TWSZ-COM_DHCPFilter.WhiteList.',
		':PerEnabled'   	: $('en_dhcp_reserv').checked==true ? 1 : 0,
		':FilterPCName'   	: $('reserv_host').value,
		':FilterIP'   		: $('reserv_ipaddr').value,
		':FilterMAC' 		: $('reserv_macaddr').value,
		"obj-action" 		: "add-set",
		"getpage" 			: "html/index.html",
		"errorpage" 		: "html/index.html",
		"var:menu" 			: G_Menu,
		"var:page" 			: G_Page,
		'var:sys_Token' : G_SysToken,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" : ViewState.Save()
	}, true);
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("BLAN052").disabled = true;
	$("BLAN057").disabled = true;
	$("BLAN055").disabled = true;
	for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
	{
		$("delete_"+i).disabled = true;
	}
}
function Edit_Handle(index)
{
	var num = find_num(index);
	
	var path_enable=	':'+G_DHCPResAddr[num][5] + 'PerEnabled';
	var path_pcname=	':'+G_DHCPResAddr[num][5] + 'FilterPCName';
	var path_ip=		':'+G_DHCPResAddr[num][5] + 'FilterIP';
	var path_mac=		':'+G_DHCPResAddr[num][5] + 'FilterMAC';
	
	$H({		
		'obj-action' 	    : 'set',
		"getpage" 			: "html/index.html",
		"errorpage" 		: "html/index.html",
		"var:menu" 			: G_Menu,
		"var:page" 			: G_Page,
		'var:sys_Token' : G_SysToken,
		"var:errorpage" 	: G_Page,
		'var:CacheLastData' : ViewState.Save()
	}, true);
	
	$F(path_enable,		$('en_dhcp_reserv').checked==true? 1 : 0);
	$F(path_pcname,		$('reserv_host').value);
	$F(path_ip,			$('reserv_ipaddr').value);
	$F(path_mac,		$('reserv_macaddr').value);
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("BLAN052").disabled = true;
	$("BLAN057").disabled = true;
	$("BLAN055").disabled = true;
	for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
	{
		$("delete_"+i).disabled = true;
	}
}
function Del_Handle(index)
{
	var num = find_num(index);//读取相应节点路径序号
	
	if(num != -1)
	{
		$H({
			'del-obj'			:G_DHCPResAddr[num][5],
			':PerEnabled'   	: $('en_dhcp_reserv_'+index).checked==true? 1 : 0,
			':FilterPCName'   	: $('en_dhcp_host_'+index).innerHTML,
			':FilterIP'   		: $('en_dhcp_ipaddr_'+index).innerHTML,
			':FilterMAC' 		: $('en_dhcp_macaddr_'+index).innerHTML,
			"obj-action" 		: "del",
			"getpage" 			: "html/index.html",
			"errorpage" 		: "html/index.html",
			"var:menu" 			: G_Menu,
			"var:page" 			: G_Page,
			"var:errorpage" 	: G_Page,
			'var:sys_Token' : G_SysToken,
			"var:CacheLastData" : ViewState.Save()
		}, true);
		$('uiPostForm').submit();
		$("menu").style.display="none";
		$("content").style.display="none";
		$("mbox").style.display="";
		$("BLAN052").disabled = true;
		$("BLAN057").disabled = true;
		$("BLAN055").disabled = true;
		for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
		{
			$("delete_"+i).disabled = true;
		}
	}
	else
		return false;
	
	return true;
}

var g_edit = 0;
var g_table_index = 1;

function check_reserv(index)
{
	var table = $("reserves_list");
	var rows = table.getElementsByTagName("span");//get all <span>

	for(var i=1;i<rows.length;i=i+3)
	{
		if(rows[i].innerHTML == $('reserv_ipaddr').value)
		{
			alert(SEcode["lang_ip_dup"]);
			return true;
		}
		else if(rows[i+1].innerHTML == $('reserv_macaddr').value)
		{
			alert(SEcode["lang_mac_dup"]);
			return true;
		}
	}
	
	return false;
}
function AddDHCPReserv()
{
	var i=0;
	
	if(!CheckValidity.IPMaskVaild($("reserv_ipaddr").value)) return false;
	
	if(g_edit!=0)//Edit
	{
		i=g_edit;
		Edit_Handle(i);
	}
	else//ADD
	{
		i=g_table_index;
		if (i > 24) 
		{
			alert(SEcode["lang_exce_maxnum"]);
			return false;
		}	
		
		if(!check_reserv(i))
		{
			Add_Handle(i);
		}
		else
		{
			//alert("列表中已经存在");
			return false;
		}
		
	}
	
}

function ClearDHCPReserv()
{		
	$("en_dhcp_reserv").checked = false;
	$("reserv_host").value = "";
	$("reserv_ipaddr").value = "";
	$("reserv_macaddr").value = "";
	$("uiPostForm").setAttribute("modified", "true");		
}
function OnEdit(i)
{
	$("en_dhcp_reserv").checked=$("en_dhcp_reserv_"+i).checked;
	$("reserv_host").value=$("en_dhcp_host_"+i).innerHTML;
	$("reserv_ipaddr").value=$("en_dhcp_ipaddr_"+i).innerHTML;
	$("reserv_macaddr").value=$("en_dhcp_macaddr_"+i).innerHTML;
	g_edit=i;		
}
function OnDelete(i) 
{
	var z;
	var table = $("reserves_list");
	var rows = table.getElementsByTagName("tr");		
	
	if(!Del_Handle(i))
		return false;
	
	for (z=1; z<=rows.length; z++) 
	{
		if(rows[z]!=null)
		{
			if (rows[z].id==i)
			{
				table.deleteRow(z);					
			}												
		}	
	}
									
}

function action_radio(name, flag)
{				
	var $ = document.getElementsByName(name);
	for (var i=0; i<$.length; i++)
	{						
		if(flag == "disable") {$[i].disabled = true;}
		else                  {$[i].disabled = false;}
	}					
}
function on_check_netbios()
{
	var netbios_learn = $("netbios_learn");
	if($("netbios_enable").checked)
	{									
		$("netbios_learn").disabled = false;
		on_check_learn();
	}
	else
	{			
		action_radio("winstype", "disable");
		$("netbios_learn").disabled = true;			
		$("netbios_scope").disabled = true;
		$("primarywins").disabled   = true;
		$("secondarywins").disabled = true;		
	}									
}
function on_check_learn()
{
	if($("netbios_learn").checked)
	{										
		action_radio("winstype", "disable");			
		$("netbios_scope").disabled = true;
		$("primarywins").disabled   = true;
		$("secondarywins").disabled = true;				
	}
	else
	{			
		action_radio("winstype", "enable");
		$("netbios_scope").disabled = false;
		$("primarywins").disabled   = false;
		$("secondarywins").disabled = false;	
	}						
}
function OnClickDHCPSvr()
{
	if($('dhcpsvr').checked == false)
	{
		$('dhcpsvr').value = 0;
		$('startip').disabled = true;
		$('endip').disabled = true;
		$('leasetime').disabled = true;
		$('broadcast').disabled = true;
	}
	else
	{
		$('dhcpsvr').value = 1;
		$('startip').disabled = false;
		$('endip').disabled = false;
		$('leasetime').disabled = false;
		$('broadcast').disabled = false;
	}
}

//创建保留地址列表
function createResTb(){
	
	for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
	{
		var data = [	'<input type="checkbox" id="en_dhcp_reserv_'+i+'">',
					'<span id="en_dhcp_host_'+i+'"></span>',
					'<span id="en_dhcp_ipaddr_'+i+'"></span>',
					'<span id="en_dhcp_macaddr_'+i+'"></span>',
					'<a href="javascript:OnEdit('+i+');"><img src="/html/skin/img_edit.gif"></a>',
					'<a href="javascript:OnDelete('+i+');" id="delete_'+i+'"><img src="/html/skin/img_delete.gif"></a>'
					];
					
		var str;
		var rows = $("reserves_list").getElementsByTagName("tr");
		var tagTR = null;
		var tagTD = null;
		tagTR = $("reserves_list").insertRow(rows.length);
		tagTR.id = i;
		/* save the rule in the table */
		for (j=0; j<data.length; j++)
		{
			tagTD = tagTR.insertCell(j);
			tagTD.id = i+"_"+j;
			tagTD.className = "content";
			
			str = data[j];
			tagTD.innerHTML = str; 
		}
		$("en_dhcp_reserv_"+i).checked = G_DHCPResAddr[i-1][3]=="0"?false:true;
		$("en_dhcp_host_"+i).innerHTML = G_DHCPResAddr[i-1][4];
		$("en_dhcp_ipaddr_"+i).innerHTML = G_DHCPResAddr[i-1][1];
		$("en_dhcp_macaddr_"+i).innerHTML = G_DHCPResAddr[i-1][2];
		
		g_table_index++;
	}
}

function createDHCPClientTb()
{
	
	for(var i=1; i<LanHosts.length + 1; i++)
	{
		if(LanHosts[i-1][5]=="DHCP")
		{
			var host	= LanHosts[i-1][1];
			var ipaddr	= LanHosts[i-1][3];
			var mac		= LanHosts[i-1][2];
			var expires	= LanHosts[i-1][4];
			
			var data	= [host, ipaddr, mac, expires];
			
			var str;
			var rows = $("leases_list").getElementsByTagName("tr");
			var tagTR = null;
			var tagTD = null;
			tagTR = $("leases_list").insertRow(rows.length);
			tagTR.id = i;
			/* save the rule in the table */
			for (j=0; j<data.length; j++)
			{
				tagTD = tagTR.insertCell(j);
				tagTD.id = i+"_"+j;
				tagTD.className = "content";
				
				str = data[j];
				tagTD.innerHTML = str; 
			}
		}
	}
}

function isValidIpAddressRange()
{
	var startIp = parseInt($("startip").value);
	var endIp   = parseInt($("endip").value);
	
	if (startIp >= endIp)	
		return false;

	return true;
}
//将ip/mask转化为数字
function IpToNum(ip)
{
	var num = 0;	
	var tmp = ip.split(".");
	
	num = tmp[0]*Math.pow(2,24) + tmp[1]*Math.pow(2,16) + tmp[2]*Math.pow(2,8) + tmp[3]*1;
	
	return num;
} 
function Ajax_handler(_text)
{
	try{
		eval(_text);
	}catch(e){
		uiPageRefresh();
		return;
	}
	G_SysToken = G_AjaxToken;
	if(G_Error == '1')
	{
		dealWithError();
		$("menu").style.display="";
		$("content").style.display="";
		$("mbox").style.display="none";
		$("BLAN052").disabled = false;
		$("BLAN057").disabled = false;
		$("BLAN055").disabled = false;
		for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
		{
			$("delete_"+i).disabled = false;
		}
	}	
	else
	{
		top.location = "http://" + G_IPAddress;
	}
}
//提交配置
function uiSubmit(){
	if(true == $("dhcpsvr").checked)
		if(!isValidIpAddressRange()){
			alert(SEcode['lang_start_less_end']); 
			return false;
		}
	var _path_lan = ":InternetGatewayDevice.LANDevice.1."
	var _path_host = ":InternetGatewayDevice.LANDevice.1.LANHostConfigManagement."
	var _path_ipif = ":InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface."
	
	var IP_address = $("ipaddr").value;
	if (!CheckValidity.IP("ipaddr",SEcode['lang_invalid_ip']))
		return false;
	if (!CheckValidity.Mask("netmask",SEcode['lang_invalid_mask']))
		return false;
	if (!CheckValidity.isNetIP( $("ipaddr").value, $("netmask").value ) )
		return false;
	//if (!CheckValidity.Domain("domain","Invalid Domain format!") )
		//return false;
	var _wantype=findProtocol();
	if(_wantype=="static")
	{	
		if(isSameSubNet(G_WANConn['ExternalIPAddress'],G_WANConn['SubnetMask'],$("ipaddr").value,$("netmask").value))
		{
			alert(SEcode['lang_lan_wan_conflict']);
			return false;
		}
	}
	if(_wantype=="pppoe"&&_conflict=="1")
	{
		if(G_WANConn['LocalNetMask'] == "") G_WANConn['LocalNetMask'] = "255.255.255.255";
		if(isSameSubNet(G_WANConn['StaticIPAddress'],G_WANConn['LocalNetMask'],$("ipaddr").value,$("netmask").value))
		{
			alert(SEcode['lang_lan_wan_conflict']);
			return false;
		}
	}
	if((_wantype=="pptp"||_wantype=="l2tp")&&_conflict=="1")
	{		
		if(isSameSubNet(G_WANConn['X_TWSZ-COM_VPN_CLIENT'],G_WANConn['X_TWSZ-COM_VPN_NETMASK'],$("ipaddr").value,$("netmask").value))
		{
			alert(SEcode['lang_lan_wan_conflict']);
			return false;
		}
	}
	var change_ip = 0;
	var IsNotSameNet = -1;
	if(($('netmask').value != G_SubMask))
		IsNotSameNet = 1;//不同网段
	else
	{
		var ulSubnetMask = IpToNum($('netmask').value);
		var Ip_0 = IpToNum(G_IPAddress);
		var Ip_1 = IpToNum($('ipaddr').value);
		
		if((ulSubnetMask & Ip_0) == (ulSubnetMask & Ip_1))//同网段
		{
			IsNotSameNet = 0;
		}
		else
		{
			IsNotSameNet = 1;//不同网段
		}
	}
	
	if(IsNotSameNet == 1)//改变网段
	{
		var msg= SEcode["tcpiplan_dhcp_001"];
		if (!confirm(msg))
			return false;
		
		$H({
			"obj-action" 		: "set",
			"getpage" 			: "html/page/Wait_lan.html",
			"errorpage" 		: "html/index.html",
			"var:menu" 			: G_Menu,
			"var:page" 			: G_Page,
			'var:sys_Token' : G_SysToken,
			"var:errorpage" 	: G_Page,
			"var:CacheLastData" : ViewState.Save()
		}, true);			
	}else{
		if($('ipaddr').value != G_IPAddress)//改变IP，但网段没变
		{
			var msg= SEcode["tcpiplan_dhcp_002"];
			if (!confirm(msg))
				return false;
			change_ip = 1;
			$H({
				"obj-action" 		: "set",
				"getpage" 			: "html/page/portforwd.ajax.js",
				"errorpage" 		: "html/page/portforwd.ajax.js",
				'var:sys_Token' : G_SysToken,
				'ajax'          : 'ok',
				"var:menu" 			: G_Menu,
				"var:page" 			: G_Page,
				"var:errorpage" 	: G_Page,
				"var:CacheLastData" : ViewState.Save()
			}, true);
		}
		else
		{	
			$H({
				"obj-action" 		: "set",
				"getpage" 			: "html/index.html",
				"errorpage" 		: "html/index.html",
				'var:sys_Token' : G_SysToken,
				"var:menu" 			: G_Menu,
				"var:page" 			: G_Page,
				"var:errorpage" 	: G_Page,
				"var:CacheLastData" : ViewState.Save()
			}, true);		
		}
	}
	
	// ip address
	$F( _path_ipif + '1.IPInterfaceIPAddress'		, $('ipaddr').value );
	$F( _path_ipif + '1.IPInterfaceSubnetMask'		, $('netmask').value );
    $F( _path_host + 'HostName'					    , $('domain').value );
	
	// dhcp option
	$F( _path_host + 'IPRouters' 					, $('ipaddr').value );
	$F( _path_host + 'DHCPServerEnable' 			, true == $("dhcpsvr").checked ? '1' : '0' );
	$F( _path_host + 'X_TWSZ-COM_DNSRelayEnable'	, true == $("dnsr").checked ? '1' : '0' );
	$F( _path_host + 'DHCPServerAlwaysBroadcastEnable'	, true == $("broadcast").checked ? '1' : '0' );
	
		
	var lan_ip = $('ipaddr').value;
	if (true == $("dhcpsvr").checked)
	{
		var start_ip = lan_ip.split('.')[0]+'.'+lan_ip.split('.')[1]+'.'+lan_ip.split('.')[2]+'.'+$('startip').value;
		var end_ip = lan_ip.split('.')[0]+'.'+lan_ip.split('.')[1]+'.'+lan_ip.split('.')[2]+'.'+$('endip').value;
		
		$F( _path_host + 'MinAddress' 					, start_ip );
		$F( _path_host + 'MaxAddress' 					, end_ip );
		$F( _path_host + 'DHCPLeaseTime' 				, $('leasetime').value );
		$F( _path_host + 'DomainName'					, $('device').value );
	}
	if(change_ip == 1)
	{
		var _url = "/cgi-bin/webproc?getpage=html/page/portforwd.ajax.js&var:page=*";
		G_ajax = Ajax.getInstance(_url, "", 0, Ajax_handler, null_errorfunc);
		G_ajax.post($("uiPostForm"));
		$("menu").style.display="none";
		$("content").style.display="none";
		$("mbox").style.display="";
		$("BLAN052").disabled = true;
		$("BLAN057").disabled = true;
		$("BLAN055").disabled = true;
		for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
		{
			$("delete_"+i).disabled = true;
		}
	}
	else
	{
		$('uiPostForm').submit();
		if(IsNotSameNet != 1)
		{
			$("menu").style.display="none";
			$("content").style.display="none";
			$("mbox").style.display="";
		}
		$("BLAN052").disabled = true;
		$("BLAN057").disabled = true;
		$("BLAN055").disabled = true;
		for(var i=g_table_index; i<G_DHCPResAddr.length + 1; i++)
		{
			$("delete_"+i).disabled = true;
		}
	}
}
function null_errorfunc()
{

	return true;
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

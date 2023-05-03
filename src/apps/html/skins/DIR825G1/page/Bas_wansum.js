//WANAccessType
//var G_WANAccessType = "<?get :InternetGatewayDevice.WANDevice.1.WANCommonInterfaceConfig.WANAccessType?>";

//1. about vlan lan
var G_vlan_lan   = [];
var m = 0;
<?objget :InternetGatewayDevice.Layer2Bridging.Bridge. "BridgeKey BridgeName X_TWSZ-COM_AssociatedLANDevice"
`	G_vlan_lan[m] = [];
	G_vlan_lan[m][0] = "$01"; //BridgeKey
	G_vlan_lan[m][1] = "$02"; //BridgeName
	G_vlan_lan[m][2] = "$03"; //X_TWSZ-COM_AssociatedLANDevice
	G_vlan_lan[m][3] = "InternetGatewayDevice.Layer2Bridging.Bridge.$00.";
	m++;
`?>

//2. filter
var G_bing_vlan = [];
var m = 0;
<?objget :InternetGatewayDevice.Layer2Bridging.Filter. "FilterKey FilterBridgeReference FilterInterface"
`	G_bing_vlan[m] = [];
	G_bing_vlan[m][0] = "$01"; //FilterKey
	G_bing_vlan[m][1] = "$02"; //FilterBridgeReference
	G_bing_vlan[m][2] = "$03"; //FilterInterface
	G_bing_vlan[m][3] = "InternetGatewayDevice.Layer2Bridging.Filter.$00.";
	m++;
`?>

//3. Avail_interface
var G_Avail_Interface = [];
var n = 0;
<?objget :InternetGatewayDevice.Layer2Bridging.AvailableInterface. "AvailableInterfaceKey InterfaceType InterfaceReference" 
`	<?if eq `WANInterface` `<?echo $22?>`
	`	G_Avail_Interface[n] = [];
		G_Avail_Interface[n][0] = "<?echo $21?>"; //AvailableInterfaceKey
		G_Avail_Interface[n][1] = "<?echo $22?>"; //InterfaceType
		G_Avail_Interface[n][2] = "<?echo $23?>"; //InterfaceReference
		n++;
	`?>
`?>

var G_SelWan = 0;
var m = 0;

var G_WanInfo = [];

<? objget :InternetGatewayDevice.WANDevice. "WANConnectionNumberOfEntries"
`	<? objget :InternetGatewayDevice.WANDevice.$10.WANConnectionDevice. "X_TWSZ-COM_VLANID WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
	`	<? if gt $12 0
		`	<? objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection. "ConnectionStatus Name Enable AddressingType ConnectionType X_TWSZ-COM_ProtocolType X_TWSZ-COM_ServiceList X_TWSZ-COM_IPv6Config.ConnectionStatus ExternalIPAddress"
			`	G_WanInfo[m] = [];
				G_WanInfo[m]['Vlanid']            = "$21"; //X_TWSZ-COM_VLANID
				G_WanInfo[m]['path']                = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection.$00."; //Path
				G_WanInfo[m]['ConnectionStatus']    = "$01";
				G_WanInfo[m]['Name']                = "$02",
				G_WanInfo[m]['Enable']              = "$03";
				G_WanInfo[m]['AddressingType']      = "$04";
				G_WanInfo[m]['ConnectionType']      = "$05";        //pppoe
				G_WanInfo[m]['ProtocolType']        = "$06";        //X_TWSZ-COM_ProtocolType
				G_WanInfo[m]['ServiceList']         = "$07";        // wan service Type
				G_WanInfo[m]['IPv6ConnectionStatus'] = "$08";       // X_TWSZ-COM_IPv6Config.ConnectionStatus
				G_WanInfo[m]['ExternalIPAddress']    = "$09"; //ExternalIPAddress
			`?>
			++m;
		`?>

		<? if gt $13 0
		`	<? objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection. "ConnectionStatus Name Enable ConnectionTrigger ConnectionType X_TWSZ-COM_ProtocolType X_TWSZ-COM_ServiceList X_TWSZ-COM_IPv6Config.ConnectionStatus X_TWSZ-COM_VPN_INTERFACE_REF ExternalIPAddress"
			`	G_WanInfo[m] = [];
				G_WanInfo[m]['Vlanid']            = "$21"; //X_TWSZ-COM_VLANID
				G_WanInfo[m]['path']               = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection.$00.";//Path
				G_WanInfo[m]['ConnectionStatus']   = "$01";//ConnectionStatus
				G_WanInfo[m]['Name']               = "$02";//name;
				G_WanInfo[m]['Enable']             = "$03";//Enable;
				G_WanInfo[m]['ConnectionTrigger']  = "$04";//pppoe Trigger
				//G_WanInfo[m]['AddressingType']     = "";
				G_WanInfo[m]['ConnectionType']     = "$05";
				G_WanInfo[m]['ProtocolType']       = "$06"; //X_TWSZ-COM_ProtocolType
				G_WanInfo[m]['ServiceList']        = "$07";
				G_WanInfo[m]['IPv6ConnectionStatus'] = "$08"; // X_TWSZ-COM_IPv6Config.ConnectionStatus
				G_WanInfo[m]['X_TWSZ-COM_VPN_INTERFACE_REF'] = "$09"; // X_TWSZ-COM_VPN_INTERFACE_REF
				G_WanInfo[m]['ExternalIPAddress']        = "$0a"; //ExternalIPAddress
			`?>
			++m;
		`?>
	`?>
`?>

var G_DefaultRouter = "<?get :InternetGatewayDevice.Layer3Forwarding.DefaultConnectionService?>";
var G_GWMode = "<?get :InternetGatewayDevice.X_TWSZ-COM_DEFAULT_GW.Active?>";
var flag = 0;
var G_uiWanStatus = 0,formid = 'uiPostForm';

function uiOnload(){
	/*flag = 1;
	Form.Radio("AutoGW_Switch", G_GWMode);
    initWanTable();
	setqueryWanStatus();*/
}
function setqueryWanStatus()
{
	G_uiWanStatus = setTimeout("queryWanStatus()",5000);	
}
function queryWanStatus()
{	var Idx = 0;
	$H({
	   'obj-action'       : 'set',
	   'getpage'          : 'html/page/Bas_wanstatus_ajax.js',
	   'var:page'         :  G_Page
	},true,formid);
	var _url = "/cgi-bin/webproc";
	ajax = Ajax.getInstance(_url, "", 0, queryResult,errorHandle);
	ajax.post($(formid));	
}
function queryResult(_js_)
{
	if( evaljs(_js_) ){
		eval(_js_);
	}else{
	return false;
	}
	var _len_;
	if((_len_ = G_WanStatus.length )!= G_WanInfo.length){
	return false;
	}
	for(var i = 0; i < _len_; i++){
		if(G_WanStatus[i]["ProtocolType"] == "IPv4"){
			if( (G_WanStatus[i]["ConnectionStatus"] != G_WanInfo[i]["ConnectionStatus"]) ||
				(G_WanStatus[i]['ExternalIPAddress'] != G_WanInfo[i]["ExternalIPAddress"])
			){
				return Refresh_Bas_wansum();
			}
		}else if(G_WanStatus[i]["ProtocolType"] == "IPv6"){
			if(G_WanStatus[i]["IPv6ConnectionStatus"] != G_WanInfo[i]["IPv6ConnectionStatus"]){
				return Refresh_Bas_wansum();
			}
		}else{
			if( (G_WanStatus[i]["ConnectionStatus"] != G_WanInfo[i]["ConnectionStatus"]) ||
				(G_WanStatus[i]['ExternalIPAddress'] != G_WanInfo[i]["ExternalIPAddress"]) ||
				(G_WanStatus[i]["IPv6ConnectionStatus"] != G_WanInfo[i]["IPv6ConnectionStatus"]) )
			{
				return Refresh_Bas_wansum();
			}			
		}
	}
	setqueryWanStatus();
}
function Refresh_Bas_wansum(){
	return uiPageRefresh();
}

function initWanTable() {
	//var dslTable = [];
	var ethernetTable = [];

	for (var i = 0; i < G_WanInfo.length; i++) {
			if ((G_WanInfo[i]['path']) && (G_WanInfo[i]['path'].indexOf("InternetGatewayDevice.WANDevice.3.") < 0)) {
				ethernetTable.push(createEthernetTable(i));	
			}
	}
	if (ethernetTable.length > 0) {
		$T('ta_ethwanlists', ethernetTable);
	}
	/* When wan number of connections exceeds the limit, disabled add buttons */
	if (ethernetTable.length >= 8) {
		$('WINFO030').disabled = true;
	} else {
		$('WINFO030').disabled = false;
	}
	var _dfroute;
	for(var i=0, len=G_WanInfo.length; i<len; i++){
		//alert(G_WanInfo[i]['path']);
		_dfroute = G_WanInfo[i]['path'].replace(/\.$/,"");
		if(G_DefaultRouter == _dfroute){
			$('RAD_dfRoute'+i).checked = true;
            G_SelWan = i;
		}else{
			$('RAD_dfRoute'+i).checked = false;
		}
		if(G_WanInfo[i]['ConnectionType'] == "IP_Bridged")
		{
			$('RAD_dfRoute'+i).disabled = true;
		}
		if(G_WanInfo[i]['iServiceList'] == "TR069")
		{
			$('RAD_dfRoute'+i).disabled = true;
		}
	}
}

function findProtocol(index){
    var _protocol = 'Bridge';
	var Conn_Type = G_WanInfo[index]['path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";
	if(Conn_Type == 'PPP')
	{
		if(G_WanInfo[index]['ConnectionType']=="PPTP_Relay"){
			_protocol = 'PPTP';
		}else if(G_WanInfo[index]['ConnectionType']=="L2TP_Relay"){
			_protocol = 'L2TP';
		}else{
			_protocol = 'PPPoE';
		}
	}else
	{
		if(G_WanInfo[index]['ConnectionType'] != 'IP_Bridged')
		{
			if(G_WanInfo[index]['AddressingType'] == 'Static')
			{
				_protocol = 'Static';
			}else{
				_protocol = 'DHCP';
			}
		}
	}
	return _protocol;
}

function findStatus(index){
	var Protocol = G_WanInfo[index]['ProtocolType'];
	var ConnStatus_v4 ;
	var ConnStatus_v6 ;
	var ConnStatus_v4_6 ;
	
	if(G_WanInfo[index]['ConnectionStatus'] == "Connected") {
		if(G_WanInfo[index]['ConnectionTrigger'] == "OnDemand"&& G_WanInfo[index]['ExternalIPAddress'] == "")
			ConnStatus_v4 = SEcode[8005];
		else
			ConnStatus_v4 = SEcode[8004];
	}
	else{
		ConnStatus_v4 = SEcode[8005];
	}
  
	if (G_WanInfo[index]['IPv6ConnectionStatus'] == "GlobalConnected"){
		ConnStatus_v6 = SEcode[8011] ;
	}else if (G_WanInfo[index]['IPv6ConnectionStatus'] == "LinkLocalConnected"){
		ConnStatus_v6 = SEcode[8010] ;
	}else{
		ConnStatus_v6 = SEcode[8005];
	}
  
	switch(Protocol){
		case "IPv4" :
			return ConnStatus_v4;
			break;
		case "IPv6" :
		  return ConnStatus_v6;
			break;
		case "IPv4_6" : 
			ConnStatus_v4_6 = ConnStatus_v4 +"/"+ConnStatus_v6 ;
			return ConnStatus_v4_6;
			break;
		default :
			return SEcode[8005];
	}
}

function doDfRoute(index){
	if(!confirm("lang_wan_as_gate")){
        $('RAD_dfRoute'+G_SelWan).checked = true;
		return false;
	}
    G_SelWan = index; // 保存新的路由条目
	var _dfRoute = G_WanInfo[index]['path'].replace(/\.$/,"");
	//如果已有默认路由, 不需要清掉默认路由, 任何时候用户都要可以上网
	//if(_dfRoute == G_DefaultRouter){
	//	_dfRoute = '';
	//}
	
	$H({
		':InternetGatewayDevice.Layer3Forwarding.DefaultConnectionService' : _dfRoute,
		'obj-action'    : 'set',
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html'
	},true);
	$('uiPostForm').submit();
}

function findPPPManual(index){

	var Connect = '<span id="ppp_connect_'+index+'" class="value"><input type="button" onClick="ctrlConnect('+index+''+",1"+')" value="Connect" id="ppp_connect_'+index+'" modified="false"></span>';
		Disconnect = '<span id="ppp_disconnect_'+index+'" class="value"><input type="button" onClick="ctrlConnect('+index+''+",0"+')" value="Disconnect" id="ppp_disconnect_'+index+'" modified="false"></span>';
	//var Connect = '<img src="/html/skin/dial.gif" style="cursor:pointer;" title="Connect" value="'+index+'" onClick="ctrlConnect('+index+''+",1"+')"/>',
	//    Disconnect = '<img src="/html/skin/fault.gif" style="cursor:pointer;" title="Disconnect" value="'+index+'" onClick="ctrlConnect('+index+''+",0"+')"/>';
	var ConnType = G_WanInfo[index]['path'].indexOf('WANIPConnection') > -1 ? "IP" : "PPP";
	
	if(ConnType == "PPP" && G_WanInfo[index]["ConnectionTrigger"] == "Manual"){
		if(findStatus(index).indexOf("Connected") > -1 )
			return Disconnect;
		else
			return Connect;
		
	}else{
		return "N/A";
	}
}
function checkBridgeBind(wanIndex)
{
	var v = 0;
	if(G_WanInfo[wanIndex]['ConnectionType'] == "IP_Bridged")
	{
		for(var i =0; i < G_Avail_Interface.length; i++)
		{
			//path : G_Avail_Interface[i][2]
			if(G_WanInfo[wanIndex]['path'].indexOf(G_Avail_Interface[i][2])>-1)
			{
				for(j = 0; j < G_bing_vlan.length; j++)
				{
					//FilterInterface : G_bing_vlan[j][2]
					if(G_Avail_Interface[i][0] == G_bing_vlan[j][2])
					{
						for(k = 0; k < G_vlan_lan.length; k++)
						{
							//BridgeKey : G_vlan_lan[k][0]
							if(G_bing_vlan[j][1] == G_vlan_lan[k][0])
							{
								//如果查询绑定到了internet的bridge, 则需要先解除绑定关系
								if(G_vlan_lan[k][2] != "InternetGatewayDevice.LANDevice.1")
								{
									v = 1;
									break;
								}
							}
						}
					}
				}				
			}
		}
	}
	return v;
}

function createEthernetTable( wanIndex ){
	var value_array = [];
	var v = 0;
	if(checkBridgeBind(wanIndex)) v =1;
		
    value_array.push( G_WanInfo[wanIndex]['Vlanid'] );      //VLAN ID
    //value_array.push( G_WanInfo[wanIndex]['ConnectionType'] == 'PPPoE' ? G_WanInfo[wanIndex]['ConnectionType'] : (G_WanInfo[wanIndex]['ConnectionType'] == "IP_Bridged"? "Bridge" : G_WanInfo[wanIndex]['AddressingType']) );   // Protocol
    value_array.push(findProtocol(wanIndex));
    value_array.push( G_WanInfo[wanIndex]["Name"] );      //VLAN ID
	value_array.push( findStatus(wanIndex) );  //Status
    value_array.push('<input type="radio" name="RAD_dfRoute" id="RAD_dfRoute'+ wanIndex +'" onClick="doDfRoute('+ wanIndex +')" ' + ( ( (G_GWMode == 'UserInIf') && (G_WanInfo[wanIndex]['ProtocolType'] != 'IPv6') && (G_WanInfo[wanIndex]['ServiceList'] != 'TR069')) ? '' : 'disabled') +  '>');   //Default Route
    value_array.push( findPPPManual(wanIndex) ); //Action
	if(v)
	value_array.push('<img src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit" onclick="EditEntry('+ wanIndex +',\'' + G_WanInfo[wanIndex]['Name'] + '\',\'' + G_vlan_lan[k][1] + '\')"/>'); //Edit
	else
	value_array.push('<img src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit" onclick="EditEntry('+ wanIndex +')"/>'); //Edit
	value_array.push('<img src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete" onclick="RemoveEntry('+ wanIndex +')"/>');//Drop
						  
    return value_array;
}

function EditEntry(index,bridgename,bindname){
	var Path = G_WanInfo[index]['path'];
	var ref_vpn = "0";
	for (var i = 0; i < G_WanInfo.length; ++i)
	{
		if (G_WanInfo[i]["X_TWSZ-COM_VPN_INTERFACE_REF"] == Path)
		{
			if(index == i) continue;
			ref_vpn = "1";
		}
	}
	$H({
		'var:conn_type' : Path.indexOf('PPP') > -1 ? 'PPP' : 'IP', //IP/PPP
		'var:secIdx'    : Path.split('.')[4],
		'var:thdIdx'    : Path.split('.')[6],
		'var:action' 	: "1",
		'var:ref_vpn' 	: ref_vpn,
		'var:bridgename': bridgename,
		'var:bindname' 	: bindname,
		'var:menu'      : G_Menu,
		'var:page' 	    : G_Page,
		'var:subpage' 	: G_WanInfo[index]['PVC'] == undefined ? "Bas_ethwan":"Bas_wan",
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html'
	},true);
	
	$('uiPostForm').submit();
}

function RemoveEntry(index){
	if(!confirm(SEcode[1001])){
		return false;
	}
	var isDisable=0;

	if(0 == flag)
	{
		return false;
	}

	flag = 0;

	/*add by wuyouhui to support pptp/l2tp dynamic dial start at 2012-02-18*/
	for(var j = 0, _len = G_WanInfo.length; j < _len; j++){
		if (G_WanInfo[index]["path"] == G_WanInfo[j]["X_TWSZ-COM_VPN_INTERFACE_REF"])
		{
			isDisable = 1;
			break;
		}
	}
	
	if (isDisable)
	{   flag=1;
		alert('It is bind by the other wan connections, please delete "'+G_WanInfo[j]["Name"]+'" first!');
		return false;
	}
	
	$H({
		'mid'           : '0430',
		'del-obj'       : G_WanInfo[index]['path'],
		'obj-action'    : 'del',
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html'
	},true);
	$('uiPostForm').submit();
}

function ctrlConnect(xValue, xFlag){
    var _Path = G_WanInfo[xValue]['path'];
	$H({
        'var:menu'      : G_Menu,
        'var:page'      : G_Page,
        'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'obj-action'    : 'set'
	},true);
	var id_1 = "ppp_connect_" + xValue;
	var id_2 = "ppp_disconnect_" + xValue;
	if( $(id_1) !=undefined && $(id_1) !=null )disCtrl(id_1, 0);
	if( $(id_2) !=undefined && $(id_2) !=null )disCtrl(id_2, 0);
    $F(':' + _Path + 'ConnectionTrigger', 'Manual');
    $F(':' + _Path + 'X_TWSZ-COM_ConnectionAction', xFlag == '1' ? "Connect" : "Disconnect");
	
	$('uiPostForm').submit();
}

function uiAdd(page){
	var newIndex = 0;
	if (page == 'eth'){
	    var addstyle = 2;
	}
	else{
	    var addstyle = 1;
	}
	if (G_WanInfo.length == 0 ){
        newIndex = 0;
    }else{
	    for(var i = G_WanInfo.length; i > 0; i--){
		    if(G_WanInfo[i - 1]['path'].split('.')[2] == addstyle){
			    newIndex = G_WanInfo[i - 1]['path'].split('.')[4]; //get the last wanconnection index
				break;
			}
		}
    }
    newIndex = Number(newIndex); // convert to number
	$H({
	   	"obj-action" 		: "set",
		"getpage" 		    : "html/index.html",
		"errorpage" 		: "html/index.html",
        'var:secIdx'        : newIndex + 1 ,   // last index + 1
		"var:action" 		: "0",
		"var:menu" 		    : G_Menu,
		"var:page" 		    : G_Page,
		'var:subpage' 		: addstyle == 2 ? "Bas_ethwan" : "Bas_wan",
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" : ViewState.Save()
	}, true);
		
	$('uiPostForm').submit();
}
	
function uiSubmit(){
	$H({
		':InternetGatewayDevice.X_TWSZ-COM_DEFAULT_GW.Active' : Form.Radio("AutoGW_Switch"),
		'obj-action'    : 'set',
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html'
	},true);
	$('uiPostForm').submit();
}


function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
	
}

addListeners(uiOnload, dealWithError);

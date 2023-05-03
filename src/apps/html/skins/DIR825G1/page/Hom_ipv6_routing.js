/*  ڴJavaScript  */

var wanConnection = new Array();
var n = 0;

<?objget :InternetGatewayDevice.WANDevice. ""
	`
<?objget :InternetGatewayDevice.WANDevice.$10.WANConnectionDevice. "WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection. "Name X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType"
		`	wanConnection[n] = new Array();
			wanConnection[n][0] = "$01";//name
			wanConnection[n][1] = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection.$00";//path
            		wanConnection[n][2] = "$02";
			wanConnection[n][3] = "$03"; //X_TWSZ-COM_ProtocolType
			++n;
		`?>
	`?>
	<?if gt $12 0
	`	<?objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection. "Name X_TWSZ-COM_ServiceList X_TWSZ-COM_ProtocolType"
		`	wanConnection[n] = new Array();
			wanConnection[n][0] = "$01";//name
			wanConnection[n][1] = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection.$00";//path
            		wanConnection[n][2] = "$02";
			wanConnection[n][3] = "$03"; //X_TWSZ-COM_ProtocolType
			++n;
		`?>
	`?>
`?>
`?>
//获取桥连接名
var m = 0;
<?objget :InternetGatewayDevice.LANDevice. "X_TWSZ-COM_DeviceName"
`	wanConnection[n] = new Array();
	wanConnection[n][0] = "LAN Group" + (m+1);//name
	wanConnection[n][1] = "InternetGatewayDevice.LANDevice.$00";//path
	wanConnection[n][2] = "";
	wanConnection[n][3] = "1";
	++n;
	++m;
`?>

//获取V6静态路由信息
/*
var V6_StaticRoutingList = new Array();
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding. "Enable Status DestIPv6Address DestPrefixLength GatewayIPv6Address Interface ForwardingMetric"
`	V6_StaticRoutingList[n] = new Array();
	V6_StaticRoutingList[n][0] = "$01"; //Enable
	V6_StaticRoutingList[n][1] = "$02"; //Status
	V6_StaticRoutingList[n][2] = "$03"; //DestIPv6Address
	V6_StaticRoutingList[n][3] = "$04"; //DestPrefixLength
	V6_StaticRoutingList[n][4] = "$05"; //GatewayIPv6Address
	V6_StaticRoutingList[n][5] = "$06"; //Interface
	V6_StaticRoutingList[n][6] = "$07"; //ForwardingMetric
	V6_StaticRoutingList[n][7] = "InternetGatewayDevice.X_TWSZ-COM_IPv6Layer3Forwarding.IPv6Forwarding.$00.";	//path
	n++;
`?>
*/
var G_RouteInfo = new Array();
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_IPv6RouteInfo. "DestIPv6Address DestPrefixLength GatewayIPv6Address  ForwardingMetric Interface"
`	G_RouteInfo[m] = new Array();
	G_RouteInfo[m][0] = "$01"; //DestIPAddress
	G_RouteInfo[m][1] = "$02"; //GatewayIPAddress
	G_RouteInfo[m][2] = "$03"; //DestSubnetMask
	G_RouteInfo[m][3] = "$04"; //ForwardingMetric
	G_RouteInfo[m][4] = "$05"; //Interface
	G_RouteInfo[m][5] = "$00"; //index
	m++;
`?>


function createV6Table()
{
	var array_value = [];
	var len=0;
	Table.Clear('routing_list');
	
	for(var i = 0; i < G_RouteInfo.length; i++){
		array_value[len] = [];
		
		array_value[len].push(G_RouteInfo[i][0]+"/" + G_RouteInfo[i][1]); //DestIPAddress
		array_value[len].push(G_RouteInfo[i][2]); //DestIPAddress
		array_value[len].push(G_RouteInfo[i][3]); //DestIPAddress
		array_value[len].push(G_RouteInfo[i][4]); //DestIPAddress
		
		len++;
	}
	
	$T('routing_list',array_value);
}


function uiOnload()
{
	createV6Table();
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

//get route info
var G_RouteInfo = new Array();
var m = 0;

<?objget :InternetGatewayDevice.X_TWSZ-COM_RouteInfo. "DestIPAddress GatewayIPAddress DestSubnetMask  Flags Ref ForwardingMetric Interface"
`	G_RouteInfo[m] = new Array();
	G_RouteInfo[m][0] = "$01"; //DestIPAddress
	G_RouteInfo[m][1] = "$02"; //GatewayIPAddress
	G_RouteInfo[m][2] = "$03"; //DestSubnetMask
	G_RouteInfo[m][3] = "$04"; //Flags
	G_RouteInfo[m][4] = "$05"; //Ref
	G_RouteInfo[m][5] = "$06"; //ForwardingMetric
	G_RouteInfo[m][6] = "$07"; //Interface
	G_RouteInfo[m][7] = "$00"; //index
	m++;
`?>

//由于RouteList显示的路由条目重复，需加入RouteInfo中已有staticRoutingList的过滤
function createTable()
{
	var array_value = [];
	var len=0;
	Table.Clear('routing_list');
	
	for(var m = 0; m<G_RouteInfo.length; m++){		
      array_value[len] = [];
	  array_value[len].push(G_RouteInfo[m][0]); //DestIPAddress	  
	  array_value[len].push(G_RouteInfo[m][1]); //GatewayIPAddress
	  array_value[len].push(G_RouteInfo[m][2]); //DestSubnetMask
	  array_value[len].push(G_RouteInfo[m][5]); //metric
	  array_value[len].push(G_RouteInfo[m][6]); //interface
	  array_value[len].push(G_RouteInfo[m][4]); //Ref
	  len++; 		
	}
	$T('routing_list',array_value);
}

function uiOnload()
{
	createTable();
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

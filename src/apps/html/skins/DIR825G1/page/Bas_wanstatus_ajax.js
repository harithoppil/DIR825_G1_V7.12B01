var n = 0;

var G_WanStatus = [];

<? objget :InternetGatewayDevice.WANDevice. "WANConnectionNumberOfEntries"
`	<? objget :InternetGatewayDevice.WANDevice.$10.WANConnectionDevice. "X_TWSZ-COM_VLANID WANIPConnectionNumberOfEntries WANPPPConnectionNumberOfEntries"
	`	<? if gt $12 0
		`	<? objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection. "ConnectionStatus X_TWSZ-COM_ProtocolType X_TWSZ-COM_IPv6Config.ConnectionStatus ExternalIPAddress"
			`	G_WanStatus[n] = [];
				G_WanStatus[n]['path']                = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANIPConnection.$00."; //Path
				G_WanStatus[n]['ConnectionStatus']    = "$01";
				G_WanStatus[n]['ProtocolType']        = "$02";        //X_TWSZ-COM_ProtocolType
				G_WanStatus[n]['IPv6ConnectionStatus'] = "$03";       // X_TWSZ-COM_IPv6Config.ConnectionStatus
				G_WanStatus[n]['ExternalIPAddress']    = "$04"; //ExternalIPAddress
			`?>
			++n;
		`?>

		<? if gt $13 0
		`	<? objget :InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection. "ConnectionStatus X_TWSZ-COM_ProtocolType X_TWSZ-COM_IPv6Config.ConnectionStatus ExternalIPAddress"
			`	G_WanStatus[n] = [];
				G_WanStatus[n]['path']               = "InternetGatewayDevice.WANDevice.$30.WANConnectionDevice.$20.WANPPPConnection.$00.";//Path
				G_WanStatus[n]['ConnectionStatus']   = "$01";//ConnectionStatus
				G_WanStatus[n]['ProtocolType']       = "$02"; //X_TWSZ-COM_ProtocolType
				G_WanStatus[n]['IPv6ConnectionStatus'] = "$03"; // X_TWSZ-COM_IPv6Config.ConnectionStatus
				G_WanStatus[n]['ExternalIPAddress']    = "$04"; //ExternalIPAddress
			`?>
			++n;
		`?>
	`?>
`?>

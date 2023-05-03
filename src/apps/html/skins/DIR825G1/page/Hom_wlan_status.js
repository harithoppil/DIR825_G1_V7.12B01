/*  JavaScript Document:dhcplist */
var WlanHosts_24 = [];
var m = 0;
<?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.AssociatedDevice. "AssociatedDeviceMACAddress AssociatedDeviceIPAddress X_TWSZ-COM_AssociatedDeviceRate X_TWSZ-COM_AssociatedDeviceRSSI X_TWSZ-COM_AssociatedDeviceType"
`	WlanHosts_24[m] = [];
	WlanHosts_24[m][0] = "<?echo $11?>";
	WlanHosts_24[m][1] = "<?echo $12?>";
	WlanHosts_24[m][2] = "<?echo $15?>";
	WlanHosts_24[m][3] = "<?echo $13?>";
	WlanHosts_24[m][4] = "<?echo $14?>";
	++m;		
`?>
<?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration.3.AssociatedDevice. "AssociatedDeviceMACAddress AssociatedDeviceIPAddress X_TWSZ-COM_AssociatedDeviceRate X_TWSZ-COM_AssociatedDeviceRSSI X_TWSZ-COM_AssociatedDeviceType"
`	WlanHosts_24[m] = [];
	WlanHosts_24[m][0] = "<?echo $11?>";
	WlanHosts_24[m][1] = "<?echo $12?>";
	WlanHosts_24[m][2] = "<?echo $15?>";
	WlanHosts_24[m][3] = "<?echo $13?>";
	WlanHosts_24[m][4] = "<?echo $14?>";
	++m;		
`?>
var WlanHosts_5 = [];
var n = 0;
<?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.AssociatedDevice. "AssociatedDeviceMACAddress AssociatedDeviceIPAddress X_TWSZ-COM_AssociatedDeviceRate X_TWSZ-COM_AssociatedDeviceRSSI X_TWSZ-COM_AssociatedDeviceType"
`	WlanHosts_5[n] = [];
	WlanHosts_5[n][0] = "<?echo $11?>";
	WlanHosts_5[n][1] = "<?echo $12?>";
	WlanHosts_5[n][2] = "<?echo $15?>";
	WlanHosts_5[n][3] = "<?echo $13?>";
	WlanHosts_5[n][4] = "<?echo $14?>";
	++n;		
`?>
<?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration.4.AssociatedDevice. "AssociatedDeviceMACAddress AssociatedDeviceIPAddress X_TWSZ-COM_AssociatedDeviceRate X_TWSZ-COM_AssociatedDeviceRSSI X_TWSZ-COM_AssociatedDeviceType"
`	WlanHosts_5[n] = [];
	WlanHosts_5[n][0] = "<?echo $11?>";
	WlanHosts_5[n][1] = "<?echo $12?>";
	WlanHosts_5[n][2] = "<?echo $15?>";
	WlanHosts_5[n][3] = "<?echo $13?>";
	WlanHosts_5[n][4] = "<?echo $14?>";
	++n;		
`?>
var LanHosts = [];
var t = 0;
<?objget :InternetGatewayDevice.LANDevice. "Hosts.HostNumberOfEntries"
`	<?if gt $11 0
	`	<?objget :InternetGatewayDevice.LANDevice.$20.Hosts.Host. "HostName MACAddress IPAddress LeaseTimeRemaining VendorClassID AddressSource"
		`	LanHosts[t] = [];
			LanHosts[t][0] = t+1;
			LanHosts[t][1] = "<?echo $11?>"=="ZFc1cmJtOTNiZz09"?"unknown":strAnsi2Unicode((Base64.Decode("<?echo $11?>")));
			LanHosts[t][2] = "<?echo $12?>";
			LanHosts[t][3] = "<?echo $13?>";
			LanHosts[t][4] = "<?echo $14?>";
			++t;
		`?>
	`?>
`?>
function uiOnload(){	
	for(var i=0;i<m;i++)
	{
		for(var j=0;j<t;j++)
		{
			if(WlanHosts_24[i][0].toUpperCase() == LanHosts[j][2].toUpperCase())
			WlanHosts_24[i][1] = LanHosts[j][3];
		}
	}
	for(var i=0;i<n;i++)
	{
		for(var j=0;j<t;j++)
		{
			if(WlanHosts_5[i][0].toUpperCase() == LanHosts[j][2].toUpperCase())
			WlanHosts_5[i][1] = LanHosts[j][3];
		}
	}
	$('client_cnt').innerHTML = m;
	$('client_cnt_Aband').innerHTML = n;
	$T('client_list',WlanHosts_24);
	$T('client_list_Aband',WlanHosts_5);
}

addListeners(uiOnload);
var en_Wlan = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.1.Enable?>";
var en_Wlan_5G = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.2.Enable?>";

function uiOnload(){	
	if(en_Wlan=="0" && en_Wlan_5G =="0")
	{
		$("WLANSUM013").disabled = true;
	}
	else
	{
		$("WLANSUM013").disabled = false;
	}
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];

	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

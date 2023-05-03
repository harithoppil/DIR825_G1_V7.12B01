var Radio_list=[];
var m=0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_Radio. "TransmitPower IsolateSTA"
`	Radio_list[m]=[];
	Radio_list[m][0] 			 = "InternetGatewayDevice.X_TWSZ-COM_Radio.$00.";//path
	Radio_list[m][1]             = "$01";//TransmitPower
	Radio_list[m][2]             = "$02";//X_TWSZ-COM_APIsolate
	m++;
`?>
var G_Coexistence = "<?get :InternetGatewayDevice.X_TWSZ-COM_Radio.1.HtBssCoex?>";
var G_Wireless = [];
var n = 0;
 <?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. "WMMEnable"
        `	
            G_Wireless[n] = ['InternetGatewayDevice.LANDevice.1.WLANConfiguration.$00.', //path
                     '$01'//WMMEnable
                     ];
    n++;
`?>
function uiSubmit()
{

    //2.4G
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.TransmitPower',                 $('tx_power').value);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.WMMEnable',        Form.Checkbox('en_wmm') == true?'1':'0');
	//$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.X_TWSZ-COM_APIsolate',          Form.Checkbox('wlan_partition'));
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.IsolateSTA',          Form.Checkbox('wlan_partition'));
	
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.1.HtBssCoex',                     Form.Radio('coexist_type'));

	//5.8G
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.TransmitPower',                 $('tx_power_Aband').value);
	$F(':InternetGatewayDevice.LANDevice.1.WLANConfiguration.2.WMMEnable',         Form.Checkbox('en_wmm_Aband') == true?'1':'0');
	//$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.X_TWSZ-COM_APIsolate',          Form.Checkbox('wlan_partition_Aband'));
	$F(':InternetGatewayDevice.X_TWSZ-COM_Radio.2.IsolateSTA',                    Form.Checkbox('wlan_partition_Aband'));
	
	$H({
        'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:errorpage' : G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'var:CacheLastData' : ViewState.Save()
	});
	$F('obj-action', 'set');
    $('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('Adv_wireless001').disabled= true;
	$('Adv_wireless024').disabled= true;
}

function uiOnload()
{
	setJSONValue({
			"tx_power"		: Radio_list[0][1],
			"wlan_partition"	: Radio_list[0][2],
			"en_wmm"	: G_Wireless[0][1],
			"tx_power_Aband"		: Radio_list[1][1],
			"wlan_partition_Aband"	: Radio_list[1][2],
			"en_wmm_Aband": G_Wireless[1][1]
		});
	Form.Radio("coexist_type", G_Coexistence);
}

function dealWithError(){
	if (G_Error != 1){
		return false;
	}
	
	var arrayHint = [];
	
	dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);

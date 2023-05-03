/*  ڴJavaScript  */
/*URL Filter*/
var G_URLFilterEnabled = "<?get :InternetGatewayDevice.X_TWSZ-COM_URL_Filter.Enable?>";
var G_URLFilterMode    = "<?get :InternetGatewayDevice.X_TWSZ-COM_URL_Filter.FilterMode?>";
var G_URLBlackList = [];
var G_BlackListNum = 0;

<?objget :InternetGatewayDevice.X_TWSZ-COM_URL_Filter.BlackList. "URL"
`	G_URLBlackList[G_BlackListNum] = [];
	G_URLBlackList[G_BlackListNum][0] = "InternetGatewayDevice.X_TWSZ-COM_URL_Filter.BlackList.$00.";//index
	G_URLBlackList[G_BlackListNum][1] = "$01";		//URL
	G_BlackListNum++;
`?>
var G_URLWhiteList = [];
var G_WhiteListNum = 0;

<?objget :InternetGatewayDevice.X_TWSZ-COM_URL_Filter.WhiteList. "URL"
`	G_URLWhiteList[G_WhiteListNum] = [];
	G_URLWhiteList[G_WhiteListNum][0] = "InternetGatewayDevice.X_TWSZ-COM_URL_Filter.WhiteList.$00.";//index
	G_URLWhiteList[G_WhiteListNum][1] = "$01";		//URL
	G_WhiteListNum++;
`?>



function OnClickClearURL()
{
	for (var i=1; i<=40; i+=1)	$("url_"+i).value="";
}

function uiOnload()
{
	$('url_mode').value = G_URLFilterMode;
	if($('url_mode').value == "Allow"){
		for(var i=1; i<=40; i+=1)
		{
			$('url_'+i).value = G_URLWhiteList[i-1][1];	
		}
	}
	else{
		for(var i=1; i<=40; i+=1)
		{
			$('url_'+i).value = G_URLBlackList[i-1][1];	
		}
	}
}
/*submit a macfilter entry*/
function uiSubmit()
{	
	var url;
	if($("url_mode").value == "Deny")
	{	
		$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.Enable' , '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.FilterMode' , 'Deny');
		for(i=1; i<=40; i++)
		{
			if($('url_'+i).value.indexOf("http://")>-1)
				url = $('url_'+i).value.substring(7);
			else
				url = $('url_'+i).value;
			$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.BlackList.'+i+'.URL' , url.toLowerCase());
		}
	}
	else if($("url_mode").value == "Allow")
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.Enable' , '1');
		$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.FilterMode' , 'Allow');
		for(i=1; i<=40; i++)
		{			
			if($('url_'+i).value.indexOf("http://")>-1)
				url = $('url_'+i).value.substring(7);		
			else
				url = $('url_'+i).value;
			$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.WhiteList.'+i+'.URL' , url.toLowerCase());
		}
	}
	else
	{
		$F(':InternetGatewayDevice.X_TWSZ-COM_URL_Filter.Enable' , '0');
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
	$('Adv_url002').disabled= true;
	$('Adv_url011').disabled= true;
}

function dealWithError(){
         if (G_Error != 1){ return false; }
         var arrayHint = [];
         dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);




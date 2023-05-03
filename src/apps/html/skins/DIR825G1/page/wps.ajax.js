var t = 0;
var G_WPS_StartWPSSession = [];
<?objget :InternetGatewayDevice.LANDevice.1.WLANConfiguration. 'WPS.X_TWSZ-COM_StartWpsSession'
`	G_WPS_StartWPSSession[t] = '$01';
	t++;
`?>
G_AjaxToken  = "<?echo $var:sys_Token?>";
var G_Error	    = "<?error found?>"; 
var G_Error_Msg	= "<?error message?>";


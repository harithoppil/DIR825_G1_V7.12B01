var G_inbf = [];
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList. "Name IPRange Action SchedvsUsed SchedpfUsed SchedrmUsed"
`	G_inbf[m] = [];
	G_inbf[m][0] = "InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList.$00.";
	G_inbf[m][1] = "$01";
	G_inbf[m][2] = "$02";
	G_inbf[m][3] = "$03";
	G_inbf[m][4] = "$04";
	G_inbf[m][5] = "$05";
	G_inbf[m][6] = "$06";
	m++;
`?>
<?mget :InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1. "IPInterfaceIPAddress IPInterfaceSubnetMask"
`	G_IPAddress = "$01";
	G_SubMask 	= "$02";
`?>
var G_edit= -1;
function uiOnload(){
  createListTb();
}
function createListTb(){
	var array_value = [];
	Table.Clear('inbftable');	
	for(var i = 0, _len = G_inbf.length; i < _len; i++){
			array_value[i] = [G_inbf[i][1],
							  G_inbf[i][3]=="Allow"?data_languages.Public.innerHTML.Public012:data_languages.Public.innerHTML.Public013,							  
							  G_inbf[i][2],
							  '<img src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit" onclick="uiEdit(' + i + ')"/>',
							  '<img src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete" onclick="uiDelete(' + i + ')" id="delete_'+i+'"/>'				  
							  ];
			}
	$T('inbftable', array_value);
}

function uiEdit(_i){
	$("inbfdesc").value = G_inbf[_i][1];
	$("inbfact").value = G_inbf[_i][3];
	for (var j=1; j<=8; j+=1)
	{
		$("en_inbf"+j).checked = false; 
		$("inbf_startip"+j).value = "0.0.0.0";
		$("inbf_endip"+j).value = "255.255.255.255";	
	}
	var iprange= G_inbf[_i][2].split(",");
	for (var k=1; k<=iprange.length; k+=1)
	{		
		$("en_inbf"+k).checked = true; 
		$("inbf_startip"+k).value = iprange[k-1].split("-")[0];
		$("inbf_endip"+k).value = iprange[k-1].split("-")[1];
	}	
	$("inbfsubmit").value = data_languages.Public.innerHTML.Public011;
	G_edit = _i;
}

function uiDelete(_i){	
	if((G_inbf[_i][4]!= 0)||(G_inbf[_i][5]!= 0)||(G_inbf[_i][6]!= 0))
	{
		alert(SEcode['lang_rule_is_used']);
        return false;
	}
	if(!confirm(SEcode[1001])){
		return false;
	}
   $H({
		'var:menu'      : G_Menu,
		'var:page'      : G_Page,
		'var:sys_Token' : G_SysToken,
		'var:errorpage' : G_Page,
		'getpage'       : 'html/index.html',
		'errorpage'     : 'html/index.html',
		'obj-action'    : 'del',
		'del-obj'       : G_inbf[_i][0],
		'var:CacheLastData'	: ViewState.Save()
	},true);
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('inbfsubmit').disabled= true;
	for(var i=0; i<G_inbf.length; i++)
	{
		$("delete_"+i).disabled= true;
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

function uiSubmit()
{
	var ipRangeList;
	
	if ($("inbfdesc").value == "")
	{
		alert(SEcode['lang_name_not_blank']);
		return;
	}


	for(var j=0; j < $("inbfdesc").value.length ; j++)
	{
		if ($("inbfdesc").value.charAt(j) == " ")
		{
			alert(SEcode['lang_name_not_blank_space']);
			return;
		}
	}
	for (var j=1; j<=8; j+=1)
	{
		if($("en_inbf"+j).checked == true)	break;
		if(j == 8)	
		{
			alert(SEcode['lang_one_range']);
			return;
		}	
	}
	ipRangeList = '';
	for (var j=1; j<=8; j+=1)
	{
		if($("en_inbf"+j).checked == true)
		{			
			if ($("inbf_startip"+j).value == "" || !checkipaddr($("inbf_startip"+j).value))
			{
				alert(SEcode['lang_start_ip']);
				$("inbf_startip"+j).focus();
				return false;
			}
			if (isSameSubNet($("inbf_startip"+j).value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_start_ip_not_same']);
				$("inbf_startip"+j).focus();
				return false;
			}
			if ($("inbf_endip"+j).value == "" || !checkipaddr($("inbf_endip"+j).value))
			{
				alert(SEcode['lang_end_ip']);
				$("inbf_endip"+j).focus();
				return false;
			}
			if (isSameSubNet($("inbf_endip"+j).value,G_SubMask,G_IPAddress,G_SubMask))
			{
				alert(SEcode['lang_end_ip_not_same']);
				$("inbf_endip"+j).focus();
				return false;
			}
			ipRangeList+=$('inbf_startip'+j).value+'-'+$('inbf_endip'+j).value;
			ipRangeList+=',';
		}
	}
	if(ipRangeList.length > 1)
		ipRangeList=ipRangeList.substring(0,ipRangeList.length-1);
	$H({			
			'var:menu'      : G_Menu,
		    'var:page'      : G_Page,
			'var:sys_Token' : G_SysToken,
		    'getpage'       : 'html/index.html',
		    'errorpage'     : 'html/index.html',
		    'var:errorpage' : G_Page,
		    'var:CacheLastData': ViewState.Save()
		},true);
		
	if(G_edit==-1)
	{
		$H({	
			'obj-action'    : 'add-set',
			'add-obj'       : 'InternetGatewayDevice.X_TWSZ-COM_INBOUND.RuleList.',
			':Name'         : $('inbfdesc').value,
			':IPRange'      : ipRangeList,
			':Action'       : $('inbfact').value
		});
	}
	else
	{
		if((G_inbf[G_edit][4]!= 0)||(G_inbf[G_edit][5]!= 0)||(G_inbf[G_edit][6]!= 0))
		{
			alert(SEcode['lang_rule_is_used']);
			return false;
		}
		$F(':' + G_inbf[G_edit][0] + 'Name', $('inbfdesc').value);
		$F(':' + G_inbf[G_edit][0] + 'IPRange', ipRangeList);
		$F(':' + G_inbf[G_edit][0] + 'Action', $('inbfact').value);
		$F('obj-action', 'set');
	}
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('inbfsubmit').disabled= true;
	for(var i=0; i<G_inbf.length; i++)
	{
		$("delete_"+i).disabled= true;
	}
}

function dealWithError(){
         if (G_Error != 1){ return false; }
         var arrayHint = [];
         dealErrorMsg(arrayHint, G_Error_Msg);
}

addListeners(uiOnload, dealWithError);


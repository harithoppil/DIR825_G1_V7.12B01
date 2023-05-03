/* JavaScript Document */
var G_SchedEntry = [];
var n = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry. "SchedName SchedUsed EnableAllDays SelectDays EnableAllTimes StartTime EndTime SchedvsUsed SchedpfUsed SchedptUsed SchednfUsed"
`	G_SchedEntry[n] = [];
	G_SchedEntry[n][0] = "$01";   	//ScheduleName
	G_SchedEntry[n][1] = "$02";	//ScheduleUsed	
	G_SchedEntry[n][2] = "$03";	//EnableAllDays
	G_SchedEntry[n][3] = "$04";	//SelectDays
	G_SchedEntry[n][4] = "$05";	//EnableAllTimes
	G_SchedEntry[n][5] = "$06";	//StartTime
	G_SchedEntry[n][6] = "$07";	//EndTime
	G_SchedEntry[n][7] = "InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.$00.";    //path
	G_SchedEntry[n][8] = "$00";   	//index
	G_SchedEntry[n][9] = "$08";   	//SchedvsUsed
	G_SchedEntry[n][10] = "$09";   	//SchedpfUsed
	G_SchedEntry[n][11] = "$0a";   	//SchedptUsed
	G_SchedEntry[n][12] = "$0b";   	//SchednfUsed
	++n;
`?>
	
var G_ACLWanGroup = []
var m = 0;
<?objget :InternetGatewayDevice.X_TWSZ-COM_ACL.RACL.1.Service. "Schedule"
`	G_ACLWanGroup[m] = [];
	G_ACLWanGroup[m][0] = "$01";   	//ScheduleName
	++m;
`?>
//全局变量
var editIdx   = -1;

function uiOnload(){
	CreateTable();
}

//select all day
function changeDaysSelect(){
	var _input=$('daysArea').getElementsByTagName('INPUT');
	if($('schallweek').checked){
		for(var k = 0, _len = _input.length; k < _len; k++){
			_input[k].checked = true;
			_input[k].disabled = true;
		}
	}else{
		for(var k = 0, _len = _input.length; k < _len; k++){
			_input[k].checked = false;
			_input[k].disabled = false;	
		}
	}
}
//Set all time
function SetWholeDay(){	
	if($('sch24hrs').checked){
		$('schstarthrs').value="00";
		$('schstartmin').value="00";
		$('schendhrs').value="23";
		$('schendmin').value="59";
		$('schstarthrs').disabled=true;
		$('schstartmin').disabled=true;
		$('schendhrs').disabled=true;
		$('schendmin').disabled=true;
	}else{
		$('schstarthrs').value="";
		$('schstartmin').value="";
		$('schendhrs').value="";
		$('schendmin').value="";
		$('schstarthrs').disabled=false;
		$('schstartmin').disabled=false;
		$('schendhrs').disabled=false;
		$('schendmin').disabled=false;
	}
	return true;	
}
function CreateTable(){
	var value_array = [];
	for(var i = 0; i < G_SchedEntry.length; i++){
		value_array[i] = [];
		value_array[i].push(G_SchedEntry[i][0]); //schedules name
		value_array[i].push(G_SchedEntry[i][3].substring(0,G_SchedEntry[i][3].length-1)); //schedules name
		
		value_array[i].push(G_SchedEntry[i][5]+'~'+G_SchedEntry[i][6]);
		value_array[i].push('<img src="/html/skin/pen.gif" style="cursor:pointer;" title="Edit" onclick="EditEntry('+ i +')"/>');
		value_array[i].push('<img src="/html/skin/cross.gif" style="cursor:pointer;" title="Delete" onclick="RemoveEntry('+ i +')" id="delete_'+ i +'"/>');
	}	
	$T('schtable',value_array);

}
//编辑
function EditEntry(_idx){
	editIdx = _idx;	//编辑标志
	var starttime= G_SchedEntry[_idx][5];
	var num=starttime.indexOf(':');
	var starttimeh=starttime.substring(0,num);
	var starttimem=starttime.substring(num+1,starttime.length+1);
	var endtime= G_SchedEntry[_idx][6];
	var num=endtime.indexOf(':');
	var endtimeh=endtime.substring(0,num);
	var endtimem=endtime.substring(num+1,endtime.length+1);
	
	var day=G_SchedEntry[_idx][3];			
	if(day.indexOf('Sun')!=-1){
		var SunEnable=1; //Sun
	}else{
		var SunEnable=0; //Sun
	}
	if(day.indexOf('Mon')!=-1)
		var MonEnable=1; //Mon
	else
		var MonEnable=0; //Mon
	if(day.indexOf('Tue')!=-1)
		var TueEnable=1; //Tue
	else
		var TueEnable=0; //Tue
	if(day.indexOf('Wed')!=-1)
		var WedEnable=1; //Wed
	else
		var WedEnable=0; //Wed
	if(day.indexOf('Thu')!=-1)
		var ThuEnable=1; //Thu
	else
		var ThuEnable=0; //Thu
	if(day.indexOf('Fri')!=-1)
		var FriEnable=1; //Fri
	else
		var FriEnable=0; //Fri
	if(day.indexOf('Sat')!=-1)
		var SatEnable=1; //Sat
	else
		var SatEnable=0; //Sat
	Form.Radio("schdayselect", G_SchedEntry[_idx][2]);
	setJSONValue({
		"schdesc" 	: G_SchedEntry[_idx][0],
		"schsun"	: SunEnable,
		"schmon" 	: MonEnable,
		"schtue" 	: TueEnable,
		"schwed" 	: WedEnable,
		"schthu" 	: ThuEnable,
		"schfri" 	: FriEnable,
		"schsat" 	: SatEnable,
		"sch24hrs" 	: G_SchedEntry[_idx][4],
		"schstarthrs" : starttimeh,
		"schstartmin" : starttimem,
		"schendhrs" 	: endtimeh,
		"schendmin" 	: endtimem
	});
	if($('sch24hrs').checked){
		$('schstarthrs').disabled=true;
		$('schstartmin').disabled=true;
		$('schendhrs').disabled=true;
		$('schendmin').disabled=true;
	}
	else
	{
		$('schstarthrs').disabled=false;
		$('schstartmin').disabled=false;
		$('schendhrs').disabled=false;
		$('schendmin').disabled=false;
	}
	if($('schallweek').checked){
		var _input=$('daysArea').getElementsByTagName('INPUT');
		for(var k = 0, _len = _input.length; k < _len; k++){
			_input[k].disabled = true;
		}
	}
	else
	{
		var _input=$('daysArea').getElementsByTagName('INPUT');
		for(var k = 0, _len = _input.length; k < _len; k++){
			_input[k].disabled = false;
		}
	}
	
	$("schsubmit").value = data_languages.Public.innerHTML.Public011;
}
//
function FullFillTimeBlank(_nodes){
	var num;
	num = parseInt(_nodes[1].value,10);
	if (num <= 9)
		_nodes[1].value = "0" + num;
	num = parseInt(_nodes[2].value,10);
	if (num <= 9)
		_nodes[2].value = "0" + num;
	num = parseInt(_nodes[3].value,10);
	if (num <= 9)
		_nodes[3].value = "0" + num;
	num = parseInt(_nodes[4].value,10);
	if (num <= 9)
		_nodes[4].value = "0" + num;
}
function uiSubmit(){
	var _nodes = $('schdesc','schstarthrs','schstartmin','schendhrs','schendmin');
	FullFillTimeBlank(_nodes);	
	var starttime=_nodes[1].value+':'+_nodes[2].value;
	var endtime=_nodes[3].value+':'+_nodes[4].value;
	var selectdays='';
	if($('schsun').checked)
	     selectdays='Sun,';
	if($('schmon').checked)
	     selectdays+='Mon,';
	if($('schtue').checked)
	     selectdays+='Tue,';
	if($('schwed').checked)
	     selectdays+='Wed,';
	if($('schthu').checked)
	     selectdays+='Thu,';
	if($('schfri').checked)
	     selectdays+='Fri,';
	if($('schsat').checked)
	     selectdays+='Sat,';
	if(editIdx==-1){		
		if(G_SchedEntry.length==10)
		{
			alert(SEcode['lang_sec_num']);
			return false;
		}
		$H({
		   	'add-obj'		:'InternetGatewayDevice.X_TWSZ-COM_SCHEDULES.SchedEntry.',
			':SchedName'   		: _nodes[0].value,
			':EnableAllDays'   	: Form.Radio('schdayselect')=='1'?1:0,
			':SelectDays'   	: selectdays,
			':EnableAllTimes' 	: Form.Checkbox('sch24hrs'),
			':StartTime'   		: starttime,
			':EndTime'   		: endtime,
			':IsUpdateSch'		: '0',
			'obj-action' 		: 'add-set',
			'var:menu' 		: G_Menu,
			'var:page' 		: G_Page,
			'var:sys_Token' : G_SysToken,
			'var:errorpage' 	: G_Page,
			'getpage' 		: 'html/index.html',
			'errorpage' 		: 'html/index.html',
			'var:CacheLastData'	: ViewState.Save()
		}, true);
	}else{
		if((G_SchedEntry[editIdx][9]!= 0)||(G_SchedEntry[editIdx][10]!= 0)||(G_SchedEntry[editIdx][11]!= 0)||(G_SchedEntry[editIdx][12]!= 0)||(G_SchedEntry[editIdx][1]!= 0))
		{
			alert(SEcode['lang_sec_used']);
			return false;
		}
		$H({
			'obj-action' 		: 'set',
			'var:menu' 		: G_Menu,
			'var:page' 		: G_Page,
			'var:sys_Token' : G_SysToken,
			'var:errorpage' 	: G_Page,
			'getpage' 		: 'html/index.html',
			'errorpage' 		: 'html/index.html',
			'var:CacheLastData'	: ViewState.Save()
		}, true);
		var path=':'+G_SchedEntry[editIdx][7];
		$F(path+'SchedName',_nodes[0].value);
		var EnableValue=0;
		if($('schallweek').checked){
			EnableValue=1;
		}
		$F(path+'EnableAllDays', EnableValue);
		$F(path+'SelectDays', selectdays);
		EnableValue=0;
		if($('sch24hrs').checked){
			EnableValue=1;
		}
		$F(path+'EnableAllTimes', EnableValue);
		$F(path+'StartTime', starttime);
		$F(path+'EndTime', endtime);

		$F(path+'IsUpdateSch', "1");
	}
	
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("schsubmit").disabled = true;
	for(var i = 0; i < G_SchedEntry.length; i++)
	{
		$("delete_"+i).disabled = true;
	}
}

//删除
function RemoveEntry(Idx){
    var SchedpfUsed = 0;
	
    for(var i = 0; i < G_ACLWanGroup.length; i++)
	{
	    //alert(i);
	    //alert(G_ACLWanGroup[i][0]);
		//alert(G_SchedEntry[Idx][7]);
		if(G_ACLWanGroup[i][0] == G_SchedEntry[Idx][7])
		{
			SchedpfUsed = 1;
			break;
		}
	}	
	if((G_SchedEntry[Idx][9]!= 0)||(1 == SchedpfUsed) /*|| (G_SchedEntry[Idx][10]!= 0)*/||(G_SchedEntry[Idx][11]!= 0)||(G_SchedEntry[Idx][12]!= 0)||(G_SchedEntry[Idx][1]!= 0))
	{
		alert(SEcode['lang_sec_used']);
        return false;
	}
	if(!confirm(SEcode[1001])){
		return false;
	}
	var _path = G_SchedEntry[Idx][7];
	$H({
	   	"del-obj" 		: _path,
	   	"obj-action" 		: "del",
		"getpage" 		: "html/index.html",
		"errorpage" 		: "html/index.html",
		'var:sys_Token' : G_SysToken,
		"var:menu" 		: G_Menu,
		"var:page" 		: G_Page,
		"var:errorpage" 	: G_Page,
		"var:CacheLastData" 	: ViewState.Save()
	}, true);
	$('uiPostForm').submit();
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$("schsubmit").disabled = true;
	for(var i = 0; i < G_SchedEntry.length; i++)
	{
		$("delete_"+i).disabled = true;
	}
}

//错误处理函数
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//监听加载与错误处理函数
addListeners(uiOnload, dealWithError);
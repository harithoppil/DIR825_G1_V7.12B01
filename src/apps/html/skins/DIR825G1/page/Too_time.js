
<?mget :InternetGatewayDevice.Time. "X_TWSZ-COM_Enable X_TWSZ-COM_Status X_TWSZ-COM_Type NTPServer1 NTPServer2 CurrentLocalTime LocalTimeZone LocalTimeZoneName DaylightSavingsUsed X_TWSZ-COM_DateTime DaylightSavingsStart DaylightSavingsEnd DaylightSavingsDelta"
`	G_Enable 		= "$01";
	G_Status 		= "$02";
	G_Type 			= "$03";
	G_NTPServer 		= "$04";
	G_NTPServer2 		= "$05";
	G_CurrentLocalTime	= "$06";
	G_LocalTimeZone		= "$07";
	G_LocalTimeZoneName	= "$08";
	G_DaylightSavingsUsed 	= "$09";
	G_DateTime 		= "$0a";
	G_DaylightSavingsStart        = "$0b";
	G_DaylightSavingsEnd          = "$0c";
	G_DaylightSavingsDelta        = "$0d";
`?>
var runTime = G_CurrentLocalTime;
var tmpSplit = runTime.split(' ')
var count=0;
var YEAR = Number((tmpSplit[0].split('-'))[0]);
var	MONTH =Number((tmpSplit[0].split('-'))[1]);
var DAY =Number((tmpSplit[0].split('-'))[2]);	
var HOUR =Number((tmpSplit[1].split(':'))[0]);
var MINUTE =Number((tmpSplit[1].split(':'))[1]);
var SEC =Number((tmpSplit[1].split(':'))[2]);

//加载初始化
function uiOnload()
{
	var _start = TimeToValue(G_DaylightSavingsStart,'S','M');
	var _end   = TimeToValue(G_DaylightSavingsEnd,'E','M');
	
	setJSONValue({
		
		'daylight_sm'            : _start[0],
		'daylight_sw'             : _start[1],
		'daylight_sd'              : _start[2],
		'daylight_st'             : _start[3],
		
		'daylight_em'           : _end[0],
		'daylight_ew'             : _end[1],
		'daylight_ed'              : _end[2],
		'daylight_et'             : _end[3]
	});
	
	setJSONValue({
		'ntp_enable'              : G_Enable,
		'ntp_server'         : G_NTPServer,
		'st_time'           : G_CurrentLocalTime,
		'timezone'          	  : G_LocalTimeZone || '56',
		'daylight' 					: G_DaylightSavingsUsed,
		'daylight_offset'            : G_DaylightSavingsDelta
	});
	OnClickNtpEnb();
	DaylightSetEnable();
	var CurrentLocalTime = G_CurrentLocalTime.split(' ');
	var date = CurrentLocalTime[0].split('-');
	var time = CurrentLocalTime[1].split(':');	
	setJSONValue({
		'year'              : date[0],
		'month'         	 : date[1]
	});
	createDays();
	setJSONValue({
		'day'               : Number(date[2]),
		'hour'              : time[0],
		'minute'            : time[1],
		'second'            : time[2]
	});
}
function TimeShowChange_S(_M_W)
{
	var _time = ValueToTime($('daylight_sm','daylight_sw','daylight_sd','daylight_st'),':00:00');
	var _start = TimeToValue(_time,'S',_M_W);
}

function TimeShowChange_E(_M_W)
{
	var _time = ValueToTime($('daylight_em','daylight_ew','daylight_ed','daylight_et'),':00:00');
	var _end = TimeToValue(_time,'E',_M_W);	
}
function ValueToTime(_arr,_m){ //-> value -> time;
	//2000-04-01T02:00:00
	var _year=0, _month=0, _day=0, _hour=0;
	var w_array = [6,0,1,2,3,4,5];
	_year  = G_CurrentLocalTime.substr(0,4);
	_month = Number(_arr[0].value);
	//alert(_month);
	_hour  = _arr[3].value;
	//计算日期，必须要计算出当月的第一天是星期几
	var y_w_d = new Date(_year, _month, '01');
	_week  = y_w_d.getUTCDay(); //得出星期
	//_week += 1;
	//if(_week == 0) _week = 7;
	//日期 = （第N周 - 1）*7 + 前几天 + 后几天
	
	var n_th = Number(_arr[1].value);
	var w_th = Number(_arr[2].value);
	if(n_th == 0)
	{
		//_day = w_array[_week] - w_array[_idx] + 1;
		_day = w_array[w_th] - _week + 1;
	} 
	else if(n_th >= 1)
	{
		//_day = (n_th - 1)*7 + w_array[_week] + 7 - w_array[_idx] + 1;
		_day = (6 - _week + 1) + (n_th - 1)*7 + (w_array[w_th] + 1);
	}
	
	_month += 1;
	//不对用户输入的日期进行判断

	if(_day<=0)
		_day = 1;
	return _year + '-' +
		   (_month >= 10 ? _month : ('0' + _month)) + '-' +
		   (_day >= 10 ? _day : ('0' + _day)) + 'T' + 
		   (_hour >= 10 ? _hour : ('0' + _hour)) + _m;
}
function TimeToValue(_time,_S_E,_M_W){ //-> time -> value
	//2000-04-01T02:00:00
	var _split = _time.split('T');
	var time_1 = _split[0].split('-');
	var time_2 = _split[1].split(':');
	var w_array = [1,2,3,4,5,6,0]; 
	//----------
	var _month, _th, _week, _hour, _first_week;
	_month = Number(time_1[1]) - 1; //得出月份
	_day   = Number(time_1[2]); //得出日期
	_hour  = Number(time_2[0]); //得出小时
	var y_w_d = new Date(time_1[0], _month, _day);
	var _init_y_w_d = new Date(time_1[0], _month, '01');
	_week  = y_w_d.getUTCDay(); //得出星期
	_first_week = _init_y_w_d.getUTCDay();
	//_week += 1;
	if(_week == 7) _week = 0;
	//计算是第几周
	var _int = Math.floor(_day/7);
	var _remainder = _day%7;
//	alert(_int)
	//剩余天数减一如果大于星期数，则加
	if((_remainder - 1) > _week){
		_int += 1;
	}
	//整除且刚好是星期六, 则减
	if(_remainder == 0 && _week == 6){
		_int -= 1;
	}
	//alert('_week=' + _S_E + _week);
	TimeShow(time_1[0],_month,_int,_first_week,_S_E,_M_W);
	
	return [_month, _int, w_array[_week], _hour];
}
function TimeShow(_year,_month,_int,_week,_S_E,_M_W)
{
	var _num_th_show=0, _num_week_show=0, _month_day=0;
	var _w_array = [1,2,3,4,5,6,0]; 
	var _th_options = new Array();
	var _th_values = new Array();
	var _week_options = new Array();
	var _week_values = new Array();
	//alert(_year+_month+_int+_week+_S_E);
	/*var _split = _time.split('T');
	var time_1 = _split[0].split('-');
	var time_2 = _split[1].split(':');*/
	//----------
	/*var _month, _th, _week, _hour;
	_month = Number(time_1[1]) - 1; //得出月份
	_day   = Number(time_1[2]); //得出日期
	_hour  = Number(time_2[0]); //得出小时
	var y_w_d = new Date(time_1[0], _month, _day);
	_week  = y_w_d.getUTCDay(); //得出星期*/
	
	if(_month=='1')
	{
		_month_day = (((0 == _year % 4) && (0 != (_year % 100))) || (0 == _year % 400)) ? 29 : 28; 
	}
	else if(_month=='0'||_month=='2'||_month=='4'||_month=='6'||_month=='7'||_month=='9'||_month=='11')
	{
		_month_day = 31;
	}
	else
	{
		_month_day = 30;
	}
	
	//alert('_month_day'+_month_day);
	for(var _i=0,_bool=1; _bool>0; _i++)
	{
		_bool = ((_month_day - ((6 - _week + 1) + _i*7)) <= 0 ) ? 0 : 1;
		if(_bool==0)
		{
			_num_th_show = _i+1;
		}
	}
	
	//alert(_week);
	for(var _j=0; _j<_num_th_show; _j++)
	{
		_th_values.push(_j);
		_th_options.push(lang_th[_j]);
	}
	
	//alert('_th_values=' + _th_values);
	//alert('int=' + _int);
	if(_int==0)
	{
		for(var _n=_week,_m=0; _m<(6 - _week + 1); _m++,_n++)
		{
			_week_values.push(_w_array[_n]);
			_week_options.push(lang_week[_n]);
		}
	}
	else if(_int==(_num_th_show-1))
	{
		for(var _m=0; _m<(_month_day - ((6 - _week + 1) + (_num_th_show-2)*7)); _m++)
		{
			_week_values.push(_w_array[_m]);
			_week_options.push(lang_week[_m]);
		}
	}
	else
	{
		for(var _m=0; _m<7; _m++)
		{
			_week_values.push(_w_array[_m]);
			_week_options.push(lang_week[_m]);
		}
	}

	//alert('_week_values=' + _week_values);
	if(_S_E=='S')
	{
		if(_M_W == 'M')
		{
			$S('daylight_sw', _th_options, _th_values);
			$('daylight_sw').value =  _int;
			$S('daylight_sd', _week_options, _week_values);
		}
		else if(_M_W == 'W')
		{
			$S('daylight_sd', _week_options, _week_values);
		}		
	}
	else if(_S_E=='E')
	{
		if(_M_W == 'M')
		{
			$S('daylight_ew', _th_options, _th_values);
			$('daylight_ew').value =  _int;
			$S('daylight_ed', _week_options, _week_values);
		}
		else if(_M_W == 'W')
		{
			$S('daylight_ed', _week_options, _week_values);
		}
	}
}

var lang_th = [data_languages.Too_time.innerHTML.TIME147,data_languages.Too_time.innerHTML.TIME148,data_languages.Too_time.innerHTML.TIME149,data_languages.Too_time.innerHTML.TIME150,data_languages.Too_time.innerHTML.TIME151,data_languages.Too_time.innerHTML.TIME152];
var lang_week = [data_languages.Too_schedules.innerHTML.SCS008,data_languages.Too_schedules.innerHTML.SCS009,data_languages.Too_schedules.innerHTML.SCS010,data_languages.Too_schedules.innerHTML.SCS011,data_languages.Too_schedules.innerHTML.SCS012,data_languages.Too_schedules.innerHTML.SCS013,data_languages.Too_schedules.innerHTML.SCS007];
function createDays()
{	
	var _day_options = new Array();
	var _day_values = new Array();
	var _year = $('year').value; 
	var _month = $('month').value; 
	var _day_array = ["1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31"]; 
	var _day_options = new Array();
	var _day_values = new Array();
	if(_month=='02')
	{
		_day = (((0 == _year % 4) && (0 != (_year % 100))) || (0 == _year % 400)) ? 29 : 28; 
	}
	else if(_month=='01'||_month=='03'||_month=='05'||_month=='07'||_month=='08'||_month=='10'||_month=='12')
	{
		_day = 31;
	}
	else
	{
		_day = 30;
	}	
	
	for(var _m=0; _m<_day; _m++)
	{
		_day_values.push(_day_array[_m]);
		_day_options.push(_day_array[_m]);
	}

	$S('day', _day_options, _day_values);
}
function checkTimeZone(){
	var TimeZone = $('timezone').options;
	
	for(var i = 0; i < TimeZone.length; i++){
		if(G_LocalTimeZone == '' || TimeZone[i].value == G_LocalTimeZone){
			return;
		}
	}
	//对'+'与'-'的符号进行交换处理
	TimeZone[26] = new Option('(GMT' + (G_LocalTimeZone.indexOf('-') > -1 ? G_LocalTimeZone.replace(/\-/,'+') : G_LocalTimeZone.replace(/\+/,'-')) + ')');
	TimeZone[26].value = G_LocalTimeZone;
}



function searchTimeZoneName(){
	//TimeZoneName和TimeZone的对应关系图
	var timeZoneNameMap = {
		'1':'IDLE',//-12
		'2':'UTC',//-11
		'3':'HST',//-10
		'4':'AKST',//-9
		'5':'PST',//-8
		'6':'MST',//-7
		'7':'MST',//-7
		'8':'MST',//-7
		'9':'CST',//-6
		'10':'CST',//-6
		'11':'CST',//-6
		'12':'CST',//-6
		'13':'EST',//-5
		'14':'EST',//-5
		'15':'VEN',//-430
		'16':'AST',//-4
		'17':'AST',//-4
		'18':'AST',//-4
		'19':'NST',//-330
		'20':'BRT',//-3
		'21':'BRT',//-3
		'22':'BRT',//-3
		'23':'FNT',//-2
		'24':'WAT',//-1
		'25':'WAT',//-1
		'26':'GMT',//0
		'27':'GMT',//0
		'28':'MET',//+1
		'29':'MET',//+1
		'30':'MET',//+1
		'31':'MET',//+1
		'32':'MET',//+1
		'33':'EET',//+2
		'34':'EET',//+2
		'35':'EET',//+2
		'36':'EET',//+2
		'37':'EET',//+2
		'38':'EET',//+2
		'39':'EAT',//+3
		'40':'EAT',//+3
		'41':'EAT',//+3
		'42':'IRT',//+330
		'43':'MUT',//+4
		'44':'MUT',//+4
		'45':'MUT',//+4
		'46':'NZT',//+430
		'47':'TFT',//+5
		'48':'LKA',//+530
		'49':'LKA',//+530
		'50':'NPL', //+545
		'51':'ALMT',//+600
		'52':'ALMT',//+600
		'53':'MMT', //+630
		'54':'WAST',//+7
		'55':'WAST',//+7
		'56':'CCT',//+8
		'57':'CCT',//+8
		'58':'CCT',//+8
		'59':'CCT',//+8
		'60':'CCT',//+8
		'61':'CCT',//+8
		'62':'JST',//+9
		'63':'JST',//+9
		'64':'JST',//+9
		'65':'JST',//+9
		'66':'JST',//+9
		'67':'AEST',//+10
		'68':'AEST',//+10
		'69':'AEST',//+10
		'70':'AEST',//+10
		'71':'AEST',//+10
		'72':'AESST',//+11
		'73':'AESST',//+11
		'74':'AESST',//+12
		'75':'NZT',//+12
		'76':'NZT',//+12
		'77':'NZDT',//+13
		'78':'NZDT'//+13
	};
	for(var i in timeZoneNameMap){
		if(i == arguments[0]){
			return timeZoneNameMap[i];
		}
	}
	return G_LocalTimeZoneName;
}

//重组时间
function cbnationTime(){
	var _dateTime, 
	_Month = $('month').value || '00',
	_Date = $('day').value || '00',
	_Year = $('year').value || '0000',
	_Hour = $('hour').value || '00',
	_Min = $('minute').value || '00',
	_Sec = $('second').value || '00';
	_dateTime = _Year +'-'+ _Month +'-'+ _Date +'T'+ _Hour +':'+ _Min +':'+ _Sec;
	//alert(_dateTime);
	return _dateTime;
}

function setdlTime( eleArray, time ){

    if ( typeof(time) != undefined && typeof(time) == 'string'){
        var mdy = time.split('T')[0];
        var hm = time.split('T')[1];

        eleArray[0].value = mdy.split('-')[0]   || '0000'; //year
        eleArray[1].value = mdy.split('-')[1]   || '00'; //month
        eleArray[2].value = mdy.split('-')[2]   || '00'; //date
        eleArray[3].value = hm.split(':')[0]    || '00';  //hour
        eleArray[4].value = hm.split(':')[1]    || '00';  //minute

    }else{
        var _Year   = eleArray[0].value;
        var _Month  = eleArray[1].value;
        var _Date   = eleArray[2].value;
        var _Hour   = eleArray[3].value;
        var _Min    = eleArray[4].value;
        return( _Year +'-'+ _Month +'-'+ _Date +'T'+ _Hour +':'+ _Min +':00' );
    }

}

function uiclkDaylight(){
	if(Form.Checkbox('CHB_AutoEnable') == true){
		$('dlstart').style.display = "";
        $('dlend').style.display = "";
	}else{
		$('dlstart').style.display = "none";
        $('dlend').style.display = "none";
	}
}

//提交数据
function uiSubmit(){

		
	$H({
		':InternetGatewayDevice.Time.X_TWSZ-COM_Type' 		: Form.Checkbox('ntp_enable') ? "NtpServer" : "Manually",
		':InternetGatewayDevice.Time.X_TWSZ-COM_Enable' 	: Form.Checkbox('ntp_enable') ? "1" : "0",
		':InternetGatewayDevice.Time.NTPServer1' 		: Form.Checkbox('ntp_enable') ? $('ntp_server').value : undefined,		
		':InternetGatewayDevice.Time.LocalTimeZone' 		: Form.Checkbox('ntp_enable') ? Form.Select('timezone') : undefined,
		':InternetGatewayDevice.Time.LocalTimeZoneName' 	: Form.Checkbox('ntp_enable') ? searchTimeZoneName(Form.Select('timezone')) : undefined,
		//':InternetGatewayDevice.Time.DaylightSavingsUsed' 	: Form.Checkbox('ntp_enable') ? Form.Checkbox('daylight') : undefined,
		':InternetGatewayDevice.Time.DaylightSavingsUsed' 	: Form.Checkbox('daylight'),
		':InternetGatewayDevice.Time.DaylightSavingsDelta' 	: Form.Checkbox('ntp_enable') ? Form.Select('daylight_offset') : undefined,
        ':InternetGatewayDevice.Time.DaylightSavingsStart'    : Form.Checkbox('ntp_enable') ? ValueToTime($('daylight_sm','daylight_sw','daylight_sd','daylight_st'),':00:00') : undefined,
        ':InternetGatewayDevice.Time.DaylightSavingsEnd'      : Form.Checkbox('ntp_enable') ? ValueToTime($('daylight_em','daylight_ew','daylight_ed','daylight_et'),':00:00') : undefined,
		':InternetGatewayDevice.Time.X_TWSZ-COM_DateTime' 	: !Form.Checkbox('ntp_enable') ? cbnationTime() : undefined,
		'obj-action' 		: 'set',
		'var:menu' 		: G_Menu,
		'var:page'  		: G_Page,
		'var:errorpage' 	: G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage' 		: 'html/index.html',
		'errorpage' 		: 'html/index.html',
		'var:CacheLastData' 	: ViewState.Save()
	}, true);
	$('uiPostForm').submit();	
	$("menu").style.display="none";
	$("content").style.display="none";
	$("mbox").style.display="";
	$('TIME003').disabled= true;
	$('TIME106').disabled= true;
	$('manual_sync').disabled= true;
}
function setComputerTime()
{
	$H({
		':InternetGatewayDevice.Time.X_TWSZ-COM_Type' 		: "PCClock",
		':InternetGatewayDevice.Time.X_TWSZ-COM_Enable' 	: "0",
		'obj-action' 		: 'set',
		'var:menu' 		: G_Menu,
		'var:page'  		: G_Page,
		'var:errorpage' 	: G_Page,
		'var:sys_Token' : G_SysToken,
		'getpage' 		: 'html/index.html',
		'errorpage' 		: 'html/index.html',
		'var:CacheLastData' 	: ViewState.Save()
	}, true);
	
	var obj_date = new Date();
	var _TimezoneOffset = obj_date.getTimezoneOffset()/60;
	var _zone = reTurnZone(_TimezoneOffset);
	var _year = obj_date.getFullYear(),
		_month= obj_date.getMonth() + 1,
		_day  = obj_date.getDate(),
		_hour = obj_date.getHours() < 10 ? "0"+obj_date.getHours() : obj_date.getHours(),
		_minutes = obj_date.getMinutes() < 10 ? "0"+obj_date.getMinutes() : obj_date.getMinutes(),
		_seconds  = obj_date.getSeconds() < 10 ? "0"+obj_date.getSeconds() : obj_date.getSeconds();
		_datetime = _year +"-"+ _month +"-"+ _day +"T"+_hour+":"+_minutes+":"+_seconds;
	$F(':InternetGatewayDevice.Time.DaylightSavingsUsed', 	"0");
	//$F(':InternetGatewayDevice.Time.LocalTimeZone', 	_zone);
	//$F(':InternetGatewayDevice.Time.LocalTimeZoneName', 	searchTimeZoneName(_zone));
	$F(':InternetGatewayDevice.Time.X_TWSZ-COM_DateTime', 	_datetime);
	
	$('uiPostForm').submit();
	$('TIME003').disabled= true;
	$('TIME106').disabled= true;
	$('manual_sync').disabled= true;
}
function DayLightTimeObj()
	{		
		var i, t;
		for(i=0; i<24; i++)
		{										
			if(i<12) 														
				mark="AM";			
			else if(i>=12)				
				mark="PM";			
			
			if(i<10)			
				t = "0"+i+":00:00";	
			else
				t = i+":00:00";	
			
			if(i==0 || i==12)			
				document.write("<option value=\""+t+"\">12:00 "+mark+"</option>");			
			else			
				document.write("<option value=\""+t+"\">"+i+":00 "+mark+"</option>");			
		}																		
	}
function SelectTimeZone(dst_auto_flag)
	{				
		var timezonep, dst;
										
		if(dst_auto_flag)
		{
			timezonep = "/runtime/services/timezone/zone:"+($("timezone").selectedIndex + 1)+"/dst";
			dst = XG(timezone_p+timezonep);
			if(dst!="")
				{ $("daylight").checked =true; }				
			else 
				{ $("daylight").checked =false; }
			offsetv = "+01:00";			
		}
		else
		{			
			if($("daylight").checked == true)
			{
				timezonep = "/device/time/dstmanual";
				dst = XG(devtime_p+timezonep);	
				offsetp = "/device/time/dstoffset";
				offsetv = XG(devtime_p+offsetp);
			}
			else
			{
				dst = "";
				offsetv = "+01:00";
			}		
		}
				
		if(dst !="")
		{						
			var mystr = dst.split(",");
			var mystr2 = "";
			var i, j, k;
									
			SelectOption("daylight_offset", offsetv);
				
			for(i=1;i <mystr.length; i++) 
			{											
				mystr2 = mystr[i].split(".");
				for(j=0;j <mystr2.length; j++) 
				{					
					switch(j)
					{
						case 0:															
							SelectOption((i==1?"daylight_sm":"daylight_em"), mystr2[j].substring(1));															
							break;
						case 1:
							SelectOption((i==1?"daylight_sw":"daylight_ew"), mystr2[j]);
							break;			
						case 2:
							var mystr3 = mystr2[j].split("/");							
							for(k=0;k <mystr3.length; k++) 
							{																
								if(k==0)
									SelectOption((i==1?"daylight_sd":"daylight_ed"), mystr3[k]);
								else	
									SelectOption((i==1?"daylight_st":"daylight_et"), mystr3[k]);	
							}	
							break;												
					}					
				}																						
			}				
		}
		else
		{																				
			SelectOption("daylight_offset", offsetv);
			for(i=1; i<3; i++)
			{
				SelectOption((i==1?"daylight_sm":"daylight_em"), 1);	
				SelectOption((i==1?"daylight_sw":"daylight_ew"), 1);
				SelectOption((i==1?"daylight_sd":"daylight_ed"), 0);
				SelectOption((i==1?"daylight_st":"daylight_et"), "00:00:00");	
			}
		}
		
		DaylightSetEnable();				
	}
function OnClickNtpEnb()
	{
		NtpEnDiSomething();
	}
function NtpEnDiSomething()
	{
		var dis = $("ntp_enable").checked ? false : true;
		/* ntp part */
		$("ntp_server").disabled  = $("ntp_sync").disabled = dis;
		$("manual_sync").disabled = !(dis);
		/* manual part */
		$("year").disabled = $("month").disabled  = $("day").disabled = !(dis);
		$("hour").disabled = $("minute").disabled = $("second").disabled = !(dis);
	}
function DrawDayMenu()
	{
		var old_day_value = S2I($("day").value);

		var year = S2I($("year").value);
		var mon  = S2I($("month").value);
		var days = GetDaysInMonth(year, mon);
				
		for (var i=0;i<days;i++)
		{
			$("day").options[i]=new Option(i+1, i+1);			
		}
		
		$("day").length=days;

		if( days>=old_day_value ) $("day").value=old_day_value;
	}
function GetDaysInMonth(year, mon)
	{
		var days;
		if (mon==1 || mon==3 || mon==5 || mon==7 || mon==8 || mon==10 || mon==12) days=31;
		else if (mon==4 || mon==6 || mon==9 || mon==11) days=30;
		else if (mon==2)
		{
			if (((year % 4)==0) && ((year % 100)!=0) || ((year % 400)==0)) { days=29; }
			else { days=28; }
		}
		return (days);
	}
function OnClickNTPSync()
{
	if($("ntp_server").value==="")
	{
		alert(SEcode['lang_time_server']);
		return false;
	}
	$("sync_msg").innerHTML = "Synchronizing ...";	
}
function onClickManualSync()
{
	$("sync_pc_msg").innerHTML = "Synchronizing ...";	
}
var S2I	= function(str) {return isNaN(str)?0:parseInt(str, 10);}
function OnChangeMonth()	{ DrawDayMenu(); }
function OnChangeYear()	{ DrawDayMenu(); }
function DaylightSetEnable()
	{				
		if($("daylight").checked)
		{
			$("daylight_offset").disabled = false;
			$("daylight_sm").disabled = false;	
			$("daylight_sw").disabled = false;
			$("daylight_sd").disabled = false;
			$("daylight_st").disabled = false;
			$("daylight_em").disabled = false;	
			$("daylight_ew").disabled = false;
			$("daylight_ed").disabled = false;
			$("daylight_et").disabled = false;							
		}
		else
		{
			$("daylight_offset").disabled = true;
			$("daylight_sm").disabled = true;	
			$("daylight_sw").disabled = true;
			$("daylight_sd").disabled = true;
			$("daylight_st").disabled = true;
			$("daylight_em").disabled = true;	
			$("daylight_ew").disabled = true;
			$("daylight_ed").disabled = true;
			$("daylight_et").disabled = true;	
		}				
	}
function reTurnZone(xValue){
	if(xValue == undefined){
		var xValue = -8;
	}
	if(xValue == 0){
		xValue = "+00:00";
	}else if(xValue > 0){
		if(xValue > 9)
			xValue = "-"+Math.abs(xValue)+":"+"00";
		else
			xValue = "-0"+Math.abs(xValue)+":"+"00";
	}else{
		if(xValue < -9)
			xValue = "+"+Math.abs(xValue)+":"+"00";
		else
			xValue = "+0"+Math.abs(xValue)+":"+"00";
	}
	
	return xValue;
}


function uiRefreshDate()
{
	getInfo();
	
	if (G_Status == 'Synchronized'){
		$('timeShow').innerHTML = '<font color="#33CC66" >' + G_CurrentLocalTime  + '</font>';
	}else{
	    $('timeShow').innerHTML = '<font color="#33CC66" >' + G_CurrentLocalTime  + '</font>';
		//$('timeShow').innerHTML = '<font color="#ff0000" >' + G_CurrentLocalTime  + data_languages.Too_time.innerHTML.TIME004  + '</font>';
	}
	
	onClkSynchron();
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
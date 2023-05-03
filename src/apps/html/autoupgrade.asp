<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Pragma" content="No-cache" />
<meta http-equiv="Cache-Control" content="no-cache, must-revalidate" />
<meta http-equiv="Expires" content="-1" />
<title>Sitecom Wireless Modem Router N300</title>
<link rel="stylesheet" type="text/css" href="/html/skin/main.css">
<style type="text/css">
.black_overlay{ position: absolute; top: 0%; left: 0%; width: 100%; height: 100%; background-color:black; z-index:1001; -moz-opacity: 0.08; opacity:.20; /* for mozilla */ filter: alpha(opacity=20); /* for IE */ }
.white_content { position: absolute; top: 225px; left: 40%; width: 20%; height: 115; padding: 16px; text-align:center; border: 1px solid #BBD2DB; background-color: #BBD2DB; z-index:1002; overflow: auto; }
</style>
<script type="text/javascript" src="/html/js/ajax.js"></script>
<script type="text/javascript" src="/html/js/boot.js"></script>
<script type="text/javascript" src="/html/js/seboot.js"></script>
<script type="text/javascript">

var G_FWUpdate = 0;

function uiSubmit(action){
    if (action == 1){
        $('mainbtn').style.display = 'none';
        $('progress').style.display = '';
    //   var _url = document.location.href;
    //    _url = _url.substring(0, _url.lastIndexOf('/'));
    //    setTimeout(function(){document.location.replace(_url);}, 180000);
        G_FWUpdate = 1;
    }


    $('autoUpgradeAction').value = action;
    var _url = "/cgi-bin/webupg";
    ajax = Ajax.getInstance( _url, "", 0, normalCommitProc);
    ajax.post($('autoUpgradeform'));

    
    return true;
}

/* for multi-language */

function loadScript(url, callback){

    var script = document.createElement("script")
    script.type = "text/javascript";

    if (script.readyState){  //IE
        script.onreadystatechange = function(){
            if (script.readyState == "loaded" ||
                    script.readyState == "complete"){
                script.onreadystatechange = null;
                callback();
            }
        };
    } else {  //Others
        script.onload = function(){
            callback();
        };
    }

    script.src = url;
    document.getElementsByTagName("head")[0].appendChild(script);
}

function normalCommitProc(responseText){
    if (responseText.indexOf('Successed') != -1){
        if (G_FWUpdate == 1){
            setTimeout(function(){
                        $('mainbtn').style.display = 'none';
                        $('progress').style.display = 'none';
                        $('commitConfirm').style.display = 'none';
                        $('errorConfirm').style.display = 'none';
                        $('updateSuccess').style.display = '';
                    }, 180000);
        return;
    }
        $('mainbtn').style.display = 'none';
        $('progress').style.display = 'none';
        $('commitConfirm').style.display = '';
        $('errorConfirm').style.display = 'none';

    }else{
        $('mainbtn').style.display = 'none';
        $('progress').style.display = 'none';
        $('commitConfirm').style.display = 'none';
        $('errorConfirm').style.display = '';
    }

}

function setLang(lang_value){
	if(lang_value == "CYL"){
		return false;
	}
	Cookie.Set('language', lang_value);
	uiOnload();
}

function uiOnload(){
    var lang = Cookie.Get('language');
    if(lang == null || lang == 'CYL' ){
        lang = '/html/languages/en_us/';
    }else{
        lang = '/html/languages/' + lang + '/';
    }

    loadScript(lang + 'error.js', function(){});
    loadScript(lang + 'languages.js', function(){});
    setTimeout(TopSelfMenu, 500);
	
  //  TopSelfMenu();
}
/* end */

addListeners(uiOnload);

</script>
</head>

<body>
    <div class="container">
        <div class="header">
            <img style="float:left;padding:15px 0 20px 10px;" src="/html/skin/header.png" alt="header" />
            <img style="float:right;padding:20px 10px 0 0" src="/html/skin/logo.png" alt="logo" />
            <div class="menu">
                <ul id="menu"></ul>
                <div style="float:right;padding:0;margin:-42px 10px 0 0;">
                    <select id="TOPM100" onchange="setLang(this.value);">
                        <option value="CYL">Language</option>
                        <option value="ENU">English</option>
                        <option value="DEU">Deutsch</option>
                        <option value="FRA">Français</option>
                        <option value="NLD">Nederlands</option>
                        <option value="ITA">Italiano</option>
                        <option value="ESP">Español</option>
                        <option value="PTB">Português</option>
                    <!--<option value="DAN">Polish</option>-->
                        <option value="RUS">Русский</option>
                    </select>
                </div>
            </div>
            <!-- Second Menu -->
            <br /><br />
        </div>
        <div class="content">
            <table width="900px" border="0" cellspacing="2" id="mainbtn">
                <tr>
                    <td class="background" style="width:25%; " id="AUG001">Install the fir :</td>
                    <td >
                        <input type="button" id="AUG002" value="Install" onclick="uiSubmit(1)"></div>
                    </td>
                </tr>
                <tr>
                    <td class="background" style="width:25%; " id="AUG003">Reminders :</td>
                    <td >
                        <input type="button" id="AUG004" value="Red M Ler" onclick="uiSubmit(2)"></div>
                    </td>
                </tr>
                <tr>
                    <td class="background" style="width:25%; " id="AUG005">Do not remind me :</td>
                    <td >
                        <input type="button" id="AUG006" value="For this version" onclick="uiSubmit(3)">
                        <input type="button" id="AUG007" value="Any more" onclick="uiSubmit(4)">
                    </td>
                </tr>
            </table>
            <table width="900px" border="0" cellspacing="2" id="progress" style="display:none">
                <tr>
                    <td>Firmware upgrade confirmed, please wait for the upgrading process...
                        <marquee style='border:1px solid #000000' direction=right width=910 scrollamount=1 scrolldelay=300 bgcolor=#FFFFFF>
                            <table cellspacing=1 cellpadding=0>
                                <tr height=16><td bgcolor=#ff7700 width=65535></td></tr>
                            </table>
                        </marquee>
                    </td>
                </tr>
            </table>
            <table width="900px" border="0" cellspacing="2" id="commitConfirm" style="display:none">
                <tr>
                    <td id="AUG008">Saved Successfully</td>
                </tr>
                <tr>
                    <td ><input type="button" id="AUG009" value="Close" onclick="window.close()"></td>
                </tr>
            </table>
            <table width="900px" border="0" cellspacing="2" id="updateSuccess" style="display:none">
                <tr>
                    <td id="AUG012">Upgrade Successfully</td>
                </tr>
                <tr>
                    <td ><input type="button" id="AUG013" value="Close" onclick="window.close()"></td>
                </tr>
            </table>
            <table width="900px" border="0" cellspacing="2" id="errorConfirm" style="display:none">
                <tr>
                    <td id="AUG010">Unknown error! please contact the developer.</td>
                </tr>
                <tr>
                    <td ><input type="button" id="AUG011" value="Close" onclick="window.close()"></td>
                </tr>
            </table>
        </div>
        <div class="footer">
            <a class="footer" href="http://www.sitecom.com">www.sitecom.com</a> | <span id="copyright">© 1996 - 2012 Sitecom Europe BV, all rights reserved</span> 
        </div>
    </div>
    <form id="autoUpgradeform" method="post" action="/cgi-bin/webupg">
        <input type="hidden" name="name" value="autoupgrade" />
        <input type="hidden" name="autoUpgradeAction" id="autoUpgradeAction" value="0" />
    </form>
</body>
</html>
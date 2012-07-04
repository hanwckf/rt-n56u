<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_4#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

var varload = 0;
var lan_ipaddr = '<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>';


function initial(){
	show_banner(1);
	show_menu(5,6,4);	
	show_footer();
}

function restoreRule(){
	var alert_string = "<#Setting_factorydefault_hint1#>";
	if(lan_ipaddr != "192.168.1.1")
		alert_string += "<#Setting_factorydefault_iphint#>\n\n";			
	alert_string += "<#Setting_factorydefault_hint2#>";
	if(confirm(alert_string))
	{
		document.form.action1.blur();
		showtext($("loading_text"), "<#SAVE_restart_desc#>");
		showLoading();
		document.restoreform.submit();
	}
	else
		return false;
}

function saveSetting(){
	location.href='Settings_RT-N56U.CFG';
}

function uploadSetting(){
  var file_obj = document.form.file;
	
	if(file_obj.value == ""){
		alert("<#JS_fieldblank#>");
		file_obj.focus();
	}
	else if(file_obj.value.length < 6 ||
					file_obj.value.lastIndexOf(".CFG")  < 0 || 
					file_obj.value.lastIndexOf(".CFG") != (file_obj.value.length)-4){		
		alert("<#Setting_upload_hint#>");
		file_obj.focus();
	}
	else{		
		disableCheckChangedStatus();
		showtext($("loading_text"), "<#SET_ok_desc#>");
		document.form.submit();
	}	
}
</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body onload="initial();">
<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

<div id="LoadingBar" class="popup_bg">
    <center>
        <div class="container-fluid" style="margin-top: 150px;">
            <div class="well" style="background-color: #212121; width: 60%;">
                <div class="progress" style="max-width: 450px; text-align: left;">
                    <div class="bar" id="proceeding_img"><span id="proceeding_img_text"></span></div>
                </div>
                <div class="alert alert-danger" style="max-width: 400px;"><#SAVE_restart_desc#></div>
            </div>
        </div>
    </center>
</div>

<div id="hiddenMask" class="popup_bg" style="position: absolute; margin-left: -10000px;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
		    </div>
		  <div class="drImg"><img src="images/DrsurfImg.gif"></div>
			<div style="height:70px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div>
<!--for uniform show, useless but have exist-->

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="upload.cgi" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="current_page" value="Advanced_SettingBackup_Content.asp">
<input type="hidden" name="next_page" value="Advanced_SettingBackup_Content.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span2">
            <!--Sidebar content-->
            <!--=====Beginning of Main Menu=====-->
            <div class="well sidebar-nav side_nav" style="padding: 0px;">
                <ul id="mainMenu" class="clearfix"></ul>
                <ul class="clearfix">
                    <li>
                        <div id="subMenu" class="accordion"></div>
                    </li>
                </ul>
            </div>
        </div>

        <div class="span10">
            <!--Body content-->
            <div class="row-fluid">
                <div class="span12">
                    <div class="box well grad_colour_dark_blue">
                        <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_4#></h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>
                                <div class="alert alert-info" style="margin: 10px;"><#Setting_save_upload_desc#></div>

                                <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,19,1)"><#Setting_factorydefault_itemname#></a></th>
                                        <td>
                                            <input class="btn btn-info" style="width: 219px;" onclick="restoreRule();" type="button" value="<#CTL_restore#>" name="action1" />
                                            <input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>" />
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,19,2)"><#Setting_save_itemname#></a></th>
                                        <td>
                                            <input class="btn btn-info" style="width: 219px;" onclick="saveSetting();" type="button" value="<#CTL_onlysave#>" name="action2" />
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,19,3)"><#Setting_upload_itemname#></a></th>
                                        <td>
                                            <input name="uploadbutton" type="button" class="btn btn-primary" style="width: 219px;" onclick="uploadSetting();" value="<#CTL_upload#>" />
                                            <input type="file" name="file" class="input" size="30" />
                                        </td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

</form>

 <!--==============Beginning of hint content=============-->
 <div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
     <form name="hint_form"></form>
     <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

     <div id="hintofPM" style="display:none;">
         <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
         <thead>
             <tr>
                 <td>
                     <div id="helpname" class="AiHintTitle"></div>
                     <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
                 </td>
             </tr>
         </thead>

             <tr>
                 <td valign="top" >
                     <div class="hint_body2" id="hint_body"></div>
                     <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                 </td>
             </tr>
         </table>
     </div>
 </div>
 <!--==============Ending of hint content=============-->

<div id="footer"></div>

<form method="post" name="restoreform" action="apply.cgi" target="hidden_frame" style="position: absolute; margin-left: -10000px">
<input type="hidden" name="action_mode" value="Restore">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="sid_list" value="General;">
</form>

<form name="hint_form"></form>
</body>
</html>

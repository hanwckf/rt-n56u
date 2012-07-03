<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
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
</head>

<body onload="initial();">
<div id="TopBanner"></div>

<div id="LoadingBar" class="popup_bar_bg" style="background-image: url(/images/popup_bg2.gif);">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div class="Bar_container">
			<span id="proceeding_img_text" ></span>
			<div id="proceeding_img"></div>
		</div>
		<div id="loading_text" style="margin:5px auto; width:85%;"><#SAVE_restart_desc#></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="hiddenMask" class="popup_bg">
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

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
		
		<td valign="top" width="202">				
		<div  id="mainMenu"></div>	
		<div  id="subMenu"></div>		
		</td>				
		
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
          
		<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
			<thead>
			<tr>
				<td><#menu5_6#> - <#menu5_6_4#></td>
			</tr>
			</thead>	
            <tr>
              <td bgcolor="#FFFFFF"><#Setting_save_upload_desc#></td>
            </tr>
            <tr>
			  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="6" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          <tr>
            <th width="30%" align="right" bgcolor="#7aa3bd"><a class="hintstyle"  href="javascript:void(0);" onclick="openHint(19,1)"><#Setting_factorydefault_itemname#></a></th>
            <td>
              <input class="button" onclick="restoreRule();" type="button" value="<#CTL_restore#>" name="action1" />
              <input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>" /></td>
          </tr>
          <tr>
            <th align="right" bgcolor="#7aa3bd"><a class="hintstyle"  href="javascript:void(0);" onclick="openHint(19,2)"><#Setting_save_itemname#></a></th>
            <td>
              <input class="button" onclick="saveSetting();" type="button" value="<#CTL_onlysave#>" name="action2" />
            </td>
          </tr>
          <tr bgcolor="#a7d2e2">
            <th align="right" bgcolor="#7aa3bd"><a class="hintstyle"  href="javascript:void(0);" onclick="openHint(19,3)"><#Setting_upload_itemname#></a></th>
            <td>
			  			<input name="uploadbutton" type="button" class="button" onclick="uploadSetting();" value="<#CTL_upload#>" />
              <input type="file" name="file" class="input" size="30" />
            </td>
          </tr>
        </table>
			  </td>
            </tr>
		</table>
		</td>
</form>
          <td id="help_td" style="width:15px;" valign="top">
            <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
            <div id="hintofPM" style="display:none;">
              <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
			  	<thead>
                <tr>
                  <td><div id="helpname" class="AiHintTitle"></div><a href="javascript:void(0);" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a></td>
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
		  </td>
        </tr>
      </table>		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

<form method="post" name="restoreform" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="Restore">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="sid_list" value="General;">
</form>

<form name="hint_form"></form>
</body>
</html>

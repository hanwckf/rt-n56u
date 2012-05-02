<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title>Wireless Router <#Web_Title#> - <#menu5_4_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/wcdma_list.js"></script>
<script type="text/javascript" src="/cdma2000_list.js"></script>
<script type="text/javascript" src="/td-scdma_list.js"></script>
<script type="text/javascript" src="/wimax_list.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>]; // [[MAC, associated, authorized], ...]

function initial(){
	show_banner(1);
	show_menu(5, 4, 5);
	show_footer();
	
//	enable_auto_hint(21, 7);
}

function applyRule(){
	showLoading(); 
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "/Advanced_Printer_others.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(21, 7);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">

<input type="hidden" name="current_page" value="Advanced_Printer_others.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="General;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("", "firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
        <td width="23">&nbsp;</td>
        
        <!--=====Beginning of Main Menu=====-->
        <td valign="top" width="202">
          <div id="mainMenu"></div>
          <div id="subMenu"></div>
        </td>
        
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>
                <br />
                <!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
                <td align="left" valign="top" >
                
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
        <thead>
        <tr>
                <td><#menu5_4_5#></td>
        </tr>
        </thead>
        <tr>
        <td bgcolor="#FFFFFF">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<tr>
			<th width="40%">
				<#PrinterPortU2E#>
			</th>
			<td>
				<input type="radio" value="1" name="u2ec_enable" class="input" <% nvram_match_x("General", "u2ec_enable", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="u2ec_enable" class="input" <% nvram_match_x("General", "u2ec_enable", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th>
				<#PrinterPortLPR#>
			</th>
			<td>
				<input type="radio" value="1" name="lprd_enable" class="input" <% nvram_match_x("General", "lprd_enable", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="lprd_enable" class="input" <% nvram_match_x("General", "lprd_enable", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th>
				<#PrinterPortRAW#>
			</th>
			<td>
				<select name="rawd_enable" class="input">
					<option value="0" <% nvram_match_x("General", "rawd_enable", "0","selected"); %>><#checkbox_No#></option>
					<option value="1" <% nvram_match_x("General", "rawd_enable", "1","selected"); %>><#checkbox_Yes#></option>
					<option value="2" <% nvram_match_x("General", "rawd_enable", "2","selected"); %>><#checkbox_Yes#> (bidirectional)</option>
				</select>
			</td>
		</tr>
		<tr align="right">
			<td colspan="2">
				<input type="button" class="button" value="<#CTL_apply#>" onclick="applyRule();">
			</td>
		</tr>
	</table>
	</td>
	</tr>
</table>
</td>
</form>

                                        <!--==============Beginning of hint content=============-->
                                        <td id="help_td" style="width:15px;" valign="top">
                                                <form name="hint_form"></form>
                                                <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
                                                <div id="hintofPM" style="display:none;">
                                                        <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
                                                                <thead>
                                                                <tr>
                                                                        <td>
                                                                                <div id="helpname" class="AiHintTitle"></div>
                                                                                <a href="javascript:closeHint();">
                                                                                        <img src="images/button-close.gif" class="closebutton">
                                                                                </a>
                                                                        </td>
                                                                </tr>
                                                                </thead>
                                                                <tr>
                                                                        <td valign="top">
                                                                                <div class="hint_body2" id="hint_body"></div>
                                                                                <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                                                                        </td>
                                                                </tr>
                                                        </table>
                                                </div>
                                        </td>
                                        <!--==============Ending of hint content=============-->
                                </tr>
                        </table>
                </td>
    <td width="10" align="center" valign="top">&nbsp;</td>
        </tr>
</table>

<div id="footer"></div>
</body>
</html>

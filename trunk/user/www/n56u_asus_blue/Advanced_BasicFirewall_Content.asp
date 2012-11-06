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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>

<% login_state_hook(); %>

<% usb_apps_check(); %>

function initial(){
	show_banner(1);
	show_menu(5,5,1);
	show_footer();
	enable_auto_hint(8, 6);
	
	load_body();
	
	if(found_app_torr()){
		$("torrent_row").style.display = "";
	}
	if(found_app_aria()){
		$("aria_row").style.display = "";
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		inputRCtrl1(document.form.misc_http_x, 1);
		
		if(isModel() != "WL520gc" && isModel() != "SnapAP"){
			inputRCtrl1(document.form.misc_ping_x, 1);
		}
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_BasicFirewall_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_range(document.form.misc_httpport_x, 80, 65535))
		return false;
	
	if(!validate_range(document.form.sshd_wport, 22, 65535))
		return false;

	if(!validate_range(document.form.ftpd_wport, 21, 65535))
		return false;

	return true;
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(8, 6);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="Advanced_BasicFirewall_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
		
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />
		<!--===================================Beginning of Main Content===========================================-->		
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
	<thead>
	<tr>
		<td><#menu5_5#> - <#menu5_5_1#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
	  <td bgcolor="#FFFFFF"><#FirewallConfig_display2_sectiondesc#></td>
	  </tr>
	</tbody>
	<tr>
	  <td bgcolor="#FFFFFF">		
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
	  <thead>
		<tr>
			<td colspan="2"><#menu5_5#></td>
		</tr>
	  </thead>
          <tr>
            <th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,6);"><#FirewallConfig_FirewallEnable_itemname#></a></th>
            <td><input type="radio" value="1" name="fw_enable_x"  onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '1')" <% nvram_match_x("","fw_enable_x", "1", "checked"); %>/><#checkbox_Yes#>
            	<input type="radio" value="0" name="fw_enable_x"  onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '0')" <% nvram_match_x("","fw_enable_x", "0", "checked"); %>/><#checkbox_No#>
            	</td>
          </tr>
          <tr>
		<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,7);"><#FirewallConfig_DoSEnable_itemname#></a></th>
		<td>
			<input type="radio" value="1" name="fw_dos_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_dos_x', '1')" <% nvram_match_x("", "fw_dos_x", "1", "checked"); %>/><#checkbox_Yes#>
			<input type="radio" value="0" name="fw_dos_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_dos_x', '0')" <% nvram_match_x("", "fw_dos_x", "0", "checked"); %>/><#checkbox_No#>
		</td>
	</tr>
          <tr>
		<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,7);"><#FirewallConfigSynFlood#></a></th>
		<td>
			<input type="radio" value="1" name="fw_syn_cook" class="input" <% nvram_match_x("", "fw_syn_cook", "1", "checked"); %>/><#checkbox_Yes#>
			<input type="radio" value="0" name="fw_syn_cook" class="input" <% nvram_match_x("", "fw_syn_cook", "0", "checked"); %>/><#checkbox_No#>
		</td>
	</tr>
          <tr>
            <th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,1);"><#FirewallConfig_WanLanLog_itemname#></a></th>
            <td>
              <select name="fw_log_x" class="input" onchange="return change_common(this, 'FirewallConfig', 'fw_log_x')">
                <option value="none" <% nvram_match_x("FirewallConfig","fw_log_x", "none","selected"); %>>None</option>
                <option value="drop" <% nvram_match_x("FirewallConfig","fw_log_x", "drop","selected"); %>>Dropped</option>
                <option value="accept" <% nvram_match_x("FirewallConfig","fw_log_x", "accept","selected"); %>>Accepted</option>
                <option value="both" <% nvram_match_x("FirewallConfig","fw_log_x", "both","selected"); %>>Both</option>
              </select>
            </td>
          </tr>
          <tr>
          <th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,5);"><#FirewallConfig_x_WanPingEnable_itemname#></a></th>
          <td>
		  <input type="radio" value="1" name="misc_ping_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_ping_x', '1')" <% nvram_match_x("","misc_ping_x", "1", "checked"); %>/><#checkbox_Yes#>
		  <input type="radio" value="0" name="misc_ping_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_ping_x', '0')" <% nvram_match_x("","misc_ping_x", "0", "checked"); %>/><#checkbox_No#>
		  </td>
          </tr>
        </table>
        </td>
	</tr>

	<tr>
	  <td bgcolor="#FFFFFF">
	  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
		<thead>
		<tr>
			<td colspan="2"><#Adm_Access_WAN#></td>
		</tr>
		</thead>
		<tr>
			<th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,2);"><#FirewallConfig_x_WanWebEnable_itemname#></a></th>
			<td>
				<input type="radio" value="1" name="misc_http_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_http_x', '1')" <% nvram_match_x("","misc_http_x", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="misc_http_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_http_x', '0')" <% nvram_match_x("","misc_http_x", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,3);"><#FirewallConfig_x_WanWebPort_itemname#></a></th>
			<td>
				<input type="text" maxlength="5" size="5" name="misc_httpport_x" class="input" value="<% nvram_get_x("", "misc_httpport_x"); %>" onkeypress="return is_number(this)"/>
			</td>
		</tr>
		<tr>
			<th align="right"><#Adm_System_sshd_wopen#></th>
			<td>
				<input type="radio" name="sshd_wopen" class="input" value="1" <% nvram_match_x("", "sshd_wopen", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="sshd_wopen" class="input" value="0" <% nvram_match_x("", "sshd_wopen", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><#Adm_System_sshd_wport#></th>
			<td>
				 <input type="text" maxlength="5" size="5" name="sshd_wport" class="input" value="<% nvram_get_x("FirewallConfig","sshd_wport"); %>" onkeypress="return is_number(this)"/>
			</td>
		</tr>
		<tr>
			<th align="right"><#Adm_System_ftpd_wopen#></th>
			<td>
				<input type="radio" name="ftpd_wopen" class="input" value="1" <% nvram_match_x("", "ftpd_wopen", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="ftpd_wopen" class="input" value="0" <% nvram_match_x("", "ftpd_wopen", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><#Adm_System_ftpd_wport#></th>
			<td>
				 <input type="text" maxlength="5" size="5" name="ftpd_wport" class="input" value="<% nvram_get_x("FirewallConfig","ftpd_wport"); %>" onkeypress="return is_number(this)"/>
			</td>
		</tr>
		<tr id="torrent_row" style="display:none;">
			<th align="right"><#Adm_System_trmd_ropen#></th>
			<td>
				<input type="radio" name="trmd_ropen" class="input" value="1" <% nvram_match_x("", "trmd_ropen", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="trmd_ropen" class="input" value="0" <% nvram_match_x("", "trmd_ropen", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr id="aria_row" style="display:none;">
			<th align="right"><#Adm_System_aria_ropen#></th>
			<td>
				<input type="radio" name="aria_ropen" class="input" value="1" <% nvram_match_x("", "aria_ropen", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="aria_ropen" class="input" value="0" <% nvram_match_x("", "aria_ropen", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
	</table>
	</td>
	</tr>

	<tr>
            <td bgcolor="#FFFFFF" align="right"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>
	</tr>

</table>
</td>
</form>

          <td id="help_td" style="width:15px;" valign="top">
<form name="hint_form"></form>
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
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>

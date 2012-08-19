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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>

<% login_state_hook(); %>
var client_mac = login_mac_str();
var macfilter_num_x = '<% nvram_get_x("FirewallConfig", "macfilter_num_x"); %>';
var smac = client_mac.split(":");
var simply_client_mac = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];

function initial(){
	show_banner(1);
	show_menu(5,5,4);
	show_footer();
	
	enable_auto_hint(18, 1);

	load_body();

	change_macfilter();
}

function applyRule(){
	if(prevent_lock()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_MACFilter_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function prevent_lock(){
	if(document.form.macfilter_enable_x.value == "1"){
		if(macfilter_num_x == 0){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.macfilter_list_x_0.value = simply_client_mac;
				markGroup(document.form.MFList, 'MFList', 64, ' Add ');
				document.form.submit();
			}
			else
				return false;
		}
		else
			return true;
	}
	else
		return true;
}

function done_validating(action){
	refreshpage();
}

function change_macfilter() {
	if(document.form.macfilter_enable_x.value!="0"){
		$("mac_drop_row").style.display = "";
	}
	else{
		$("mac_drop_row").style.display = "none";
	}
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(18, 1);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
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
<input type="hidden" name="current_page" value="Advanced_MACFilter_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="MFList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
		
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_5#> - <#menu5_5_3#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#FirewallConfig_display5_sectiondesc#></td>
	</tr>
	</tbody>
	<tr>
	  <td bgcolor="#FFFFFF">
	  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
        <tr>
          <th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,1);"><#FirewallConfig_MFMethod_itemname#></a></th>
          <td><select name="macfilter_enable_x" class="input" onchange="change_macfilter()">
              <option value="0" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "0","selected"); %>><#CTL_Disabled#></option>
              <option value="1" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "1","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
              <option value="2" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "2","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
          </select></td>
        </tr>
        <tr id="mac_drop_row" style="display:none;">
            <th><#MAC_BlockHost#></th>
            <td>
                <input type="radio" value="1" name="fw_mac_drop" <% nvram_match_x("","fw_mac_drop", "1", "checked"); %>/><#checkbox_Yes#>
                <input type="radio" value="0" name="fw_mac_drop" <% nvram_match_x("","fw_mac_drop", "0", "checked"); %>/><#checkbox_No#>
            </td>
        </tr>
        <tr>
          <th id="UrlList"><#FirewallConfig_MFhwaddr_itemname#>
              <input type="hidden" name="macfilter_num_x_0" value="<% nvram_get_x("FirewallConfig", "macfilter_num_x"); %>" readonly="1" />
		  </th>
          <td><input type="text" maxlength="12" class="input" size="20" name="macfilter_list_x_0" onKeyPress="return is_hwaddr()">
   		  	  <input class="button" type="submit" onclick="markGroup(this, 'MFList', 64, ' Add ');" name="MFList" value="<#CTL_add#>" size="12"/>
			  <br/><span>*<#JS_validmac#></span>
		  </td>
        </tr>
        <tr>
          <th><#FirewallConfig_MFList_groupitemname#></th>
          <td>
		  <select size="8" class="input" name="MFList_s" multiple="multiple" style="font-size:12px; font-weight:bold; vertical-align:middle;">
            <% nvram_get_table_x("FirewallConfig","MFList"); %>
          </select>
		  <input class="button" type="submit" onClick="return markGroup(this, 'MFList', 64, ' Del ');" name="MFList" value="<#CTL_del#>" size="12">
		  </td>
        </tr>
		<tr align="right">
          <td colspan="2" >
		  <input name="button" type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>"/></td>
          </tr>
      </table></td>
	  </tr>
</table>
</td>
</form>
          <td id="help_td" style="width:15px;" valign="top">

            <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
            <div id="hintofPM" style="display:none;">
            	<form name="hint_form"></form>
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

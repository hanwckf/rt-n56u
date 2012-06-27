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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_6#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state_5g.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var check_hwnat = '<% check_hwnat(); %>';
var hwnat = '<% nvram_get_x("",  "hwnat"); %>';

function initial(){

	show_banner(1);
	
	show_menu(5,1,6);
	
	show_footer();
	
	enable_auto_hint(3, 20);

	load_body();
	if(document.form.wl_gmode.value == "1"){	//n only
		inputCtrl(document.form.HT_OpMode, 1);
		$("wl_wme_tr").style.display = "none";
		
	}else if(document.form.wl_gmode.value == "2"){	//Auto
		$("HT_OpMode").value = "0";
		inputCtrl(document.form.HT_OpMode, 0);
		$("wl_wme_tr").style.display = "none";
			
	}else{
		$("HT_OpMode").value = "0";
		inputCtrl(document.form.HT_OpMode, 0);
		$("wl_wme_tr").style.display = "";
	}
}

function applyRule(){

	if(validForm()){
		updateDateTime(document.form.current_page.value);
		
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WAdvanced_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_range(document.form.wl_frag, 256, 2346)
			|| !validate_range(document.form.wl_rts, 0, 2347)
			|| !validate_range(document.form.wl_dtim, 1, 255)
			|| !validate_range(document.form.wl_bcn, 20, 1000)
			)
		return false;

	if(document.form.wl_radio_x[0].checked){
		if(!validate_timerange(document.form.wl_radio_time_x_starthour, 0)
				|| !validate_timerange(document.form.wl_radio_time_x_startmin, 1)
				|| !validate_timerange(document.form.wl_radio_time_x_endhour, 2)
				|| !validate_timerange(document.form.wl_radio_time_x_endmin, 3)
				)
			return false;
		
		var starttime = eval(document.form.wl_radio_time_x_starthour.value + document.form.wl_radio_time_x_startmin.value);
		var endtime = eval(document.form.wl_radio_time_x_endhour.value + document.form.wl_radio_time_x_endmin.value);
		
		if(starttime == endtime){
			alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
				document.form.wl_radio_time_x_starthour.focus();
				document.form.wl_radio_time_x_starthour.select;
			return false;
		}
	}

	if((document.form.wl_radio_x[0].checked ==true) 
		&& (document.form.wl_radio_date_x_Sun.checked ==false)
		&& (document.form.wl_radio_date_x_Mon.checked ==false)
		&& (document.form.wl_radio_date_x_Tue.checked ==false)
		&& (document.form.wl_radio_date_x_Wed.checked ==false)
		&& (document.form.wl_radio_date_x_Thu.checked ==false)
		&& (document.form.wl_radio_date_x_Fri.checked ==false)
		&& (document.form.wl_radio_date_x_Sat.checked ==false)){
			alert("<#WLANConfig11b_x_RadioEnableDate_itemname#><#JS_fieldblank#>");
			document.form.wl_radio_x[0].checked=false;
			document.form.wl_radio_x[1].checked=true;
			return false;
	}
	
	return true;
}

function done_validating(action){
	refreshpage();
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(3, 16);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">

<input type="hidden" name="wl_gmode" value="<% nvram_get_x("WLANConfig11b","wl_gmode"); %>">
<input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>">

<input type="hidden" name="current_page" value="Advanced_WAdvanced_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANAuthentication11a;WLANConfig11b;LANHostConfig;PrinterStatus;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="wl_radio_date_x" value="<% nvram_get_x("WLANConfig11b","wl_radio_date_x"); %>">
<input type="hidden" name="wl_radio_time_x" value="<% nvram_get_x("WLANConfig11b","wl_radio_time_x"); %>">
<input type="hidden" name="hwnat" value="<% nvram_get_x("PrinterStatus","hwnat"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
		
		<td valign="top" width="202">				
		<div  id="mainMenu"></div>	
		<div  id="subMenu"></div>		
		</td>				
		
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<br />
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_1#> - <#menu5_1_6#> (5GHz)</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#WLANConfig11b_display5_sectiondesc#></td>
	</tr>
	</tbody>	
	<tr>
	  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" id="WAdvTable">
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 1);"><#WLANConfig11b_x_RadioEnable_itemname#></a></th>
			  <td>
				<input type="radio" value="1" name="wl_radio_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_radio_x', '1')" <% nvram_match_x("WLANConfig11b","wl_radio_x", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="wl_radio_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_radio_x', '0')" <% nvram_match_x("WLANConfig11b","wl_radio_x", "0", "checked"); %>/><#checkbox_No#>
			  </td>
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 2);"><#WLANConfig11b_x_RadioEnableDate_itemname#></a></th>
			  <td>
				<input type="checkbox" class="input" name="wl_radio_date_x_Sun" onChange="return changeDate();"/>Sun
				<input type="checkbox" class="input" name="wl_radio_date_x_Mon" onChange="return changeDate();"/>Mon
				<input type="checkbox" class="input" name="wl_radio_date_x_Tue" onChange="return changeDate();"/>Tue
				<input type="checkbox" class="input" name="wl_radio_date_x_Wed" onChange="return changeDate();"/>Wed
				<input type="checkbox" class="input" name="wl_radio_date_x_Thu" onChange="return changeDate();"/>Thu
				<input type="checkbox" class="input" name="wl_radio_date_x_Fri" onChange="return changeDate();"/>Fri
				<input type="checkbox" class="input" name="wl_radio_date_x_Sat" onChange="return changeDate();"/>Sat
			  </td>
			</tr>
			<tr>
			  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 3);"><#WLANConfig11b_x_RadioEnableTime_itemname#></a></th>
			  <td>
				<input type="text" maxlength="2" class="input" size="2" name="wl_radio_time_x_starthour" onKeyPress="return is_number(this)"/>:
				<input type="text" maxlength="2" class="input" size="2" name="wl_radio_time_x_startmin" onKeyPress="return is_number(this)"/>-
				<input type="text" maxlength="2" class="input" size="2" name="wl_radio_time_x_endhour" onKeyPress="return is_number(this)"/>:
				<input type="text" maxlength="2" class="input" size="2" name="wl_radio_time_x_endmin" onKeyPress="return is_number(this)"/>
			  </td>
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
			  <td>
				<input type="radio" value="1" name="wl_ap_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_ap_isolate", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="wl_ap_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_ap_isolate", "0", "checked"); %>/><#checkbox_No#>
			  </td>
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 5);"><#WIFIGuestIsolate#></a></th>
			  <td>
				<input type="radio" value="1" name="wl_mbssid_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="wl_mbssid_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "0", "checked"); %>/><#checkbox_No#>
			  </td>
			</tr>
			<tr id="wl_rate">
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 6);"><#WLANConfig11b_DataRateAll_itemname#></a></th>
			  <td>
				<select name="wl_rate" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_rate')">
					 <option value="0" <% nvram_match_x("WLANConfig11b","wl_rate", "0","selected"); %>>Auto</option>
				</select>
			  </td>
			</tr>
			<tr>
			    <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
			    <td>
				<select name="wl_mcastrate" class="input" onClick="openHint(3, 7);">
					<option value="0" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5~15 Mbps</option>
					<option value="1" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 13~30 Mbps</option>
					<option value="2" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5~45 Mbps</option>
					<option value="3" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 13~30 Mbps</option>
					<option value="4" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 26~60 Mbps</option>
					<option value="5" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
					<option value="6" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
					<option value="7" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
					<option value="8" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
				</select>
			    </td>
			</tr>
			<!-- 2008.03 James. patch for Oleg's patch. } -->
			
			<!--tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 8);"><#WLANConfig11b_DataRate_itemname#></a></th>
			  <td>
			  	<select name="wl_rateset" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_rateset')">
				  <option value="default" <% nvram_match_x("WLANConfig11b","wl_rateset", "default","selected"); %>>Default</option>
				  <option value="all" <% nvram_match_x("WLANConfig11b","wl_rateset", "all","selected"); %>>All</option>
				  <option value="12" <% nvram_match_x("WLANConfig11b","wl_rateset", "12","selected"); %>>1, 2 Mbps</option>
				</select></td>
			</tr-->
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 9);"><#WLANConfig11b_x_Frag_itemname#></a></th>
			  	<td>
			  		<input type="text" maxlength="5" size="5" name="wl_frag" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_frag"); %>" onKeyPress="return is_number(this)" onChange="page_changed()" onBlur="validate_range(this, 256, 2346)">
				</td>
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 10);"><#WLANConfig11b_x_RTS_itemname#></a></th>
			  	<td>
			  		<input type="text" maxlength="5" size="5" name="wl_rts" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_rts"); %>" onKeyPress="return is_number(this)">
			  	</td>
			</tr>
			<tr>
			  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 11);"><#WLANConfig11b_x_DTIM_itemname#></a></th>
				<td>
			  		<input type="text" maxlength="5" size="5" name="wl_dtim" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_dtim"); %>" onKeyPress="return is_number(this)"  onBlur="validate_range(this, 1, 255)">
				</td>
			  
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 12);"><#WLANConfig11b_x_Beacon_itemname#></a></th>
				<td>
					<input type="text" maxlength="5" size="5" name="wl_bcn" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_bcn"); %>" onKeyPress="return is_number(this)" onBlur="validate_range(this, 20, 1000)">
				</td>
			</tr>
			<tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 13);"><#WLANConfig11b_x_TxBurst_itemname#></a></th>
				<td>
					<select name="TxBurst" class="input" onChange="return change_common(this, 'WLANConfig11b', 'TxBurst')">
						<option value="0" <% nvram_match_x("WLANConfig11b","TxBurst", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						<option value="1" <% nvram_match_x("WLANConfig11b","TxBurst", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
					</select>
				</td>
			</tr>			
			<tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 16);"><#WLANConfig11b_x_PktAggregate_itemname#></a></th>
				<td>
					<select name="PktAggregate" class="input" onChange="return change_common(this, 'WLANConfig11b', 'PktAggregate')">
						<option value="0" <% nvram_match_x("WLANConfig11b","PktAggregate", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						<option value="1" <% nvram_match_x("WLANConfig11b","PktAggregate", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
					</select>
				</td>
			</tr>
			<!--Greenfield by Lock Add in 2008.10.01 -->
			<tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 19);"><#WLANConfig11b_x_HT_OpMode_itemname#></a></th>
				<td>
					<select class="input" id="HT_OpMode" name="HT_OpMode" onChange="return change_common(this, 'WLANConfig11b', 'HT_OpMode')">
						<option value="0" <% nvram_match_x("WLANConfig11b","HT_OpMode", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						<option value="1" <% nvram_match_x("WLANConfig11b","HT_OpMode", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
					</select>
				</td>
			</tr>
			<!--Greenfield by Lock Add in 2008.10.01 -->
			<tr id="wl_wme_tr">
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 14);"><#WLANConfig11b_x_WMM_itemname#></a></th>
			  <td>
				<select name="wl_wme" id="wl_wme" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme')">
			  	  <option value="0" <% nvram_match_x("WLANConfig11b", "wl_wme", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
			  	  <option value="1" <% nvram_match_x("WLANConfig11b", "wl_wme", "1", "selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
				</select>
			  </td>
			</tr>
			<tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,15);"><#WLANConfig11b_x_NOACK_itemname#></a></th>
			  <td>
				<select name="wl_wme_no_ack" id="wl_wme_no_ack" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme_no_ack')">
			  	  <option value="off" <% nvram_match_x("WLANConfig11b","wl_wme_no_ack", "off","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
			  	  <option value="on" <% nvram_match_x("WLANConfig11b","wl_wme_no_ack", "on","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
				</select>
			  </td>
			</tr>
			<tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,17);"><#WLANConfig11b_x_APSD_itemname#></a></th>
				<td>
                  <select name="APSDCapable" class="input" onchange="return change_common(this, 'WLANConfig11b', 'APSDCapable')">
                    <option value="0" <% nvram_match_x("WLANConfig11b","APSDCapable", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                    <option value="1" <% nvram_match_x("WLANConfig11b","APSDCapable", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
                  </select>
				</td>
			</tr>
			<tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,18);"><#WLANConfig11b_x_DLS_itemname#></a></th>
				<td>
					<select name="DLSCapable" class="input" onChange="return change_common(this, 'WLANConfig11b', 'DLSCapable')">
						<option value="0" <% nvram_match_x("WLANConfig11b","DLSCapable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						<option value="1" <% nvram_match_x("WLANConfig11b","DLSCapable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
					</select>
				</td>
			</tr>
			<tr align="right">
				<td colspan="2">
					<input type="button" class="button2" value="<#GO_2G#>" onclick="location.href='Advanced_WAdvanced2g_Content.asp';">    
					<input class="button" onclick="applyRule();" type="button" value="<#CTL_apply#>"/></td>
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
	  <div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
		<img src="images/help.gif">
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:closeHint();"><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tbody>
		  <tr>
			<td valign="top">
			  <div id="hint_body" class="hint_body2"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
		  </tbody>
		</table>
	  </div>
	</td>
	<!--==============Ending of hint content=============-->
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

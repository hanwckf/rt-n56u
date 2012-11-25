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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_6#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script type="text/javaScript" src="/jquery.js"></script>
<script>
var $j = jQuery.noConflict();

<% login_state_hook(); %>
<% board_caps_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,6,2);
	show_footer();
	
	if (!support_but_wps()){
		$('tbl_wps_actions').style.display="none";
	}
	
	if (!support_led_all()){
		document.form.front_leds.remove(4);
		document.form.front_leds.remove(3);
	}
	
	if (support_led_phy() < 2){
		$('row_eth_phy_led1').style.display="none";
		if (support_led_phy() < 1){
			$('row_eth_phy_led0').style.display="none";
		}
	}
	
	change_ez_short(document.form.ez_action_short.value);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Tweaks_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_ez_short(ez_short){
	if(ez_short == "0"){
		inputCtrl(document.form.ez_action_long, 0);
		document.form.ez_action_long.value = "0";
	}
	else{
		inputCtrl(document.form.ez_action_long, 1);
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="Advanced_Tweaks_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;General;">
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
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_6#> - <#menu5_6_6#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"></td>
	</tr>
	</tbody>

	<tr>
	 <td bgcolor="#FFFFFF">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="tbl_wps_actions">
	<thead>
	<tr>
	    <td colspan="2"><#TweaksWPSAction#></td>
	</tr>
	</thead>
	<tr>
	    <th width="40%"><#TweaksWPSEventShort#></th>
	    <td align="left">
		<select name="ez_action_short" class="input" onchange="change_ez_short(this.value);">
			<option value="0" <% nvram_match_x("General", "ez_action_short", "0","selected"); %>>Nothing</option>
			<option value="1" <% nvram_match_x("General", "ez_action_short", "1","selected"); %>>WiFi radio On/Off trigger</option>
			<option value="2" <% nvram_match_x("General", "ez_action_short", "2","selected"); %>>WiFi 2.4GHz force On/Off trigger</option>
			<option value="3" <% nvram_match_x("General", "ez_action_short", "3","selected"); %>>WiFi 5GHz force On/Off trigger</option>
			<option value="4" <% nvram_match_x("General", "ez_action_short", "4","selected"); %>>WiFi 2.4 and 5GHz force On/Off trigger</option>
			<option value="5" <% nvram_match_x("General", "ez_action_short", "5","selected"); %>>Safe removal all USB</option>
			<option value="6" <% nvram_match_x("General", "ez_action_short", "6","selected"); %>>WAN down</option>
			<option value="7" <% nvram_match_x("General", "ez_action_short", "7","selected"); %>>WAN reconnect</option>
			<option value="8" <% nvram_match_x("General", "ez_action_short", "8","selected"); %>>WAN up/down toggle</option>
			<option value="10" <% nvram_match_x("General", "ez_action_short", "10","selected"); %>>Front LED On/Off trigger</option>
			<option value="9" <% nvram_match_x("General", "ez_action_short", "9","selected"); %>>Run user script (/opt/bin/on_wps.sh 1)</option>
		</select>
	    </td>
	</tr>
	<tr>
	    <th><#TweaksWPSEventLong#></th>
	    <td align="left">
		<select name="ez_action_long" class="input">
			<option value="0" <% nvram_match_x("General", "ez_action_long", "0","selected"); %>>Nothing</option>
			<option value="1" <% nvram_match_x("General", "ez_action_long", "1","selected"); %>>WiFi 2.4GHz force On/Off trigger</option>
			<option value="2" <% nvram_match_x("General", "ez_action_long", "2","selected"); %>>WiFi 5GHz force On/Off trigger</option>
			<option value="3" <% nvram_match_x("General", "ez_action_long", "3","selected"); %>>WiFi 2.4 and 5GHz force On/Off trigger</option>
			<option value="4" <% nvram_match_x("General", "ez_action_long", "4","selected"); %>>Safe removal all USB</option>
			<option value="5" <% nvram_match_x("General", "ez_action_long", "5","selected"); %>>WAN down</option>
			<option value="6" <% nvram_match_x("General", "ez_action_long", "6","selected"); %>>WAN reconnect</option>
			<option value="9" <% nvram_match_x("General", "ez_action_long", "9","selected"); %>>WAN up/down toggle</option>
			<option value="11" <% nvram_match_x("General", "ez_action_long", "11","selected"); %>>Front LED On/Off trigger</option>
			<option value="7" <% nvram_match_x("General", "ez_action_long", "7","selected"); %>>Router reboot</option>
			<option value="8" <% nvram_match_x("General", "ez_action_long", "8","selected"); %>>Router shutdown (prepare)</option>
			<option value="10" <% nvram_match_x("General", "ez_action_long", "10","selected"); %>>Run user script (/opt/bin/on_wps.sh 2)</option>
		</select>
	    </td>
	</tr>
	</table>
	</td>
	</tr>

	<tr>
	 <td bgcolor="#FFFFFF">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	<thead>
	<tr>
	    <td colspan="2"><#TweaksLEDEvents#></td>
	</tr>
	</thead>
	<tr>
	    <th width="40%"><#TweaksLEDFront#></th>
	    <td align="left">
		<select name="front_leds" class="input">
			<option value="0" <% nvram_match_x("General", "front_leds", "0","selected"); %>>All LED</option>
			<option value="1" <% nvram_match_x("General", "front_leds", "1","selected"); %>>WiFi and Power LED</option>
			<option value="2" <% nvram_match_x("General", "front_leds", "2","selected"); %>>WiFi LED only</option>
			<option value="3" <% nvram_match_x("General", "front_leds", "3","selected"); %>>Power LED only</option>
			<option value="4" <% nvram_match_x("General", "front_leds", "4","selected"); %>>Hide All</option>
		</select>
	    </td>
	</tr>
	<tr id="row_eth_phy_led0">
	    <th><#TweaksLEDEth0#></th>
	    <td align="left">
		<select name="ether_led0" class="input">
			<option value="0" <% nvram_match_x("LANHostConfig","ether_led0", "0","selected"); %>>Link 1000 Mbps, TX/RX activity</option>
			<option value="1" <% nvram_match_x("LANHostConfig","ether_led0", "1","selected"); %>>Link 100 Mbps, TX/RX activity</option>
			<option value="2" <% nvram_match_x("LANHostConfig","ether_led0", "2","selected"); %>>Link 10 Mbps, TX/RX activity</option>
			<option value="3" <% nvram_match_x("LANHostConfig","ether_led0", "3","selected"); %>>Link 100/10 Mbps, TX/RX activity</option>
			<option value="4" <% nvram_match_x("LANHostConfig","ether_led0", "4","selected"); %>>Link 1000 Mbps</option>
			<option value="5" <% nvram_match_x("LANHostConfig","ether_led0", "5","selected"); %>>Link 100 Mbps</option>
			<option value="6" <% nvram_match_x("LANHostConfig","ether_led0", "6","selected"); %>>Link 10 Mbps</option>
			<option value="7" <% nvram_match_x("LANHostConfig","ether_led0", "7","selected"); %>>Link, TX/RX activity</option>
			<option value="8" <% nvram_match_x("LANHostConfig","ether_led0", "8","selected"); %>>Link, RX activity</option>
			<option value="9" <% nvram_match_x("LANHostConfig","ether_led0", "9","selected"); %>>Link, TX activity</option>
			<option value="10" <% nvram_match_x("LANHostConfig","ether_led0", "10","selected"); %>>Duplex, Collision</option>
			<option value="11" <% nvram_match_x("LANHostConfig","ether_led0", "11","selected"); %>>LED OFF</option>
		</select>
	    </td>
	</tr>
	<tr id="row_eth_phy_led1">
	    <th><#TweaksLEDEth1#></th>
	    <td align="left">
		<select name="ether_led1" class="input">
			<option value="0" <% nvram_match_x("LANHostConfig","ether_led1", "0","selected"); %>>Link 1000 Mbps, TX/RX activity</option>
			<option value="1" <% nvram_match_x("LANHostConfig","ether_led1", "1","selected"); %>>Link 100 Mbps, TX/RX activity</option>
			<option value="2" <% nvram_match_x("LANHostConfig","ether_led1", "2","selected"); %>>Link 10 Mbps, TX/RX activity</option>
			<option value="3" <% nvram_match_x("LANHostConfig","ether_led1", "3","selected"); %>>Link 100/10 Mbps, TX/RX activity</option>
			<option value="4" <% nvram_match_x("LANHostConfig","ether_led1", "4","selected"); %>>Link 1000 Mbps</option>
			<option value="5" <% nvram_match_x("LANHostConfig","ether_led1", "5","selected"); %>>Link 100 Mbps</option>
			<option value="6" <% nvram_match_x("LANHostConfig","ether_led1", "6","selected"); %>>Link 10 Mbps</option>
			<option value="7" <% nvram_match_x("LANHostConfig","ether_led1", "7","selected"); %>>Link, TX/RX activity</option>
			<option value="8" <% nvram_match_x("LANHostConfig","ether_led1", "8","selected"); %>>Link, RX activity</option>
			<option value="9" <% nvram_match_x("LANHostConfig","ether_led1", "9","selected"); %>>Link, TX activity</option>
			<option value="10" <% nvram_match_x("LANHostConfig","ether_led1", "10","selected"); %>>Duplex, Collision</option>
			<option value="11" <% nvram_match_x("LANHostConfig","ether_led1", "11","selected"); %>>LED OFF</option>
		</select>
	    </td>
	</tr>
	</table>
	</td>
	</tr>
	<tr>
	    <td bgcolor="#FFFFFF" colspan="2" align="right"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>
	</tr>
</table></td>
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

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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_5#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>

<% lanlink(); %>

<% board_caps_hook(); %>

function initial(){
	final_flag = 1;	// for the function in general.js
	
	show_banner(1);
	
	if(sw_mode == "3")
		show_menu(5,2,2);
	else
		show_menu(5,2,5);
	
	if (!support_switch_igmp() || sw_mode != "3"){
		$('row_igmp_snoop').style.display="none";
	}
	
	show_footer();
	
	enable_auto_hint(4, 2);
	
	$("linkstate_wan").innerHTML  = lanlink_etherlink_wan();
	$("linkstate_lan1").innerHTML = lanlink_etherlink_lan1();
	$("linkstate_lan2").innerHTML = lanlink_etherlink_lan2();
	$("linkstate_lan3").innerHTML = lanlink_etherlink_lan3();
	$("linkstate_lan4").innerHTML = lanlink_etherlink_lan4();
}

function applyRule(){
	showLoading();
	
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "Advanced_Switch_Content.asp";
	document.form.next_page.value = "";
	
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(4, 2);return unload_body();">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/DrsurfImg.gif"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Switch_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
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
		    <div id="tabMenu" class="submenuBlock"></div>
		<br>
		<!--===================================Beginning of Main Content===========================================-->
	<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
		
	<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_2#> - <#menu5_2_5#></td>
	</tr>
	</thead>
	<tbody>
	  <tr>
	    <td bgcolor="#FFFFFF"><#Switch_desc#></td>
	  </tr>
	</tbody>
	<tr>
	    <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<thead>
			<tr>
				<td colspan="2"><#SwitchBase#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchJumbo#></th>
				<td align="left">
					<select name="ether_jumbo" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_jumbo", "0","selected"); %>>Up to 1536 bytes</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_jumbo", "1","selected"); %>>Up to 16000 bytes</option>
					</select>
				</td>
			</tr>
			<tr>
				<th><#SwitchGreen#></th>
				<td style="font-weight:normal;" align="left">
					<input type="radio" value="1" name="ether_green" class="input" <% nvram_match_x("LANHostConfig", "ether_green", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="ether_green" class="input" <% nvram_match_x("LANHostConfig", "ether_green", "0", "checked"); %> /><#checkbox_No#>
				</td>
			</tr>
			<tr id="row_igmp_snoop">
				<th><#SwitchIgmp#></th>
				<td style="font-weight:normal;" align="left">
					<input type="radio" value="1" name="ether_igmp" class="input" <% nvram_match_x("LANHostConfig", "ether_igmp", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="ether_igmp" class="input" <% nvram_match_x("LANHostConfig", "ether_igmp", "0", "checked"); %> /><#checkbox_No#>
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
				<td colspan="2">WAN</td>
				<td><#SwitchState#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchFlow#></th>
				<td width="30%" align="left">
					<select name="ether_flow_wan" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_flow_wan", "0","selected"); %>>RX/TX</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_flow_wan", "1","selected"); %>>RX (Asymmetric Pause)</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_flow_wan", "2","selected"); %>>Disabled</option>
					</select>
				</td>
				<td rowspan="2" align="left" id="linkstate_wan"></td>
			</tr>
			<tr>
				<th width="40%"><#SwitchLink#></th>
				<td width="30%" align="left">
					<select name="ether_link_wan" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_link_wan", "0","selected"); %>>Auto</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_link_wan", "1","selected"); %>>1000 Mbps, Full Duplex</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_link_wan", "2","selected"); %>>100 Mbps, Full Duplex</option>
						<option value="3" <% nvram_match_x("LANHostConfig","ether_link_wan", "3","selected"); %>>100 Mbps, Half Duplex</option>
						<option value="4" <% nvram_match_x("LANHostConfig","ether_link_wan", "4","selected"); %>>10 Mbps, Full Duplex</option>
						<option value="5" <% nvram_match_x("LANHostConfig","ether_link_wan", "5","selected"); %>>10 Mbps, Half Duplex</option>
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
				<td colspan="2">LAN 1</td>
				<td><#SwitchState#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchFlow#></th>
				<td width="30%" align="left">
					<select name="ether_flow_lan1" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "0","selected"); %>>RX/TX</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "1","selected"); %>>RX (Asymmetric Pause)</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "2","selected"); %>>Disabled</option>
					</select>
				</td>
				<td rowspan="2" align="left" id="linkstate_lan1"></td>
			</tr>
			<tr>
				<th width="40%"><#SwitchLink#></th>
				<td width="30%" align="left">
					<select name="ether_link_lan1" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan1", "0","selected"); %>>Auto</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan1", "1","selected"); %>>1000 Mbps, Full Duplex</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan1", "2","selected"); %>>100 Mbps, Full Duplex</option>
						<option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan1", "3","selected"); %>>100 Mbps, Half Duplex</option>
						<option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan1", "4","selected"); %>>10 Mbps, Full Duplex</option>
						<option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan1", "5","selected"); %>>10 Mbps, Half Duplex</option>
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
				<td colspan="2">LAN 2</td>
				<td><#SwitchState#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchFlow#></th>
				<td width="30%" align="left">
					<select name="ether_flow_lan2" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "0","selected"); %>>RX/TX</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "1","selected"); %>>RX (Asymmetric Pause)</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "2","selected"); %>>Disabled</option>
					</select>
				</td>
				<td rowspan="2" align="left" id="linkstate_lan2"></td>
			</tr>
			<tr>
				<th width="40%"><#SwitchLink#></th>
				<td width="30%" align="left">
					<select name="ether_link_lan2" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan2", "0","selected"); %>>Auto</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan2", "1","selected"); %>>1000 Mbps, Full Duplex</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan2", "2","selected"); %>>100 Mbps, Full Duplex</option>
						<option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan2", "3","selected"); %>>100 Mbps, Half Duplex</option>
						<option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan2", "4","selected"); %>>10 Mbps, Full Duplex</option>
						<option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan2", "5","selected"); %>>10 Mbps, Half Duplex</option>
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
				<td colspan="2">LAN 3</td>
				<td><#SwitchState#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchFlow#></th>
				<td width="30%" align="left">
					<select name="ether_flow_lan3" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "0","selected"); %>>RX/TX</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "1","selected"); %>>RX (Asymmetric Pause)</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "2","selected"); %>>Disabled</option>
					</select>
				</td>
				<td rowspan="2" align="left" id="linkstate_lan3"></td>
			</tr>
			<tr>
				<th width="40%"><#SwitchLink#></th>
				<td width="30%" align="left">
					<select name="ether_link_lan3" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan3", "0","selected"); %>>Auto</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan3", "1","selected"); %>>1000 Mbps, Full Duplex</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan3", "2","selected"); %>>100 Mbps, Full Duplex</option>
						<option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan3", "3","selected"); %>>100 Mbps, Half Duplex</option>
						<option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan3", "4","selected"); %>>10 Mbps, Full Duplex</option>
						<option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan3", "5","selected"); %>>10 Mbps, Half Duplex</option>
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
				<td colspan="2">LAN 4</td>
				<td><#SwitchState#></td>
			</tr>
			</thead>
			<tr>
				<th width="40%"><#SwitchFlow#></th>
				<td width="30%" align="left">
					<select name="ether_flow_lan4" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "0","selected"); %>>RX/TX</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "1","selected"); %>>RX (Asymmetric Pause)</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "2","selected"); %>>Disabled</option>
					</select>
				</td>
				<td rowspan="2" align="left" id="linkstate_lan4"></td>
			</tr>
			<tr>
				<th width="40%"><#SwitchLink#></th>
				<td width="30%" align="left">
					<select name="ether_link_lan4" class="input">
						<option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan4", "0","selected"); %>>Auto</option>
						<option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan4", "1","selected"); %>>1000 Mbps, Full Duplex</option>
						<option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan4", "2","selected"); %>>100 Mbps, Full Duplex</option>
						<option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan4", "3","selected"); %>>100 Mbps, Half Duplex</option>
						<option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan4", "4","selected"); %>>10 Mbps, Full Duplex</option>
						<option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan4", "5","selected"); %>>10 Mbps, Half Duplex</option>
					</select>
				</td>
			</tr>
		</table>
	    </td>
	</tr>
	<tr bgcolor="#FFFFFF" align="right">
		<td><input class="button" onclick="applyRule()" type="button" value="<#CTL_apply#>"/></td>
	</tr>
    </table>
    </td>
					
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
</form>

<div id="footer"></div>
</body>
</html>

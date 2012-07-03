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
<title>Wireless Router <#Web_Title#> - <#NAT_passthrough_itemname#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(1);
	show_menu(5,3,6);
	show_footer();
}

function applyRule(){
			showLoading();
	
			document.form.action_mode.value = " Apply ";
			document.form.current_page.value = "/Advanced_NATPassThrough_Content.asp";
			document.form.next_page.value = "";
			document.form.submit();	
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>		
		<td valign="top" width="202">				
			<div id="mainMenu"></div>	
			<div id="subMenu"></div>		
		</td>				
				
    <td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
		<br />

			<!--===================================Beginning of Main Content===========================================-->
			<input type="hidden" name="current_page" value="Advanced_NATPassThrough_Content.asp">
			<input type="hidden" name="next_page" value="">
			<input type="hidden" name="next_host" value="">
			<input type="hidden" name="sid_list" value="IPConnection;PPPConnection;">
			<input type="hidden" name="group_id" value="">
			<input type="hidden" name="modified" value="0">
			<input type="hidden" name="action_mode" value="">			
			<input type="hidden" name="first_time" value="">
			<input type="hidden" name="action_script" value="">
			<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
			<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
			<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
						<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
						
						<thead>
							<tr>
								<td><#menu5_3#> - <#NAT_passthrough_itemname#></td>
							</tr>
						</thead>							
						
						<tbody bgcolor="#FFFFFF">
							<tr>
								<td><#NAT_passthrough_desc#></td>
							</tr>
							<tr>
							  <td>
								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										<tr>
          						<th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#NAT_PPTP_Passthrough#></a></th>
          						<td>
								<select name="fw_pt_pptp" class="input" onChange="return change_common(this, 'IPConnection','fw_pt_pptp')">
									<option value="1" <% nvram_match_x("IPConnection","fw_pt_pptp", "1","selected"); %>><#CTL_Enabled#></option>
									<option value="0" <% nvram_match_x("IPConnection","fw_pt_pptp", "0","selected"); %>><#CTL_Disabled#></option>	
								</select>
          						</td>
          					</tr>
          					<tr>
          						<th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,24);"><#NAT_L2TP_Passthrough#></a></th>
          						<td>
								<select name="fw_pt_l2tp" class="input" onChange="return change_common(this, 'IPConnection','fw_pt_l2tp')">
									<option value="1" <% nvram_match_x("IPConnection","fw_pt_l2tp", "1","selected"); %>><#CTL_Enabled#></option>
									<option value="0" <% nvram_match_x("IPConnection","fw_pt_l2tp", "0","selected"); %>><#CTL_Disabled#></option>	
								</select>
          						</td>
          					</tr>
          					<tr>
          						<th width="40%"  align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,25);"><#NAT_IPSec_Passthrough#></a></th>
          						<td>
								<select name="fw_pt_ipsec" class="input" onChange="return change_common(this, 'IPConnection','fw_pt_ipsec')">
									<option value="1" <% nvram_match_x("IPConnection","fw_pt_ipsec", "1","selected"); %>><#CTL_Enabled#></option>
									<option value="0" <% nvram_match_x("IPConnection","fw_pt_ipsec", "0","selected"); %>><#CTL_Disabled#></option>	
								</select>
          						</td>
          					</tr>
						<tr>
							<th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
          						<td>
								<select name="wan_pppoe_relay_x" class="input" onChange="return change_common(this, 'PPPConnection','wan_pppoe_relay_x')">
									<option value="1" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "1","selected"); %>><#CTL_Enabled#></option>
									<option value="0" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "0","selected"); %>><#CTL_Disabled#></option>
								</select>
          						</td>
						</tr>
						<!--tr>
  	         					<th><#NAT_RTSP_Passthrough#></th>
    	       					<td>
												<select name="fw_pt_rtsp" class="input">
													<option class="content_input_fd" value="0" <% nvram_match_x("",  "fw_pt_rtsp", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1" <% nvram_match_x("",  "fw_pt_rtsp", "1","selected"); %>><#btn_Enable#></option>
												</select>			
        	    				</td>
           					</tr-->
										<!--tr>
  	         					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
    	       					<td>
												<select name="fw_pt_pppoerelay" class="input">
													<option class="content_input_fd" value="0" <% nvram_match_x("",  "fw_pt_pppoerelay", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1" <% nvram_match_x("",  "fw_pt_pppoerelay", "1","selected"); %>><#btn_Enable#></option>
												</select>			
        	    				</td>
           					</tr-->
										<tr align="right">	
											<td colspan="2">
												<input type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>"/>
											</td>
										</tr>           					
								</table>
								</td>
							</tr>						
					</tbody>
					</table>
				</td>
				
			<td id="help_td" style="width:15px;" align="right" valign="top">		  
	  		<div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
	  		<div id="hintofPM" style="display:none;">
				<form name="hint_form"></form>
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
	  			</div><!--End of hintofPM-->	
				</td>				
				
			</tr>
		</table>
	</td>	
</form>	
	
		
  <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>

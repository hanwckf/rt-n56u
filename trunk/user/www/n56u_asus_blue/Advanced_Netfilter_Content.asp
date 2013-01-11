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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_5#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>

<% nf_values(); %>

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,5,2);
	show_footer();
	enable_auto_hint(8, 6);
	
	load_body();
	
	if (sw_mode == "4"){
		$("row_nat_loop").style.display = "none";
	}
	else{
		$("row_nat_loop").style.display = "";
	}
	
	$("nf_count").innerHTML = '  (' + nf_conntrack_count() + ' in use)';
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Netfilter_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if (document.form.nf_alg_ftp0.value!="")
		if(!validate_range(document.form.nf_alg_ftp0, 21, 65535))
			return false;

	if (document.form.nf_alg_ftp1.value!="")
		if(!validate_range(document.form.nf_alg_ftp1, 1024, 65535))
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

<input type="hidden" name="current_page" value="Advanced_Netfilter_Content.asp">
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
		<td><#menu5_5#> - <#menu5_5_5#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
	  <td bgcolor="#FFFFFF"><#NFilter_desc#></td>
	  </tr>
	</tbody>

	<tr>
	  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
	  <thead>
	  <tr>
		<td colspan="2"><#NFilterConfig#></td>
	  </tr>
	  </thead>
          <tr>
            <th width="40%" align="right"><#NFilterMaxConn#></th>
            <td>
              <select name="nf_max_conn" class="input">
                <option value="8192" <% nvram_match_x("FirewallConfig","nf_max_conn", "8192", "selected"); %>>8192</option>
                <option value="16384" <% nvram_match_x("FirewallConfig","nf_max_conn", "16384", "selected"); %>>16384 (HW_NAT FoE Max)</option>
                <option value="32768" <% nvram_match_x("FirewallConfig","nf_max_conn", "32768", "selected"); %>>32768</option>
                <option value="65536" <% nvram_match_x("FirewallConfig","nf_max_conn", "65536","selected"); %>>65536</option>
                <option value="131072" <% nvram_match_x("FirewallConfig","nf_max_conn", "131072", "selected"); %>>131072 (Slow)</option>
                <option value="262144" <% nvram_match_x("FirewallConfig","nf_max_conn", "262144", "selected"); %>>262144 (Very Slow)</option>
              </select>
              <span style="color:#A55" id="nf_count"></span>
            </td>
          </tr>
          <tr>
            <th width="40%" align="right"><#NFilterNatType#></th>
            <td>
              <select name="nf_nat_type" class="input">
                <option value="2" <% nvram_match_x("FirewallConfig","nf_nat_type", "2", "selected"); %>>Classical Linux Hybrid NAT</option>
                <option value="0" <% nvram_match_x("FirewallConfig","nf_nat_type", "0", "selected"); %>>Restricted Cone NAT</option>
                <option value="1" <% nvram_match_x("FirewallConfig","nf_nat_type", "1", "selected"); %>>Full Cone NAT</option>
              </select>
            </td>
          </tr>
	<tr id="row_nat_loop">
	  <th align="right">NAT loopback?</th>
	  <td>
	    <input type="radio" name="nf_nat_loop" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_nat_loop", "1", "checked"); %>/><#checkbox_Yes#>
	    <input type="radio" name="nf_nat_loop" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_nat_loop", "0", "checked"); %>/><#checkbox_No#>
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
			<td colspan="2"><#vpn_passthrough_itemname#></td>
		</tr>
		</thead>
		<tr>
			<th width="40%" align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#VPN_PPTP_Passthrough#></a></th>
			<td>
				<input type="radio" name="fw_pt_pptp" class="input" value="1" <% nvram_match_x("", "fw_pt_pptp", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="fw_pt_pptp" class="input" value="0" <% nvram_match_x("", "fw_pt_pptp", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,24);"><#VPN_L2TP_Passthrough#></a></th>
			<td>
				<input type="radio" name="fw_pt_l2tp" class="input" value="1" <% nvram_match_x("", "fw_pt_l2tp", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="fw_pt_l2tp" class="input" value="0" <% nvram_match_x("", "fw_pt_l2tp", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,25);"><#VPN_IPSec_Passthrough#></a></th>
			<td>
				<input type="radio" name="fw_pt_ipsec" class="input" value="1" <% nvram_match_x("", "fw_pt_ipsec", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="fw_pt_ipsec" class="input" value="0" <% nvram_match_x("", "fw_pt_ipsec", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
			<td>
				<input type="radio" name="fw_pt_pppoe" class="input" value="1" <% nvram_match_x("", "fw_pt_pppoe", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="fw_pt_pppoe" class="input" value="0" <% nvram_match_x("", "fw_pt_pppoe", "0", "checked"); %>/><#checkbox_No#>
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
			<td colspan="2">Application-Level Gateway (ALG)</td>
		</tr>
		</thead>
		<tr>
			<th width="40%" align="right">FTP ALG (ports)</th>
			<td>
				<input type="text" maxlength="5" size="5" name="nf_alg_ftp0" class="input" value="<% nvram_get_x("", "nf_alg_ftp0"); %>" onkeypress="return is_number(this)"/>&nbsp;,&nbsp;
				<input type="text" maxlength="5" size="5" name="nf_alg_ftp1" class="input" value="<% nvram_get_x("", "nf_alg_ftp1"); %>" onkeypress="return is_number(this)"/>
			</td>
		</tr>
		<tr>
			<th align="right">PPTP ALG</th>
			<td>
				<input type="radio" name="nf_alg_pptp" class="input" value="1" <% nvram_match_x("", "nf_alg_pptp", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="nf_alg_pptp" class="input" value="0" <% nvram_match_x("", "nf_alg_pptp", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right">H.323 ALG</th>
			<td>
				<input type="radio" name="nf_alg_h323" class="input" value="1" <% nvram_match_x("", "nf_alg_h323", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="nf_alg_h323" class="input" value="0" <% nvram_match_x("", "nf_alg_h323", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right">RTSP ALG</th>
			<td>
				<input type="radio" name="nf_alg_rtsp" class="input" value="1" <% nvram_match_x("", "nf_alg_rtsp", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="nf_alg_rtsp" class="input" value="0" <% nvram_match_x("", "nf_alg_rtsp", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th align="right">SIP ALG</th>
			<td>
				<input type="radio" name="nf_alg_sip" class="input" value="1" <% nvram_match_x("", "nf_alg_sip", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="nf_alg_sip" class="input" value="0" <% nvram_match_x("", "nf_alg_sip", "0", "checked"); %>/><#checkbox_No#>
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

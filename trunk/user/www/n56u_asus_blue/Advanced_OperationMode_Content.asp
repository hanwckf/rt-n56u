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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_1_title#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

var auth_mode = '<% nvram_get_x("", "wl_auth_mode"); %>';
var wep_x = '<% nvram_get_x("", "wl_wep_x"); %>';
var auth_mode2 = '<% nvram_get_x("", "rt_auth_mode"); %>';
var wep_x2 = '<% nvram_get_x("", "rt_wep_x"); %>';
var sw_mode = '<% nvram_get_x("", "sw_mode"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(2);
	
	show_menu(5,6,3);
	
	show_footer();

	if(sw_mode=="1" || sw_mode=="3"){
		$('wl_rt').style.display="";
		$('rt_wo_nat').style.display="none";
	}else if(sw_mode=="4"){
		$('wl_rt').style.display="none";
		$('rt_wo_nat').style.display="";
	}
	
	setScenerion(sw_mode);
}

function saveMode(){
	
	if(sw_mode == "1"){ 
		if(document.form.sw_mode[0].checked == true){
			alert("<#op_already_configured#>");
			return false;
		}
	}else if(sw_mode == "4"){ 
		if(document.form.sw_mode[1].checked == true){
			alert("<#op_already_configured#>");
			return false;
		}
	}else if(sw_mode == "3"){
		if(document.form.sw_mode[2].checked == true){
			alert("<#op_already_configured#>");
			return false;
		}
	}
	
	document.form.target="hidden_frame";
	document.form.current_page.value = "Advanced_OperationMode_Content.asp";
	document.form.action_mode.value = " Apply ";
	
	if(document.form.sw_mode[0].checked == true || document.form.sw_mode[1].checked == true){
		document.form.action="/start_apply.htm";
	}else{
		document.form.flag.value = 'ap_mode_AOC';
		document.form.action="/start_apply2.htm";
	}
	
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

var $j = jQuery.noConflict();
var id_WANunplungHint;

function setScenerion(mode){
	if(mode == '1' || mode == '4'){
		$j("#Senario").css("background","url(/images/gw.jpg) no-repeat");
		$j("#radio2").hide();
		$j("#Internet_span").css("display", "");
		$j("#ap-line").css("display", "none");
		$j("#AP").hide();
		$j("#mode_desc").html("<#OP_GW_desc1#><#OP_GW_desc2#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
	}	
	else if(mode == '3'){
		$j("#Senario").css("background", "url(/images/ap.jpg) no-repeat");
		$j("#radio2").css("display", "none");
		$j("#Internet_span").css("display", "");
		$j("#ap-line").css("display", "none");
		$j("#AP").css("display", "");
		$j("#AP").html("<#Device_type_02_RT#>");
		$j("#mode_desc").html("<#OP_AP_desc1#><#OP_AP_desc2#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
		clearTimeout(id_WANunplungHint);
		$j("#Unplug-hint").css("display", "none");
	}
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(11, 3);return unload_body();">
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
<form method="post" name="form" id="ruleForm" action="/QIS_wizard.htm">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="sid_list" value="IPConnection;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="prev_page" value="/Advanced_OperationMode_Content.asp">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="flag" value="">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("", "lan_ipaddr"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
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
		
<table width="95%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
	<thead>
	<tr>
		<td><#t1SYS#> - <#t2OP#></td>
	</tr>
	</thead>
	<!--tbody>
	<tr>
	  <td bgcolor="#FFFFFF"><#OP_desc1#></td>
	  </tr>
	</tbody-->
	<tr>
	  <td bgcolor="#C0DAE4">
	<fieldset style="width:95%; margin:0 auto; padding-bottom:3px;">
	<legend>
		<span style="font-size:13px; font-weight:bold;">
			<div id="wl_rt" style="display:none;"><input type="radio" name="sw_mode" class="input" value="1" onclick="setScenerion(1);" <% nvram_match_x("IPConnection", "sw_mode", "1", "checked"); %>><#OP_GW_item#></div>
			<div id="rt_wo_nat" style="display:none;"><input type="radio" name="sw_mode" class="input" value="4" onclick="setScenerion(4);" <% nvram_match_x("IPConnection", "sw_mode", "4", "checked"); %>><#OP_GW_item#></div>	
			<!--input type="radio" name="sw_mode" class="input" value="4" onclick="setScenerion(4);" <% nvram_match_x("IPConnection", "sw_mode", "4", "checked"); %>> <#OP_RT_item#>  -->
			<div id="wl_ap" style="margin-left:270px;margin-top:-22px;"><input type="radio" name="sw_mode" class="input" value="3" onclick="setScenerion(3);" <% nvram_match_x("IPConnection", "sw_mode", "3", "checked"); %>><#OP_AP_item#></div>
		</span>
	</legend>
	<div id="mode_desc" style="position:relative;display:block; height:60px;z-index:90;">
		<#OP_GW_desc1#>
	</div>
		<br/><br/>
	<div id="Senario">
		<span style="margin:165px 0px 0px 190px;"><#Web_Title#></span>
		<span id="AP" style="margin:165px 0px 0px 280px;"><#Device_type_03_AP#></span>
		<span id="Internet_span" style="margin:165px 0px 0px 390px;"><#Internet#></span>
		<span style="margin:165px 0px 0px 20px;"><#Wireless_Clients#></span>
		<div id="ap-line" style="border:0px solid #333;margin:77px 0px 0px 245px;width:133px; height:41px; position:absolute; background:url(/images/wanlink.gif) no-repeat;"></div>
		<div id="Unplug-hint" style="border:2px solid red; background-color:#FFF; padding:3px;margin:0px 0px 0px 150px;width:250px; position:absolute; display:block; display:none;"><#web_redirect_suggestion1#></div>
	</div>	
	</fieldset>
	  </td>
	</tr>
	<tr>
		<td align="right" bgColor="#C0DAE4">
			<input name="button" type="button" class="button" onClick="saveMode();" value="<#CTL_onlysave#>">
		</td>
	</tr>
</table>
</td>
		<td id="help_td" style="width:15px;display:none;" valign="top">
			<div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
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
	  	</div><!--End of hintofPM--> 
		 </td>
		</tr>
      </table>
		<!--===================================Ending of Main Content===========================================-->
	</td>
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<form name="hint_form"></form>
<div id="footer"></div>
</body>
</html>

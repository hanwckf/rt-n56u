<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>AiDisk Wizard</title>
<link rel="stylesheet" type="text/css" href="aidisk.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>
var ddns_server_x = '<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>';
var ddns_hostname_x = '<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';
var ddns_return_code = '<% nvram_get_ddns("LANHostConfig", "ddns_return_code"); %>';
var wan_ipaddr_t='<% nvram_get_x("IPConnection","wan_ipaddr_t"); %>';

var ddns_hostname_title;
var $j = jQuery.noConflict();

function initial(){
	//parent.show_help_iframe(3);
	parent.hideLoading();
	
	if(this.ddns_server_x == "WWW.ASUS.COM" && this.ddns_hostname_x != ''){
		this.ddns_hostname_title = this.ddns_hostname_x.substring(0, this.ddns_hostname_x.indexOf('.'));
		$("DDNSName").value = this.ddns_hostname_title;
	}
	else{
		$("DDNSName").value = "<#asusddns_inputhint#>";
	}
	
	switch_ddns();
}

function switch_ddns(){
	if(document.DDNSForm.check_asus_ddns[0].checked){
		parent.setASUSDDNS_enable("1");
		document.DDNSForm.ddns_server_x.value = "WWW.ASUS.COM";
		
		//parent.show_help_iframe(5);
	}
	else{
		var ddns_enable_x = '<% nvram_get_x("LANHostConfig", "ddns_enable_x"); %>';
		parent.setASUSDDNS_enable(ddns_enable_x);
		document.DDNSForm.ddns_server_x.value = this.ddns_server_x;
		
		if(parent.show_iframe_page("statusframe").indexOf("ASUS_DDNS_TOS.asp") > 0)
			parent.show_help_iframe(3);
	}
	
	show_TOS_checkbox();
	show_next_button();
	
	parent.restore_help_td();
	parent.openHint(15, 3);
}

function show_TOS_checkbox(){
	if(document.DDNSForm.check_asus_ddns[0].checked){
		//$("TOS_checkbox").style.display = "";
		document.DDNSForm.asusddns_tos_agreement.value = "1";
		$("ddnsname_input").style.display = "";
		$("DDNSName").focus();
		$("DDNSName").select();
	}
	else{
		document.DDNSForm.asusddns_tos_agreement.value = "0";		
		$("ddnsname_input").style.display = "none";	
		//$("TOS_checkbox").style.display = "none";
	}
	
	//agree_TOS();
	if(this.ddns_server_x == "WWW.ASUS.COM")
		check_return_code();
	
	//$("alert_block").style.display = "none";
	//show_ddnsname_input();
	show_next_button();
}
/*
function agree_TOS(){
	if(!document.DDNSForm.agree_ASUSDDNS_TOS.checked){
		document.DDNSForm.asusddns_tos_agreement.value = "0";
	}
	else{
		document.DDNSForm.asusddns_tos_agreement.value = "1";
		
		if(this.ddns_server_x == "WWW.ASUS.COM")
			check_return_code();
	}
	$("alert_block").style.display = "none";
	//show_ddnsname_input();
	show_next_button();
}

function show_ddnsname_input(){
	if(document.DDNSForm.check_asus_ddns[0].checked
		&& document.DDNSForm.agree_ASUSDDNS_TOS.checked)
	{
		$("ddnsname_input").style.display = "";		
		$("DDNSName").focus();
	}
	else
		$("ddnsname_input").style.display = "none";
}
*/
function show_next_button(){

	if(document.DDNSForm.check_asus_ddns[0].checked || 
	   document.DDNSForm.check_asus_ddns[1].checked)
	{
		$("gotonext_block").style.display = "block";
		$("gotonext_disabled").style.display = "none";
	}
	else{
		$("gotonext_block").style.display = "none";
		$("gotonext_disabled").style.display = "block";
	}
}

function show_alert_block(alert_str){
	$("alert_block").style.display = "block";
	
	showtext($("alert_str"), alert_str);
}

function hide_alert_block(){
	$("alert_block").style.display = "none";
}

function check_return_code(){
	parent.hideLoading();

	if(this.ddns_return_code == 'register,-1')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_2#>");
	else if(this.ddns_return_code == 'register,200'){
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_3#>");
		parent.setASUSDDNS_enable("1");
		
		document.DDNSForm.action = "/aidisk/Aidisk-4.asp";
		document.DDNSForm.target = "";
		document.DDNSForm.submit();
		
		return;
	}
	else if(this.ddns_return_code == 'register,203')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+this.ddns_hostname_x+"' <#LANHostConfig_x_DDNS_alarm_registered#>");
	else if(this.ddns_return_code == 'register,220'){
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_4#>");
		parent.setASUSDDNS_enable("1");
		
		document.DDNSForm.action = "/aidisk/Aidisk-4.asp";
		document.DDNSForm.target = "";
		document.DDNSForm.submit();
		
		return;
	}
	else if(this.ddns_return_code == 'register,230'){
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_5#>");
		parent.setASUSDDNS_enable("1");
		
		document.DDNSForm.action = "/aidisk/Aidisk-4.asp";
		document.DDNSForm.target = "";
		document.DDNSForm.submit();
		
		return;
	}
	else if(this.ddns_return_code == 'register,233')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+this.ddns_hostname_x+"' <#LANHostConfig_x_DDNS_alarm_registered_2#>");
	else if(this.ddns_return_code == 'register,296')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_6#>");
	else if(this.ddns_return_code == 'register,297')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_7#>");
	else if(this.ddns_return_code == 'register,298')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_8#>");
	else if(this.ddns_return_code == 'register,299')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_9#>");
	else if(this.ddns_return_code == 'register,401')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_10#>");
	else if(this.ddns_return_code == 'register,407')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_11#>");
	else if(this.ddns_return_code == 'time-out' || this.ddns_return_code == 'connect_fail')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_12#>");
	else if(this.ddns_return_code == 'unknown_error')
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_2#>");	

	this.ddns_return_code = "";
}

function verify_ddns_name(){
	if(validate_ddns_hostname($("DDNSName"))){
		parent.showLoading();
		
		document.DDNSForm.current_page.value = "/aidisk/Aidisk-3.asp";
		document.DDNSForm.action_script.value = "hostname_check";
		document.DDNSForm.action_mode.value = "Update";
		document.DDNSForm.ddns_hostname_x.value = $("DDNSName").value+".asuscomm.com";
		
		document.DDNSForm.flag.value = "nodetect";
		
		document.DDNSForm.target = "hidden_frame";
		document.DDNSForm.submit();
	}
}

function go_next_page(){
	if(document.DDNSForm.check_asus_ddns[0].checked && this.ddns_return_code == ""){		
		verify_ddns_name();
		
		return;
	}
	
	document.DDNSForm.action = "/aidisk/Aidisk-4.asp";
	document.DDNSForm.target = "";
	document.DDNSForm.submit();
}

function go_pre_page(){
	document.DDNSForm.action = "/aidisk/Aidisk-2.asp";
	document.DDNSForm.target = "";
	document.DDNSForm.submit();
}

function validate_ddns_hostname(o){
	dot = 0;
	s = o.value;
	
	if(s == ""){
		show_alert_block("<#QKSet_account_nameblank#>");
		return false;
	}
	
	if(!validate_string(o)){
		return false;
	}
	
	for(var i = 0; i < s.length; ++i){
		c = s.charCodeAt(i);
		if(c == 46){
			++dot;
			
			if(dot > 2){
				show_alert_block("<#LANHostConfig_x_DDNS_alarm_7#>");
				return false;
			}
		}
		
		if(!validate_hostnamechar(c)){
			show_alert_block("<#LANHostConfig_x_DDNS_alarm_13#> '"+s.charAt(i)+"' !");
			return false;
		}
	}
	
	return true;
}

function validate_hostnamechar(ch){
	if(ch >= 48 && ch <= 57)
		return true;
	
	if(ch >= 97 && ch <= 122)
		return true;
	
	if(ch >= 65 && ch <= 90)
		return true;
	
	if(ch == 45)
		return true;
	
	if(ch == 46)
		return true;
	
	return false;
}

function done_validating(action){
	document.DDNSForm.action = "/aidisk/Aidisk-3.asp";
	document.DDNSForm.target = "";
	document.DDNSForm.submit();
}

function showLoading(seconds, flag){
	parent.showLoading(seconds, flag);
}

function hideLoading(flag){
	parent.hideLoading(flag);
}

function checkDDNSReturnCode(){
    $j.ajax({
        url: '/ajax_ddnscode.asp',
        dataType: 'script', 
        error: function(xhr){
                checkDDNSReturnCode();
        },
        success: function(response){
                if(ddns_return_code == 'ddns_query')
                        setTimeout("checkDDNSReturnCode();", 500);
                else 
                        check_return_code();
        }
   });
}
</script>
</head>

<body onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="GET" name="DDNSForm" action="/start_apply.htm">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">

<input type="hidden" name="asusddns_tos_agreement" value="<% nvram_get_x("LANHostConfig","asusddns_tos_agreement"); %>">
<input type="hidden" name="ddns_server_x" value="<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>">
<input type="hidden" name="ddns_hostname_x" value="<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>">

<input type="hidden" name="flag" value="">

<table width="400" border="0" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td colspan="2" valign="top">
			<table width="250" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td width="64"><img src="../images/aidisk-01.gif" width="64" height="59" /></td>
					<td width="29"><p><img src="../images/aidisk-arrow-2.gif" width="29" height="29" /></p></td>
					<td width="64"><img src="../images/aidisk-02_r.gif" width="64" height="59" /></td>
					<td width="29"><img src="../images/aidisk-arrow-2.gif" width="29" height="29" /></td>
					<td><img src="../images/aidisk-03.gif" width="64" height="59" /></td>
				</tr>
			</table>
		</td>
	</tr>
	
	<tr>
		<td colspan="2" class="title"><#Step2_desp#></td>
	</tr>
	
	<tr>
		<td colspan="2" class="textbox">
			<!--input type="radio" name="check_asus_ddns" onClick="switch_ddns();"-->
			<input type="radio" id="d1" name="check_asus_ddns" <% nvram_match_x("LANHostConfig", "asusddns_tos_agreement", "1", "checked"); %> onClick="switch_ddns();">
			<span><label for="d1"><#DDNSterm_agreeword#></label> <a href="#" onclick="parent.show_help_iframe(5);"><#DDNS_termofservice_Title#></a></span><br/>
				<!--div id="TOS_checkbox" class="aidiskdesc" style="display:none;">
					<input type="checkbox" name="agree_ASUSDDNS_TOS" <% nvram_match_x("LANHostConfig", "asusddns_tos_agreement", "1", "checked"); %> onClick="agree_TOS();">
					<span><#DDNSterm_agreeword#></span>
				</div-->
				
				<div id="ddnsname_input" class="aidiskdesc" style="display:none;">
					<input type="text" name="DDNSName" id="DDNSName" class="inputtext">.asuscomm.com
					<div id="alert_block" style="color:#CC0000; margin-left:5px; font-size:11px;display:none;">
						<span id="alert_str"></span>
					</div>
				</div>
				
				<br/>
			<input type="radio" id="d2" name="check_asus_ddns" onClick="switch_ddns();" >
			<span><label for="d2"><#neednotDDNS#></label></span>
		</td>
	</tr>
	<tr>
		<td align="right" height="50">
			<div class="short_btn"><a href="javascript:go_pre_page();"><#btn_pre#></a></div>
		</td>		
		<td>
			<div id="gotonext_block" class="short_btn"><a id="gotonext" href="javascript:go_next_page();"><#btn_next#></a></div>
			<div id="gotonext_disabled" class="short_btn_disabled"><#btn_next#></div>
		</td>
	</tr>
</table>
</form>
</body>
</html>

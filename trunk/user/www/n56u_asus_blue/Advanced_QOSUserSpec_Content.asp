<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="/jquery.js"></script>
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_3_2#></title>
<link rel="stylesheet" type="text/css" href="/index_style.css"> 
<link rel="stylesheet" type="text/css" href="/form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var NAME_WIDTH = 13;
var IP_WIDTH = 16;
var PORT_WIDTH = 12;

var qos_ubw = '<% nvram_get_x("PrinterStatus", "qos_ubw"); %>';
var qos_service_enable_x = '<% nvram_get_x("PrinterStatus", "qos_service_enable"); %>';
var qos_dfragment_enable_x = '<% nvram_get_x("PrinterStatus", "qos_dfragment_enable"); %>';
var qos_dfragment_size_x = '<% nvram_get_x("PrinterStatus", "qos_dfragment_size"); %>';

var x_USRRuleList = [<% get_nvram_list("PrinterStatus", "x_USRRuleList"); %>];
var hwnat = '<% nvram_get_x("",  "hwnat"); %>';

function initial(){
	show_banner(1);
	show_menu(5,3,2);
	show_footer();
	
	frmload();
	
	enable_auto_hint(20, 2);
	showBMUserSpec();
}

function frmload(){
	document.form.qos_dfragment_size.value=qos_dfragment_size_x;
	document.form.qos_dfragment_size.disabled = false;
	if(qos_dfragment_enable_x=='1')
		document.form.qos_dfragment_enable_w.checked = true;
	
	Fragment_display();
}	

function Fragment_display(){
	if(document.form.qos_dfragment_enable_w.checked==true){
		document.form.qos_dfragment_enable.value = 1;
		blocking('Fragment' ,true);
	}
	else{
		document.form.qos_dfragment_enable.value = 0;
		blocking('Fragment' ,false);
	}
}

function applyRule(){
	if(document.form.qos_dfragment_enable_w.checked && !validate_dfragment_size())
		return 0;
	
	/*if(document.form.qos_manual_ubw.value >= "0")
		document.form.hwnat_suggest.value = 1;

	if(document.form.hwnat.value == 1 && document.form.hwnat_suggest.value == 1 && sw_mode == "1"){
			if(confirm("<#BasicConfig_HWNAT_suggestion#>"))
				document.form.hwnat.value = "0";
	}*/
	
	showLoading();
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "/Advanced_QOSUserSpec_Content.asp";
	document.form.next_page.value = "";

	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function validate_duplicate_USRRuleList(ip_value, port_value){	// 2008.01 James.
	for(var i = 0; i < x_USRRuleList.length; ++i){
		if(x_USRRuleList[i][1] == ip_value && x_USRRuleList[i][2] == port_value){
			alert('<#JS_duplicate#>');
			
			return false;
		}
	}
	
	return true;
}

function validate_dfragment_size(){
	with(document.form){
		if(!isNaN(qos_dfragment_size.value)){
			if(qos_dfragment_size.value < 0 || qos_dfragment_size.value > 100){
				alert('<#BM_alert_port1#> 0 <#BM_alert_to#> 100.');
				qos_dfragment_size.value = '0';
				
				return false;
			}
		}
		else{
			alert('"'+qos_dfragment_size.value+'" <#BM_alert_port2#> 0 <#BM_alert_to#> 100.');
			qos_dfragment_size.value = '0';
            
			return false;
		}
	}
	
	return true;
}

function showBMUserSpec(){
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(x_USRRuleList.length == 0)
		code +='<tr><td style="color:#CC0000;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < x_USRRuleList.length; i++){
		if(x_USRRuleList[i][4] == "1"){
			x_USRRuleList[i][4] = "<#Priority_Level_1#>";
			//document.form.hwnat_suggest.value = 1;
		}
		else if(x_USRRuleList[i][4] == "6"){
			x_USRRuleList[i][4] = "<#Priority_Level_3#>";
			//document.form.hwnat_suggest.value = 1;
		}
		else
			x_USRRuleList[i][4] = "<#Priority_Level_2#>";
		
			code +='<tr id="row'+i+'">';
			code +='<td width="96">'+ x_USRRuleList[i][0] +'</td>';		//Service name
			code +='<td width="94">'+ x_USRRuleList[i][1] +'</td>';		//IP
			code +='<td width="76">'+ x_USRRuleList[i][2] +'&nbsp;</td>';	//port
			code +='<td width="34">'+ x_USRRuleList[i][4] +'</td>';		//Priority
			code +='<td width=\"20\"><input type=\"checkbox\" name=\"x_USRRuleList_s\" value='+ i +' onClick="changeBgColor(this,'+i+');" id=\"check'+ i +'\"></td>';
		if(i == 0)
			code +="<td width='58' style='background:#C0DAE4;' rowspan=" + x_USRRuleList.length + "><input style=\"padding:2px 2px 0px 2px\" class=\"button\" type=\"submit\" onclick=\"markGroup(this, 'x_USRRuleList', 32, ' Del ')\" name=\"x_USRRuleList\" value=\"<#CTL_del#>\"/></td>";
	    	code +='</tr>';
		}
	}
	code +='</table>';
	$("BMUSerSpec_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
 		$("row" + num).style.background='#FF9';
	else
 		$("row" + num).style.background='#FFF';
}
</script>
</head>

<body onLoad="initial();" onunLoad="return unload_body();">

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_QOSUserSpec_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="PrinterStatus;">
<input type="hidden" name="group_id" value="x_USRRuleList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="qos_rulenum_x_0" value="<% nvram_get_x("PrinterStatus", "qos_rulenum_x"); %>" readonly="1">
<input type="hidden" name="qos_rule_w" value="">
<input type="hidden" name="qos_dfragment_enable" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="hwnat_suggest" value="">
<input type="hidden" name="hwnat" value="<% nvram_get_x("PrinterStatus","hwnat"); %>">

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
			<table width="500" border="0" align="center" cellpadding="4" cellspacing="0" class="FormTitle" table>
				<thead>
				<tr>
					<td><#BM_title_User#></td>
				</tr>
				</thead>
				
				<tbody>
				<tr>
					<td bgcolor="#FFFFFF"><#BM_user_desc#></td>
				</tr>
				</tbody>
				
				<tr>
					<td bgcolor="#FFFFFF">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<td colspan="2"><#BM_status#></td>
							</tr>
							</thead>
							
							<tr>
								<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 1);"><#BM_measured_uplink_speed#></a></th>
								<td width="60%"><% nvram_get_x("PrinterStatus", "qos_ubw"); %> Kb/s</td>
							</tr>
							
							<tr>
								<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);"><#BM_manual_uplink_speed#></a></th>
								<td width="60%"><input type="text" maxlength="10" name="qos_manual_ubw" onKeyPress="return is_number_sp(event, this);" class="input" size="8" value="<% nvram_get_x("PrinterStatus", "qos_manual_ubw"); %>"> Kb/s</td>
							</tr>
						</table>
					</td>
				</tr>
				
				<tr>
					<td bgcolor="#FFFFFF">		
						<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<td colspan="5" id="TriggerList"><#BM_UserList_title#></td>
							</tr>
							</thead>
							
							<tr>
								<th style="text-align:center;"><#BM_UserList1#></th>
								<th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,3);"><#BM_UserList2#></a></th>
								<th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,4);"><#BM_UserList3#></a></th>
								<th style="text-align:center;"><#BM_UserList4#></th>
								<th></th>
							</tr>
							
							<tr>
								<td><input type="text" maxlength="15" size="14" name="qos_service_name_x_0" class="input" onKeyPress="return is_string(this)"></td>
								<td><input type="text" maxlength="15" size="14" name="qos_ip_x_0" class="input" onKeyPress="return is_iprange(this)" onKeyUp="change_iprange(this)"></td>
								<td><input type="text" maxlength="11" size="10" name="qos_port_x_0" class="input" onKeyPress="return is_portrange(this)"></td>
								<td width="70">
									<select name='qos_prio_x_0' class="input">
										<option value='1'><#Priority_Level_1#></option>
										<option value='4' selected="selected"><#Priority_Level_2#></option>
										<option value='6'><#Priority_Level_3#></option>
									</select>
								</td>
								<td valign="bottom" bgcolor="#FFFFFF" width="60">
									<input type="submit" name="x_USRRuleList2" class="button" onClick="return markGroup(this, 'x_USRRuleList', 32, ' Add ');" value="<#CTL_add#>" size="12" />
								</td>
							</tr>
							</table>
							  <div id="BMUSerSpec_Block"></div>
							<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">							
							<tr bgcolor="#FFFFFF">
								<td colspan="5"><input type="checkbox" name="qos_dfragment_enable_w" onclick='Fragment_display();' style="vertical-align:middle; "><#BM_pkt_flg#></td>
							</tr>
							<tr bgcolor="#FFFFFF">
								<td colspan="5">
									<div id='Fragment' style='display:block'>
										<table width="100%" border="0" cellpadding="2" cellspacing="2" style="font-size:12px; ">
											<tr>
												<td width="20%" align="right">			  
													<#BM_pkt_size#>
												</td>
												<td colspan="2">
													<input type="text" name="qos_dfragment_size" class="input" size="3" maxlength="3" value="">
													<span>(<#BM_alert_port1#> 0 <#BM_alert_to#> 100)</span>
												</td>
											</tr>
											
											<tr>
												<td width="20%" align="right" valign="top">
													<#BM_note#>
												</td>
												<td colspan="2">
													<ol>
														<li><#BM_note1#></li>
														<li><#BM_note2#></li>
														<li><#BM_note3#></li>
													</ol>
												</td>
											</tr>
										</table>
									</div>
								</td>
							</tr>		  
							
							<tr>
								<td align="right"  colspan="5">
									<input name="button" type="button" class="button" onClick="applyRule()" value="<#CTL_apply#>"/>
								</td>
							</tr>
						</table>
					</td>
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

<div id="footer"></div>

</body>
</html>

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

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_3_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var TriggerList = [<% get_nvram_list("IPConnection", "TriggerList"); %>];

function applyRule(){
	showLoading();
	
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "/Advanced_PortTrigger_Content.asp";
	document.form.next_page.value = "";

	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function change_wizard(o, id){
	for(var i = 0; i < wItem.length; i++){
		if(wItem[i][0] != null){
			if(o.value == wItem[i][0]){
				document.form.autofw_outport_x_0.value = wItem[i][1];
				document.form.autofw_outproto_x_0.value = wItem[i][2];
				document.form.autofw_inport_x_0.value = wItem[i][3];
				document.form.autofw_inproto_x_0.value = wItem[i][4];
				document.form.autofw_desc_x_0.value = wItem[i][0];
				
				break;
			}
		}
	}
}

function showTriggerList(){
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(TriggerList.length == 0)
		code +='<tr><td><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < TriggerList.length; i++){
		code +='<tr id="row'+i+'">';
		code +='<td width="">'+ TriggerList[i][4] +'</td>';			//desp		
		code +='<td width="84">'+ TriggerList[i][0] +'</td>';		//Port  range
		code +='<td width="55">'+ TriggerList[i][1] +'</td>';		//local IP
		code +='<td width="83">'+ TriggerList[i][2] +'&nbsp;</td>';	//local port
		code +='<td width="35">'+ TriggerList[i][3] +'</td>';		//proto
		code +='<td width=\"27\"><input type=\"checkbox\" name=\"TriggerList_s\" value='+ i +' onClick="changeBgColor(this,'+i+');" id=\"check'+ i +'\"></td>';
		if(i == 0)
			code +="<td width='75' style='background:#C0DAE4;' rowspan=" + TriggerList.length + "><input style=\"padding:2px 2px 0px 2px\" class=\"button\" type=\"submit\" onclick=\"markGroup(this, 'TriggerList', 64, ' Del ');\" name=\"TriggerList\" value=\"<#CTL_del#>\"/></td>";
		
		code +='</tr>';
		}
	}
	code +='<tfoot><tr align="right">';
	code +='<td colspan="8"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>';
	code +='</tr></tfoot>';
	code +='</table>';
	
	$("TriggerList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
 		$("row" + num).style.background='#FF9';
	else
 		$("row" + num).style.background='#FFF';
}

function trigger_validate_duplicate_noalert(o, v, l, off){
	for (var i=0; i<o.length; i++)
	{
		if (entry_cmp(o[i][0].toLowerCase(), v.toLowerCase(), l)==0){
			return false;
		}
	}
	return true;
}

function trigger_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++)
	{
		if(entry_cmp(o[i][1].toLowerCase(), v.toLowerCase(), l) == 0){
			alert('<#JS_duplicate#>');	
			return false;
		}
	}
	return true;
}

function trigger_markGroup(o, s, c, b) {	
	document.form.group_id.value = s;	
	
	if(b == " Add "){
		if (document.form.autofw_num_x_0.value >= c){				
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		else if ((document.form.autofw_inport_x_0.value == document.form.autofw_outport_x_0.value) 
							&& (document.form.autofw_outproto_x_0.value == document.form.autofw_inproto_x_0.value)){
			alert("<#JS_validport#>");
			document.form.autofw_inport_x_0.focus();
			return false;
		}
		else if (!validate_portrange(document.form.autofw_inport_x_0, "") ||
						 !validate_portrange(document.form.autofw_outport_x_0, "")){
			return false;
		}
		else if (document.form.autofw_inport_x_0.value == ""){				
			alert("<#JS_fieldblank#>");
			document.form.autofw_inport_x_0.focus();
			return false
		}
		else if (document.form.autofw_outport_x_0.value == ""){
			alert("<#JS_fieldblank#>");
			document.form.autofw_outport_x_0.focus();
			return false
		}
		else if (!trigger_validate_duplicate_noalert(TriggerList, document.form.autofw_outport_x_0.value, 5, 0) &&
				 !trigger_validate_duplicate_noalert(TriggerList, document.form.autofw_inport_x_0.value, 5, 0) &&
				 !trigger_validate_duplicate(TriggerList, document.form.autofw_outproto_x_0.value, 3, 12)
				){	
			return false;				
		}
	}
	
	pageChanged = 0;
	pageChangedCount = 0;
	
	document.form.action_mode.value = b;
	return true;		
}		
</script>
</head>

<body onload="show_banner(1); show_menu(5,3,2); show_footer();load_body();	showTriggerList();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_PortTrigger_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="IPConnection;PPPConnection;">
<input type="hidden" name="group_id" value="TriggerList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="autofw_num_x_0" value="<% nvram_get_x("IPConnection", "autofw_num_x"); %>" readonly="1" />

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
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
	<thead>
	<tr>
		<td><#t1NAT#> - <#t2Trig#></td>
	</tr>
	</thead>
	
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#IPConnection_autofwEnable_sectiondesc#></td>
	</tr>
	</tbody>
	
	<tr>
	  <td bgcolor="#FFFFFF">
	    <table width="100%" border="1" align="center" cellpadding="3" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
	  	  <thead>
          <tr>
            <td colspan="6" id="TriggerList"><#IPConnection_TriggerList_groupitemdesc#></td>
          </tr>
		  </thead>
		  
          <tr>
            <th colspan="3"><#IPConnection_autofwEnable_itemname#></th>
            <td colspan="3">
			  <input type="radio" value="1" name="autofw_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '1')" <% nvram_match_x("IPConnection","autofw_enable_x", "1", "checked"); %>/><#checkbox_Yes#>
			  <input type="radio" value="0" name="autofw_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '0')" <% nvram_match_x("IPConnection","autofw_enable_x", "0", "checked"); %>/><#checkbox_No#>
			</td>
		  </tr>
		  
		  <tr>
            <th colspan="3" align="right" id="TriggerList"><#IPConnection_TriggerList_widzarddesc#></th>
            <td colspan="3" >
              <select name="TriggerKnownApps" class="input" onChange="change_wizard(this, 'TriggerKnownApps');">
              	<option value="User Defined"><#Select_menu_default#></option>
              </select>
            </th>
          </tr>
          <tr>
			<th style="text-align:center;"><#IPConnection_autofwDesc_itemname#></th>
            <th width="84" style="text-align:center;"><#IPConnection_autofwOutPort_itemname#></th>
            <th width="55" style="text-align:center;"><#IPConnection_autofwOutProto_itemname#></th>
            <th width="83" style="text-align:center;"><#IPConnection_autofwInPort_itemname#></th>
            <th width="69" style="text-align:center;"><#IPConnection_autofwInProto_itemname#></th>
            <th width="75">&nbsp;</th>
          </tr>
          <tr bgcolor="#FFFFFF">
            <td align="center" bgcolor="#FFFFFF">
              <input type="text" maxlength="18" size="15" name="autofw_desc_x_0" class="input" onKeyPress="return is_string(this)"/>
            </td>
            <td align="center" bgcolor="#FFFFFF">
              <input type="text" maxlength="11" class="input" size="10" name="autofw_outport_x_0" value="" onKeyPress="return is_portrange(this)"/>
            </td>
            <td align="center" bgcolor="#FFFFFF">
              <select name="autofw_outproto_x_0" class="input">
              	<option value="TCP" <% nvram_match_list_x("IPConnection","autofw_outproto_x", "TCP","selected", 0); %>>TCP</option>
              	<option value="UDP" <% nvram_match_list_x("IPConnection","autofw_outproto_x", "UDP","selected", 0); %>>UDP</option>
              </select>
            </td>
            <td align="center" bgcolor="#FFFFFF">
              <input type="text" maxlength="11" class="input" size="10" name="autofw_inport_x_0" value="" onKeyPress="return is_portrange(this)"/>
            </td>
            <td align="center" bgcolor="#FFFFFF">
              <select name="autofw_inproto_x_0" class="input">
              	<option value="TCP" <% nvram_match_list_x("IPConnection","autofw_inproto_x", "TCP","selected", 0); %>>TCP</option>
              	<option value="UDP" <% nvram_match_list_x("IPConnection","autofw_inproto_x", "UDP","selected", 0); %>>UDP</option>
              </select>
            </td>
            <td align="center">
              <input class="button" type="submit" onClick="return trigger_markGroup(this, 'TriggerList', 64, ' Add ');" name="TriggerList2" value="<#CTL_add#>"/>
            </td>
            </tr>
        </table>
	<div id="TriggerList_Block"></div>
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

<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_3_3#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#autofw_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                change_common_radio(this, 'IPConnection', 'autofw_enable_x', '1');
                $j("#autofw_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#autofw_enable_x_1").attr("checked", "checked");
                $j("#autofw_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, 'IPConnection', 'autofw_enable_x', '0');
                $j("#autofw_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#autofw_enable_x_0").attr("checked", "checked");
                $j("#autofw_enable_x_1").removeAttr("checked");
            }
        });
        $j("#autofw_enable_x_on_of label.itoggle").css("background-position", $j("input#autofw_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

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
	code +='<table width="100%" cellspacing="0" cellpadding="3" class="table">';
	if(TriggerList.length == 0)
		code +='<tr><td><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < TriggerList.length; i++){
            code +='<tr id="row'+i+'">';
            code +='<td width="25%">'+ TriggerList[i][4] +'</td>';			//desp
            code +='<td width="15%">'+ TriggerList[i][0] +'</td>';		//Port  range
            code +='<td width="15%" style="padding-left: 13px;">'+ TriggerList[i][1] +'</td>';		//local IP
            code +='<td width="15%" style="padding-left: 13px;">'+ TriggerList[i][2] +'&nbsp;</td>';	//local port
            code +='<td width="15%" style="padding-left: 14px;">'+ TriggerList[i][3] +'</td>';		//proto
            code +='<td style="padding-left: 14px;"><input type=\"checkbox\" name=\"TriggerList_s\" value='+ i +' id=\"check'+ i +'\"></td>';
            code +='</tr>';

            if(TriggerList.length > 0)
            {
                code += '<tr>';
                code += '<td colspan="5">&nbsp;</td>'
                code += '<td style="padding-left: 14px;" ><input class="btn btn-danger span6" type="submit" style="max-width: 219px" onclick="markGroup(this, \'TriggerList\', 32,\' Del \');" value="<#CTL_del#>"/></td>';
                code += '</tr>'
            }
        }
	}
	code +='</table>';
	
	$("TriggerList_Block").innerHTML = code;
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

<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body onload="show_banner(1); show_menu(5,4,2); show_footer();load_body();	showTriggerList();" onunLoad="return unload_body();">
<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

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
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="autofw_num_x_0" value="<% nvram_get_x("IPConnection", "autofw_num_x"); %>" readonly="1" />

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span2">
            <!--Sidebar content-->
            <!--=====Beginning of Main Menu=====-->
            <div class="well sidebar-nav side_nav" style="padding: 0px;">
                <ul id="mainMenu" class="clearfix"></ul>
                <ul class="clearfix">
                    <li>
                        <div id="subMenu" class="accordion"></div>
                    </li>
                </ul>
            </div>
        </div>

        <div class="span10">
            <!--Body content-->
            <div class="row-fluid">
                <div class="span12">
                    <div class="box well grad_colour_dark_blue">
                        <h2 class="box_head round_top"><#t1NAT#> - <#t2Trig#></h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>
                                <div class="alert alert-info" style="margin: 10px;"><#IPConnection_autofwEnable_sectiondesc#></div>

                                <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th colspan="6" id="TriggerList" style="background-color: #E3E3E3;"><#IPConnection_TriggerList_groupitemdesc#></th>
                                    </tr>
                                    <tr>
                                        <th colspan="3" width="50%"><#IPConnection_autofwEnable_itemname#></th>
                                        <td colspan="3">
                                            <div class="main_itoggle">
                                                <div id="autofw_enable_x_on_of">
                                                    <input type="checkbox" id="autofw_enable_x_fake" <% nvram_match_x("WLANConfig11b", "autofw_enable_x", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "autofw_enable_x", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" value="1" name="autofw_enable_x" id="autofw_enable_x_1" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '1')" <% nvram_match_x("IPConnection","autofw_enable_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                <input type="radio" value="0" name="autofw_enable_x" id="autofw_enable_x_0" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '0')" <% nvram_match_x("IPConnection","autofw_enable_x", "0", "checked"); %>/><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th colspan="3" id="TriggerList"><#IPConnection_TriggerList_widzarddesc#></th>
                                        <td colspan="3" >
                                            <select name="TriggerKnownApps" class="input" onChange="change_wizard(this, 'TriggerKnownApps');">
                                                <option value="User Defined"><#Select_menu_default#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th width="25%"><#IPConnection_autofwDesc_itemname#></th>
                                        <th width="15%"><#IPConnection_autofwOutPort_itemname#></th>
                                        <th width="15%"><#IPConnection_autofwOutProto_itemname#></th>
                                        <th width="15%"><#IPConnection_autofwInPort_itemname#></th>
                                        <th width="15%"><#IPConnection_autofwInProto_itemname#></th>
                                        <th width="15%">&nbsp;</th>
                                    </tr>
                                    <tr>
                                        <td>
                                            <input type="text" maxlength="18" size="15" name="autofw_desc_x_0" class="span12" onKeyPress="return is_string(this)"/>
                                        </td>
                                        <td>
                                            <input type="text" maxlength="11" class="span12" size="10" name="autofw_outport_x_0" value="" onKeyPress="return is_portrange(this)"/>
                                        </td>
                                        <td>
                                            <select name="autofw_outproto_x_0" class="span12">
                                                <option value="TCP" <% nvram_match_list_x("IPConnection","autofw_outproto_x", "TCP","selected", 0); %>>TCP</option>
                                                <option value="UDP" <% nvram_match_list_x("IPConnection","autofw_outproto_x", "UDP","selected", 0); %>>UDP</option>
                                            </select>
                                        </td>
                                        <td>
                                            <input type="text" maxlength="11" class="span12" size="10" name="autofw_inport_x_0" value="" onKeyPress="return is_portrange(this)"/>
                                        </td>
                                        <td>
                                            <select name="autofw_inproto_x_0" class="span12">
                                                <option value="TCP" <% nvram_match_list_x("IPConnection","autofw_inproto_x", "TCP","selected", 0); %>>TCP</option>
                                                <option value="UDP" <% nvram_match_list_x("IPConnection","autofw_inproto_x", "UDP","selected", 0); %>>UDP</option>
                                            </select>
                                        </td>
                                        <td>
                                            <input class="btn btn-primary span6" type="submit" onClick="return trigger_markGroup(this, 'TriggerList', 32, ' Add ');" name="TriggerList2" value="<#CTL_add#>"/>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td colspan="6" style="border: 0 none;">
                                            <div id="TriggerList_Block"></div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td colspan="8" style="border: 0 none;">
                                            <center><input name="button" type="button" class="btn btn-primary"  style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center>
                                        </td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

</form>

<!--==============Beginning of hint content=============-->
<div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
    <form name="hint_form"></form>
    <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

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
    </div>
</div>
<!--==============Ending of hint content=============-->

<div id="footer"></div>

</body>
</html>

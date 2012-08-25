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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,5,5);
	show_footer();
	
	enable_auto_hint(10, 5);
	enable_lw();
	enable_lw_1();
	load_body();
}

function applyRule(){
	if(validForm()){
		updateDateTime(document.form.current_page.value);
		
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_Firewall_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

var LWFilterList = [<% get_nvram_list("FirewallConfig", "LWFilterList"); %>];

function validForm(){
	
	
	if((document.form.fw_lw_enable_x[0].checked ==true || document.form.fw_lw_enable_x_1[0].checked ==true ) 
		&& (document.form.filter_lw_date_x_Sun.checked ==false)
		&& (document.form.filter_lw_date_x_Mon.checked ==false)
		&& (document.form.filter_lw_date_x_Tue.checked ==false)
		&& (document.form.filter_lw_date_x_Wed.checked ==false)
		&& (document.form.filter_lw_date_x_Thu.checked ==false)
		&& (document.form.filter_lw_date_x_Fri.checked ==false)
		&& (document.form.filter_lw_date_x_Sat.checked ==false)){
			alert("<#FirewallConfig_LanWanActiveDate_itemname#><#JS_fieldblank#>");
			document.form.fw_lw_enable_x[0].checked=false;
			document.form.fw_lw_enable_x[1].checked=true;
			return false;			
	}	
	
if(document.form.fw_lw_enable_x[0].checked == 1){	
	if(!validate_timerange(document.form.filter_lw_time_x_starthour, 0)
			|| !validate_timerange(document.form.filter_lw_time_x_startmin, 1)
			|| !validate_timerange(document.form.filter_lw_time_x_endhour, 2)
			|| !validate_timerange(document.form.filter_lw_time_x_endmin, 3)
			){	return false; }

	var starttime = eval(document.form.filter_lw_time_x_starthour.value + document.form.filter_lw_time_x_startmin.value);
	var endtime = eval(document.form.filter_lw_time_x_endhour.value + document.form.filter_lw_time_x_endmin.value);
	
	if(starttime > endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint#>");
		document.form.filter_lw_time_x_startmin.value="00";
		document.form.filter_lw_time_x_starthour.value="00";
		return false;  
	}else if(starttime == endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		document.form.filter_lw_time_x_startmin.value="00";
		document.form.filter_lw_time_x_starthour.value="00";		
		return false;  
	}	
			
}

if(document.form.fw_lw_enable_x_1[0].checked == 1){	
	if(!validate_timerange(document.form.filter_lw_time_x_1_starthour, 0)
			|| !validate_timerange(document.form.filter_lw_time_x_1_startmin, 1)
			|| !validate_timerange(document.form.filter_lw_time_x_1_endhour, 2)
			|| !validate_timerange(document.form.filter_lw_time_x_1_endmin, 3)
			){	return false; }

	var starttime_1 = eval(document.form.filter_lw_time_x_1_starthour.value + document.form.filter_lw_time_x_1_startmin.value);
	var endtime_1 = eval(document.form.filter_lw_time_x_1_endhour.value + document.form.filter_lw_time_x_1_endmin.value);
	
	if(starttime_1 > endtime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint#>");
		document.form.filter_lw_time_x_1_startmin.value="00";
		document.form.filter_lw_time_x_1_starthour.value="00";
		return false;  
	}else	if(starttime_1 == endtime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		document.form.filter_lw_time_x_1_startmin.value="00";
		document.form.filter_lw_time_x_1_starthour.value="00";		
		return false;  
	}	
			
}

if(document.form.fw_lw_enable_x[0].checked == 1 && document.form.fw_lw_enable_x_1[0].checked == 1){
	if(starttime < starttime_1){
		if(!(endtime < starttime_1)){
			alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
			return false; 
		}
	}else if(starttime_1 < starttime){
		if(!(endtime_1 < starttime)){
			alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
			return false; 
		}
	}else if(starttime == starttime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
		return false;
	}

}

	
	if(!validate_portlist(document.form.filter_lw_icmp_x, 'filter_lw_icmp_x'))
		return false;

	
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_wizard(o, id){
	for(var i = 0; i < wItem.length; i++){
		if(wItem[i][0] != null){
			if(o.value == wItem[i][0]){
				if(wItem[i][2] == "TCP")
					document.form.filter_lw_proto_x_0.options[0].selected = 1;
				else if(wItem[i][2] == "UDP")
					document.form.filter_lw_proto_x_0.options[8].selected = 1;
				
				document.form.filter_lw_dstport_x_0.value = wItem[i][1];
			}
		}
	}
}

function enable_lw(){
	if(document.form.fw_lw_enable_x[1].checked == 1)
		$("lw_time").style.display = "none";
	else 
		$("lw_time").style.display = "";
	return change_common_radio(this, 'FirewallConfig', 'lw_enable_x', '1')
}

function enable_lw_1(){
	if(document.form.fw_lw_enable_x_1[1].checked == 1)
		$("lw_time_1").style.display = "none";
	else 
		$("lw_time_1").style.display = "";
	return change_common_radio(this, 'FirewallConfig', 'lw_enable_x_1', '1')
}

function valid_subnet(){
	if(document.form.filter_lw_srcip_x_0.value.split("*").length >= 2){
		if(!valid_IP_subnet(document.form.filter_lw_srcip_x_0))
			return false;
	}else if(!valid_IP_form(document.form.filter_lw_srcip_x_0))
		return false;

	if(document.form.filter_lw_dstip_x_0.value.split("*").length >= 2){
		if(!valid_IP_subnet(document.form.filter_lw_dstip_x_0))
			return false;
	}else if(!valid_IP_form(document.form.filter_lw_dstip_x_0))
		return false;

	return true;
}


function valid_IP_subnet(obj){
	var ipPattern1 = new RegExp("(^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.(\\*)$)", "gi");
	var ipPattern2 = new RegExp("(^([0-9]{1,3})\\.([0-9]{1,3})\\.(\\*)\\.(\\*)$)", "gi");
	var ipPattern3 = new RegExp("(^([0-9]{1,3})\\.(\\*)\\.(\\*)\\.(\\*)$)", "gi");
	var ipPattern4 = new RegExp("(^(\\*)\\.(\\*)\\.(\\*)\\.(\\*)$)", "gi");
	var parts = obj.value.split(".");
	if(!ipPattern1.test(obj.value) && !ipPattern2.test(obj.value) && !ipPattern3.test(obj.value) && !ipPattern4.test(obj.value)){
		alert(obj.value + " <#JS_validip#>");
		obj.focus();
		obj.select();
		return false;
	}else if(parts[0] == 0 || parts[0] > 255 || parts[1] > 255 || parts[2] > 255){
		alert(obj.value + " <#JS_validip#>");
		obj.focus();
		obj.select();
		return false;
	}else
		return true;
}

function valid_IP_form(obj){
	if(obj.value == ""){
		return true;
	}else{	//without netMask
		if(!validate_ipaddr_final(obj, obj.name)){
			obj.focus();
			obj.select();
			return false;
		}else
			return true;
	}
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(10, 5);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="Advanced_Firewall_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="LWFilterList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="filter_lw_date_x" value="<% nvram_get_x("FirewallConfig","filter_lw_date_x"); %>">
<input type="hidden" name="filter_lw_time_x" value="<% nvram_get_x("FirewallConfig","filter_lw_time_x"); %>">
<input type="hidden" name="filter_lw_time_x_1" value="<% nvram_get_x("FirewallConfig","filter_lw_time_x_1"); %>">
<input type="hidden" name="filter_lw_num_x_0" value="<% nvram_get_x("FirewallConfig", "filter_lw_num_x"); %>" readonly="1">

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
		
<table width="98%" border="0" align="center" cellpadding="4" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_5#> - <#menu5_5_4#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#FirewallConfig_display1_sectiondesc#></td>
	</tr>
	</tbody>
  
	<tr>  
	<td bgcolor="#FFFFFF">
	<table width="100%" border="1" align="center" cellpadding="2" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	<thead>
          <tr>
            <td colspan="6" id="LWFilterList"><#menu5_5_4#></td>
          </tr>
        </thead>
        
        <tr>
          <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,3);"><#FirewallConfig_LanWanDefaultAct_itemname#></a></th>
          <td>
          	<select name="filter_lw_default_x" class="input" onChange="return change_common(this, 'FirewallConfig', 'filter_lw_default_x')">
          		<option value="DROP" <% nvram_match_x("FirewallConfig","filter_lw_default_x", "DROP","selected"); %>><#WhiteList#></option>
          		<option value="ACCEPT" <% nvram_match_x("FirewallConfig","filter_lw_default_x", "ACCEPT","selected"); %>><#BlackList#></option>
          	</select>
	  			</td>
        </tr>                
        <tr>
          <th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,5);"><#FirewallConfig_LanWanFirewallEnable_itemname#> 1:</a></th>
          <td>
						<input type="radio" value="1" name="fw_lw_enable_x" onClick="enable_lw();" <% nvram_match_x("FirewallConfig","fw_lw_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="fw_lw_enable_x" onClick="enable_lw();" <% nvram_match_x("FirewallConfig","fw_lw_enable_x", "0", "checked"); %>><#checkbox_No#>
	  			</td>
        </tr>
        	<tr id="lw_time">
          	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,2);"><#FirewallConfig_LanWanActiveTime_itemname#> 1:</a></th>
          	<td>
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_starthour" onKeyPress="return is_number(this)">:
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_startmin" onKeyPress="return is_number(this)">-
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_endhour" onKeyPress="return is_number(this)">:
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_endmin" onKeyPress="return is_number(this)">		  
		  			</td>
        	</tr>        
        <tr>
          <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,5);"><#FirewallConfig_LanWanFirewallEnable_itemname#> 2:</a></th>
          <td>
						<input type="radio" value="1" name="fw_lw_enable_x_1" onClick="enable_lw_1();" <% nvram_match_x("FirewallConfig","fw_lw_enable_x_1", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="fw_lw_enable_x_1" onClick="enable_lw_1();" <% nvram_match_x("FirewallConfig","fw_lw_enable_x_1", "0", "checked"); %>><#checkbox_No#>
	  			</td>
        </tr>        
        	<tr id="lw_time_1">
          	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,2);"><#FirewallConfig_LanWanActiveTime_itemname#> 2:</a></th>
          	<td>
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_1_starthour" onKeyPress="return is_number(this)">:
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_1_startmin" onKeyPress="return is_number(this)">-
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_1_endhour" onKeyPress="return is_number(this)">:
							<input type="text" maxlength="2" class="input" size="2" style="width: 20px;" name="filter_lw_time_x_1_endmin" onKeyPress="return is_number(this)">		  
		  			</td>
        	</tr>        

					<!--      //   Viz modify 2 time period 2011.11  {{       -->                
           <tr>
          	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,1);"><#FirewallConfig_LanWanActiveDate_itemname#></a></th>
          	<td>
			<input type="checkbox" name="filter_lw_date_x_Mon" class="input" onChange="return changeDate();"><#DAY_Mon#>
			<input type="checkbox" name="filter_lw_date_x_Tue" class="input" onChange="return changeDate();"><#DAY_Tue#>
			<input type="checkbox" name="filter_lw_date_x_Wed" class="input" onChange="return changeDate();"><#DAY_Wed#>
			<input type="checkbox" name="filter_lw_date_x_Thu" class="input" onChange="return changeDate();"><#DAY_Thu#>
			<input type="checkbox" name="filter_lw_date_x_Fri" class="input" onChange="return changeDate();"><#DAY_Fri#>
			<input type="checkbox" name="filter_lw_date_x_Sat" class="input" onChange="return changeDate();"><#DAY_Sat#>
			<input type="checkbox" name="filter_lw_date_x_Sun" class="input" onChange="return changeDate();"><#DAY_Sun#>
		</td>
        	</tr>

        	<tr>
          	<th ><a class="hintstyle" href="javascript:void(0);" onClick="openHint(10,4);"><#FirewallConfig_LanWanICMP_itemname#></a></th>
          	<td>
          		<input type="text" maxlength="32" class="input" size="40" name="filter_lw_icmp_x" value="<% nvram_get_x("FirewallConfig","filter_lw_icmp_x"); %>" onKeyPress="return is_portlist(this)">
          	</td>
        	</tr>
        <!--      //  }} Viz modify 2 time period 2011.11         -->        
	</table>
	
	</td>
	</tr>
	<tr>  
	<td bgcolor="#FFFFFF">
	  <table width="100%" border="1" align="center" cellpadding="2" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	  <thead>
	   <tr>
	   <td colspan="6" id="LWFilterList"><#FirewallConfig_LWFilterList_groupitemdesc#></td>
	   </tr>
	  </thead>		  
          <tr>
            <th colspan="3" align="right" bgcolor="#7aa3bd"><#FirewallConfig_LWFilterList_widzarddesc#></th>
            <td colspan="2" bgcolor="#a7d2e2"><select name="LWKnownApps" class="input" onChange="change_wizard(this, 'LWKnownApps');">
              <option value="User Defined">User Defined</option>
            </select></td>
            <td rowspan="3" valign="bottom" bgcolor="#FFFFFF" style="width:50px;">
            	<input class="button" type="submit" onclick="if(valid_subnet()){return markGroup(this, 'LWFilterList', 64, ' Add ');}" name="LWFilterList" value="<#CTL_add#>" style="padding:0px; margin:0px;"/>
            </td>
          </tr>
          <tr align="center">
            <th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,3);"><#FirewallConfig_LanWanSrcIP_itemname#></a></th>
            <th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,2);"><#FirewallConfig_LanWanSrcPort_itemname#></a></th>
            <th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,3);"><#FirewallConfig_LanWanDstIP_itemname#></a></th>
            <th style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,2);"><#FirewallConfig_LanWanDstPort_itemname#></a></th>
            <th style="text-align:center;"><#FirewallConfig_LanWanProFlag_itemname#></th>
          </tr>
          <tr>
            <td><input type="text" maxlength="15" class="input" size="14" name="filter_lw_srcip_x_0" onKeyPress="return is_iprange(this)" onKeyUp="change_iprange(this)"></td>
            <td><input type="text" maxlength="11" class="input" size="10" name="filter_lw_srcport_x_0" value="" onKeyPress="return is_portrange(this)"></td>
            <td><input type="text" maxlength="15" class="input" size="14" name="filter_lw_dstip_x_0" onKeyPress="return is_iprange(this)" onKeyUp="change_iprange(this)"></td>
            <td><input type="text" maxlength="11" class="input" size="10" name="filter_lw_dstport_x_0" value="" onKeyPress="return is_portrange(this)"></td>
            <td>
		<select name="filter_lw_proto_x_0" class="input"><option value="TCP" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP","selected", 0); %>>TCP</option><option value="TCP ALL" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP ALL","selected", 0); %>>TCP ALL</option><option value="TCP SYN" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP SYN","selected", 0); %>>TCP SYN</option><option value="TCP ACK" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP ACK","selected", 0); %>>TCP ACK</option><option value="TCP FIN" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP FIN","selected", 0); %>>TCP FIN</option><option value="TCP RST" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP RST","selected", 0); %>>TCP RST</option><option value="TCP URG" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP URG","selected", 0); %>>TCP URG</option><option value="TCP PSH" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "TCP PSH","selected", 0); %>>TCP PSH</option><option value="UDP" <% nvram_match_list_x("FirewallConfig","filter_lw_proto_x", "UDP","selected", 0); %>>UDP</option></select>
	    </td>
          </tr>
          <tr>
            <td colspan="5" align="center">
			<select size="8" class="input" name="LWFilterList_s" multiple="true"  style="font-family:'Courier New', Courier, mono; font-size:12px; width:470px; font-weight:bold;">
				<% nvram_get_table_x("FirewallConfig","LWFilterList"); %>
            </select></td>
            <td width="50" style="width:50px;">
            	<input class="button" type="submit" onclick="return markGroup(this, 'LWFilterList', 64, ' Del ');" name="LWFilterList2" value="<#CTL_del#>" style="font-size:11px; margin:0px 0px 0px 5px; padding:0px 3px 0px 3px;">
            </td>
          </tr>
          
          <table width="100%" border="1" align="center" cellpadding="2" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          
          <tr align="right">
            <td colspan="6">
            	<input name="button" type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>"/>
            </td>
          </tr>
        </td>
	</tr>
        </table>
	</tr>
</table>
</td>
</form>

          <td id="help_td" valign="top"">
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
					<div class="hint_body2" id="hint_body" style="width:120px;"></div>
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

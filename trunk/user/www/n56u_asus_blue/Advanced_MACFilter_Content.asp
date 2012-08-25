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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>

<% login_state_hook(); %>
var client_mac = login_mac_str();
var smac = client_mac.split(":");

var MACList = [<% get_nvram_list("FirewallConfig", "MFList"); %>];

function initial(){
	show_banner(1);
	show_menu(5,5,4);
	show_footer();

	load_body();

	change_macfilter();
	showMFList();
}

function applyRule(){
	if(prevent_lock()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_MACFilter_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function prevent_lock(){
	if(document.form.macfilter_enable_x.value == "1"){
		if(document.form.macfilter_num_x_0.value < 1){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.macfilter_list_x_0.value = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];
				document.form.macfilter_time_x_0.value = "00002359";
				document.form.macfilter_date_x_0.value = "1111111";
				markGroupMAC(document.form.MFList2, 64, ' Add ');
				document.form.submit();
			}
			else
				return false;
		}
		else
			return true;
	}
	else
		return true;
}

function validNewRow(max_rows) {
	if (document.form.macfilter_num_x_0.value >= max_rows){
		alert("<#JS_itemlimit1#> " + max_rows + " <#JS_itemlimit2#>");
		return false;
	}

	if (document.form.macfilter_list_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.macfilter_list_x_0.focus();
		document.form.macfilter_list_x_0.select();
		return false;
	}

	if (!validate_hwaddr(document.form.macfilter_list_x_0)){
		return false;
	}

	if ((document.form.macfilter_date_x_Sun.checked == false) &&
		(document.form.macfilter_date_x_Mon.checked == false) &&
		(document.form.macfilter_date_x_Tue.checked == false) &&
		(document.form.macfilter_date_x_Wed.checked == false) &&
		(document.form.macfilter_date_x_Thu.checked == false) &&
		(document.form.macfilter_date_x_Fri.checked == false) &&
		(document.form.macfilter_date_x_Sat.checked == false)){
		alert("<#MAC_Days#> - <#JS_fieldblank#>");
		return false;
	}

	if (!validate_timerange(document.form.macfilter_time_x_starthour, 0) ||
		!validate_timerange(document.form.macfilter_time_x_startmin, 1) ||
		!validate_timerange(document.form.macfilter_time_x_endhour, 2) ||
		!validate_timerange(document.form.macfilter_time_x_endmin, 3)){
		return false;
	}

	var starttime = eval(document.form.macfilter_time_x_starthour.value + document.form.macfilter_time_x_startmin.value);
	var endtime = eval(document.form.macfilter_time_x_endhour.value + document.form.macfilter_time_x_endmin.value);

	if(starttime > endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint#>");
		document.form.macfilter_time_x_starthour.focus();
		document.form.macfilter_time_x_starthour.select();
		return false;
	}

	if(starttime == endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		document.form.macfilter_time_x_starthour.focus();
		document.form.macfilter_time_x_starthour.select();
		return false;
	}

	return true;
}

function updateDT() {
	document.form.macfilter_date_x_0.value = setDateCheck(
		document.form.macfilter_date_x_Sun,
		document.form.macfilter_date_x_Mon,
		document.form.macfilter_date_x_Tue,
		document.form.macfilter_date_x_Wed,
		document.form.macfilter_date_x_Thu,
		document.form.macfilter_date_x_Fri,
		document.form.macfilter_date_x_Sat);
	document.form.macfilter_time_x_0.value = setTimeRange(
		document.form.macfilter_time_x_starthour,
		document.form.macfilter_time_x_startmin,
		document.form.macfilter_time_x_endhour,
		document.form.macfilter_time_x_endmin);
}

function markGroupMAC(o, c, b) {
	document.form.group_id.value = "MFList";
	if(b == " Add "){
		if (validNewRow(c) == false)
			return false;
		
		updateDT();
	}
	pageChanged = 0;
	pageChangedCount = 0;
	document.form.action_mode.value = b;
	return true;
}

function format_time(nvtime) {
	if (nvtime == "")
		nvtime = "00002359";
	return nvtime.substring(0, 2) + ":" + nvtime.substring(2, 4) + " - " + nvtime.substring(4, 6) + ":" + nvtime.substring(6, 8);
}

function format_date(nvdate) {
	var caption = "";
	if (getDateCheck(nvdate, 1) == true)
		caption = caption + ", <#DAY_Mon#>";
	if (getDateCheck(nvdate, 2) == true)
		caption = caption + ", <#DAY_Tue#>";
	if (getDateCheck(nvdate, 3) == true)
		caption = caption + ", <#DAY_Wed#>";
	if (getDateCheck(nvdate, 4) == true)
		caption = caption + ", <#DAY_Thu#>";
	if (getDateCheck(nvdate, 5) == true)
		caption = caption + ", <#DAY_Fri#>";
	if (getDateCheck(nvdate, 6) == true)
		caption = caption + ", <#DAY_Sat#>";
	if (getDateCheck(nvdate, 0) == true)
		caption = caption + ", <#DAY_Sun#>";
	
	if (caption == "")
		caption = "-";
	else
		caption = caption.substring(2);
	
	return caption;
}

function showMFList(){
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';

	if(MACList.length == 0) {
		code +='<tr><td><#IPConnection_VSList_Norule#></td></tr>';
		
		document.form.macfilter_time_x_0.value = "00002359";
		document.form.macfilter_date_x_0.value = "1111111";
	}
	else{
		for(var i = 0; i < MACList.length; i++){
		    code +='<tr id="row' + i + '">';
		    code +='<td width="20%">' + MACList[i][0] + '</td>';
		    code +='<td width="22%" style="text-align: center;">' + format_time(MACList[i][1]) + '</td>';
		    code +='<td width="43%">' + format_date(MACList[i][2]) + '</td>';
		    code +='<td width="15%" style="text-align: center;"><input type="checkbox" name="MFList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		    code +='</tr>';
		}
		
		code += '<tfoot><tr>';
		code += '<td colspan="3">&nbsp;</td>'
		code += '<td><input class="button" type="submit" onclick="markGroupMAC(this, 64, \' Del \');" name="MFList" value="<#CTL_del#>" style="width: 76px;"></td>';
		code += '</tr></tfoot>'
		
		var last_row = MACList.length - 1;
		document.form.macfilter_time_x_0.value = MACList[last_row][1];
		document.form.macfilter_date_x_0.value = MACList[last_row][2];
	}

	document.form.macfilter_date_x_Sun.checked = getDateCheck(document.form.macfilter_date_x_0.value, 0);
	document.form.macfilter_date_x_Mon.checked = getDateCheck(document.form.macfilter_date_x_0.value, 1);
	document.form.macfilter_date_x_Tue.checked = getDateCheck(document.form.macfilter_date_x_0.value, 2);
	document.form.macfilter_date_x_Wed.checked = getDateCheck(document.form.macfilter_date_x_0.value, 3);
	document.form.macfilter_date_x_Thu.checked = getDateCheck(document.form.macfilter_date_x_0.value, 4);
	document.form.macfilter_date_x_Fri.checked = getDateCheck(document.form.macfilter_date_x_0.value, 5);
	document.form.macfilter_date_x_Sat.checked = getDateCheck(document.form.macfilter_date_x_0.value, 6);

	document.form.macfilter_time_x_starthour.value = getTimeRange(document.form.macfilter_time_x_0.value, 0);
	document.form.macfilter_time_x_startmin.value = getTimeRange(document.form.macfilter_time_x_0.value, 1);
	document.form.macfilter_time_x_endhour.value = getTimeRange(document.form.macfilter_time_x_0.value, 2);
	document.form.macfilter_time_x_endmin.value = getTimeRange(document.form.macfilter_time_x_0.value, 3);

	$("MFList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#FF9';
	else
		$("row" + num).style.background='#FFF';
}

function done_validating(action){
	refreshpage();
}

function change_macfilter() {
	if(document.form.macfilter_enable_x.value!="0"){
		$("mac_drop_row").style.display = "";
	}
	else{
		$("mac_drop_row").style.display = "none";
	}
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(18, 1);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
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
<input type="hidden" name="current_page" value="Advanced_MACFilter_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="MFList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="macfilter_time_x_0" value="00002359">
<input type="hidden" name="macfilter_date_x_0" value="1111111">
<input type="hidden" name="macfilter_num_x_0" value="<% nvram_get_x("", "macfilter_num_x"); %>" readonly="1" />

<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_5#> - <#menu5_5_3#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#FirewallConfig_display5_sectiondesc#></td>
	</tr>
	</tbody>
	<tr>
	  <td bgcolor="#FFFFFF">
	  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
        <tr>
          <th width="43%"><#FirewallConfig_MFMethod_itemname#></th>
          <td><select name="macfilter_enable_x" class="input" onchange="change_macfilter()">
              <option value="0" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "0","selected"); %>><#CTL_Disabled#></option>
              <option value="1" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "1","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
              <option value="2" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "2","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
          </select></td>
        </tr>
        <tr id="mac_drop_row" style="display:none;">
            <th><#MAC_BlockHost#></th>
            <td>
                <input type="radio" value="1" name="fw_mac_drop" <% nvram_match_x("","fw_mac_drop", "1", "checked"); %>/><#checkbox_Yes#>
                <input type="radio" value="0" name="fw_mac_drop" <% nvram_match_x("","fw_mac_drop", "0", "checked"); %>/><#checkbox_No#>
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
                    <td colspan="4"><#FirewallConfig_MFList_groupitemname#></td>
                </tr>
                </thead>
                <tr>
                    <th width="20%" style="text-align:center;"><#FirewallConfig_MFhwaddr_itemname#></th>
                    <th width="22%" style="text-align:center;"><#MAC_Time#></th>
                    <th width="43%" style="text-align:center;"><#MAC_Days#></th>
                    <th width="15%">&nbsp;</th>
                </tr>
                <tr>
                    <td align="center">
                        <input type="text" class="input" size="12" maxlength="12" name="macfilter_list_x_0" onkeypress="return is_hwaddr()" />
                    </td>
                    <td align="center">
                        <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_starthour" onKeyPress="return is_number(this)">:
                        <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_startmin" onKeyPress="return is_number(this)">&nbsp;-
                        <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_endhour" onKeyPress="return is_number(this)">:
                        <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_endmin" onKeyPress="return is_number(this)">
                    </td>
                    <td align="center">
                        <input type="checkbox" name="macfilter_date_x_Mon" class="input"><#DAY_Mon#>
                        <input type="checkbox" name="macfilter_date_x_Tue" class="input"><#DAY_Tue#>
                        <input type="checkbox" name="macfilter_date_x_Wed" class="input"><#DAY_Wed#>
                        <input type="checkbox" name="macfilter_date_x_Thu" class="input"><#DAY_Thu#>
                        <input type="checkbox" name="macfilter_date_x_Fri" class="input"><#DAY_Fri#>
                        <input type="checkbox" name="macfilter_date_x_Sat" class="input"><#DAY_Sat#>
                        <input type="checkbox" name="macfilter_date_x_Sun" class="input"><#DAY_Sun#>
                    </td>
                    <td align="center">
                        <input class="button" type="submit" onclick="return markGroupMAC(this, 64, ' Add ');" name="MFList2" value="<#CTL_add#>" style="width: 76px;"/>
                    </td>
                </tr>
                </table>
                <div id="MFList_Block"></div>
                <tr align="right">
                    <td colspan="4">
                        <input name="button" type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>"/></td>
                    </td>
                </tr>
	    </td>
</table>
</td>
</form>

          <td style="width:15px;" valign="top">
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


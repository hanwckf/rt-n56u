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
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(1);
	show_menu(5,5,3);
	show_footer();
	
	enable_auto_hint(9, 2);
	enable_filter();
	enable_filter_1();
	load_body();
}

function cross_midnight(){
	/*document.form.keyword_date_x_Sun.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Mon.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Tue.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Wed.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Thu.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Fri.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Sat.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;
	document.form.keyword_date_x_Sun.disabled = (document.form.keyword_date_x_Mid.checked == false)?false:true;*/
	return changeDate();
}

function applyRule(){
	if(validForm()){
		updateDateTime(document.form.current_page.value);
		
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_KeywordFilter_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	var starttime = eval(document.form.keyword_time_x_starthour.value + document.form.keyword_time_x_startmin.value);
	var endtime = eval(document.form.keyword_time_x_endhour.value + document.form.keyword_time_x_endmin.value);
	var starttime_1 = eval(document.form.keyword_time_x_starthour_1.value + document.form.keyword_time_x_startmin_1.value);
	var endtime_1 = eval(document.form.keyword_time_x_endhour_1.value + document.form.keyword_time_x_endmin_1.value);

if(document.form.keyword_enable_x[0].checked == 1){	
	if(!validate_timerange(document.form.keyword_time_x_starthour, 0)
			|| !validate_timerange(document.form.keyword_time_x_startmin, 1)
			|| !validate_timerange(document.form.keyword_time_x_endhour, 2)
			|| !validate_timerange(document.form.keyword_time_x_endmin, 3)
			)
		return false;
}
if(document.form.keyword_enable_x_1[0].checked == 1){	
	if(!validate_timerange(document.form.keyword_time_x_starthour_1, 0)
			|| !validate_timerange(document.form.keyword_time_x_startmin_1, 1)
			|| !validate_timerange(document.form.keyword_time_x_endhour_1, 2)
			|| !validate_timerange(document.form.keyword_time_x_endmin_1, 3)
			)
		return false;
}	

	if(starttime > endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint#>");
		return false;  
	}
	if(starttime == endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		return false;  
	}
	if(starttime_1 > endtime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint#>");
		return false;  
	}
	if(starttime_1 == endtime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		return false;  
	}

if(document.form.keyword_enable_x[0].checked == 1 && document.form.keyword_enable_x_1[0].checked == 1){
	if(starttime < starttime_1){
		if(!(endtime < starttime_1)){
			alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
			return false; 
		}
	}
	if(starttime_1 < starttime){
		if(!(endtime_1 < starttime)){
			alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
			return false; 
		}
	}
	if(starttime == starttime_1){
		alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
		return false; 		
	}
}	
	return true;
}

function enable_filter(){
	if(document.form.keyword_enable_x[1].checked == 1)
		$("keyword_time").style.display = "none";
	else 
		$("keyword_time").style.display = "";	
	return change_common_radio(this, 'FirewallConfig', 'keyword_enable_x', '1')
}

function enable_filter_1(){
	if(document.form.keyword_enable_x_1[1].checked == 1)
		$("keyword_time_1").style.display = "none";
	else 
		$("keyword_time_1").style.display = "";	
	return change_common_radio(this, 'FirewallConfig', 'keyword_enable_x_1', '1')
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(9, 2);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_KeywordFilter_Content.asp">
<input type="hidden" name="next_page" value="Advanced_KeywordFilter_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="KeywordList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="keyword_date_x" value="<% nvram_get_x("FirewallConfig","keyword_date_x"); %>">
<input type="hidden" name="keyword_time_x" value="<% nvram_get_x("FirewallConfig","keyword_time_x"); %>">
<input type="hidden" name="keyword_time_x_1" value="<% nvram_get_x("FirewallConfig","keyword_time_x_1"); %>">
<input type="hidden" name="keyword_num_x_0" value="<% nvram_get_x("FirewallConfig", "keyword_num_x"); %>" readonly="1">

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
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_5#> - <#menu5_5_5#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#FirewallConfig_KeywordFilterEnable_sectiondesc#></td>
	</tr>
	</tbody>	
	<tr>
	  <td bgcolor="#FFFFFF"><table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
        <!--tr>
          <th><#FirewallConfig_UrlFilterEnable_itemname#></th>
          	<td>
          		<input type="radio" value="1" name="keyword_enable_x" onClick="return change_common_radio(this, 'FirewallConfig', 'url_enable_x', '1')" <% nvram_match_x("FirewallConfig","keyword_enable_x", "1", "checked"); %>><#checkbox_Yes#>
          		<input type="radio" value="0" name="keyword_enable_x" onClick="return change_common_radio(this, 'FirewallConfig', 'url_enable_x', '0')" <% nvram_match_x("FirewallConfig","keyword_enable_x", "0", "checked"); %>><#checkbox_No#>
          	</td>
        </tr-->
        
        <tr>
          <th><!--a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"--><#FirewallConfig_KeywordFilterEnable_itemname#> 1:</th>
          	  <td>  
          		<input type="radio" value="1" name="keyword_enable_x" onClick="enable_filter();" <% nvram_match_x("FirewallConfig","keyword_enable_x", "1", "checked"); %>><#CTL_Enabled#>
          		<input type="radio" value="0" name="keyword_enable_x" onClick="enable_filter();" <% nvram_match_x("FirewallConfig","keyword_enable_x", "0", "checked"); %>><#CTL_Disabled#>
		  </td>
        </tr>
        
	<tr id="keyword_time">
          <th><#FirewallConfig_KeywordActiveTime_itemname#> 1:</th>
	        <td>
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_starthour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_startmin" onKeyPress="return is_number(this)">-
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endhour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endmin" onKeyPress="return is_number(this)">
		</td>
        </tr>
        
        <tr>
          <th><!--a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"--><#FirewallConfig_KeywordFilterEnable_itemname#> 2:</th>
          	<td>  
          		<input type="radio" value="1" name="keyword_enable_x_1" onClick="enable_filter_1();" <% nvram_match_x("FirewallConfig","keyword_enable_x_1", "1", "checked"); %>><#CTL_Enabled#>
          		<input type="radio" value="0" name="keyword_enable_x_1" onClick="enable_filter_1();" <% nvram_match_x("FirewallConfig","keyword_enable_x_1", "0", "checked"); %>><#CTL_Disabled#>
		</td>
        </tr>
        
	<tr id="keyword_time_1">
          <th><#FirewallConfig_KeywordActiveTime_itemname#> 2:</th>
          	<td>
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_starthour_1" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_startmin_1" onKeyPress="return is_number(this)">-
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endhour_1" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endmin_1" onKeyPress="return is_number(this)">
		</td>
        </tr>
        
        <tr>
          <th><#FirewallConfig_KeywordActiveDate_itemname#></th>
          <td>
		  	<input type="checkbox" name="keyword_date_x_Sun" class="input" onChange="return changeDate();">Sun
			<input type="checkbox" name="keyword_date_x_Mon" class="input" onChange="return changeDate();">Mon			
			<input type="checkbox" name="keyword_date_x_Tue" class="input" onChange="return changeDate();">Tue
			<input type="checkbox" name="keyword_date_x_Wed" class="input" onChange="return changeDate();">Wed
			<input type="checkbox" name="keyword_date_x_Thu" class="input" onChange="return changeDate();">Thu
			<input type="checkbox" name="keyword_date_x_Fri" class="input" onChange="return changeDate();">Fri
			<input type="checkbox" name="keyword_date_x_Sat" class="input" onChange="return changeDate();">Sat
		  </td>
        </tr>
        
        <!--tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,2);"><#FirewallConfig_URLActiveTime_itemname#></a></th>
          <td>
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_starthour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_startmin" onKeyPress="return is_number(this)">-
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endhour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="keyword_time_x_endmin" onKeyPress="return is_number(this)">
		  </td>
        </tr-->
	<tr>
		  <th id="KeywordList"><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,3);"><#FirewallConfig_KeywordList_groupitemname#></a></th>		
		  <td><input type="text" maxlength="32" size="36" onClick="openHint(9,3);" name="keyword_keyword_x_0" class="input" onKeyPress="return is_string(this)">
			  <input class="button" type="submit" onClick="if(validForm()){return markGroup(this, 'KeywordList', 128, ' Add ');}" name="KeywordList" value="<#CTL_add#>" size="12">
		  </td>
		  </tr>
		<tr>
		  <th>&nbsp;</th>
		  <td>
		  <select size="8" class="input" name="KeywordList_s" multiple="multiple" style="width:70%; font-size:12px; font-weight:bold; font-family:verdana; vertical-align:middle;">
            <% nvram_get_table_x("FirewallConfig","KeywordList"); %>
          </select>
		  <input class="button" type="submit" onClick="return markGroup(this, 'KeywordList', 128, ' Del ');" name="KeywordList" value="<#CTL_del#>" size="12">
		  </td>
		</tr>
		<tr>
		  <td colspan="2"  align="right"><input name="button" type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>"/>
	      </td>
        </tr>
      </table></td>
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

<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_3#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var client_mac = login_mac_str();
var macfilter_num_x = '<% nvram_get_x("FirewallConfig", "macfilter_num_x"); %>';
var smac = client_mac.split(":");
var simply_client_mac = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];

function initial(){
	show_banner(1);
	show_menu(5,6,3);
	show_footer();
	
	enable_auto_hint(18, 1);

	load_body();
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
		if(macfilter_num_x == 0){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.macfilter_list_x_0.value = simply_client_mac;
				markGroup(document.form.MFList, 'MFList', 32, ' Add ');
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

function done_validating(action){
	refreshpage();
}
</script>
<style>
    .nav-tabs > li > a {
          padding-right: 6px;
          padding-left: 6px;
    }
</style>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(18, 1);return unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
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
    <input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

    <div class="container-fluid">
        <div class="row-fluid">
            <div class="span3">
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

            <div class="span9">
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_display5_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,1);"><#FirewallConfig_MFMethod_itemname#></a></th>
                                            <td>
                                                <select name="macfilter_enable_x" class="input" onchange="return change_common(this, 'FirewallConfig', 'macfilter_enable_x')">
                                                    <option value="0" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "0","selected"); %>><#CTL_Disabled#></option>
                                                    <option value="1" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "1","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
                                                    <option value="2" <% nvram_match_x("FirewallConfig","macfilter_enable_x", "2","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th id="UrlList"><#FirewallConfig_MFhwaddr_itemname#></th>
                                            <td>
                                                <div style="float: left;">
                                                    <input type="hidden" name="macfilter_num_x_0" value="<% nvram_get_x("FirewallConfig", "macfilter_num_x"); %>" readonly="1" />
                                                    <input type="text" maxlength="12" class="input" size="20" name="macfilter_list_x_0" onKeyPress="return is_hwaddr()">
                                                </div>

                                                <button class="btn" style="margin-left: 5px;" type="submit" onclick="markGroup(this, 'MFList', 32, ' Add ');" name="MFList" size="12"><i class="icon icon-plus"></i></button>
                                                <div class="alert alert-danger" style="margin-top: 5px;">*<#JS_validmac#></div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#FirewallConfig_MFList_groupitemname#></th>
                                            <td>
                                                <div style="float: left;">
                                                    <select size="8" class="input" name="MFList_s" multiple="multiple" style="font-size:12px; font-weight:bold; vertical-align:top;">
                                                        <% nvram_get_table_x("FirewallConfig","MFList"); %>
                                                    </select>
                                                </div>
                                                <button class="btn btn-danger" style="margin-left: 5px;" type="submit" onClick="return markGroup(this, 'MFList', 32, ' Del ');" name="MFList" size="12"><i class="icon icon-minus icon-white"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <br />
                                                <center><input class="btn btn-primary" style="width: 219px" onclick="applyRule();" type="button" value="<#CTL_apply#>" /></center>
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
</div>
</body>
</html>

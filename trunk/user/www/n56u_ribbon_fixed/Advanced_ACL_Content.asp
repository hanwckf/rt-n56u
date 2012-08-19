<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_4#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state_5g.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>

var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var client_mac = login_mac_str();
var wl_macnum_x = '<% nvram_get_x("FirewallConfig", "wl_macnum_x"); %>';
var smac = client_mac.split(":");
var simply_client_mac = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];

function initial(){
	show_banner(1);
	
	show_menu(5,2,4);
	
	show_footer();	
}

function applyRule(){
	if(prevent_lock()){
		showLoading();	
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_ACL_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function done_validating(action){
	refreshpage();
}

function prevent_lock(){

	if(document.form.wl_macmode.value == "allow"){
		if(wl_macnum_x == 0){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.wl_maclist_x_0.value = simply_client_mac;
				markGroup(document.form.ACLList2, 'ACLList', 32, ' Add ');
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
</script>
</head>

<body onload="initial();">
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
    <input type="hidden" name="current_page" value="Advanced_ACL_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_WSecurity_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="DeviceSecurity11a;">
    <input type="hidden" name="group_id" value="ACLList">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
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
                            <h2 class="box_head round_top"><#menu5_1#> - <#menu5_1_4#> (5GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#DeviceSecurity11a_display1_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;">
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,1);"><#FirewallConfig_MFMethod_itemname#></a>
                                            </th>
                                            <td style="border-top: 0 none;">
                                                <select name="wl_macmode" class="input"  onChange="return change_common(this, 'DeviceSecurity11a', 'wl_macmode')">
                                                    <option class="content_input_fd" value="disabled" <% nvram_match_x("DeviceSecurity11a","wl_macmode", "disabled","selected"); %>><#CTL_Disabled#></option>
                                                    <option class="content_input_fd" value="allow" <% nvram_match_x("DeviceSecurity11a","wl_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
                                                    <option class="content_input_fd" value="deny" <% nvram_match_x("DeviceSecurity11a","wl_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%" id="rt_RBRList"><#FirewallConfig_MFhwaddr_itemname#></th>
                                            <td>
                                                <div style="float: left;">
                                                    <input type="hidden" name="wl_macnum_x_0" value="<% nvram_get_x("DeviceSecurity11a", "wl_macnum_x"); %>" readonly="1" /></th>
                                                    <input type="text" maxlength="12" class="input" size="14" name="wl_maclist_x_0" onkeypress="return is_hwaddr()" style="float:left;" />
                                                </div>

                                                <button class="btn" style="margin-left: 5px;" type="submit" onclick="return markGroup(this, 'ACLList', 32, ' Add ');" name="ACLList2" size="12"><i class="icon icon-plus"></i></button>
                                                <div class="alert alert-danger" style="margin-top: 5px;">*<#JS_validmac#></div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th align="right"><#FirewallConfig_MFList_groupitemname#></th>
                                            <td>
                                                <div style="float: left;">
                                                    <select size="6" name="ACLList_s" multiple="multiple" class="input" style="font-size:12px; font-weight:bold; vertical-align:top;">
                                                        <% nvram_get_table_x("DeviceSecurity11a","ACLList"); %>
                                                    </select>
                                                    <button class="btn btn-danger" type="submit" onClick="return markGroup(this, 'ACLList', 32, ' Del ');" name="ACLList" size="12"><i class="icon icon-minus icon-white"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td style="margin-top: 10px;">
                                                <br />
                                                <input class="btn btn-info" type="button"  value="<#GO_2G#>" onclick="location.href='Advanced_ACL2g_Content.asp';">
                                            </td>
                                            <td>
                                                <br />
                                                <input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" />
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

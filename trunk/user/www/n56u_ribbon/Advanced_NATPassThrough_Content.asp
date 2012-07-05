<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Wireless Router <#Web_Title#> - <#NAT_passthrough_itemname#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#fw_pt_pptp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_pt_pptp_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#fw_pt_pptp_1").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pptp_0").removeAttr("checked").removeAttr("selected");
                change_common(this, 'IPConnection','fw_pt_pptp');
            },
            onClickOff: function(){
                $j("#fw_pt_pptp_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_pptp_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pptp_1").removeAttr("checked").removeAttr("selected");;
                change_common(this, 'IPConnection','fw_pt_pptp');
            }
        });
        $j("#fw_pt_pptp_on_of label.itoggle").css("background-position", $j("input#fw_pt_pptp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_pt_l2tp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_pt_l2tp_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#fw_pt_l2tp_1").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_l2tp_0").removeAttr("checked").removeAttr("selected");
                change_common(this, 'IPConnection','fw_pt_l2tp');
            },
            onClickOff: function(){
                $j("#fw_pt_l2tp_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_l2tp_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_l2tp_1").removeAttr("checked").removeAttr("selected");;
                change_common(this, 'IPConnection','fw_pt_l2tp');
            }
        });
        $j("#fw_pt_l2tp_on_of label.itoggle").css("background-position", $j("input#fw_pt_l2tp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_pt_ipsec_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_pt_ipsec_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#fw_pt_ipsec_1").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_ipsec_0").removeAttr("checked").removeAttr("selected");
                change_common(this, 'IPConnection','fw_pt_ipsec');
            },
            onClickOff: function(){
                $j("#fw_pt_ipsec_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_ipsec_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_ipsec_1").removeAttr("checked").removeAttr("selected");;
                change_common(this, 'IPConnection','fw_pt_ipsec');
            }
        });
        $j("#fw_pt_ipsec_on_of label.itoggle").css("background-position", $j("input#fw_pt_ipsec_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#wan_pppoe_relay_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wan_pppoe_relay_x_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#wan_pppoe_relay_x_1").attr("checked", "checked").attr("selected", "selected");
                $j("#wan_pppoe_relay_x_0").removeAttr("checked").removeAttr("selected");
                change_common(this, 'PPPConnection','wan_pppoe_relay_x');
            },
            onClickOff: function(){
                $j("#wan_pppoe_relay_x_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#wan_pppoe_relay_x_0").attr("checked", "checked").attr("selected", "selected");
                $j("#wan_pppoe_relay_x_1").removeAttr("checked").removeAttr("selected");;
                change_common(this, 'PPPConnection','wan_pppoe_relay_x');
            }
        });
        $j("#wan_pppoe_relay_x_on_of label.itoggle").css("background-position", $j("input#wan_pppoe_relay_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(1);
	show_menu(5,4,6);
	show_footer();
}

function applyRule(){
			showLoading();
	
			document.form.action_mode.value = " Apply ";
			document.form.current_page.value = "/Advanced_NATPassThrough_Content.asp";
			document.form.next_page.value = "";
			document.form.submit();	
}
</script>

<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body onload="initial();">
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
<input type="hidden" name="current_page" value="Advanced_NATPassThrough_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="IPConnection;PPPConnection;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

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
                        <h2 class="box_head round_top"><#menu5_3#> - <#NAT_passthrough_itemname#></h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>
                                <div class="alert alert-info" style="margin: 10px;"><#NAT_passthrough_desc#></div>

                                <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,23);"><#NAT_PPTP_Passthrough#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="fw_pt_pptp_on_of">
                                                    <input type="checkbox" id="fw_pt_pptp_fake" <% nvram_match_x("IPConnection", "fw_pt_pptp", "1", "value=1 checked"); %><% nvram_match_x("IPConnection", "fw_pt_pptp", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" id="fw_pt_pptp_1" name="fw_pt_pptp" value="1" <% nvram_match_x("IPConnection","fw_pt_pptp", "1", "checked selected"); %>><#checkbox_Yes#>
                                                <input type="radio" id="fw_pt_pptp_0" name="fw_pt_pptp" value="0" <% nvram_match_x("IPConnection","fw_pt_pptp", "0", "checked selected"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,24);"><#NAT_L2TP_Passthrough#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="fw_pt_l2tp_on_of">
                                                    <input type="checkbox" id="fw_pt_l2tp_fake" <% nvram_match_x("IPConnection", "fw_pt_l2tp", "1", "value=1 checked"); %><% nvram_match_x("IPConnection", "fw_pt_l2tp", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" id="fw_pt_l2tp_1" name="fw_pt_l2tp" value="1" <% nvram_match_x("IPConnection","fw_pt_l2tp", "1", "checked selected"); %>><#checkbox_Yes#>
                                                <input type="radio" id="fw_pt_l2tp_0" name="fw_pt_l2tp" value="0" <% nvram_match_x("IPConnection","fw_pt_l2tp", "0", "checked selected"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,25);"><#NAT_IPSec_Passthrough#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="fw_pt_ipsec_on_of">
                                                    <input type="checkbox" id="fw_pt_ipsec_fake" <% nvram_match_x("IPConnection", "fw_pt_ipsec", "1", "value=1 checked"); %><% nvram_match_x("IPConnection", "fw_pt_ipsec", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" id="fw_pt_ipsec_1" name="fw_pt_ipsec" value="1" <% nvram_match_x("IPConnection","fw_pt_ipsec", "1", "checked selected"); %>><#checkbox_Yes#>
                                                <input type="radio" id="fw_pt_ipsec_0" name="fw_pt_ipsec" value="0" <% nvram_match_x("IPConnection","fw_pt_ipsec", "0", "checked selected"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="wan_pppoe_relay_x_on_of">
                                                    <input type="checkbox" id="wan_pppoe_relay_x_fake" <% nvram_match_x("PPPConnection", "wan_pppoe_relay_x", "1", "value=1 checked"); %><% nvram_match_x("PPPConnection", "wan_pppoe_relay_x", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" id="wan_pppoe_relay_x_1" name="wan_pppoe_relay_x" value="1" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "1", "checked selected"); %>><#checkbox_Yes#>
                                                <input type="radio" id="wan_pppoe_relay_x_0" name="wan_pppoe_relay_x" value="0" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "0", "checked selected"); %>><#checkbox_No#>
                                            </div>
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
</body>
</html>

<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_10_1#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('front_led_all', change_led_all);
});

</script>
<script>

<% hardware_pins(); %>

function initial(){
	show_banner(1);
	show_menu(5,8,1);
	show_footer();

	if (!support_but_wps()){
		showhide_div('tbl_wps_actions', 0);
	}else{
		if (!support_5g_radio()){
			var o1 = document.form.ez_action_short;
			var o2 = document.form.ez_action_long;
			o1.options[2].text = "<#TweaksWPSItem02#>";
			o1.options[5].text = "<#TweaksWPSItem03#>";
			o2.options[1].text = o1.options[2].text;
			o2.options[4].text = o1.options[5].text;
			o1.remove(6);
			o1.remove(6);
			o1.remove(3);
			o1.remove(3);
			o2.remove(5);
			o2.remove(5);
			o2.remove(2);
			o2.remove(2);
		}
	}

	var switch_type = support_switch_type();
	if (switch_type != 0) {
		var o3 = document.form.ether_led0;
		o3.remove(0);
		o3.remove(0);
		o3.remove(0);
		o3.remove(0);
		o3.remove(0);
		o3.remove(0);
		o3.remove(0);
		o3.remove(1);
		o3.remove(1);
		o3.remove(1);
	}

	if (!support_led_pwr())
		showhide_div("row_led_pwr", 0);

	showhide_div("row_led_wan", support_led_wan());
	showhide_div("row_led_lan", support_led_lan());
	showhide_div("row_led_usb", support_led_usb());
	showhide_div("row_led_wif", support_led_wif());
	change_led_all();

	if (support_led_phy() < 2){
		showhide_div('row_eth_phy_led1', 0);
		if (support_led_phy() < 1)
			showhide_div('row_eth_phy_led0', 0);
	}

	change_ez_short(document.form.ez_action_short.value);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Tweaks_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_ez_short(ez_short){
	if(ez_short == "0"){
		inputCtrl(document.form.ez_action_long, 0);
		document.form.ez_action_long.value = "0";
	}else{
		inputCtrl(document.form.ez_action_long, 1);
	}
}

function change_led_all(){
	var led_all = document.form.front_led_all[0].checked;
	showhide_div("row_led_wan", led_all && support_led_wan());
	showhide_div("row_led_lan", led_all && support_led_lan());
	showhide_div("row_led_usb", led_all && support_led_usb());
	showhide_div("row_led_wif", led_all && support_led_wif());
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">

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

    <input type="hidden" name="current_page" value="Advanced_Tweaks_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;General;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">

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
                            <h2 class="box_head round_top"><#menu5_10#> - <#menu5_10_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Tweaks_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_wps_actions">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#TweaksWPSAction#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#TweaksWPSEventShort#></th>
                                            <td>
                                                <select name="ez_action_short" class="input" style="width: 320px;" onchange="change_ez_short(this.value);">
                                                    <option value="0"  <% nvram_match_x("", "ez_action_short", "0", "selected"); %>><#TweaksWPSItem00#></option>
                                                    <option value="1"  <% nvram_match_x("", "ez_action_short", "1", "selected"); %>><#TweaksWPSItem01#></option>
                                                    <option value="2"  <% nvram_match_x("", "ez_action_short", "2", "selected"); %>><#TweaksWPSItem02#> 2.4</option>
                                                    <option value="3"  <% nvram_match_x("", "ez_action_short", "3", "selected"); %>><#TweaksWPSItem02#> 5</option>
                                                    <option value="4"  <% nvram_match_x("", "ez_action_short", "4", "selected"); %>><#TweaksWPSItem02#> 2.4 & 5</option>
                                                    <option value="11" <% nvram_match_x("", "ez_action_short", "11","selected"); %>><#TweaksWPSItem03#> 2.4</option>
                                                    <option value="12" <% nvram_match_x("", "ez_action_short", "12","selected"); %>><#TweaksWPSItem03#> 5</option>
                                                    <option value="13" <% nvram_match_x("", "ez_action_short", "13","selected"); %>><#TweaksWPSItem03#> 2.4 & 5</option>
                                                    <option value="5"  <% nvram_match_x("", "ez_action_short", "5", "selected"); %>><#TweaksWPSItem10#></option>
                                                    <option value="10" <% nvram_match_x("", "ez_action_short", "10","selected"); %>><#TweaksWPSItem11#></option>
                                                    <option value="6"  <% nvram_match_x("", "ez_action_short", "6", "selected"); %>><#TweaksWPSItem20#></option>
                                                    <option value="7"  <% nvram_match_x("", "ez_action_short", "7", "selected"); %>><#TweaksWPSItem21#></option>
                                                    <option value="8"  <% nvram_match_x("", "ez_action_short", "8", "selected"); %>><#TweaksWPSItem22#></option>
                                                    <option value="9"  <% nvram_match_x("", "ez_action_short", "9", "selected"); %>><#TweaksWPSItem32#> /opt/bin/on_wps.sh 1</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#TweaksWPSEventLong#></th>
                                            <td>
                                                <select name="ez_action_long" class="input" style="width: 320px;">
                                                    <option value="0"  <% nvram_match_x("", "ez_action_long", "0", "selected"); %>><#TweaksWPSItem00#></option>
                                                    <option value="1"  <% nvram_match_x("", "ez_action_long", "1", "selected"); %>><#TweaksWPSItem02#> 2.4</option>
                                                    <option value="2"  <% nvram_match_x("", "ez_action_long", "2", "selected"); %>><#TweaksWPSItem02#> 5</option>
                                                    <option value="3"  <% nvram_match_x("", "ez_action_long", "3", "selected"); %>><#TweaksWPSItem02#> 2.4 & 5</option>
                                                    <option value="12" <% nvram_match_x("", "ez_action_long", "12","selected"); %>><#TweaksWPSItem03#> 2.4</option>
                                                    <option value="13" <% nvram_match_x("", "ez_action_long", "13","selected"); %>><#TweaksWPSItem03#> 5</option>
                                                    <option value="14" <% nvram_match_x("", "ez_action_long", "14","selected"); %>><#TweaksWPSItem03#> 2.4 & 5</option>
                                                    <option value="4"  <% nvram_match_x("", "ez_action_long", "4", "selected"); %>><#TweaksWPSItem10#></option>
                                                    <option value="11" <% nvram_match_x("", "ez_action_long", "11","selected"); %>><#TweaksWPSItem11#></option>
                                                    <option value="5"  <% nvram_match_x("", "ez_action_long", "5", "selected"); %>><#TweaksWPSItem20#></option>
                                                    <option value="6"  <% nvram_match_x("", "ez_action_long", "6", "selected"); %>><#TweaksWPSItem21#></option>
                                                    <option value="9"  <% nvram_match_x("", "ez_action_long", "9", "selected"); %>><#TweaksWPSItem22#></option>
                                                    <option value="7"  <% nvram_match_x("", "ez_action_long", "7", "selected"); %>><#TweaksWPSItem30#></option>
                                                    <option value="8"  <% nvram_match_x("", "ez_action_long", "8", "selected"); %>><#TweaksWPSItem31#></option>
                                                    <option value="10" <% nvram_match_x("", "ez_action_long", "10","selected"); %>><#TweaksWPSItem32#> /opt/bin/on_wps.sh 2</option>
                                                    <option value="15" <% nvram_match_x("", "ez_action_long", "15","selected"); %>><#TweaksWPSItem33#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>
                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#TweaksLEDEvents#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#TweaksLEDALL#></th>

                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="front_led_all_on_of">
                                                        <input type="checkbox" id="front_led_all_fake" <% nvram_match_x("", "front_led_all", "1", "value=1 checked"); %><% nvram_match_x("", "front_led_all", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="front_led_all" id="front_led_all_1" class="input" onclick="change_led_all();" <% nvram_match_x("", "front_led_all", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="front_led_all" id="front_led_all_0" class="input" onclick="change_led_all();" <% nvram_match_x("", "front_led_all", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_led_wan">
                                            <th><#TweaksLEDWAN#></th>
                                            <td>
                                                <select name="front_led_wan" class="input" style="width: 320px;">
                                                    <option value="0" <% nvram_match_x("", "front_led_wan", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "front_led_wan", "1","selected"); %>><#TweaksLEDItem00#></option>
                                                    <option value="2" <% nvram_match_x("", "front_led_wan", "2","selected"); %>><#TweaksLEDItem03#> (*)</option>
                                                    <option value="3" <% nvram_match_x("", "front_led_wan", "3","selected"); %>><#TweaksLEDItem04#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_led_lan" style="display:none;">
                                            <th><#TweaksLEDLAN#></th>
                                            <td>
                                                <select name="front_led_lan" class="input" style="width: 320px;">
                                                    <option value="0" <% nvram_match_x("", "front_led_lan", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "front_led_lan", "1","selected"); %>><#TweaksLEDItem00#> (*)</option>
                                                    <option value="2" <% nvram_match_x("", "front_led_lan", "2","selected"); %>><#TweaksLEDItem01#></option>
                                                    <option value="3" <% nvram_match_x("", "front_led_lan", "3","selected"); %>><#TweaksLEDItem02#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_led_usb" style="display:none;">
                                            <th><#TweaksLEDUSB#></th>
                                            <td>
                                                <select name="front_led_usb" class="input" style="width: 320px;">
                                                    <option value="0" <% nvram_match_x("", "front_led_usb", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "front_led_usb", "1","selected"); %>><#TweaksLEDItem05#> (*)</option>
                                                    <option value="2" <% nvram_match_x("", "front_led_usb", "2","selected"); %>><#TweaksLEDItem06#></option>
                                                    <option value="3" <% nvram_match_x("", "front_led_usb", "3","selected"); %>><#TweaksLEDItem07#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_led_wif" style="display:none;">
                                            <th><#TweaksLEDWIF#></th>
                                            <td>
                                                <select name="front_led_wif" class="input" style="width: 320px;">
                                                    <option value="0" <% nvram_match_x("", "front_led_wif", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "front_led_wif", "1","selected"); %>><#TweaksLEDItem09#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_led_pwr">
                                            <th><#TweaksLEDPWR#></th>
                                            <td>
                                                <select name="front_led_pwr" class="input" style="width: 320px;">
                                                    <option value="0" <% nvram_match_x("", "front_led_pwr", "0","selected"); %>><#btn_Disable#>, <#TweaksLEDItem11#></option>
                                                    <option value="1" <% nvram_match_x("", "front_led_pwr", "1","selected"); %>><#btn_Enable#>, <#TweaksLEDItem11#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_eth_phy_led0">
                                            <th><#TweaksLEDEth0#></th>
                                            <td>
                                                <select name="ether_led0" class="input" style="width: 320px;">
                                                    <option value="0"  <% nvram_match_x("", "ether_led0", "0", "selected"); %>>Link 1000 Mbps, TX/RX activity</option>
                                                    <option value="1"  <% nvram_match_x("", "ether_led0", "1", "selected"); %>>Link 100 Mbps, TX/RX activity</option>
                                                    <option value="2"  <% nvram_match_x("", "ether_led0", "2", "selected"); %>>Link 10 Mbps, TX/RX activity</option>
                                                    <option value="3"  <% nvram_match_x("", "ether_led0", "3", "selected"); %>>Link 100/10 Mbps, TX/RX activity (*)</option>
                                                    <option value="4"  <% nvram_match_x("", "ether_led0", "4", "selected"); %>>Link 1000 Mbps</option>
                                                    <option value="5"  <% nvram_match_x("", "ether_led0", "5", "selected"); %>>Link 100 Mbps</option>
                                                    <option value="6"  <% nvram_match_x("", "ether_led0", "6", "selected"); %>>Link 10 Mbps</option>
                                                    <option value="7"  <% nvram_match_x("", "ether_led0", "7", "selected"); %>>Link, TX/RX activity</option>
                                                    <option value="8"  <% nvram_match_x("", "ether_led0", "8", "selected"); %>>Link, RX activity</option>
                                                    <option value="9"  <% nvram_match_x("", "ether_led0", "9", "selected"); %>>Link, TX activity</option>
                                                    <option value="10" <% nvram_match_x("", "ether_led0", "10","selected"); %>>Duplex, Collision</option>
                                                    <option value="11" <% nvram_match_x("", "ether_led0", "11","selected"); %>><#btn_Disable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_eth_phy_led1">
                                            <th><#TweaksLEDEth1#></th>
                                            <td>
                                                <select name="ether_led1" class="input" style="width: 320px;">
                                                    <option value="0"  <% nvram_match_x("", "ether_led1", "0", "selected"); %>>Link 1000 Mbps, TX/RX activity (*)</option>
                                                    <option value="1"  <% nvram_match_x("", "ether_led1", "1", "selected"); %>>Link 100 Mbps, TX/RX activity</option>
                                                    <option value="2"  <% nvram_match_x("", "ether_led1", "2", "selected"); %>>Link 10 Mbps, TX/RX activity</option>
                                                    <option value="3"  <% nvram_match_x("", "ether_led1", "3", "selected"); %>>Link 100/10 Mbps, TX/RX activity</option>
                                                    <option value="4"  <% nvram_match_x("", "ether_led1", "4", "selected"); %>>Link 1000 Mbps</option>
                                                    <option value="5"  <% nvram_match_x("", "ether_led1", "5", "selected"); %>>Link 100 Mbps</option>
                                                    <option value="6"  <% nvram_match_x("", "ether_led1", "6", "selected"); %>>Link 10 Mbps</option>
                                                    <option value="7"  <% nvram_match_x("", "ether_led1", "7", "selected"); %>>Link, TX/RX activity</option>
                                                    <option value="8"  <% nvram_match_x("", "ether_led1", "8", "selected"); %>>Link, RX activity</option>
                                                    <option value="9"  <% nvram_match_x("", "ether_led1", "9", "selected"); %>>Link, TX activity</option>
                                                    <option value="10" <% nvram_match_x("", "ether_led1", "10","selected"); %>>Duplex, Collision</option>
                                                    <option value="11" <% nvram_match_x("", "ether_led1", "11","selected"); %>><#btn_Disable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <td style="border: 0 none;">
                                                <center><input type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center>
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

    <div id="footer"></div>
</div>
</body>
</html>

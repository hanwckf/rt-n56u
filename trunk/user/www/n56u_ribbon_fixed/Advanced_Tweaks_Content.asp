<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_6#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
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

<% login_state_hook(); %>
<% board_caps_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,8,2);
	show_footer();
	
	if (!support_but_wps()){
		$('tbl_wps_actions').style.display="none";
	}
	
	if (!support_led_all()){
		document.form.front_leds.remove(4);
		document.form.front_leds.remove(3);
	}
	
	if (support_led_phy() < 2){
		$('row_eth_phy_led1').style.display="none";
		if (support_led_phy() < 1){
			$('row_eth_phy_led0').style.display="none";
		}
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
	}
	else{
		inputCtrl(document.form.ez_action_long, 1);
	}
}

</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
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
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_Tweaks_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;General;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_6#></h2>
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
                                                <select name="ez_action_short" class="input" onchange="change_ez_short(this.value);">
                                                    <option value="0" <% nvram_match_x("General", "ez_action_short", "0","selected"); %>>Nothing</option>
                                                    <option value="1" <% nvram_match_x("General", "ez_action_short", "1","selected"); %>>WiFi radio On/Off trigger</option>
                                                    <option value="2" <% nvram_match_x("General", "ez_action_short", "2","selected"); %>>WiFi 2.4GHz force On/Off trigger</option>
                                                    <option value="3" <% nvram_match_x("General", "ez_action_short", "3","selected"); %>>WiFi 5GHz force On/Off trigger</option>
                                                    <option value="4" <% nvram_match_x("General", "ez_action_short", "4","selected"); %>>WiFi 2.4 and 5GHz force On/Off trigger</option>
                                                    <option value="5" <% nvram_match_x("General", "ez_action_short", "5","selected"); %>>Safe removal all USB</option>
                                                    <option value="6" <% nvram_match_x("General", "ez_action_short", "6","selected"); %>>WAN down</option>
                                                    <option value="7" <% nvram_match_x("General", "ez_action_short", "7","selected"); %>>WAN reconnect</option>
                                                    <option value="8" <% nvram_match_x("General", "ez_action_short", "8","selected"); %>>WAN up/down toggle</option>
                                                    <option value="10" <% nvram_match_x("General", "ez_action_short", "10","selected"); %>>Front LED On/Off trigger</option>
                                                    <option value="9" <% nvram_match_x("General", "ez_action_short", "9","selected"); %>>Run user script (/opt/bin/on_wps.sh 1)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#TweaksWPSEventLong#></th>
                                            <td>
                                                <select name="ez_action_long" class="input">
                                                    <option value="0" <% nvram_match_x("General", "ez_action_long", "0","selected"); %>>Nothing</option>
                                                    <option value="1" <% nvram_match_x("General", "ez_action_long", "1","selected"); %>>WiFi 2.4GHz force On/Off trigger</option>
                                                    <option value="2" <% nvram_match_x("General", "ez_action_long", "2","selected"); %>>WiFi 5GHz force On/Off trigger</option>
                                                    <option value="3" <% nvram_match_x("General", "ez_action_long", "3","selected"); %>>WiFi 2.4 and 5GHz force On/Off trigger</option>
                                                    <option value="4" <% nvram_match_x("General", "ez_action_long", "4","selected"); %>>Safe removal all USB</option>
                                                    <option value="5" <% nvram_match_x("General", "ez_action_long", "5","selected"); %>>WAN down</option>
                                                    <option value="6" <% nvram_match_x("General", "ez_action_long", "6","selected"); %>>WAN reconnect</option>
                                                    <option value="9" <% nvram_match_x("General", "ez_action_long", "9","selected"); %>>WAN up/down toggle</option>
                                                    <option value="11" <% nvram_match_x("General", "ez_action_long", "11","selected"); %>>Front LED On/Off trigger</option>
                                                    <option value="7" <% nvram_match_x("General", "ez_action_long", "7","selected"); %>>Router reboot</option>
                                                    <option value="8" <% nvram_match_x("General", "ez_action_long", "8","selected"); %>>Router shutdown (prepare)</option>
                                                    <option value="10" <% nvram_match_x("General", "ez_action_long", "10","selected"); %>>Run user script (/opt/bin/on_wps.sh 2)</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#TweaksLEDEvents#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#TweaksLEDFront#></th>
                                            <td>
                                                <select name="front_leds" class="input">
                                                    <option value="0" <% nvram_match_x("General", "front_leds", "0","selected"); %>>All LED</option>
                                                    <option value="1" <% nvram_match_x("General", "front_leds", "1","selected"); %>>WiFi and Power LED</option>
                                                    <option value="2" <% nvram_match_x("General", "front_leds", "2","selected"); %>>WiFi LED only</option>
                                                    <option value="3" <% nvram_match_x("General", "front_leds", "3","selected"); %>>Power LED only</option>
                                                    <option value="4" <% nvram_match_x("General", "front_leds", "4","selected"); %>>Hide All</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_eth_phy_led0">
                                            <th><#TweaksLEDEth0#></th>
                                            <td>
                                                <select name="ether_led0" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_led0", "0","selected"); %>>Link 1000 Mbps, TX/RX activity</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_led0", "1","selected"); %>>Link 100 Mbps, TX/RX activity</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_led0", "2","selected"); %>>Link 10 Mbps, TX/RX activity</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_led0", "3","selected"); %>>Link 100/10 Mbps, TX/RX activity</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_led0", "4","selected"); %>>Link 1000 Mbps</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_led0", "5","selected"); %>>Link 100 Mbps</option>
                                                    <option value="6" <% nvram_match_x("LANHostConfig","ether_led0", "6","selected"); %>>Link 10 Mbps</option>
                                                    <option value="7" <% nvram_match_x("LANHostConfig","ether_led0", "7","selected"); %>>Link, TX/RX activity</option>
                                                    <option value="8" <% nvram_match_x("LANHostConfig","ether_led0", "8","selected"); %>>Link, RX activity</option>
                                                    <option value="9" <% nvram_match_x("LANHostConfig","ether_led0", "9","selected"); %>>Link, TX activity</option>
                                                    <option value="10" <% nvram_match_x("LANHostConfig","ether_led0", "10","selected"); %>>Duplex, Collision</option>
                                                    <option value="11" <% nvram_match_x("LANHostConfig","ether_led0", "11","selected"); %>>LED OFF</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_eth_phy_led1">
                                            <th><#TweaksLEDEth1#></th>
                                            <td>
                                                <select name="ether_led1" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_led1", "0","selected"); %>>Link 1000 Mbps, TX/RX activity</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_led1", "1","selected"); %>>Link 100 Mbps, TX/RX activity</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_led1", "2","selected"); %>>Link 10 Mbps, TX/RX activity</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_led1", "3","selected"); %>>Link 100/10 Mbps, TX/RX activity</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_led1", "4","selected"); %>>Link 1000 Mbps</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_led1", "5","selected"); %>>Link 100 Mbps</option>
                                                    <option value="6" <% nvram_match_x("LANHostConfig","ether_led1", "6","selected"); %>>Link 10 Mbps</option>
                                                    <option value="7" <% nvram_match_x("LANHostConfig","ether_led1", "7","selected"); %>>Link, TX/RX activity</option>
                                                    <option value="8" <% nvram_match_x("LANHostConfig","ether_led1", "8","selected"); %>>Link, RX activity</option>
                                                    <option value="9" <% nvram_match_x("LANHostConfig","ether_led1", "9","selected"); %>>Link, TX activity</option>
                                                    <option value="10" <% nvram_match_x("LANHostConfig","ether_led1", "10","selected"); %>>Duplex, Collision</option>
                                                    <option value="11" <% nvram_match_x("LANHostConfig","ether_led1", "11","selected"); %>>LED OFF</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
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

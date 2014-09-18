<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_2_5#></title>
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
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('ether_green');
	init_itoggle('ether_igmp');
});

</script>
<script>

<% lanlink(); %>

<% hardware_pins(); %>

function initial(){
	var id_menu = 5;
	if(get_ap_mode()){
		id_menu = 3;
		if (lan_proto == '1')
			id_menu--;
	}

	show_banner(1);
	show_menu(5,3,id_menu);
	show_footer();

	if (sw_mode != "3"){
		$('row_igmp_snoop').style.display="none";
	}

	var switch_type = support_switch_type();
	if (switch_type == 1) {
		document.form.ether_link_wan.remove(1);
		document.form.ether_link_lan1.remove(1);
		document.form.ether_link_lan2.remove(1);
		document.form.ether_link_lan3.remove(1);
		document.form.ether_link_lan4.remove(1);
	}

	if (switch_type != 0) {
		document.form.ether_flow_wan.remove(1);
		document.form.ether_flow_lan1.remove(1);
		document.form.ether_flow_lan2.remove(1);
		document.form.ether_flow_lan3.remove(1);
		document.form.ether_flow_lan4.remove(1);
		$('ether_jumbo').options[1].text = "Up to 9000 bytes";
		$('row_ether_green').style.display="none";
	}

	var arr_speeds = [1000, 100, 10, 100, 1000, 100, 10, 0, 0, 0, 0, 0];

	var led_color_green = (switch_type == 1) ? 1 : 3;
	var led_color_orange = (switch_type == 1) ? 2 : 0;

	if (support_led_phy() > 0)
		led_color_green = parseInt("<% nvram_get_x("","ether_led0"); %>");

	if (support_led_phy() > 1)
		led_color_orange = parseInt("<% nvram_get_x("","ether_led1"); %>");

	var wan_speed = parseInt(lanlink_etherlink_wan());
	var lan1_speed = parseInt(lanlink_etherlink_lan1());
	var lan2_speed = parseInt(lanlink_etherlink_lan2());
	var lan3_speed = parseInt(lanlink_etherlink_lan3());
	var lan4_speed = parseInt(lanlink_etherlink_lan4());

	$("linkstate_wan").innerHTML   = '<span class="label ' + (wan_speed  == arr_speeds[led_color_orange] ? 'label-warning">' : (wan_speed  == arr_speeds[led_color_green]  ? 'label-success">' : 'label-info">')) + lanlink_etherlink_wan()  + '</span>';
	$("linkstate_lan1").innerHTML  = '<span class="label ' + (lan1_speed == arr_speeds[led_color_orange] ? 'label-warning">' : (lan1_speed == arr_speeds[led_color_green]  ? 'label-success">' : 'label-info">')) + lanlink_etherlink_lan1() + '</span>';
	$("linkstate_lan2").innerHTML  = '<span class="label ' + (lan2_speed == arr_speeds[led_color_orange] ? 'label-warning">' : (lan2_speed == arr_speeds[led_color_green]  ? 'label-success">' : 'label-info">')) + lanlink_etherlink_lan2() + '</span>';
	$("linkstate_lan3").innerHTML  = '<span class="label ' + (lan3_speed == arr_speeds[led_color_orange] ? 'label-warning">' : (lan3_speed == arr_speeds[led_color_green]  ? 'label-success">' : 'label-info">')) + lanlink_etherlink_lan3() + '</span>';
	$("linkstate_lan4").innerHTML  = '<span class="label ' + (lan4_speed == arr_speeds[led_color_orange] ? 'label-warning">' : (lan4_speed == arr_speeds[led_color_green]  ? 'label-success">' : 'label-info">')) + lanlink_etherlink_lan4() + '</span>';
}

function applyRule(){
	showLoading();
	
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "Advanced_Switch_Content.asp";
	document.form.next_page.value = "";
	
	document.form.submit();
}

function done_validating(action){
	refreshpage();
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
    <input type="hidden" name="current_page" value="Advanced_Switch_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Switch_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#SwitchBase#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchJumbo#></th>
                                            <td>
                                                <select name="ether_jumbo" id="ether_jumbo" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_jumbo", "0","selected"); %>>Up to 1536 bytes</option>
                                                    <option value="1" <% nvram_match_x("","ether_jumbo", "1","selected"); %>>Up to 16000 bytes</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ether_green">
                                            <th><#SwitchGreen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_green_on_of">
                                                        <input type="checkbox" id="ether_green_fake" <% nvram_match_x("", "ether_green", "1", "value=1 checked"); %><% nvram_match_x("", "ether_green", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_green" id="ether_green_1" class="input" <% nvram_match_x("", "ether_green", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_green" id="ether_green_0" class="input" <% nvram_match_x("", "ether_green", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_igmp_snoop">
                                            <th><#SwitchIgmp#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_igmp_on_of">
                                                        <input type="checkbox" id="ether_igmp_fake" <% nvram_match_x("", "ether_igmp", "1", "value=1 checked"); %><% nvram_match_x("", "ether_igmp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_igmp" id="ether_igmp_1" class="input" <% nvram_match_x("", "ether_igmp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_igmp" id="ether_igmp_0" class="input" <% nvram_match_x("", "ether_igmp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" align="center" style="background-color: #E3E3E3;">WAN</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_wan" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_flow_wan", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("","ether_flow_wan", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("","ether_flow_wan", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_wan" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_link_wan","0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("","ether_link_wan","1","selected"); %>>1000 Mbps, Full Duplex: AN</option>
                                                    <option value="2" <% nvram_match_x("","ether_link_wan","2","selected"); %>>100 Mbps, Full Duplex: AN</option>
                                                    <option value="3" <% nvram_match_x("","ether_link_wan","3","selected"); %>>100 Mbps, Half Duplex: AN</option>
                                                    <option value="4" <% nvram_match_x("","ether_link_wan","4","selected"); %>>10 Mbps, Full Duplex: AN</option>
                                                    <option value="5" <% nvram_match_x("","ether_link_wan","5","selected"); %>>10 Mbps, Half Duplex: AN</option>
                                                    <option value="6" <% nvram_match_x("","ether_link_wan","6","selected"); %>>100 Mbps, Full Duplex: Force</option>
                                                    <option value="7" <% nvram_match_x("","ether_link_wan","7","selected"); %>>100 Mbps, Half Duplex: Force</option>
                                                    <option value="8" <% nvram_match_x("","ether_link_wan","8","selected"); %>>10 Mbps, Full Duplex: Force</option>
                                                    <option value="9" <% nvram_match_x("","ether_link_wan","9","selected"); %>>10 Mbps, Half Duplex: Force</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_wan"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 1</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan1" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_flow_lan1", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("","ether_flow_lan1", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("","ether_flow_lan1", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan1" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_link_lan1","0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("","ether_link_lan1","1","selected"); %>>1000 Mbps, Full Duplex: AN</option>
                                                    <option value="2" <% nvram_match_x("","ether_link_lan1","2","selected"); %>>100 Mbps, Full Duplex: AN</option>
                                                    <option value="3" <% nvram_match_x("","ether_link_lan1","3","selected"); %>>100 Mbps, Half Duplex: AN</option>
                                                    <option value="4" <% nvram_match_x("","ether_link_lan1","4","selected"); %>>10 Mbps, Full Duplex: AN</option>
                                                    <option value="5" <% nvram_match_x("","ether_link_lan1","5","selected"); %>>10 Mbps, Half Duplex: AN</option>
                                                    <option value="6" <% nvram_match_x("","ether_link_lan1","6","selected"); %>>100 Mbps, Full Duplex: Force</option>
                                                    <option value="7" <% nvram_match_x("","ether_link_lan1","7","selected"); %>>100 Mbps, Half Duplex: Force</option>
                                                    <option value="8" <% nvram_match_x("","ether_link_lan1","8","selected"); %>>10 Mbps, Full Duplex: Force</option>
                                                    <option value="9" <% nvram_match_x("","ether_link_lan1","9","selected"); %>>10 Mbps, Half Duplex: Force</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan1"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 2</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan2" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_flow_lan2", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("","ether_flow_lan2", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("","ether_flow_lan2", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan2" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_link_lan2","0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("","ether_link_lan2","1","selected"); %>>1000 Mbps, Full Duplex: AN</option>
                                                    <option value="2" <% nvram_match_x("","ether_link_lan2","2","selected"); %>>100 Mbps, Full Duplex: AN</option>
                                                    <option value="3" <% nvram_match_x("","ether_link_lan2","3","selected"); %>>100 Mbps, Half Duplex: AN</option>
                                                    <option value="4" <% nvram_match_x("","ether_link_lan2","4","selected"); %>>10 Mbps, Full Duplex: AN</option>
                                                    <option value="5" <% nvram_match_x("","ether_link_lan2","5","selected"); %>>10 Mbps, Half Duplex: AN</option>
                                                    <option value="6" <% nvram_match_x("","ether_link_lan2","6","selected"); %>>100 Mbps, Full Duplex: Force</option>
                                                    <option value="7" <% nvram_match_x("","ether_link_lan2","7","selected"); %>>100 Mbps, Half Duplex: Force</option>
                                                    <option value="8" <% nvram_match_x("","ether_link_lan2","8","selected"); %>>10 Mbps, Full Duplex: Force</option>
                                                    <option value="9" <% nvram_match_x("","ether_link_lan2","9","selected"); %>>10 Mbps, Half Duplex: Force</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan2"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 3</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan3" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_flow_lan3", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("","ether_flow_lan3", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("","ether_flow_lan3", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan3" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_link_lan3","0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("","ether_link_lan3","1","selected"); %>>1000 Mbps, Full Duplex: AN</option>
                                                    <option value="2" <% nvram_match_x("","ether_link_lan3","2","selected"); %>>100 Mbps, Full Duplex: AN</option>
                                                    <option value="3" <% nvram_match_x("","ether_link_lan3","3","selected"); %>>100 Mbps, Half Duplex: AN</option>
                                                    <option value="4" <% nvram_match_x("","ether_link_lan3","4","selected"); %>>10 Mbps, Full Duplex: AN</option>
                                                    <option value="5" <% nvram_match_x("","ether_link_lan3","5","selected"); %>>10 Mbps, Half Duplex: AN</option>
                                                    <option value="6" <% nvram_match_x("","ether_link_lan3","6","selected"); %>>100 Mbps, Full Duplex: Force</option>
                                                    <option value="7" <% nvram_match_x("","ether_link_lan3","7","selected"); %>>100 Mbps, Half Duplex: Force</option>
                                                    <option value="8" <% nvram_match_x("","ether_link_lan3","8","selected"); %>>10 Mbps, Full Duplex: Force</option>
                                                    <option value="9" <% nvram_match_x("","ether_link_lan3","9","selected"); %>>10 Mbps, Half Duplex: Force</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan3"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 4</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan4" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_flow_lan4", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("","ether_flow_lan4", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("","ether_flow_lan4", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan4" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_link_lan4","0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("","ether_link_lan4","1","selected"); %>>1000 Mbps, Full Duplex: AN</option>
                                                    <option value="2" <% nvram_match_x("","ether_link_lan4","2","selected"); %>>100 Mbps, Full Duplex: AN</option>
                                                    <option value="3" <% nvram_match_x("","ether_link_lan4","3","selected"); %>>100 Mbps, Half Duplex: AN</option>
                                                    <option value="4" <% nvram_match_x("","ether_link_lan4","4","selected"); %>>10 Mbps, Full Duplex: AN</option>
                                                    <option value="5" <% nvram_match_x("","ether_link_lan4","5","selected"); %>>10 Mbps, Half Duplex: AN</option>
                                                    <option value="6" <% nvram_match_x("","ether_link_lan4","6","selected"); %>>100 Mbps, Full Duplex: Force</option>
                                                    <option value="7" <% nvram_match_x("","ether_link_lan4","7","selected"); %>>100 Mbps, Half Duplex: Force</option>
                                                    <option value="8" <% nvram_match_x("","ether_link_lan4","8","selected"); %>>10 Mbps, Full Duplex: Force</option>
                                                    <option value="9" <% nvram_match_x("","ether_link_lan4","9","selected"); %>>10 Mbps, Half Duplex: Force</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan4"></td>
                                        </tr>
                                        <tr>
                                            <td colspan="3" style="border-top: 0 none;">
                                                <br/>
                                                <center><input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" /></center>
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

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
	init_itoggle('ether_eee');
});

</script>
<script>

<% lanlink(); %>

<% hardware_pins(); %>

var id_update_status = 0;

function initial(){
	var id_menu = 5;
	if(get_ap_mode()){
		id_menu = 4;
		if (lan_proto == '1')
			id_menu--;
	}

	show_banner(1);
	show_menu(5,3,id_menu);
	show_footer();

	fill_ports();
	show_links();

	id_update_status = setTimeout("update_ether_status();", 3000);
}

function fill_ports(){
	var i;
	var num_ephy = support_num_ephy();
	var switch_type = support_switch_type();
	var has_wan_1g = 1;
	var has_lan_1g = 1;

	if (switch_type >= 10 && !support_ephy_w1000())
		has_wan_1g = 0;
	if (switch_type >= 10 && !support_ephy_l1000())
		has_lan_1g = 0;

	if (switch_type == 10 || switch_type == 11)
		showhide_div("row_ether_jumbo", 0);

	if (switch_type > 1){
		document.form.ether_jumbo.options[1].text = "Up to 9000 bytes";
		showhide_div("row_ether_green", 0);
	}

	fill_port_flow("ether_flow_wan", has_wan_1g, 0);
	fill_port_link("ether_link_wan", has_wan_1g, 0);

	for (i=1;i<num_ephy;i++){
		var ln = i.toString();
		fill_port_flow("ether_flow_lan"+ln, has_lan_1g, i);
		fill_port_link("ether_link_lan"+ln, has_lan_1g, i);
		if (i > 1)
			showhide_div("tbl_ephy_l"+ln, 1);
	}
}

function show_links(){
	var i, led0, led1;
	var num_ephy = support_num_ephy();
	var switch_type = support_switch_type();

	led0 = (switch_type >= 10) ? 1 : 3;
	led1 = (switch_type >= 10) ? 2 : 0;

	if (support_led_phy() > 0)
		led0 = parseInt("<% nvram_get_x("","ether_led0"); %>");
	if (support_led_phy() > 1)
		led1 = parseInt("<% nvram_get_x("","ether_led1"); %>");

	show_port_link("linkstate_wan", 0, led0, led1);

	for (i=1;i<num_ephy;i++){
		var ln = i.toString();
		show_port_link("linkstate_lan"+ln, i, led0, led1);
	}
}

function fill_port_flow(oname,ins1g,pid){
	var idx = 0;
	var o = document.form[oname];
	if (o === undefined)
		return;
	if (typeof(ether_flow_mode) === 'function')
		idx = ether_flow_mode(pid);
	add_option(o, "TX/RX", "0", idx==0);
	if (ins1g)
		add_option(o, "TX (Asymmetric Pause)", "1", idx==1);
	add_option(o, "Disabled", "2", idx==2);
}

function fill_port_link(oname,ins1g,pid){
	var idx = 0;
	var o = document.form[oname];
	if (o === undefined)
		return;
	if (typeof(ether_link_mode) === 'function')
		idx = ether_link_mode(pid);
	add_option(o, "Auto", "0", idx==0);
	if (ins1g)
		add_option(o, "1000 Mbps, Full Duplex: [AN]", "1", idx==1);
	add_option(o, "100 Mbps, Full Duplex: [AN]", "2", idx==2);
	add_option(o, "100 Mbps, Half Duplex: [AN]", "3", idx==3);
	add_option(o, "10 Mbps, Full Duplex: [AN]", "4", idx==4);
	add_option(o, "10 Mbps, Half Duplex: [AN]", "5", idx==5);
	add_option(o, "100 Mbps, Full Duplex: [Force]", "6", idx==6);
	add_option(o, "100 Mbps, Half Duplex: [Force]", "7", idx==7);
	add_option(o, "10 Mbps, Full Duplex: [Force]", "8", idx==8);
	add_option(o, "10 Mbps, Half Duplex: [Force]", "9", idx==9);
	add_option(o, "Power Off", "15", idx==15);
}

function show_port_link(oname,idx,led0,led1){
	var arr_speeds = [1000, 100, 10, 100, 1000, 100, 10, 0, 0, 0, 0, 0];
	var port_text = "No link";
	var port_speed = 0;
	var o = document.getElementById(oname);
	if (o === null)
		return;
	if (typeof(ether_link_status) === 'function') {
		port_text = ether_link_status(idx);
		port_speed = parseInt(port_text);
	}
	o.innerHTML = '<span class="label ' + (port_speed == arr_speeds[led1] ? 'label-warning">' : (port_speed == arr_speeds[led0] ? 'label-success">' : 'label-info">')) + port_text + '</span>';
}

function applyRule(){
	showLoading();

	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "Advanced_Switch_Content.asp";
	document.form.next_page.value = "";

	document.form.submit();
}

function update_ether_status(){
	clearTimeout(id_update_status);
	$j.ajax({
		url: '/status_lanlink.asp',
		dataType: 'script',
		cache: true,
		error: function(xhr){
			;
		},
		success: function(response){
			show_links();
			id_update_status = setTimeout("update_ether_status();", 3000);
		}
	});
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
                                        <tr id="row_ether_jumbo">
                                            <th><#SwitchJumbo#></th>
                                            <td>
                                                <select name="ether_jumbo" class="input">
                                                    <option value="0" <% nvram_match_x("","ether_jumbo", "0","selected"); %>>Up to 1536 bytes</option>
                                                    <option value="1" <% nvram_match_x("","ether_jumbo", "1","selected"); %>>Up to 16000 bytes</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ether_green">
                                            <th width="50%"><#btn_Enable#> Green Ethernet?</th>
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
                                        <tr>
                                            <th width="50%"><#btn_Enable#> Energy Efficient Ethernet (802.3az)?</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_eee_on_of">
                                                        <input type="checkbox" id="ether_eee_fake" <% nvram_match_x("", "ether_eee", "1", "value=1 checked"); %><% nvram_match_x("", "ether_eee", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_eee" id="ether_eee_1" class="input" <% nvram_match_x("", "ether_eee", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_eee" id="ether_eee_0" class="input" <% nvram_match_x("", "ether_eee", "0", "checked"); %> /><#checkbox_No#>
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
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_wan" class="input">
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
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan1" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan1"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l2" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 2</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan2" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan2" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan2"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l3" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 3</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan3" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan3" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan3"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l4" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 4</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan4" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan4" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan4"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l5" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 5</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan5" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan5" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan5"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l6" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 6</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan6" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan6" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan6"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ephy_l7" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN 7</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchFlow#></th>
                                            <td>
                                                <select name="ether_flow_lan7" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchLink#></th>
                                            <td>
                                                <select name="ether_link_lan7" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchState#></th>
                                            <td id="linkstate_lan7"></td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border: 0 none;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
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

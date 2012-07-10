<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_5#></title>
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
        $j('#ether_green_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ether_green_fake").attr("checked", "checked").attr("value", 1);
                $j("#ether_green_1").attr("checked", "checked");
                $j("#ether_green_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#ether_green_fake").removeAttr("checked").attr("value", 0);
                $j("#ether_green_0").attr("checked", "checked");
                $j("#ether_green_1").removeAttr("checked");
            }
        });
        $j("#ether_green_on_of label.itoggle").css("background-position", $j("input#ether_green_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>
<script>

<% lanlink(); %>

function initial(){
	final_flag = 1;	// for the function in general.js
	
	show_banner(1);
	
	if(sw_mode == "3")
		show_menu(5,3,2);
	else
		show_menu(5,3,5);
	
	show_footer();
	
	enable_auto_hint(4, 2);

    var arr_speeds          = [1000, 100, 10, 100, 1000, 100, 10];
	var led_color_green     = "<% nvram_get_x("LANHostConfig","ether_led0"); %>";
	var led_color_orange    = "<% nvram_get_x("LANHostConfig","ether_led1"); %>";
    var wan_speed           = parseInt(lanlink_etherlink_wan());
    var lan1_speed          = parseInt(lanlink_etherlink_lan1());
    var lan2_speed          = parseInt(lanlink_etherlink_lan2());
    var lan3_speed          = parseInt(lanlink_etherlink_lan3());
    var lan4_speed          = parseInt(lanlink_etherlink_lan4());

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

<body onload="initial();" onunLoad="disable_auto_hint(4, 2);return unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="hiddenMask" class="popup_bg" style="position: absolute; margin-left: -10000px;">
        <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
            <tr>
            <td>
                <div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
                    <br/>
                    <br/>
            </div>
              <div class="drImg"><img src="images/DrsurfImg.gif"></div>
                <div style="height:70px;"></div>
            </td>
            </tr>
        </table>
    <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
    <input type="hidden" name="current_page" value="Advanced_Switch_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;">Ethernet switch advanced control - WAN and LAN ports</div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">Base control</th>
                                        </tr>
                                        <tr>
                                            <th width="50%">Jumbo frames accept:</th>
                                            <td>
                                                <select name="ether_jumbo" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_jumbo", "0","selected"); %>>Up to 1536 bytes</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_jumbo", "1","selected"); %>>Up to 16000 bytes</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <th>Green LED action:</th>
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
                                            </select>
                                        </td>
                                        <tr>
                                            <th>Yellow LED action:</th>
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
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>Green Ethernet?</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_green_on_of">
                                                        <input type="checkbox" id="ether_green_fake" <% nvram_match_x("WLANConfig11b", "ether_green", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "ether_green", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_green" id="ether_green_1" class="input" <% nvram_match_x("LANHostConfig", "ether_green", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_green" id="ether_green_0" class="input" <% nvram_match_x("LANHostConfig", "ether_green", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">WAN Port</th>
                                            <td>Link Status</td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Flow Control:</th>
                                            <td width="30%">
                                                <select name="ether_flow_wan" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_flow_wan", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_flow_wan", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_flow_wan", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                            <td rowspan="2" id="linkstate_wan"></td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Link Speed:</th>
                                            <td width="30%">
                                                <select name="ether_link_wan" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_link_wan", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_link_wan", "1","selected"); %>>1000 Mbps, Full Duplex</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_link_wan", "2","selected"); %>>100 Mbps, Full Duplex</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_link_wan", "3","selected"); %>>100 Mbps, Half Duplex</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_link_wan", "4","selected"); %>>10 Mbps, Full Duplex</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_link_wan", "5","selected"); %>>10 Mbps, Half Duplex</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN1 Port</th>
                                            <td>Link Status</td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Flow Control:</th>
                                            <td width="30%">
                                                <select name="ether_flow_lan1" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan1", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                            <td rowspan="2"  id="linkstate_lan1"></td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Link Speed:</th>
                                            <td width="30%">
                                                <select name="ether_link_lan1" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan1", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan1", "1","selected"); %>>1000 Mbps, Full Duplex</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan1", "2","selected"); %>>100 Mbps, Full Duplex</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan1", "3","selected"); %>>100 Mbps, Half Duplex</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan1", "4","selected"); %>>10 Mbps, Full Duplex</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan1", "5","selected"); %>>10 Mbps, Half Duplex</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN2 Port</th>
                                            <td>Link Status</td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Flow Control:</th>
                                            <td width="30%">
                                                <select name="ether_flow_lan2" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan2", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                            <td rowspan="2" align="left" id="linkstate_lan2"></td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Link Speed:</th>
                                            <td width="30%">
                                                <select name="ether_link_lan2" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan2", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan2", "1","selected"); %>>1000 Mbps, Full Duplex</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan2", "2","selected"); %>>100 Mbps, Full Duplex</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan2", "3","selected"); %>>100 Mbps, Half Duplex</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan2", "4","selected"); %>>10 Mbps, Full Duplex</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan2", "5","selected"); %>>10 Mbps, Half Duplex</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN3 Port</th>
                                            <td>Link Status</td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Flow Control:</th>
                                            <td width="30%">
                                                <select name="ether_flow_lan3" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan3", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                            <td rowspan="2" align="left" id="linkstate_lan3"></td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Link Speed:</th>
                                            <td width="30%">
                                                <select name="ether_link_lan3" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan3", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan3", "1","selected"); %>>1000 Mbps, Full Duplex</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan3", "2","selected"); %>>100 Mbps, Full Duplex</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan3", "3","selected"); %>>100 Mbps, Half Duplex</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan3", "4","selected"); %>>10 Mbps, Full Duplex</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan3", "5","selected"); %>>10 Mbps, Half Duplex</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">LAN4 Port</th>
                                            <td>Link Status</td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Flow Control:</th>
                                            <td width="30%">
                                                <select name="ether_flow_lan4" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "0","selected"); %>>RX/TX</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "1","selected"); %>>RX (Asymmetric Pause)</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_flow_lan4", "2","selected"); %>>Disabled</option>
                                                </select>
                                            </td>
                                            <td rowspan="2" align="left" id="linkstate_lan4"></td>
                                        </tr>
                                        <tr>
                                            <th width="30%">Link Speed:</th>
                                            <td width="30%">
                                                <select name="ether_link_lan4" class="input">
                                                    <option value="0" <% nvram_match_x("LANHostConfig","ether_link_lan4", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("LANHostConfig","ether_link_lan4", "1","selected"); %>>1000 Mbps, Full Duplex</option>
                                                    <option value="2" <% nvram_match_x("LANHostConfig","ether_link_lan4", "2","selected"); %>>100 Mbps, Full Duplex</option>
                                                    <option value="3" <% nvram_match_x("LANHostConfig","ether_link_lan4", "3","selected"); %>>100 Mbps, Half Duplex</option>
                                                    <option value="4" <% nvram_match_x("LANHostConfig","ether_link_lan4", "4","selected"); %>>10 Mbps, Full Duplex</option>
                                                    <option value="5" <% nvram_match_x("LANHostConfig","ether_link_lan4", "5","selected"); %>>10 Mbps, Half Duplex</option>
                                                </select>
                                            </td>
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

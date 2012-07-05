<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_6#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state_5g.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#radio_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wl_radio_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#wl_radio_x_1").attr("checked", "checked");
                $j("#wl_radio_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wl_radio_x_fake").removeAttr("checked").attr("value", 0);
                $j("#wl_radio_x_0").attr("checked", "checked");
                $j("#wl_radio_x_1").removeAttr("checked");
            }
        });
        $j("#radio_on_of label.itoggle").css("background-position", $j("input#wl_radio_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#wl_ap_isolate_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wl_ap_isolate_fake").attr("checked", "checked").attr("value", 1);
                $j("#wl_ap_isolate_1").attr("checked", "checked");
                $j("#wl_ap_isolate_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wl_ap_isolate_fake").removeAttr("checked").attr("value", 0);
                $j("#wl_ap_isolate_0").attr("checked", "checked");
                $j("#wl_ap_isolate_1").removeAttr("checked");
            }
        });
        $j("#wl_ap_isolate_on_of label.itoggle").css("background-position", $j("input#wl_ap_isolate_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#wl_mbssid_isolate_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wl_mbssid_isolate_fake").attr("checked", "checked").attr("value", 1);
                $j("#wl_mbssid_isolate_1").attr("checked", "checked");
                $j("#wl_mbssid_isolate_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wl_mbssid_isolate_fake").removeAttr("checked").attr("value", 0);
                $j("#wl_mbssid_isolate_0").attr("checked", "checked");
                $j("#wl_mbssid_isolate_1").removeAttr("checked");
            }
        });
        $j("#wl_mbssid_isolate_on_of label.itoggle").css("background-position", $j("input#wl_mbssid_isolate_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });


</script>

<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var check_hwnat = '<% check_hwnat(); %>';
var hwnat = '<% nvram_get_x("",  "hwnat"); %>';

function initial(){

	show_banner(1);
	
	show_menu(5,2,6);
	
	show_footer();
	
	enable_auto_hint(3, 20);

	load_body();
	if(document.form.wl_gmode.value == "1"){	//n only
		inputCtrl(document.form.wl_HT_OpMode, 1);
		$("wl_wme_tr").style.display = "none";
		
	}else if(document.form.wl_gmode.value == "2"){	//Auto
		$("wl_HT_OpMode").value = "0";
		inputCtrl(document.form.wl_HT_OpMode, 0);
		$("wl_wme_tr").style.display = "none";
			
	}else{
		$("wl_HT_OpMode").value = "0";
		inputCtrl(document.form.wl_HT_OpMode, 0);
		$("wl_wme_tr").style.display = "";
	}
}

function applyRule(){

	if(validForm()){
		updateDateTime(document.form.current_page.value);
		
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WAdvanced_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
    if(!validate_range(document.form.wl_frag, 256, 2346)
            || !validate_range(document.form.wl_rts, 0, 2347)
            || !validate_range(document.form.wl_dtim, 1, 255)
            || !validate_range(document.form.wl_bcn, 20, 1000)
            )
        return false;

    if(document.form.wl_radio_x[0].checked){
        if(!validate_timerange(document.form.wl_radio_time_x_starthour, 0)
                || !validate_timerange(document.form.wl_radio_time_x_startmin, 1)
                || !validate_timerange(document.form.wl_radio_time_x_endhour, 2)
                || !validate_timerange(document.form.wl_radio_time_x_endmin, 3)
                )
            return false;

        var starttime = eval(document.form.wl_radio_time_x_starthour.value + document.form.wl_radio_time_x_startmin.value);
        var endtime = eval(document.form.wl_radio_time_x_endhour.value + document.form.wl_radio_time_x_endmin.value);

        if(starttime == endtime){
            alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
                document.form.wl_radio_time_x_starthour.focus();
                document.form.wl_radio_time_x_starthour.select;
            return false;
        }
    }

    if((document.form.wl_radio_x[0].checked ==true)
        && (document.form.wl_radio_date_x_Sun.checked ==false)
        && (document.form.wl_radio_date_x_Mon.checked ==false)
        && (document.form.wl_radio_date_x_Tue.checked ==false)
        && (document.form.wl_radio_date_x_Wed.checked ==false)
        && (document.form.wl_radio_date_x_Thu.checked ==false)
        && (document.form.wl_radio_date_x_Fri.checked ==false)
        && (document.form.wl_radio_date_x_Sat.checked ==false)){
            alert("<#WLANConfig11b_x_RadioEnableDate_itemname#><#JS_fieldblank#>");
            document.form.wl_radio_x[0].checked=false;
            document.form.wl_radio_x[1].checked=true;
            return false;
    }

    return true;
}

function done_validating(action){
	refreshpage();
}
</script>

<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(3, 16);return unload_body();">
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
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">

<input type="hidden" name="wl_gmode" value="<% nvram_get_x("WLANConfig11b","wl_gmode"); %>">
<input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>">

<input type="hidden" name="current_page" value="Advanced_WAdvanced_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANAuthentication11a;WLANConfig11b;LANHostConfig;PrinterStatus;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="wl_radio_date_x" value="<% nvram_get_x("WLANConfig11b","wl_radio_date_x"); %>">
<input type="hidden" name="wl_radio_time_x" value="<% nvram_get_x("WLANConfig11b","wl_radio_time_x"); %>">
<input type="hidden" name="hwnat" value="<% nvram_get_x("PrinterStatus","hwnat"); %>">

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
                        <h2 class="box_head round_top">Wireless - Professional (5GHz)</h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>
                                <div class="alert alert-info" style="margin: 10px;"><#WLANConfig11b_display5_sectiondesc#></div>

                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 1);"><#WLANConfig11b_x_RadioEnable_itemname#></a></th>
                                        <td style="border-top: 0 none;">
                                            <div class="main_itoggle">
                                                <div id="radio_on_of">
                                                    <input type="checkbox" id="wl_radio_x_fake" <% nvram_match_x("WLANConfig11b","wl_radio_x", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b","wl_radio_x", "0", "value=0"); %>>
                                                </div>
                                            </div>
                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" name="wl_radio_x" id="wl_radio_x_1" value="1" <% nvram_match_x("WLANConfig11b","wl_radio_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                <input type="radio" name="wl_radio_x" id="wl_radio_x_0" value="0" <% nvram_match_x("WLANConfig11b","wl_radio_x", "0", "checked"); %> /><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 2);"><#WLANConfig11b_x_RadioEnableDate_itemname#></a></th>
                                        <td>
                                            <div class="controls">
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Mon" onChange="return changeDate();" />Mon</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Tue" onChange="return changeDate();" />Tue</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Wed" onChange="return changeDate();" />Wed</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Thu" onChange="return changeDate();" />Thu</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Fri" onChange="return changeDate();" />Fri</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Sat" onChange="return changeDate();" />Sat</label>
                                                <label class="checkbox inline"><input type="checkbox" class="input" name="wl_radio_date_x_Sun" onChange="return changeDate();" />Sun</label>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 3);"><#WLANConfig11b_x_RadioEnableTime_itemname#></a></th>
                                        <td>
                                            <input type="text" maxlength="2" style="width: 25px;" size="2" name="wl_radio_time_x_starthour" onKeyPress="return is_number(this)" />:
                                            <input type="text" maxlength="2" style="width: 25px;" size="2" name="wl_radio_time_x_startmin" onKeyPress="return is_number(this)" />&nbsp;-&nbsp;
                                            <input type="text" maxlength="2" style="width: 25px;" size="2" name="wl_radio_time_x_endhour" onKeyPress="return is_number(this)" />:
                                            <input type="text" maxlength="2" style="width: 25px;" size="2" name="wl_radio_time_x_endmin" onKeyPress="return is_number(this)" />
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="wl_ap_isolate_on_of">
                                                    <input type="checkbox" id="wl_ap_isolate_fake" <% nvram_match_x("WLANConfig11b","wl_ap_isolate", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b","wl_ap_isolate", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" name="wl_ap_isolate" id="wl_ap_isolate_1" value="1" <% nvram_match_x("WLANConfig11b","wl_ap_isolate", "1", "checked"); %>><#checkbox_Yes#>
                                                <input type="radio" name="wl_ap_isolate" id="wl_ap_isolate_0" value="0" <% nvram_match_x("WLANConfig11b","wl_ap_isolate", "0", "checked"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WIFIGuestIsolate#></a></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="wl_mbssid_isolate_on_of">
                                                    <input type="checkbox" id="wl_mbssid_isolate_fake" <% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" value="1" id="wl_mbssid_isolate_1" name="wl_mbssid_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "1", "checked"); %>/><#checkbox_Yes#>
                                                <input type="radio" value="0" id="wl_mbssid_isolate_0" name="wl_mbssid_isolate" class="input" <% nvram_match_x("WLANConfig11b","wl_mbssid_isolate", "0", "checked"); %>/><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
                                        <td>
                                            <select name="wl_mcastrate" class="input">
                                                <option value="0" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5~15 Mbps</option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 13~30 Mbps</option>
                                                <option value="2" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5~45 Mbps</option>
                                                <option value="3" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 13~30 Mbps</option>
                                                <option value="4" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 26~60 Mbps</option>
                                                <option value="5" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                <option value="6" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
                                                <option value="7" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                <option value="8" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <!-- <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 8);"><#WLANConfig11b_DataRate_itemname#></a></th>
                                        <td>
                                            <select name="wl_rateset" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_rateset')">
                                                <option value="default" <% nvram_match_x("WLANConfig11b","wl_rateset", "default","selected"); %>>Default</option>
                                                <option value="all" <% nvram_match_x("WLANConfig11b","wl_rateset", "all","selected"); %>>All</option>
                                                <option value="12" <% nvram_match_x("WLANConfig11b","wl_rateset", "12","selected"); %>>1, 2 Mbps</option>
                                            </select>
                                        </td>
                                    </tr>-->
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 9);"><#WLANConfig11b_x_Frag_itemname#></a></th>
                                        <td>
                                            <input type="text" maxlength="5" size="5" name="wl_frag" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_frag"); %>" onKeyPress="return is_number(this)" onChange="page_changed()" onBlur="validate_range(this, 256, 2346)">
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 10);"><#WLANConfig11b_x_RTS_itemname#></a></th>
                                        <td>
                                            <input type="text" maxlength="5" size="5" name="wl_rts" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_rts"); %>" onKeyPress="return is_number(this)">
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 11);"><#WLANConfig11b_x_DTIM_itemname#></a></th>
                                        <td>
                                            <input type="text" maxlength="5" size="5" name="wl_dtim" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_dtim"); %>" onKeyPress="return is_number(this)"  onBlur="validate_range(this, 1, 255)">
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 12);"><#WLANConfig11b_x_Beacon_itemname#></a></th>
                                        <td>
                                            <input type="text" maxlength="5" size="5" name="wl_bcn" class="input" value="<% nvram_get_x("WLANConfig11b", "wl_bcn"); %>" onKeyPress="return is_number(this)" onBlur="validate_range(this, 20, 1000)">
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 13);"><#WLANConfig11b_x_TxBurst_itemname#></a></th>
                                        <td>
                                            <select name="wl_TxBurst" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_TxBurst')">
                                                <option value="0" <% nvram_match_x("WLANConfig11b","wl_TxBurst", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b","wl_TxBurst", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 16);"><#WLANConfig11b_x_PktAggregate_itemname#></a></th>
                                        <td>
                                            <select name="wl_PktAggregate" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_PktAggregate')">
                                                <option value="0" <% nvram_match_x("WLANConfig11b","wl_PktAggregate", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b","wl_PktAggregate", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <!--Greenfield by Lock Add in 2008.10.01 -->
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 19);"><#WLANConfig11b_x_HT_OpMode_itemname#></a></th>
                                        <td>
                                            <select class="input" id="wl_HT_OpMode" name="wl_HT_OpMode" onChange="return change_common(this, 'WLANConfig11b', 'wl_HT_OpMode')">
                                                <option value="0" <% nvram_match_x("WLANConfig11b","wl_HT_OpMode", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b","wl_HT_OpMode", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <!--Greenfield by Lock Add in 2008.10.01 -->
                                    <tr id="wl_wme_tr">
                                      <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 14);"><#WLANConfig11b_x_WMM_itemname#></a></th>
                                      <td>
                                        <select name="wl_wme" id="wl_wme" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme')">
                                          <option value="0" <% nvram_match_x("WLANConfig11b", "wl_wme", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                          <option value="1" <% nvram_match_x("WLANConfig11b", "wl_wme", "1", "selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                        </select>
                                      </td>
                                    </tr>
                                    <tr>
                                      <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 15);"><#WLANConfig11b_x_NOACK_itemname#></a></th>
                                      <td>
                                        <select name="wl_wme_no_ack" id="wl_wme_no_ack" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme_no_ack')">
                                          <option value="off" <% nvram_match_x("WLANConfig11b","wl_wme_no_ack", "off","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                          <option value="on" <% nvram_match_x("WLANConfig11b","wl_wme_no_ack", "on","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                        </select>
                                      </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 17);"><#WLANConfig11b_x_APSD_itemname#></a></th>
                                        <td>
                                          <select name="wl_APSDCapable" class="input" onchange="return change_common(this, 'WLANConfig11b', 'wl_APSDCapable')">
                                            <option value="0" <% nvram_match_x("WLANConfig11b","wl_APSDCapable", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                            <option value="1" <% nvram_match_x("WLANConfig11b","wl_APSDCapable", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                          </select>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 18);"><#WLANConfig11b_x_DLS_itemname#></a></th>
                                        <td>
                                            <select name="wl_DLSCapable" class="input" onChange="return change_common(this, 'WLANConfig11b', 'wl_DLSCapable')">
                                                <option value="0" <% nvram_match_x("WLANConfig11b","wl_DLSCapable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b","wl_DLSCapable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td style="margin-top: 10px;">
                                            <br />
                                            <input class="btn btn-info" type="button"  value="<#GO_2G#>" onclick="location.href='Advanced_WAdvanced2g_Content.asp';">
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
</body>
</html>

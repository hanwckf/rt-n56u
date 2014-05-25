<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_6#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/wireless.js"></script>
<script type="text/javascript" src="/help_wl.js"></script>
<script type="text/javascript" src="/popup.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#wl_greenap_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wl_greenap_fake").attr("checked", "checked").attr("value", 1);
                $j("#wl_greenap_1").attr("checked", "checked");
                $j("#wl_greenap_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wl_greenap_fake").removeAttr("checked").attr("value", 0);
                $j("#wl_greenap_0").attr("checked", "checked");
                $j("#wl_greenap_1").removeAttr("checked");
            }
        });
        $j("#wl_greenap_on_of label.itoggle").css("background-position", $j("input#wl_greenap_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

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

<% login_state_hook(); %>

function initial(){

	show_banner(1);
	show_menu(5,2,6);
	show_footer();

	load_body();

	if (support_5g_stream_tx()<3){
		document.form.wl_stream_tx.remove(2);
	}

	if (support_5g_stream_rx()<3){
		document.form.wl_stream_rx.remove(2);
	}

	change_wmm();
}

function change_wmm() {
	var gmode = document.form.wl_gmode.value;
	if (document.form.wl_wme.value == "0") {
		$("row_wme_no_ack").style.display = "none";
		$("row_apsd_cap").style.display = "none";
	}else{
		if (gmode != "0") { // != A only
			$("row_wme_no_ack").style.display = "none";
		} else {
			$("row_wme_no_ack").style.display = "";
		}
		$("row_apsd_cap").style.display = "";
	}
	if(gmode == "1" || gmode == "3") { // N, N/AC
		$("row_greenfield").style.display = "";
	}else{
		$("row_greenfield").style.display = "none";
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WAdvanced_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_range(document.form.wl_frag, 256, 2346)
		|| !validate_range(document.form.wl_rts, 1, 2347)
		|| !validate_range(document.form.wl_dtim, 1, 255)
		|| !validate_range(document.form.wl_bcn, 20, 1000))
		return false;

	return true;
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
    <input type="hidden" name="current_page" value="Advanced_WAdvanced_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="WLANAuthentication11a;WLANConfig11a;LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="wl_gmode" value="<% nvram_get_x("","wl_gmode"); %>">

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
                            <h2 class="box_head round_top"><#menu5_1#> - <#menu5_1_6#> (5GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#WLANConfig11b_display5_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#WIFIStreamTX#></th>
                                            <td>
                                                <select name="wl_stream_tx" class="input">
                                                    <option value="1" <% nvram_match_x("", "wl_stream_tx", "1", "selected"); %>>1T (150Mbps)</option>
                                                    <option value="2" <% nvram_match_x("", "wl_stream_tx", "2", "selected"); %>>2T (300Mbps)</option>
                                                    <option value="3" <% nvram_match_x("", "wl_stream_tx", "3", "selected"); %>>3T (450Mbps)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIStreamRX#></th>
                                            <td>
                                                <select name="wl_stream_rx" class="input">
                                                    <option value="1" <% nvram_match_x("", "wl_stream_rx", "1", "selected"); %>>1R (150Mbps)</option>
                                                    <option value="2" <% nvram_match_x("", "wl_stream_rx", "2", "selected"); %>>2R (300Mbps)</option>
                                                    <option value="3" <% nvram_match_x("", "wl_stream_rx", "3", "selected"); %>>3R (450Mbps)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIGreenAP#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_greenap_on_of">
                                                        <input type="checkbox" id="wl_greenap_fake" <% nvram_match_x("", "wl_greenap", "1", "value=1 checked"); %><% nvram_match_x("", "wl_greenap", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="wl_greenap" id="wl_greenap_1" class="input" <% nvram_match_x("", "wl_greenap", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="wl_greenap" id="wl_greenap_0" class="input" <% nvram_match_x("", "wl_greenap", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_ap_isolate_on_of">
                                                        <input type="checkbox" id="wl_ap_isolate_fake" <% nvram_match_x("","wl_ap_isolate", "1", "value=1 checked"); %><% nvram_match_x("","wl_ap_isolate", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="wl_ap_isolate" id="wl_ap_isolate_1" value="1" <% nvram_match_x("","wl_ap_isolate", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" name="wl_ap_isolate" id="wl_ap_isolate_0" value="0" <% nvram_match_x("","wl_ap_isolate", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WIFIGuestIsolate#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_mbssid_isolate_on_of">
                                                        <input type="checkbox" id="wl_mbssid_isolate_fake" <% nvram_match_x("","wl_mbssid_isolate", "1", "value=1 checked"); %><% nvram_match_x("","wl_mbssid_isolate", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_mbssid_isolate_1" name="wl_mbssid_isolate" class="input" <% nvram_match_x("","wl_mbssid_isolate", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_mbssid_isolate_0" name="wl_mbssid_isolate" class="input" <% nvram_match_x("","wl_mbssid_isolate", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 4);"><#WLANConfig11n_PremblesType_itemname#></a></th>
                                            <td>
                                                <select name="wl_preamble" class="input">
                                                    <option value="0" <% nvram_match_x("","wl_preamble", "0","selected"); %>>Long</option>
                                                    <option value="1" <% nvram_match_x("","wl_preamble", "1","selected"); %>>Short (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 9);"><#WLANConfig11b_x_Frag_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wl_frag" class="input" value="<% nvram_get_x("", "wl_frag"); %>" onKeyPress="return is_number(this)" onChange="page_changed()" onBlur="validate_range(this, 256, 2346)">
                                                &nbsp;<span style="color:#888;">[256..2346]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 10);"><#WLANConfig11b_x_RTS_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wl_rts" class="input" value="<% nvram_get_x("", "wl_rts"); %>" onKeyPress="return is_number(this)">
                                                &nbsp;<span style="color:#888;">[1..2347]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 11);"><#WLANConfig11b_x_DTIM_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="3" size="5" name="wl_dtim" class="input" value="<% nvram_get_x("", "wl_dtim"); %>" onKeyPress="return is_number(this)"  onBlur="validate_range(this, 1, 255)">
                                                &nbsp;<span style="color:#888;">[1..255]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 12);"><#WLANConfig11b_x_Beacon_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wl_bcn" class="input" value="<% nvram_get_x("", "wl_bcn"); %>" onKeyPress="return is_number(this)" onBlur="validate_range(this, 20, 1000)">
                                                &nbsp;<span style="color:#888;">[20..1000]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#SwitchIgmp#></th>
                                            <td>
                                                <select name="wl_IgmpSnEnable" class="input">
                                                    <option value="0" <% nvram_match_x("","wl_IgmpSnEnable", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","wl_IgmpSnEnable", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
                                            <td>
                                                <select name="wl_mcastrate" class="input">
                                                    <option value="0" <% nvram_match_x("", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 15 Mbps</option>
                                                    <option value="1" <% nvram_match_x("", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 30 Mbps (*)</option>
                                                    <option value="2" <% nvram_match_x("", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 45 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 30 Mbps</option>
                                                    <option value="4" <% nvram_match_x("", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 60 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
                                                    <option value="7" <% nvram_match_x("", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                    <option value="8" <% nvram_match_x("", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 13);"><#WLANConfig11b_x_TxBurst_itemname#></a></th>
                                            <td>
                                                <select name="wl_TxBurst" class="input" onChange="return change_common_wl(this, 'WLANConfig11a', 'wl_TxBurst')">
                                                    <option value="0" <% nvram_match_x("","wl_TxBurst", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","wl_TxBurst", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 16);"><#WLANConfig11b_x_PktAggregate_itemname#></a></th>
                                            <td>
                                                <select name="wl_PktAggregate" class="input" onChange="return change_common_wl(this, 'WLANConfig11a', 'wl_PktAggregate')">
                                                    <option value="0" <% nvram_match_x("","wl_PktAggregate", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","wl_PktAggregate", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIRDG#></th>
                                            <td>
                                                <select name="wl_HT_RDG" class="input">
                                                    <option value="0" <% nvram_match_x("","wl_HT_RDG", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","wl_HT_RDG", "1","selected"); %>><#btn_Enable#> (Ralink clients only)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIAutoBA#></th>
                                            <td>
                                                <select name="wl_HT_AutoBA" class="input">
                                                    <option value="0" <% nvram_match_x("","wl_HT_AutoBA", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","wl_HT_AutoBA", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_greenfield">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 19);"><#WLANConfig11b_x_HT_OpMode_itemname#></a></th>
                                            <td>
                                                <select class="input" id="wl_HT_OpMode" name="wl_HT_OpMode" onChange="return change_common_wl(this, 'WLANConfig11a', 'wl_HT_OpMode')">
                                                    <option value="0" <% nvram_match_x("","wl_HT_OpMode", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","wl_HT_OpMode", "1","selected"); %>><#btn_Enable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                          <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 14);"><#WLANConfig11b_x_WMM_itemname#></a></th>
                                          <td>
                                            <select name="wl_wme" id="wl_wme" class="input" onChange="change_wmm();">
                                              <option value="0" <% nvram_match_x("", "wl_wme", "0", "selected"); %>><#btn_Disable#></option>
                                              <option value="1" <% nvram_match_x("", "wl_wme", "1", "selected"); %>><#btn_Enable#> (*)</option>
                                            </select>
                                          </td>
                                        </tr>
                                        <tr id="row_wme_no_ack">
                                          <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 15);"><#WLANConfig11b_x_NOACK_itemname#></a></th>
                                          <td>
                                            <select name="wl_wme_no_ack" id="wl_wme_no_ack" class="input" onChange="return change_common_wl(this, 'WLANConfig11a', 'wl_wme_no_ack')">
                                              <option value="off" <% nvram_match_x("","wl_wme_no_ack", "off","selected"); %>><#btn_Disable#> (*)</option>
                                              <option value="on" <% nvram_match_x("","wl_wme_no_ack", "on","selected"); %>><#btn_Enable#></option>
                                            </select>
                                          </td>
                                        </tr>
                                        <tr id="row_apsd_cap">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 17);"><#WLANConfig11b_x_APSD_itemname#></a></th>
                                            <td>
                                              <select name="wl_APSDCapable" class="input" onchange="return change_common_wl(this, 'WLANConfig11a', 'wl_APSDCapable')">
                                                <option value="0" <% nvram_match_x("","wl_APSDCapable", "0","selected"); %> ><#btn_Disable#></option>
                                                <option value="1" <% nvram_match_x("","wl_APSDCapable", "1","selected"); %> ><#btn_Enable#> (*)</option>
                                              </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td width="50%" style="margin-top: 10px; border-top: 0 none;">
                                                <input class="btn btn-info" type="button"  value="<#GO_2G#>" onclick="location.href='Advanced_WAdvanced2g_Content.asp';">
                                            </td>
                                            <td style="border-top: 0 none;">
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

    <div id="footer"></div>
</div>
</body>
</html>

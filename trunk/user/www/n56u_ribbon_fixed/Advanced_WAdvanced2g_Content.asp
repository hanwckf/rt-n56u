<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_1_6#></title>
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
<script type="text/javascript" src="/wireless_2g.js"></script>
<script type="text/javascript" src="/help_wl.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('rt_greenap');
	init_itoggle('rt_ap_isolate');
});

</script>
<script>
var usb3_disable = '<% nvram_get_x("", "usb3_disable"); %>';

function initial(){
	show_banner(1);

	show_menu(5,1,6);

	show_footer();

	if (!support_5g_radio()) {
		document.form.goto5.style.display = "none";
		$("col_goto5").width = "33%";
	}

	if (typeof(support_2g_wid) === 'function'){
		wid = support_2g_wid();
		if (wid==7602||wid==7612){
			showhide_div("row_vga_clamp", 1);
			showhide_div("row_ldpc", 1);
		} else if (wid==7615){
			showhide_div("row_ldpc", 1);
		}
	}

	if (support_2g_stream_tx()<4) 
		document.form.rt_stream_tx.remove(3);
	if (support_2g_stream_tx()<3) 
		document.form.rt_stream_tx.remove(2);
	if (support_2g_stream_tx()<2) {
		document.form.rt_stream_tx.remove(1);
		showhide_div("row_greenap", 0);
	}

	if (support_2g_stream_rx()<4)
		document.form.rt_stream_rx.remove(3);
	if (support_2g_stream_rx()<3)
		document.form.rt_stream_rx.remove(2);
	if (support_2g_stream_rx()<2)
		document.form.rt_stream_rx.remove(1);

	if (!support_2g_inic_mii())
		showhide_div('row_mrate', 0);

	if (support_2g_turbo_qam())
		showhide_div('row_turbo_qam', 1);

	if (support_2g_airtimefairness())
		showhide_div('row_airtimefairness', 1);

	load_body();

	change_wmm();
}

function change_wmm() {
	var gm = document.form.rt_gmode.value;
	if (document.form.rt_wme.value == "0") {
		showhide_div("row_wme_no_ack", 0);
		showhide_div("row_apsd_cap", 0);
	}else{
		showhide_div("row_wme_no_ack", (gm == "5" || gm == "3" || gm == "2")?0:1);
		showhide_div("row_apsd_cap", 1);
	}
	showhide_div("row_greenfield", (gm == "3")?1:0);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WAdvanced2g_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_range(document.form.rt_frag, 256, 2346)
		|| !validate_range(document.form.rt_rts, 1, 2347)
		|| !validate_range(document.form.rt_dtim, 1, 255)
		|| !validate_range(document.form.rt_bcn, 20, 1000))
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
    <input type="hidden" name="current_page" value="Advanced_WAdvanced2g_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="WLANAuthentication11b;WLANConfig11b;LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="rt_gmode" value="<% nvram_get_x("","rt_gmode"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#menu5_1#> - <#menu5_1_6#> (2.4GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#WLANConfig11b_display5_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#WIFIStreamTX#></th>
                                            <td>
                                                <select name="rt_stream_tx" class="input">
                                                    <option value="1" <% nvram_match_x("", "rt_stream_tx", "1", "selected"); %>>1T (150Mbps)</option>
                                                    <option value="2" <% nvram_match_x("", "rt_stream_tx", "2", "selected"); %>>2T (300Mbps)</option>
                                                    <option value="3" <% nvram_match_x("", "rt_stream_tx", "3", "selected"); %>>3T (450Mbps)</option>
                                                    <option value="4" <% nvram_match_x("", "rt_stream_tx", "4", "selected"); %>>4T (600Mbps)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIStreamRX#></th>
                                            <td>
                                                <select name="rt_stream_rx" class="input">
                                                    <option value="1" <% nvram_match_x("", "rt_stream_rx", "1", "selected"); %>>1R (150Mbps)</option>
                                                    <option value="2" <% nvram_match_x("", "rt_stream_rx", "2", "selected"); %>>2R (300Mbps)</option>
                                                    <option value="3" <% nvram_match_x("", "rt_stream_rx", "3", "selected"); %>>3R (450Mbps)</option>
                                                    <option value="4" <% nvram_match_x("", "rt_stream_rx", "4", "selected"); %>>4R (600Mbps)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_vga_clamp" style="display:none">
                                            <th><#WIFIVgaClamp#></th>
                                            <td>
                                                <select name="rt_VgaClamp" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_VgaClamp", "0","selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","rt_VgaClamp", "1","selected"); %>>-4 dB</option>
                                                    <option value="2" <% nvram_match_x("","rt_VgaClamp", "2","selected"); %>>-8 dB</option>
                                                    <option value="3" <% nvram_match_x("","rt_VgaClamp", "3","selected"); %>>-12 dB</option>
                                                    <option value="4" <% nvram_match_x("","rt_VgaClamp", "4","selected"); %>>-16 dB</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_greenap">
                                            <th><#WIFIGreenAP#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="rt_greenap_on_of">
                                                        <input type="checkbox" id="rt_greenap_fake" <% nvram_match_x("", "rt_greenap", "1", "value=1 checked"); %><% nvram_match_x("", "rt_greenap", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="rt_greenap" id="rt_greenap_1" class="input" <% nvram_match_x("", "rt_greenap", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="rt_greenap" id="rt_greenap_0" class="input" <% nvram_match_x("", "rt_greenap", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="rt_ap_isolate_on_of">
                                                        <input type="checkbox" id="rt_ap_isolate_fake" <% nvram_match_x("","rt_ap_isolate", "1", "value=1 checked"); %><% nvram_match_x("","rt_ap_isolate", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="rt_ap_isolate_1" name="rt_ap_isolate" class="input" <% nvram_match_x("","rt_ap_isolate", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="rt_ap_isolate_0" name="rt_ap_isolate" class="input" <% nvram_match_x("","rt_ap_isolate", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 4);"><#WLANConfig11n_PremblesType_itemname#></a></th>
                                            <td>
                                                <select name="rt_preamble" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_preamble", "0","selected"); %>>Long</option>
                                                    <option value="1" <% nvram_match_x("","rt_preamble", "1","selected"); %>>Short (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 9);"><#WLANConfig11b_x_Frag_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="rt_frag" class="input" value="<% nvram_get_x("", "rt_frag"); %>" onKeyPress="return is_number(this,event);" onBlur="return validate_range(this, 256, 2346);"/>
                                                &nbsp;<span style="color:#888;">[256..2346]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 10);"><#WLANConfig11b_x_RTS_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="rt_rts" class="input" value="<% nvram_get_x("", "rt_rts"); %>" onKeyPress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1..2347]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 11);"><#WLANConfig11b_x_DTIM_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="3" size="5" name="rt_dtim" class="input" value="<% nvram_get_x("", "rt_dtim"); %>" onKeyPress="return is_number(this,event);" onBlur="return validate_range(this, 1, 255);"/>
                                                &nbsp;<span style="color:#888;">[1..255]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 12);"><#WLANConfig11b_x_Beacon_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="rt_bcn" class="input" value="<% nvram_get_x("", "rt_bcn"); %>" onKeyPress="return is_number(this,event);" onBlur="return validate_range(this, 20, 1000);"/>
                                                &nbsp;<span style="color:#888;">[20..1000]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_mrate">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
                                            <td>
                                                <select name="rt_mrate" class="input">
                                                    <option value="0" <% nvram_match_x("", "rt_mrate", "0", "selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("", "rt_mrate", "1", "selected"); %>>CCK 1 Mbps</option>
                                                    <option value="2" <% nvram_match_x("", "rt_mrate", "2", "selected"); %>>CCK 2 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "rt_mrate", "3", "selected"); %>>OFDM 6 Mbps (*)</option>
                                                    <option value="4" <% nvram_match_x("", "rt_mrate", "4", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "rt_mrate", "5", "selected"); %>>OFDM 12 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "rt_mrate", "6", "selected"); %>>HTMIX 15 Mbps</option>
                                                    <option value="7" <% nvram_match_x("", "rt_mrate", "7", "selected"); %>>HTMIX 30 Mbps</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ldpc" style="display:none">
                                            <th><#WIFILDPC#></th>
                                            <td>
                                                <select name="rt_ldpc" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_ldpc", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","rt_ldpc", "1","selected"); %>>11n only</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 13);"><#WLANConfig11b_x_TxBurst_itemname#></a></th>
                                            <td>
                                                <select name="rt_TxBurst" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_TxBurst')">
                                                    <option value="0" <% nvram_match_x("","rt_TxBurst", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","rt_TxBurst", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 16);"><#WLANConfig11b_x_PktAggregate_itemname#></a></th>
                                            <td>
                                                <select name="rt_PktAggregate" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_PktAggregate')">
                                                    <option value="0" <% nvram_match_x("","rt_PktAggregate", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","rt_PktAggregate", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIRDG#></th>
                                            <td>
                                                <select name="rt_HT_RDG" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_HT_RDG", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","rt_HT_RDG", "1","selected"); %>><#btn_Enable#> (Ralink clients only)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIAutoBA#></th>
                                            <td>
                                                <select name="rt_HT_AutoBA" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_HT_AutoBA", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","rt_HT_AutoBA", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WLANConfig11n_amsdu#></th>
                                            <td>
                                                <select name="rt_HT_AMSDU" class="input">
                                                    <option value="0" <% nvram_match_x("", "rt_HT_AMSDU", "0", "selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "rt_HT_AMSDU", "1", "selected"); %>><#btn_Enable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_greenfield">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 19);"><#WLANConfig11b_x_HT_OpMode_itemname#></a></th>
                                            <td>
                                                <select class="input" id="rt_HT_OpMode" name="rt_HT_OpMode" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_HT_OpMode')">
                                                    <option value="0" <% nvram_match_x("","rt_HT_OpMode", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","rt_HT_OpMode", "1","selected"); %>><#btn_Enable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                          <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 3, 14);"><#WLANConfig11b_x_WMM_itemname#></a></th>
                                          <td>
                                            <select name="rt_wme" id="rt_wme" class="input" onChange="change_wmm();">
                                              <option value="0" <% nvram_match_x("", "rt_wme", "0", "selected"); %>><#btn_Disable#></option>
                                              <option value="1" <% nvram_match_x("", "rt_wme", "1", "selected"); %>><#btn_Enable#> (*)</option>
                                            </select>
                                          </td>
                                        </tr>
                                        <tr id="row_wme_no_ack">
                                          <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 15);"><#WLANConfig11b_x_NOACK_itemname#></a></th>
                                          <td>
                                            <select name="rt_wme_no_ack" id="rt_wme_no_ack" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_wme_no_ack')">
                                              <option value="off" <% nvram_match_x("","rt_wme_no_ack", "off","selected"); %>><#btn_Disable#> (*)</option>
                                              <option value="on" <% nvram_match_x("","rt_wme_no_ack", "on","selected"); %>><#btn_Enable#></option>
                                            </select>
                                          </td>
                                        </tr>
                                        <tr id="row_apsd_cap">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 17);"><#WLANConfig11b_x_APSD_itemname#></a></th>
                                            <td>
                                              <select name="rt_APSDCapable" class="input" onchange="return change_common_rt(this, 'WLANConfig11b', 'rt_APSDCapable')">
                                                <option value="0" <% nvram_match_x("","rt_APSDCapable", "0","selected"); %> ><#btn_Disable#></option>
                                                <option value="1" <% nvram_match_x("","rt_APSDCapable", "1","selected"); %> ><#btn_Enable#> (*)</option>
                                              </select>
                                            </td>
                                        </tr>
                                        <tr id="row_turbo_qam" style="display:none">
                                            <th><#WLANConfig11b_x_turbo_qam#></th>
                                            <td>
                                                <select name="rt_turbo_qam" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_turbo_qam", "0","selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("","rt_turbo_qam", "1","selected"); %>><#btn_Enable#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_airtimefairness" style="display:none">
                                            <th><#WLANConfig11b_x_rt_airtimefairness#></th>
                                            <td>
                                                <select name="rt_airtimefairness" class="input">
                                                    <option value="0" <% nvram_match_x("","rt_airtimefairness", "0","selected"); %>><#btn_Disable#> (*)</option>
                                                    <option value="1" <% nvram_match_x("","rt_airtimefairness", "1","selected"); %>><#btn_Enable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td id="col_goto5" width="50%" style="margin-top: 10px; border-top: 0 none;">
                                                <input class="btn btn-info" type="button" name="goto5" value="<#GO_5G#>" onclick="location.href='Advanced_WAdvanced_Content.asp';">
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

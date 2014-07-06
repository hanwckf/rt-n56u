<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_5#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
    var $j = jQuery.noConflict();

    $j(document).ready(function() {

        $j('#nf_nat_loop_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nf_nat_loop_fake").attr("checked", "checked").attr("value", 1);
                $j("#nf_nat_loop_1").attr("checked", "checked");
                $j("#nf_nat_loop_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nf_nat_loop_fake").removeAttr("checked").attr("value", 0);
                $j("#nf_nat_loop_0").attr("checked", "checked");
                $j("#nf_nat_loop_1").removeAttr("checked");
            }
        });
        $j("#nf_nat_loop_on_of label.itoggle").css("background-position", $j("input#nf_nat_loop_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_pt_pptp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_pt_pptp_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#fw_pt_pptp_1").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pptp_0").removeAttr("checked").removeAttr("selected");
            },
            onClickOff: function(){
                $j("#fw_pt_pptp_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_pptp_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pptp_1").removeAttr("checked").removeAttr("selected");;
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
            },
            onClickOff: function(){
                $j("#fw_pt_l2tp_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_l2tp_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_l2tp_1").removeAttr("checked").removeAttr("selected");;
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
            },
            onClickOff: function(){
                $j("#fw_pt_ipsec_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_ipsec_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_ipsec_1").removeAttr("checked").removeAttr("selected");;
            }
        });
        $j("#fw_pt_ipsec_on_of label.itoggle").css("background-position", $j("input#fw_pt_ipsec_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_pt_pppoe_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_pt_pppoe_fake").attr("checked", "checked").attr("value", 1).attr("selected", "selected");
                $j("#fw_pt_pppoe_1").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pppoe_0").removeAttr("checked").removeAttr("selected");
            },
            onClickOff: function(){
                $j("#fw_pt_pppoe_fake").removeAttr("checked").attr("value", 0).attr("selected", "selected");
                $j("#fw_pt_pppoe_0").attr("checked", "checked").attr("selected", "selected");
                $j("#fw_pt_pppoe_1").removeAttr("checked").removeAttr("selected");
            }
        });
        $j("#fw_pt_pppoe_on_of label.itoggle").css("background-position", $j("input#fw_pt_pppoe_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#nf_alg_pptp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nf_alg_pptp_fake").attr("checked", "checked").attr("value", 1);
                $j("#nf_alg_pptp_1").attr("checked", "checked");
                $j("#nf_alg_pptp_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nf_alg_pptp_fake").removeAttr("checked").attr("value", 0);
                $j("#nf_alg_pptp_0").attr("checked", "checked");
                $j("#nf_alg_pptp_1").removeAttr("checked");
            }
        });
        $j("#nf_alg_pptp_on_of label.itoggle").css("background-position", $j("input#nf_alg_pptp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#nf_alg_h323_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nf_alg_h323_fake").attr("checked", "checked").attr("value", 1);
                $j("#nf_alg_h323_1").attr("checked", "checked");
                $j("#nf_alg_h323_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nf_alg_h323_fake").removeAttr("checked").attr("value", 0);
                $j("#nf_alg_h323_0").attr("checked", "checked");
                $j("#nf_alg_h323_1").removeAttr("checked");
            }
        });
        $j("#nf_alg_h323_on_of label.itoggle").css("background-position", $j("input#nf_alg_h323_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#nf_alg_rtsp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nf_alg_rtsp_fake").attr("checked", "checked").attr("value", 1);
                $j("#nf_alg_rtsp_1").attr("checked", "checked");
                $j("#nf_alg_rtsp_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nf_alg_rtsp_fake").removeAttr("checked").attr("value", 0);
                $j("#nf_alg_rtsp_0").attr("checked", "checked");
                $j("#nf_alg_rtsp_1").removeAttr("checked");
            }
        });
        $j("#nf_alg_rtsp_on_of label.itoggle").css("background-position", $j("input#nf_alg_rtsp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#nf_alg_sip_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nf_alg_sip_fake").attr("checked", "checked").attr("value", 1);
                $j("#nf_alg_sip_1").attr("checked", "checked");
                $j("#nf_alg_sip_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nf_alg_sip_fake").removeAttr("checked").attr("value", 0);
                $j("#nf_alg_sip_0").attr("checked", "checked");
                $j("#nf_alg_sip_1").removeAttr("checked");
            }
        });
        $j("#nf_alg_sip_on_of label.itoggle").css("background-position", $j("input#nf_alg_sip_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>

<% nf_values(); %>

function initial(){
	show_banner(1);
	show_menu(5,5,2);
	show_footer();
	showhide_div('row_nat_loop', (sw_mode == '4') ? 0 : 1);

	$("nf_count").innerHTML = nf_conntrack_count() + ' in use';
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Netfilter_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if (document.form.nf_alg_ftp0.value!="")
		if(!validate_range(document.form.nf_alg_ftp0, 21, 65535))
			return false;

	if (document.form.nf_alg_ftp1.value!="")
		if(!validate_range(document.form.nf_alg_ftp1, 1024, 65535))
			return false;

	return true;
}

function done_validating(action){
	refreshpage();
}
</script>
<style>
    .nav-tabs > li > a {
          padding-right: 6px;
          padding-left: 6px;
    }
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

    <input type="hidden" name="current_page" value="Advanced_Netfilter_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#NFilter_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#NFilterConfig#></th>
                                        </tr>
                                        <tr>
                                        <th width="50%"><#NFilterMaxConn#></th>
                                            <td>
                                                <select name="nf_max_conn" class="input">
                                                    <option value="8192" <% nvram_match_x("FirewallConfig","nf_max_conn", "8192", "selected"); %>>8192</option>
                                                    <option value="16384" <% nvram_match_x("FirewallConfig","nf_max_conn", "16384", "selected"); %>>16384 (HW_NAT FoE Max)</option>
                                                    <option value="32768" <% nvram_match_x("FirewallConfig","nf_max_conn", "32768", "selected"); %>>32768</option>
                                                    <option value="65536" <% nvram_match_x("FirewallConfig","nf_max_conn", "65536","selected"); %>>65536</option>
                                                    <option value="131072" <% nvram_match_x("FirewallConfig","nf_max_conn", "131072", "selected"); %>>131072 (Slow)</option>
                                                    <option value="262144" <% nvram_match_x("FirewallConfig","nf_max_conn", "262144", "selected"); %>>262144 (Very Slow)</option>
                                                </select>
                                                &nbsp;&nbsp;<span class="label label-info" style="padding: 6px 7px 8px 7px;" id="nf_count"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#NFilterNatType#></th>
                                            <td>
                                                <select name="nf_nat_type" class="input">
                                                    <option value="2" <% nvram_match_x("FirewallConfig","nf_nat_type", "2", "selected"); %>>Classical Linux Hybrid NAT</option>
                                                    <option value="0" <% nvram_match_x("FirewallConfig","nf_nat_type", "0", "selected"); %>>Restricted Cone NAT</option>
                                                    <option value="1" <% nvram_match_x("FirewallConfig","nf_nat_type", "1", "selected"); %>>Full Cone NAT</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_nat_loop">
                                            <th>NAT loopback?</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nf_nat_loop_on_of">
                                                        <input type="checkbox" id="nf_nat_loop_fake" <% nvram_match_x("", "nf_nat_loop", "1", "value=1 checked"); %><% nvram_match_x("", "nf_nat_loop", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nf_nat_loop" id="nf_nat_loop_1" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_nat_loop", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nf_nat_loop" id="nf_nat_loop_0" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_nat_loop", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#vpn_passthrough_itemname#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,25);"><#VPN_PPTP_Passthrough#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_pt_pptp_on_of">
                                                        <input type="checkbox" id="fw_pt_pptp_fake" <% nvram_match_x("", "fw_pt_pptp", "1", "value=1 checked"); %><% nvram_match_x("", "fw_pt_pptp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="fw_pt_pptp_1" name="fw_pt_pptp" value="1" <% nvram_match_x("FirewallConfig", "fw_pt_pptp", "1", "checked selected"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="fw_pt_pptp_0" name="fw_pt_pptp" value="0" <% nvram_match_x("FirewallConfig", "fw_pt_pptp", "0", "checked selected"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,26);"><#VPN_L2TP_Passthrough#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_pt_l2tp_on_of">
                                                        <input type="checkbox" id="fw_pt_l2tp_fake" <% nvram_match_x("", "fw_pt_l2tp", "1", "value=1 checked"); %><% nvram_match_x("", "fw_pt_l2tp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="fw_pt_l2tp_1" name="fw_pt_l2tp" value="1" <% nvram_match_x("FirewallConfig", "fw_pt_l2tp", "1", "checked selected"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="fw_pt_l2tp_0" name="fw_pt_l2tp" value="0" <% nvram_match_x("FirewallConfig", "fw_pt_l2tp", "0", "checked selected"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,27);"><#VPN_IPSec_Passthrough#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_pt_ipsec_on_of">
                                                        <input type="checkbox" id="fw_pt_ipsec_fake" <% nvram_match_x("", "fw_pt_ipsec", "1", "value=1 checked"); %><% nvram_match_x("", "fw_pt_ipsec", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="fw_pt_ipsec_1" name="fw_pt_ipsec" value="1" <% nvram_match_x("FirewallConfig", "fw_pt_ipsec", "1", "checked selected"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="fw_pt_ipsec_0" name="fw_pt_ipsec" value="0" <% nvram_match_x("FirewallConfig", "fw_pt_ipsec", "0", "checked selected"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_pt_pppoe_on_of">
                                                        <input type="checkbox" id="fw_pt_pppoe_fake" <% nvram_match_x("", "fw_pt_pppoe", "1", "value=1 checked"); %><% nvram_match_x("", "fw_pt_pppoe", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="fw_pt_pppoe_1" name="fw_pt_pppoe" value="1" <% nvram_match_x("FirewallConfig", "fw_pt_pppoe", "1", "checked selected"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="fw_pt_pppoe_0" name="fw_pt_pppoe" value="0" <% nvram_match_x("FirewallConfig", "fw_pt_pppoe", "0", "checked selected"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#NFilterALG#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">FTP ALG (ports)</th>
                                            <td>
                                                <input type="text" maxlength="5" size="5" style="width: 50px;" name="nf_alg_ftp0" class="input" value="<% nvram_get_x("", "nf_alg_ftp0"); %>" onkeypress="return is_number(this)"/>&nbsp;,&nbsp;
                                                <input type="text" maxlength="5" size="5" style="width: 50px;" name="nf_alg_ftp1" class="input" value="<% nvram_get_x("", "nf_alg_ftp1"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>PPTP ALG</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nf_alg_pptp_on_of">
                                                        <input type="checkbox" id="nf_alg_pptp_fake" <% nvram_match_x("", "nf_alg_pptp", "1", "value=1 checked"); %><% nvram_match_x("", "nf_alg_pptp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nf_alg_pptp" id="nf_alg_pptp_1" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_alg_pptp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nf_alg_pptp" id="nf_alg_pptp_0" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_alg_pptp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>RTSP ALG</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nf_alg_rtsp_on_of">
                                                        <input type="checkbox" id="nf_alg_rtsp_fake" <% nvram_match_x("", "nf_alg_rtsp", "1", "value=1 checked"); %><% nvram_match_x("", "nf_alg_rtsp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nf_alg_rtsp" id="nf_alg_rtsp_1" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_alg_rtsp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nf_alg_rtsp" id="nf_alg_rtsp_0" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_alg_rtsp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>H.323 ALG</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nf_alg_h323_on_of">
                                                        <input type="checkbox" id="nf_alg_h323_fake" <% nvram_match_x("", "nf_alg_h323", "1", "value=1 checked"); %><% nvram_match_x("", "nf_alg_h323", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nf_alg_h323" id="nf_alg_h323_1" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_alg_h323", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nf_alg_h323" id="nf_alg_h323_0" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_alg_h323", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>SIP ALG</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nf_alg_sip_on_of">
                                                        <input type="checkbox" id="nf_alg_sip_fake" <% nvram_match_x("", "nf_alg_sip", "1", "value=1 checked"); %><% nvram_match_x("", "nf_alg_sip", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nf_alg_sip" id="nf_alg_sip_1" class="input" value="1" <% nvram_match_x("FirewallConfig", "nf_alg_sip", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nf_alg_sip" id="nf_alg_sip_0" class="input" value="0" <% nvram_match_x("FirewallConfig", "nf_alg_sip", "0", "checked"); %>/><#checkbox_No#>
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

    <div id="footer"></div>
</div>
</body>
</html>

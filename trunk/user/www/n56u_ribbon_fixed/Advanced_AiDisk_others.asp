<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_4_3#></title>
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
        $j('#enable_samba_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#enable_samba_fake").attr("checked", "checked").attr("value", 1);
                $j("#enable_samba_1").attr("checked", "checked");
                $j("#enable_samba_0").removeAttr("checked");
                change_smb_enabled();
            },
            onClickOff: function(){
                $j("#enable_samba_fake").removeAttr("checked").attr("value", 0);
                $j("#enable_samba_0").attr("checked", "checked");
                $j("#enable_samba_1").removeAttr("checked");
                change_smb_enabled();
            }
        });
        $j("#enable_samba_on_of label.itoggle").css("background-position", $j("input#enable_samba_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#enable_ftp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#enable_ftp_fake").attr("checked", "checked").attr("value", 1);
                $j("#enable_ftp_1").attr("checked", "checked");
                $j("#enable_ftp_0").removeAttr("checked");
                change_ftp_enabled();
            },
            onClickOff: function(){
                $j("#enable_ftp_fake").removeAttr("checked").attr("value", 0);
                $j("#enable_ftp_0").attr("checked", "checked");
                $j("#enable_ftp_1").removeAttr("checked");
                change_ftp_enabled();
            }
        });
        $j("#enable_ftp_on_of label.itoggle").css("background-position", $j("input#enable_ftp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#nfsd_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#nfsd_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#nfsd_enable_1").attr("checked", "checked");
                $j("#nfsd_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#nfsd_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#nfsd_enable_0").attr("checked", "checked");
                $j("#nfsd_enable_1").removeAttr("checked");
            }
        });
        $j("#nfsd_enable_on_of label.itoggle").css("background-position", $j("input#nfsd_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#apps_dms_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#apps_dms_fake").attr("checked", "checked").attr("value", 1);
                $j("#apps_dms_1").attr("checked", "checked");
                $j("#apps_dms_0").removeAttr("checked");
                change_dms_enabled();
            },
            onClickOff: function(){
                $j("#apps_dms_fake").removeAttr("checked").attr("value", 0);
                $j("#apps_dms_0").attr("checked", "checked");
                $j("#apps_dms_1").removeAttr("checked");
                change_dms_enabled();
            }
        });
        $j("#apps_dms_on_of label.itoggle").css("background-position", $j("input#apps_dms_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#apps_itunes_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#apps_itunes_fake").attr("checked", "checked").attr("value", 1);
                $j("#apps_itunes_1").attr("checked", "checked");
                $j("#apps_itunes_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#apps_itunes_fake").removeAttr("checked").attr("value", 0);
                $j("#apps_itunes_0").attr("checked", "checked");
                $j("#apps_itunes_1").removeAttr("checked");
            }
        });
        $j("#apps_itunes_on_of label.itoggle").css("background-position", $j("input#apps_itunes_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#trmd_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#trmd_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#trmd_enable_1").attr("checked", "checked");
                $j("#trmd_enable_0").removeAttr("checked");
                change_trmd_enabled();
            },
            onClickOff: function(){
                $j("#trmd_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#trmd_enable_0").attr("checked", "checked");
                $j("#trmd_enable_1").removeAttr("checked");
                change_trmd_enabled();
            }
        });
        $j("#trmd_enable_on_of label.itoggle").css("background-position", $j("input#trmd_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#aria_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#aria_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#aria_enable_1").attr("checked", "checked");
                $j("#aria_enable_0").removeAttr("checked");
                change_aria_enabled();
            },
            onClickOff: function(){
                $j("#aria_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#aria_enable_0").attr("checked", "checked");
                $j("#aria_enable_1").removeAttr("checked");
                change_aria_enabled();
            }
        });
        $j("#aria_enable_on_of label.itoggle").css("background-position", $j("input#aria_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });

</script>

<script>

lan_ipaddr = '<% nvram_get_x("", "lan_ipaddr_t"); %>';

<% login_state_hook(); %>

var ddns_enable = '<% nvram_get_x("LANHostConfig", "ddns_enable_x"); %>';
var ddns_server = '<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>';
var ddns_hostname = '<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';

var usb_share_list = [<% get_usb_share_list(); %>];
var menu_open = [0, 0, 0];

function initial(){
	show_banner(1);
	show_menu(5, 7, 1);
	show_footer();

	if(!found_utl_hdparm()){
		$("row_spd").style.display = "none";
		$("row_apm").style.display = "none";
	}

	if(support_usb3()){
		$("row_usb3_disable").style.display = "";
	}

	if(!found_app_smbd() && !found_app_ftpd()){
		$("row_max_user").style.display = "none";
	}

	if(found_app_smbd()){
		$("tbl_smbd").style.display = "";
		change_smb_enabled();
	}

	if(found_app_ftpd()){
		$("tbl_ftpd").style.display = "";
		change_ftp_enabled();
	}

	if(found_app_nfsd()){
		$("tbl_nfsd").style.display = "";
	}

	if(found_app_dlna()){
		$("tbl_minidlna").style.display = "";
		change_dms_enabled();
	}

	if(found_app_ffly()){
		$("tbl_itunes").style.display = "";
	}

	if(found_app_torr()){
		$("tbl_trmd").style.display = "";
		change_trmd_enabled();
	}

	if(found_app_aria()){
		$("tbl_aria").style.display = "";
		change_aria_enabled();
	}

	if (!document.form.apps_dms[0].checked){
		$("web_dms_link").style.display = "none";
	}

	if (!document.form.apps_itunes[0].checked){
		$("web_ffly_link").style.display = "none";
	}

	if (!document.form.trmd_enable[0].checked){
		$("web_rpc_link").style.display = "none";
	}

	show_usb_share_list(0);
	show_usb_share_list(1);
	show_usb_share_list(2);
}

var window_rpc;
var window_dms;
var window_ffly;
var window_params="toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=800,height=600";

function on_rpc_link(){
	var rpc_url="http://" + lan_ipaddr + ":" + document.form.trmd_rport.value;
	window_rpc = window.open(rpc_url, "Transmission", window_params);
	window_rpc.focus();
}

function on_dms_link(){
	var dms_url="http://" + lan_ipaddr + ":8200";
	window_dms = window.open(dms_url, "Minidlna", window_params);
	window_dms.focus();
}

function on_ffly_link(){
	var ffly_url="http://" + lan_ipaddr + ":3689";
	window_ffly = window.open(ffly_url, "Firefly", window_params);
	window_ffly.focus();
}

function hide_usb_share_list(idx){
	var obj, obj3;
	if (idx == 0) {
		obj = $j("#chevron1");
		obj3 = $("share_list1");
	} else if (idx == 1) {
		obj = $j("#chevron2");
		obj3 = $("share_list2");
	} else {
		obj = $j("#chevron3");
		obj3 = $("share_list3");
	}
	obj.children('i').removeClass('icon-chevron-up').addClass('icon-chevron-down');
	obj3.style.display = "none";
	menu_open[idx] = 0;
}

function set_usb_share(num, idx){
	var path = usb_share_list[num][1];
	if (idx == 0)
		document.form.dlna_src1.value = "A,"+path+"/Audio";
	else if (idx == 1)
		document.form.dlna_src2.value = "V,"+path+"/Video";
	else
		document.form.dlna_src3.value = "P,"+path+"/Photo";
	hide_usb_share_list(idx);
}

function show_usb_share_list(idx){
	var code = "";

	for(var i = 0; i < usb_share_list.length; i++){
		if (usb_share_list[i][1]) {
			code += '<a href="javascript:void(0)"><div onclick="set_usb_share('+i+','+idx+');"><strong>'+usb_share_list[i][1]+'</strong>';
			code += ' ['+usb_share_list[i][0]+']';
			if (usb_share_list[i][2])
				code += ' ('+usb_share_list[i][2]+')';
			code += '</div></a>';
		}
	}

	if (code == "")
		code = '<div style="text-align: center;" onclick="hide_usb_share_list('+idx+');"><#Nodata#></div>';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';

	if (idx == 0) {
		$("share_list1").innerHTML = code;
	} else if (idx == 1) {
		$("share_list2").innerHTML = code;
	} else {
		$("share_list3").innerHTML = code;
	}
}

function pull_usb_share_list(obj,idx){
	var obj2, obj3, idx2, idx3;
	if(menu_open[idx] == 0){
		$j(obj).children('i').removeClass('icon-chevron-down').addClass('icon-chevron-up');
		if (idx == 0) {
			obj2 = document.form.dlna_src1;
			obj3 = $("share_list1");
			idx2 = 1;
			idx3 = 2;
		} else if (idx == 1) {
			obj2 = document.form.dlna_src2;
			obj3 = $("share_list2");
			idx2 = 0;
			idx3 = 2;
		} else {
			obj2 = document.form.dlna_src3;
			obj3 = $("share_list3");
			idx2 = 0;
			idx3 = 1;
		}
		if (menu_open[idx2])
			hide_usb_share_list(idx2);
		if (menu_open[idx3])
			hide_usb_share_list(idx3);
		obj2.focus();
		obj3.style.display = "block";
		menu_open[idx] = 1;
	}
	else
		hide_usb_share_list(idx);
}

function change_smb_enabled(){
	var a = rcheck(document.form.enable_samba);
	if (a == "0"){
		$("row_smb_wgrp").style.display = "none";
		$("row_smb_mode").style.display = "none";
		$("row_smb_lmb").style.display = "none";
	} else {
		$("row_smb_wgrp").style.display = "";
		$("row_smb_mode").style.display = "";
		$("row_smb_lmb").style.display = "";
	}
}

function change_ftp_enabled(){
	var a = rcheck(document.form.enable_ftp);
	if (a == "0"){
		$("row_ftp_mode").style.display = "none";
		$("row_ftp_log").style.display = "none";
	} else {
		$("row_ftp_mode").style.display = "";
		$("row_ftp_log").style.display = "";
	}
}

function change_dms_enabled(){
	var a = rcheck(document.form.apps_dms);
	if (a == "0"){
		$("row_dms_disc").style.display = "none";
		$("row_dms_src1").style.display = "none";
		$("row_dms_src2").style.display = "none";
		$("row_dms_src3").style.display = "none";
		$("row_dms_dnew").style.display = "none";
		$("row_dms_root").style.display = "none";
		$("row_dms_sort").style.display = "none";
	} else {
		$("row_dms_disc").style.display = "";
		$("row_dms_src1").style.display = "";
		$("row_dms_src2").style.display = "";
		$("row_dms_src3").style.display = "";
		$("row_dms_dnew").style.display = "";
		$("row_dms_root").style.display = "";
		$("row_dms_sort").style.display = "";
	}
}

function change_trmd_enabled(){
	var a = rcheck(document.form.trmd_enable);
	if (a == "0"){
		$("row_trmd_pport").style.display = "none";
		$("row_trmd_rport").style.display = "none";
	} else {
		$("row_trmd_pport").style.display = "";
		$("row_trmd_rport").style.display = "";
	}
}

function change_aria_enabled(){
	var a = rcheck(document.form.aria_enable);
	if (a == "0"){
		$("row_aria_pport").style.display = "none";
		$("row_aria_rport").style.display = "none";
	} else {
		$("row_aria_pport").style.display = "";
		$("row_aria_rport").style.display = "";
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_AiDisk_others.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function trim(str){
	return str.replace(/(^s*)|(s*$)/g, "");
}

function validForm(){
	if(!validate_range(document.form.st_max_user, 1, 50)){
		return false;
	}

	String.prototype.Trim = function(){return this.replace(/(^\s*)|(\s*$)/g,"");}
	document.form.st_samba_workgroup.value = document.form.st_samba_workgroup.value.Trim();

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

    <input type="hidden" name="current_page" value="Advanced_AiDisk_others.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="Storage;LANHostConfig;">
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
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#USB_Application_disk_miscellaneous_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr id="row_usb3_disable" style="display:none;">
                                            <th width="50%"><#StorageU3Off#></th>
                                            <td>
                                                <select name="usb3_disable" class="input">
                                                    <option value="0" <% nvram_match_x("", "usb3_disable", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "usb3_disable", "1", "selected"); %>><#checkbox_Yes#> (<#StorageU3Desc#>)</option>
                                                </select>
                                                &nbsp;<span style="color:#888">* need reboot</span>
                                            </td>
                                        </tr>
                                        <tr id="row_spd">
                                            <th width="50%"><#StorageSpindown#></th>
                                            <td>
                                                <select name="hdd_spindt" class="input">
                                                    <option value="0" <% nvram_match_x("", "hdd_spindt", "0", "selected"); %>><#ItemNever#></option>
                                                    <option value="1" <% nvram_match_x("", "hdd_spindt", "1", "selected"); %>>0h:15m</option>
                                                    <option value="2" <% nvram_match_x("", "hdd_spindt", "2", "selected"); %>>0h:30m</option>
                                                    <option value="3" <% nvram_match_x("", "hdd_spindt", "3", "selected"); %>>1h:00m</option>
                                                    <option value="4" <% nvram_match_x("", "hdd_spindt", "4", "selected"); %>>1h:30m</option>
                                                    <option value="5" <% nvram_match_x("", "hdd_spindt", "5", "selected"); %>>2h:00m</option>
                                                    <option value="6" <% nvram_match_x("", "hdd_spindt", "6", "selected"); %>>2h:30m</option>
                                                    <option value="7" <% nvram_match_x("", "hdd_spindt", "7", "selected"); %>>3h:00m</option>
                                                    <option value="8" <% nvram_match_x("", "hdd_spindt", "8", "selected"); %>>3h:30m</option>
                                                    <option value="9" <% nvram_match_x("", "hdd_spindt", "9", "selected"); %>>4h:00m</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_apm">
                                            <th><#StorageApmOff#></th>
                                            <td>
                                                <select name="hdd_apmoff" class="input">
                                                    <option value="0" <% nvram_match_x("", "hdd_apmoff", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "hdd_apmoff", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#StorageAutoChkDsk#></th>
                                            <td>
                                                <select name="achk_enable" class="input">
                                                    <option value="0" <% nvram_match_x("", "achk_enable", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "achk_enable", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#StorageCacheReclaim#></th>
                                            <td>
                                                <select name="pcache_reclaim" class="input">
                                                    <option value="0" <% nvram_match_x("", "pcache_reclaim", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "pcache_reclaim", "1", "selected"); %>>50% RAM</option>
                                                    <option value="2" <% nvram_match_x("", "pcache_reclaim", "2", "selected"); %>>70% RAM</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#StorageAllowOptw#></th>
                                            <td>
                                                <select name="optw_enable" class="input">
                                                    <option value="0" <% nvram_match_x("", "optw_enable", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "optw_enable", "1", "selected"); %>>Optware (legacy)</option>
                                                    <option value="2" <% nvram_match_x("", "optw_enable", "2", "selected"); %>>Entware</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_max_user">
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,1);"><#ShareNode_MaximumLoginUser_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_max_user" class="input" maxlength="2" size="5" value="<% nvram_get_x("", "st_max_user"); %>"/>
                                                &nbsp;<span style="color:#888;">[1..50]</span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table id="tbl_smbd" width="100%" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#StorageSMBD#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#enableCIFS#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="enable_samba_on_of">
                                                        <input type="checkbox" id="enable_samba_fake" <% nvram_match_x("", "enable_samba", "1", "value=1 checked"); %><% nvram_match_x("", "enable_samba", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="enable_samba" id="enable_samba_1" value="1" onclick="change_smb_enabled();" <% nvram_match_x("", "enable_samba", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="enable_samba" id="enable_samba_0" value="0" onclick="change_smb_enabled();" <% nvram_match_x("", "enable_samba", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_smb_wgrp">
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17, 3);"><#ShareNode_WorkGroup_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_samba_workgroup" class="input" maxlength="32" size="32" value="<% nvram_get_x("", "st_samba_workgroup"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="row_smb_mode">
                                            <th>
                                                <#StorageShare#>
                                            </th>
                                            <td>
                                                <select name="st_samba_mode" class="input" style="width: 300px;">
                                                    <option value="1" <% nvram_match_x("", "st_samba_mode", "1", "selected"); %>><#StorageShare1#></option>
                                                    <option value="3" <% nvram_match_x("", "st_samba_mode", "3", "selected"); %>><#StorageShare5#></option>
                                                    <option value="4" <% nvram_match_x("", "st_samba_mode", "4", "selected"); %>><#StorageShare2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_smb_lmb">
                                            <th>
                                                <#StorageLMB#>
                                            </th>
                                            <td>
                                                <select name="st_samba_lmb" class="input">
                                                    <option value="1" <% nvram_match_x("", "st_samba_lmb", "1", "selected"); %>><#checkbox_Yes#></option>
                                                    <option value="0" <% nvram_match_x("", "st_samba_lmb", "0", "selected"); %>><#checkbox_No#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table id="tbl_ftpd" width="100%" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#StorageFTPD#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#enableFTP#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="enable_ftp_on_of">
                                                        <input type="checkbox" id="enable_ftp_fake" <% nvram_match_x("", "enable_ftp", "1", "value=1 checked"); %><% nvram_match_x("", "enable_ftp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="enable_ftp" id="enable_ftp_1" value="1" onclick="change_ftp_enabled();" <% nvram_match_x("", "enable_ftp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="enable_ftp" id="enable_ftp_0" value="0" onclick="change_ftp_enabled();" <% nvram_match_x("", "enable_ftp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ftp_mode">
                                            <th>
                                                <#StorageShare#>
                                            </th>
                                            <td>
                                                <select name="st_ftp_mode" class="input" style="width: 300px;">
                                                    <option value="1" <% nvram_match_x("", "st_ftp_mode", "1", "selected"); %>><#StorageShare1#></option>
                                                    <option value="3" <% nvram_match_x("", "st_ftp_mode", "3", "selected"); %>><#StorageShare3#></option>
                                                    <option value="2" <% nvram_match_x("", "st_ftp_mode", "2", "selected"); %>><#StorageShare2#></option>
                                                    <option value="4" <% nvram_match_x("", "st_ftp_mode", "4", "selected"); %>><#StorageShare4#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ftp_log">
                                            <th>
                                                <#StorageLog#>
                                            </th>
                                            <td>
                                                <select name="st_ftp_log" class="input">
                                                    <option value="0" <% nvram_match_x("", "st_ftp_log", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "st_ftp_log", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_nfsd" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#StorageNFSD#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#StorageEnableNFSD#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="nfsd_enable_on_of">
                                                        <input type="checkbox" id="nfsd_enable_fake" <% nvram_match_x("", "nfsd_enable", "1", "value=1 checked"); %><% nvram_match_x("", "nfsd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nfsd_enable" id="nfsd_enable_1" value="1" <% nvram_match_x("", "nfsd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nfsd_enable" id="nfsd_enable_0" value="0" <% nvram_match_x("", "nfsd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_minidlna" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#UPnPMediaServer#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#StorageEnableDLNA#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="apps_dms_on_of">
                                                        <input type="checkbox" id="apps_dms_fake" <% nvram_match_x("", "apps_dms", "1", "value=1 checked"); %><% nvram_match_x("", "apps_dms", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="apps_dms" id="apps_dms_1" value="1" onclick="change_dms_enabled();" <% nvram_match_x("", "apps_dms", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="apps_dms" id="apps_dms_0" value="0" onclick="change_dms_enabled();" <% nvram_match_x("", "apps_dms", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_dms_link();" id="web_dms_link">Web status</a>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_src1">
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <div id="share_list1" class="alert alert-info ddown-list"></div>
                                                <div class="input-append">
                                                    <input type="text" name="dlna_src1" class="input" maxlength="255" size="32" value="<% nvram_get_x("", "dlna_src1"); %>" style="float:left; width: 260px"/>
                                                    <button class="btn btn-chevron" id="chevron1" type="button" onclick="pull_usb_share_list(this, 0);" title="Select share for audio"><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_src2">
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <div id="share_list2" class="alert alert-info ddown-list"></div>
                                                <div class="input-append">
                                                    <input type="text" name="dlna_src2" class="input" maxlength="255" size="32" value="<% nvram_get_x("", "dlna_src2"); %>" style="float:left; width: 260px"/>
                                                    <button class="btn btn-chevron" id="chevron2" type="button" onclick="pull_usb_share_list(this, 1);" title="Select share for video"><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_src3">
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <div id="share_list3" class="alert alert-info ddown-list"></div>
                                                <div class="input-append">
                                                    <input type="text" name="dlna_src3" class="input" maxlength="255" size="32" value="<% nvram_get_x("", "dlna_src3"); %>" style="float:left; width: 260px"/>
                                                    <button class="btn btn-chevron" id="chevron3" type="button" onclick="pull_usb_share_list(this, 2);" title="Select share for pictures"><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_dnew">
                                            <th width="50%">
                                                <#StorageRescanDLNA#>
                                            </th>
                                            <td>
                                                <select name="dlna_rescan" class="input">
                                                    <option value="0" <% nvram_match_x("", "dlna_rescan", "0", "selected"); %>><#StorageRescanItem0#></option>
                                                    <option value="1" <% nvram_match_x("", "dlna_rescan", "1", "selected"); %>><#StorageRescanItem1#></option>
                                                    <option value="2" <% nvram_match_x("", "dlna_rescan", "2", "selected"); %>><#StorageRescanItem2#></option>
                                                </select>
                                            </td>
                                            <td>
                                                <input type="submit" maxlength="15" class="btn btn-info" onClick="return onSubmitApply('dlna_rescan');" size="15" name="" value="Rescan!"/>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_disc">
                                            <th>
                                                <#StorageNotifyDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" name="dlna_disc" class="input" maxlength="5" size="5" value="<% nvram_get_x("", "dlna_disc"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_root">
                                            <th>
                                                <#StorageRootDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <select name="dlna_root" class="input">
                                                    <option value="0" <% nvram_match_x("", "dlna_root", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "dlna_root", "1", "selected"); %>>Browse Folders</option>
                                                    <option value="2" <% nvram_match_x("", "dlna_root", "2", "selected"); %>>Music</option>
                                                    <option value="3" <% nvram_match_x("", "dlna_root", "3", "selected"); %>>Video</option>
                                                    <option value="4" <% nvram_match_x("", "dlna_root", "4", "selected"); %>>Pictures</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_dms_sort">
                                            <th>
                                                <#StorageSortDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <select name="dlna_sort" class="input">
                                                    <option value="0" <% nvram_match_x("", "dlna_sort", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "dlna_sort", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_itunes" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#StorageFFly#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#StorageEnableFFly#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="apps_itunes_on_of">
                                                        <input type="checkbox" id="apps_itunes_fake" <% nvram_match_x("", "apps_itunes", "1", "value=1 checked"); %><% nvram_match_x("", "apps_itunes", "0", "value=0"); %>>

                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="apps_itunes" id="apps_itunes_1" value="1" <% nvram_match_x("", "apps_itunes", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="apps_itunes" id="apps_itunes_0" value="0" <% nvram_match_x("", "apps_itunes", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_ffly_link();" id="web_ffly_link">Web control</a>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_trmd" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#StorageTorrent#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,11);"><#StorageEnableTRMD#></a>
                                            </th>
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="trmd_enable_on_of">
                                                        <input type="checkbox" id="trmd_enable_fake" <% nvram_match_x("", "trmd_enable", "1", "value=1 checked"); %><% nvram_match_x("", "trmd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="trmd_enable" id="trmd_enable_1" value="1" onclick="change_trmd_enabled();" <% nvram_match_x("", "trmd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="trmd_enable" id="trmd_enable_0" value="0" onclick="change_trmd_enabled();" <% nvram_match_x("", "trmd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_trmd_pport">
                                            <th>
                                                <#StoragePPortTRMD#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" maxlength="5" size="5" name="trmd_pport" class="input" value="<% nvram_get_x("", "trmd_pport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_trmd_rport">
                                            <th width="50%">
                                                <#StorageRPortTRMD#>
                                            </th>
                                            <td>
                                               <input type="text" maxlength="5" size="5" name="trmd_rport" class="input" value="<% nvram_get_x("", "trmd_rport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                            <td>
                                               <a href="javascript:on_rpc_link();" id="web_rpc_link">Web control</a>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_aria" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#StorageAria#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,12);"><#StorageEnableAria#></a>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="aria_enable_on_of">
                                                        <input type="checkbox" id="aria_enable_fake" <% nvram_match_x("", "aria_enable", "1", "value=1 checked"); %><% nvram_match_x("", "aria_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="aria_enable" id="aria_enable_1" value="1" onclick="change_aria_enabled();" <% nvram_match_x("", "aria_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="aria_enable" id="aria_enable_0" value="0" onclick="change_aria_enabled();" <% nvram_match_x("", "aria_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_aria_pport">
                                            <th>
                                                <#StoragePPortTRMD#>
                                            </th>
                                            <td>
                                                <input type="text" maxlength="5" size="5" name="aria_pport" class="input" value="<% nvram_get_x("", "aria_pport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_aria_rport">
                                            <th>
                                                <#StorageRPortTRMD#>
                                            </th>
                                            <td>
                                               <input type="text" maxlength="5" size="5" name="aria_rport" class="input" value="<% nvram_get_x("", "aria_rport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <td style="border-top: 0 none;">
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

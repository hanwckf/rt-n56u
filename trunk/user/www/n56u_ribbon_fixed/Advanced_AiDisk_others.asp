<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_4_3#></title>
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
        $j('#enable_samba_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#enable_samba_fake").attr("checked", "checked").attr("value", 1);
                $j("#enable_samba_1").attr("checked", "checked");
                $j("#enable_samba_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#enable_samba_fake").removeAttr("checked").attr("value", 0);
                $j("#enable_samba_0").attr("checked", "checked");
                $j("#enable_samba_1").removeAttr("checked");
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
            },
            onClickOff: function(){
                $j("#enable_ftp_fake").removeAttr("checked").attr("value", 0);
                $j("#enable_ftp_0").attr("checked", "checked");
                $j("#enable_ftp_1").removeAttr("checked");
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
            },
            onClickOff: function(){
                $j("#apps_dms_fake").removeAttr("checked").attr("value", 0);
                $j("#apps_dms_0").attr("checked", "checked");
                $j("#apps_dms_1").removeAttr("checked");
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
            },
            onClickOff: function(){
                $j("#trmd_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#trmd_enable_0").attr("checked", "checked");
                $j("#trmd_enable_1").removeAttr("checked");
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
            },
            onClickOff: function(){
                $j("#aria_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#aria_enable_0").attr("checked", "checked");
                $j("#aria_enable_1").removeAttr("checked");
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

<% usb_apps_check(); %>

function initial(){
    show_banner(1);
    show_menu(5, 7, 1);
    show_footer();

    enable_auto_hint(17, 6);

    if(found_app_dlna()){
        $("tbl_minidlna").style.display = "";
    }

    if(found_app_ffly()){
        $("tbl_itunes").style.display = "";
    }

    if(found_app_torr()){
        $("tbl_torrent").style.display = "";
    }

    if(found_app_aria()){
        $("tbl_aria").style.display = "";
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

function copytob(){
}

function copytob2(){
        document.form.st_samba_workgroupb.value = encodeURIComponent(document.form.st_samba_workgroup.value);
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
                document.form.st_max_user.focus();
                document.form.st_max_user.select();
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

<body onload="initial();" onunLoad="disable_auto_hint(17, 7);return unload_body();">

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
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_AiDisk_others.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="Storage;LANHostConfig;">
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
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#USB_Application_disk_miscellaneous_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%">
                                                <#StorageSpindown#>
                                            </th>
                                            <td>
                                                <select name="hdd_spindt" class="input">
                                                    <option value="0" <% nvram_match_x("Storage", "hdd_spindt", "0", "selected"); %>><#ItemNever#></option>
                                                    <option value="1" <% nvram_match_x("Storage", "hdd_spindt", "1", "selected"); %>>0h:15m</option>
                                                    <option value="2" <% nvram_match_x("Storage", "hdd_spindt", "2", "selected"); %>>0h:30m</option>
                                                    <option value="3" <% nvram_match_x("Storage", "hdd_spindt", "3", "selected"); %>>1h:00m</option>
                                                    <option value="4" <% nvram_match_x("Storage", "hdd_spindt", "4", "selected"); %>>1h:30m</option>
                                                    <option value="5" <% nvram_match_x("Storage", "hdd_spindt", "5", "selected"); %>>2h:00m</option>
                                                    <option value="6" <% nvram_match_x("Storage", "hdd_spindt", "6", "selected"); %>>2h:30m</option>
                                                    <option value="7" <% nvram_match_x("Storage", "hdd_spindt", "7", "selected"); %>>3h:00m</option>
                                                    <option value="8" <% nvram_match_x("Storage", "hdd_spindt", "8", "selected"); %>>3h:30m</option>
                                                    <option value="9" <% nvram_match_x("Storage", "hdd_spindt", "9", "selected"); %>>4h:00m</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageApmOff#>
                                            </th>
                                            <td>
                                                <select name="hdd_apmoff" class="input">
                                                    <option value="0" <% nvram_match_x("Storage", "hdd_apmoff", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("Storage", "hdd_apmoff", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageAutoChkDsk#>
                                            </th>
                                            <td>
                                                <select name="achk_enable" class="input">
                                                    <option value="0" <% nvram_match_x("Storage", "achk_enable", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("Storage", "achk_enable", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageAllowOptw#>
                                            </th>
                                            <td>
                                                <select name="optw_enable" class="input">
                                                        <option value="0" <% nvram_match_x("Storage", "optw_enable", "0", "selected"); %>><#checkbox_No#></option>
                                                        <option value="1" <% nvram_match_x("Storage", "optw_enable", "1", "selected"); %>>Optware (legacy)</option>
                                                        <option value="2" <% nvram_match_x("Storage", "optw_enable", "2", "selected"); %>>Entware</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,1);"><#ShareNode_MaximumLoginUser_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_max_user" class="input" maxlength="2" size="5" value="<% nvram_get_x("Storage", "st_max_user"); %>"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
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
                                                        <input type="checkbox" id="enable_samba_fake" <% nvram_match_x("Storage", "enable_samba", "1", "value=1 checked"); %><% nvram_match_x("Storage", "enable_samba", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="enable_samba" id="enable_samba_1" value="1" <% nvram_match_x("Storage", "enable_samba", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="enable_samba" id="enable_samba_0" value="0" <% nvram_match_x("Storage", "enable_samba", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17, 3);"><#ShareNode_WorkGroup_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_samba_workgroup" class="input" maxlength="32" size="32" value="<% nvram_get_x("Storage", "st_samba_workgroup"); %>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageShare#>
                                            </th>
                                            <td>
                                                <select name="st_samba_mode" class="input" style="width: 300px;">
                                                    <option value="1" <% nvram_match_x("Storage", "st_samba_mode", "1", "selected"); %>><#StorageShare1#></option>
                                                    <option value="4" <% nvram_match_x("Storage", "st_samba_mode", "4", "selected"); %>><#StorageShare2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageLMB#>
                                            </th>
                                            <td>
                                                <select name="st_samba_lmb" class="input">
                                                    <option value="1" <% nvram_match_x("Storage", "st_samba_lmb", "1", "selected"); %>><#checkbox_Yes#></option>
                                                    <option value="0" <% nvram_match_x("Storage", "st_samba_lmb", "0", "selected"); %>><#checkbox_No#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
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
                                                        <input type="checkbox" id="enable_ftp_fake" <% nvram_match_x("Storage", "enable_ftp", "1", "value=1 checked"); %><% nvram_match_x("Storage", "enable_ftp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="enable_ftp" id="enable_ftp_1" value="1" <% nvram_match_x("Storage", "enable_ftp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="enable_ftp" id="enable_ftp_0" value="0" <% nvram_match_x("Storage", "enable_ftp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageShare#>
                                            </th>
                                            <td>
                                                <select name="st_ftp_mode" class="input" style="width: 300px;">
                                                    <option value="1" <% nvram_match_x("Storage", "st_ftp_mode", "1", "selected"); %>><#StorageShare1#></option>
                                                    <option value="3" <% nvram_match_x("Storage", "st_ftp_mode", "3", "selected"); %>><#StorageShare3#></option>
                                                    <option value="2" <% nvram_match_x("Storage", "st_ftp_mode", "2", "selected"); %>><#StorageShare2#></option>
                                                    <option value="4" <% nvram_match_x("Storage", "st_ftp_mode", "4", "selected"); %>><#StorageShare4#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageLog#>
                                            </th>
                                            <td>
                                                <select name="st_ftp_log" class="input">
                                                    <option value="0" <% nvram_match_x("Storage", "st_ftp_log", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("Storage", "st_ftp_log", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
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
                                                        <input type="checkbox" id="nfsd_enable_fake" <% nvram_match_x("Storage", "nfsd_enable", "1", "value=1 checked"); %><% nvram_match_x("Storage", "nfsd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="nfsd_enable" id="nfsd_enable_1" value="1" <% nvram_match_x("Storage", "nfsd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="nfsd_enable" id="nfsd_enable_0" value="0" <% nvram_match_x("Storage", "nfsd_enable", "0", "checked"); %>/><#checkbox_No#>
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
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="apps_dms_on_of">
                                                        <input type="checkbox" id="apps_dms_fake" <% nvram_match_x("Storage", "apps_dms", "1", "value=1 checked"); %><% nvram_match_x("Storage", "apps_dms", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="apps_dms" id="apps_dms_1" value="1" <% nvram_match_x("Storage", "apps_dms", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="apps_dms" id="apps_dms_0" value="0" <% nvram_match_x("Storage", "apps_dms", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageNotifyDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" name="dlna_disc" class="input" maxlength="5" size="5" value="<% nvram_get_x("Storage", "dlna_disc"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" name="dlna_src1" class="input" maxlength="255" size="32" value="<% nvram_get_x("Storage", "dlna_src1"); %>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" name="dlna_src2" class="input" maxlength="255" size="32" value="<% nvram_get_x("Storage", "dlna_src2"); %>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" name="dlna_src3" class="input" maxlength="255" size="32" value="<% nvram_get_x("Storage", "dlna_src3"); %>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageRescanDLNA#>
                                            </th>
                                            <td>
                                                <select name="dlna_rescan" class="input">
                                                    <option value="0" <% nvram_match_x("Storage", "dlna_rescan", "0", "selected"); %>>Never update</option>
                                                    <option value="1" <% nvram_match_x("Storage", "dlna_rescan", "1", "selected"); %>>Update for new files only</option>
                                                    <option value="2" <% nvram_match_x("Storage", "dlna_rescan", "2", "selected"); %>>Force update whole database</option>
                                                </select>

                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_dms_link();" id="web_dms_link">Web status</a>
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
                                                        <input type="checkbox" id="apps_itunes_fake" <% nvram_match_x("Storage", "apps_itunes", "1", "value=1 checked"); %><% nvram_match_x("Storage", "apps_itunes", "0", "value=0"); %>>

                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="apps_itunes" id="apps_itunes_1" value="1" <% nvram_match_x("Storage", "apps_itunes", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="apps_itunes" id="apps_itunes_0" value="0" <% nvram_match_x("Storage", "apps_itunes", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_ffly_link();" id="web_ffly_link">Web control</a>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_torrent" cellpadding="4" cellspacing="0" class="table" style="display:none;">
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
                                                        <input type="checkbox" id="trmd_enable_fake" <% nvram_match_x("Storage", "trmd_enable", "1", "value=1 checked"); %><% nvram_match_x("Storage", "trmd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="trmd_enable" id="trmd_enable_1" value="1" <% nvram_match_x("Storage", "trmd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="trmd_enable" id="trmd_enable_0" value="0" <% nvram_match_x("Storage", "trmd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StoragePPortTRMD#>
                                            </th>
                                            <td colspan="2">
                                                <input type="text" maxlength="5" size="5" name="trmd_pport" class="input" value="<% nvram_get_x("Storage","trmd_pport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageRPortTRMD#>
                                            </th>
                                            <td>
                                               <input type="text" maxlength="5" size="5" name="trmd_rport" class="input" value="<% nvram_get_x("Storage","trmd_rport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                            <td width="15%">
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
                                                        <input type="checkbox" id="aria_enable_fake" <% nvram_match_x("Storage", "aria_enable", "1", "value=1 checked"); %><% nvram_match_x("Storage", "aria_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="aria_enable" id="aria_enable_1" value="1" <% nvram_match_x("Storage", "aria_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="aria_enable" id="aria_enable_0" value="0" <% nvram_match_x("Storage", "aria_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StoragePPortTRMD#>
                                            </th>
                                            <td>
                                                <input type="text" maxlength="5" size="5" name="aria_pport" class="input" value="<% nvram_get_x("Storage","aria_pport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageRPortTRMD#>
                                            </th>
                                            <td>
                                               <input type="text" maxlength="5" size="5" name="aria_rport" class="input" value="<% nvram_get_x("Storage","aria_rport"); %>" onkeypress="return is_number(this)"/>
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

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
        $j('#hdd_apmoff_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#hdd_apmoff_fake").attr("checked", "checked").attr("value", 1);
                $j("#hdd_apmoff_1").attr("checked", "checked");
                $j("#hdd_apmoff_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#hdd_apmoff_fake").removeAttr("checked").attr("value", 0);
                $j("#hdd_apmoff_0").attr("checked", "checked");
                $j("#hdd_apmoff_1").removeAttr("checked");
            }
        });
        $j("#hdd_apmoff_on_of label.itoggle").css("background-position", $j("input#hdd_apmoff_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#achk_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#achk_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#achk_enable_1").attr("checked", "checked");
                $j("#achk_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#achk_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#achk_enable_0").attr("checked", "checked");
                $j("#achk_enable_1").removeAttr("checked");
            }
        });
        $j("#achk_enable_on_of label.itoggle").css("background-position", $j("input#achk_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

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

        $j('#optw_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#optw_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#optw_enable_1").attr("checked", "checked");
                $j("#optw_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#optw_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#optw_enable_0").attr("checked", "checked");
                $j("#optw_enable_1").removeAttr("checked");
            }
        });
        $j("#optw_enable_on_of label.itoggle").css("background-position", $j("input#optw_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

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
    });

</script>

<script>

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x   = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto   = '<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>';
lan_ipaddr  = '<% nvram_get_x("", "lan_ipaddr_t"); %>';
lan_trport  = '<% nvram_get_x("Storage", "trmd_rport"); %>';
tor_enable  = '<% nvram_get_x("Storage", "trmd_enable"); %>';

<% login_state_hook(); %>

var wireless = [<% wl_auth_list(); %>]; // [[MAC, associated, authorized], ...]
var ddns_enable = '<% nvram_get_x("LANHostConfig", "ddns_enable_x"); %>';
var ddns_server = '<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>';
var ddns_hostname = '<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';

<% usb_apps_check(); %>

function initial(){
    show_banner(1);
    show_menu(5, 5, 3);
    show_footer();

    enable_auto_hint(17, 6);

    xfr();

    if(found_app_dlna() == '1'){
        $("minidlna_row").style.display = "";
    }

    if(found_app_torr() == '1'){
        $("torrent_row").style.display = "";
    }

    if (tor_enable == '0'){
        $("web_rpc_link").style.display = "none";
    }
}

function xfr(){
        if(document.form.computer_name2.value != ""){
                document.form.computer_name.value = decodeURIComponent(document.form.computer_name2.value);
        }
        else{
                document.form.computer_name.value = document.form.computer_name3.value;
        }
}

var newwindow;

function on_rpc_link(){
    var rpc_url="http://" + lan_ipaddr + ":" + lan_trport;
    newwindow = window.open(rpc_url, "Transmission", "toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480");
    if (window.focus) {newwindow.focus()}
}

function blanktest(obj, flag){
        var value2 = eval("document.form."+flag+"2.value");
        var value3 = eval("document.form."+flag+"3.value");
        
        if(obj.value == ""){
                if(value2 != "")
                        obj.value = decodeURIComponent(value2);
                else
                        obj.value = value3;
                
                alert("<#JS_Shareblanktest#>");
                
                return false;
        }
        
        return true;
}

function copytob(){
       // document.form.computer_nameb.value = encodeURIComponent(document.form.computer_name.value);
}

function copytob2(){
        document.form.st_samba_workgroupb.value = encodeURIComponent(document.form.st_samba_workgroup.value);
}

function applyRule(){
        if(validForm()){
                showLoading();
                
                //copytob();
                //copytob2();
                
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
        var re = new RegExp("[^a-zA-Z0-9 _-]+", "gi");
        
        if(!validate_range(document.form.st_max_user, 1, 10)){
                document.form.st_max_user.focus();
                document.form.st_max_user.select();
                return false;
        }
        
        if(!blanktest(document.form.computer_name, "computer_name")){
                document.form.computer_name.focus();
                document.form.computer_name.select();
                return false;
        }
        
        if(re.test(document.form.computer_name.value)){
                alert("<#JS_validchar#>");
                
                document.form.computer_name.focus();
                document.form.computer_name.select();
                return false;
        }
        
        /*
        if(!validate_range(document.form.apps_upload_max, 0, 999))
                return false;
        */
        
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
    <input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

    <!--input type="hidden" name="computer_nameb" value=""-->
    <input type="hidden" name="computer_name2" value="<% nvram_get_x("Storage", "computer_name"); %>">
    <input type="hidden" name="computer_name3" value="<% nvram_get_x("Storage", "computer_name"); %>">
    <!--input type="hidden" name="st_samba_workgroupb" value="">
    <input type="hidden" name="samba_workgroup2" value="<% nvram_get_x("Storage", "st_samba_workgroupb"); %>">
    <input type="hidden" name="samba_workgroup3" value="<% nvram_get_x("Storage", "st_samba_workgroup"); %>"-->

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
                                                        <option value="0" <% nvram_match_x("Storage", "hdd_spindt", "0", "selected"); %>>Never</option>
                                                        <option value="1" <% nvram_match_x("Storage", "hdd_spindt", "1", "selected"); %>>0:15</option>
                                                        <option value="2" <% nvram_match_x("Storage", "hdd_spindt", "2", "selected"); %>>0:30</option>
                                                        <option value="3" <% nvram_match_x("Storage", "hdd_spindt", "3", "selected"); %>>1:00</option>
                                                        <option value="4" <% nvram_match_x("Storage", "hdd_spindt", "4", "selected"); %>>1:30</option>
                                                        <option value="5" <% nvram_match_x("Storage", "hdd_spindt", "5", "selected"); %>>2:00</option>
                                                        <option value="6" <% nvram_match_x("Storage", "hdd_spindt", "6", "selected"); %>>2:30</option>
                                                        <option value="7" <% nvram_match_x("Storage", "hdd_spindt", "7", "selected"); %>>3:00</option>
                                                        <option value="8" <% nvram_match_x("Storage", "hdd_spindt", "8", "selected"); %>>3:30</option>
                                                        <option value="9" <% nvram_match_x("Storage", "hdd_spindt", "9", "selected"); %>>4:00</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageApmOff#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="hdd_apmoff_on_of">
                                                        <input type="checkbox" id="hdd_apmoff_fake" <% nvram_match_x("Storage", "hdd_apmoff", "1", "value=1 checked"); %><% nvram_match_x("Storage", "hdd_apmoff", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="hdd_apmoff" id="hdd_apmoff_1" value="1" <% nvram_match_x("Storage", "hdd_apmoff", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="hdd_apmoff" id="hdd_apmoff_0" value="0" <% nvram_match_x("Storage", "hdd_apmoff", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageAutoChkDsk#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="achk_enable_on_of">
                                                        <input type="checkbox" id="achk_enable_fake" <% nvram_match_x("Storage", "achk_enable", "1", "value=1 checked"); %><% nvram_match_x("Storage", "achk_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="achk_enable" id="achk_enable_1" value="1" <% nvram_match_x("Storage", "achk_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="achk_enable" id="achk_enable_0" value="0" <% nvram_match_x("Storage", "achk_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17, 2);"><#ShareNode_DeviceName_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="computer_name" id="computer_name" class="input" maxlength="15" size="32" value=""/>
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
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,1);"><#ShareNode_MaximumLoginUser_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_max_user" class="input" maxlength="5" size="5" value="<% nvram_get_x("Storage", "st_max_user"); %>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
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
                                        <tr>
                                            <th>
                                                <#StorageAllowOptw#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="optw_enable_on_of">
                                                        <input type="checkbox" id="optw_enable_fake" <% nvram_match_x("Storage", "optw_enable", "1", "value=1 checked"); %><% nvram_match_x("Storage", "optw_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="optw_enable" id="optw_enable_1" value="1" <% nvram_match_x("Storage", "optw_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="optw_enable" id="optw_enable_0" value="0" <% nvram_match_x("Storage", "optw_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="minidlna_row" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#UPnPMediaServer#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#StorageEnableDLNA#>
                                            </th>
                                            <td>
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
                                                <#StorageSourceDLNA#>
                                            </th>
                                            <td>
                                                <input type="text" name="dlna_source" class="input" maxlength="255" size="32" value="<% nvram_get_x("Storage", "dlna_source"); %>"/>
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
                                        </tr>
                                    </table>

                                    <table width="100%" id="torrent_row" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#StorageTorrent#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <#StorageEnableTRMD#>
                                            </th>
                                            <td>
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
                                            <td>
                                                <input type="text" maxlength="5" size="5" name="trmd_pport" class="input" value="<% nvram_get_x("Storage","trmd_pport"); %>" onkeypress="return is_number(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#StorageRPortTRMD#><br/>
                                            </th>
                                            <td>
                                               <input type="text" maxlength="5" size="5" name="trmd_rport" class="input" value="<% nvram_get_x("Storage","trmd_rport"); %>" onkeypress="return is_number(this)"/>
                                               &nbsp;<a href="javascript:on_rpc_link();" id="web_rpc_link">Web control</a>
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

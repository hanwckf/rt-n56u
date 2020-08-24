<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_6_4#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
    $j('#commit_nvram, #commit_storage').click(function(){
        var $button = $j(this);
        send_commit_action($button.prop('id'), $button);
        return false;
    });
});

</script>
<script>

<% login_state_hook(); %>
var lan_ipaddr = '<% nvram_get_x("", "lan_ipaddr_t"); %>';

function initial(){
	show_banner(1);
	show_menu(5,7,5);

	if (login_safe()){
		showhide_div('row_nv_reset', 1);
		showhide_div('row_nv_backup', 1);
		showhide_div('row_nv_restore1', 1);
		showhide_div('row_nv_restore2', 1);
		showhide_div('row_st_reset', 1);
		showhide_div('row_st_backup', 1);
		showhide_div('row_st_restore1', 1);
		showhide_div('row_st_restore2', 1);
	}

	if (support_mtd_rwfs())
		showhide_div('tbl_rwfs', 1);

	show_footer();
}

function set_frm_action_apply(am){
	document.form.action = "apply.cgi";
	document.form.enctype = "application/x-www-form-urlencoded";
	document.form.action_mode.value = am;
}

function set_frm_action_upload(at){
	document.form.action = at;
	document.form.enctype = "multipart/form-data";
	document.form.action_mode.value = "";
}

function submitRule(){
	$("commit_btn").style.display = (document.form.nvram_manual_fake.value == "0") ? "none" : "";

	set_frm_action_apply(" Apply ");
	document.form.nvram_manual.value = document.form.nvram_manual_fake.value;
	document.form.rstats_stored.value = document.form.rstats_stored_fake.value;
	document.form.stime_stored.value = document.form.stime_stored_fake.value;
	if (support_mtd_rwfs())
		document.form.mtd_rwfs_mount.value = document.form.mtd_rwfs_mount_fake.value;
	document.form.submit();
}

function applyRule(){
	document.form.submit_fake.click();
}

function restoreNVRAM(){
	var alert_string = "<#Setting_factorydefault_hint1#>";
	if(lan_ipaddr != "192.168.2.1")
		alert_string += "\n<#Setting_factorydefault_iphint#>\n";
	alert_string += "\n<#Setting_factorydefault_hint2#>";
	if(confirm(alert_string)){
		document.form.action1.blur();
		showLoading();
		set_frm_action_apply(" RestoreNVRAM ");
		document.form.submit();
	}else
		return false;
}

function restoreStorage(){
	var alert_string = "<#Adm_Setting_store_hint#>";
	alert_string += "\n<#Setting_factorydefault_hint2#>";
	if(confirm(alert_string)){
		showLoadingOne();
		set_frm_action_apply(" RestoreStorage ");
		document.form.submit();
	}
	else
		return false;
}

function send_commit_action(action_id,$button){
	if(action_id == '')
		return;
	$j.ajax({
		type: "post",
		url: "/apply.cgi",
		data: {
			action_mode: " CommitFlash ",
			nvram_action: action_id
		},
		dataType: "json",
		error: function(xhr) {
			$button.addClass('alert-error');
			$button.val('Failed!');
			setTimeout("reset_btn_commit("+$button+")", 1500);
		},
		success: function(response) {
			var sys_result = (response != null && typeof response === 'object' && "sys_result" in response)
				? response.sys_result : -1;
			if(sys_result == 0)
				$button.addClass('alert-success');
			else
				$button.addClass('alert-error');
			setTimeout("reset_btn_commit('"+action_id+"')", 1500);
		}
	});
}

function saveSetting(){
	set_frm_action_apply("");
	location.href='Settings_' + document.form.productid.value + '.CFG';
}

function saveStorage(){
	set_frm_action_apply("");
	location.href='Storage_' + document.form.productid.value + '.TBZ';
}

function checkFileName(obj,ext){
	var fn = obj.value.toUpperCase();
	if(fn == ""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		return false;
	}
	else if(fn.length < 6 ||
			fn.lastIndexOf(ext) < 0 ||
			fn.lastIndexOf(ext) != (fn.length-ext.length)){
		alert("<#Setting_upload_hint#>");
		obj.focus();
		return false;
	}
	return true;
}

function uploadSetting(){
	document.form.file_st.value = "";
	if(checkFileName(document.form.file_nv, ".CFG")){
		disableCheckChangedStatus();
		set_frm_action_upload("restore_nv.cgi");
		document.form.submit();
	}
}

function uploadStorage(){
	document.form.file_nv.value = "";
	if(checkFileName(document.form.file_st, ".TBZ")){
		set_frm_action_upload("restore_st.cgi");
		document.form.submit();
	}
}

$j.fn.fileName = function() {
	var $this = $j(this),
	$val = $this.val(),
	valArray = $val.split('\\'),
	newVal = valArray[valArray.length-1],
	$button = $this.siblings('.button');
	if(newVal !== '') {
		newVal = newVal.substring(0,10);
		$button.text(newVal);
	}
};

</script>

<style>
.file {
	display: inline-block;
	width:100px;
	position: relative;
	-moz-border-radius: 4px;
	-webkit-border-radius:4px;
	border-radius: 4px;
	margin-bottom:10px;
	text-align: center;

	background-color: #f5f5f5;
	*background-color: #e6e6e6;
	background-image: -ms-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -webkit-gradient(linear, 0 0, 0 100%, from(#ffffff), to(#e6e6e6));
	background-image: -webkit-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -o-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -moz-linear-gradient(top, #ffffff, #e6e6e6);
	border: 1px solid #ddd;
}

/* style text of the upload field and add an attachment icon */
.file .button {
	font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
	font-size:11px;
	color:#555;
	height:27px;
	line-height:26px;
	display: block;
}
/* hide the real file upload input field */
.file input {
	cursor: pointer;
	height: 100%;
	position: absolute;
	right: 0;
	top: 0;
	filter: alpha(opacity=1);
	-moz-opacity: 0.01;
	font-size: 100px;
}
</style>

</head>

<body onload="initial();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="LoadingBar" class="popup_bg">
        <center>
            <div class="container-fluid" style="margin-top: 150px;">
                <div class="well" style="background-color: #212121; width: 60%;">
                    <div class="progress" style="max-width: 450px; text-align: left;">
                        <div class="bar" id="proceeding_img"><span id="proceeding_img_text"></span></div>
                    </div>
                    <div class="alert alert-danger" style="max-width: 400px;"><#SAVE_restart_desc#></div>
                </div>
            </div>
        </center>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" name="form" action="upload.cgi" target="hidden_frame" enctype="multipart/form-data">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="current_page" value="Advanced_SettingBackup_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_SettingBackup_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="General;">
    <input type="hidden" name="productid" value="<% nvram_get_x("", "productid"); %>" readonly="1">
    <input type="hidden" name="nvram_manual" value="<% nvram_get_x("", "nvram_manual"); %>">
    <input type="hidden" name="rstats_stored" value="<% nvram_get_x("", "rstats_stored"); %>">
    <input type="hidden" name="stime_stored" value="<% nvram_get_x("", "stime_stored"); %>">
    <input type="hidden" name="mtd_rwfs_mount" value="<% nvram_get_x("", "mtd_rwfs_mount"); %>">
    <input type="hidden" name="submit_fake" value="" onclick="submitRule();">

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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Setting_save_upload_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_Setting_nvram#></th>
                                        </tr>
                                        <tr id="row_nv_reset" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,19,1)"><#Setting_factorydefault_itemname#></a></th>
                                            <td>
                                                <input name="action1" class="btn btn-danger" style="width: 219px;" onclick="restoreNVRAM();" type="button" value="<#CTL_restore#>"/>
                                            </td>
                                        </tr>
                                        <tr id="row_nv_backup" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,19,2)"><#Setting_save_itemname#></a></th>
                                            <td>
                                                <input name="action2" class="btn btn-info" style="width: 219px;" onclick="saveSetting();" type="button" value="<#CTL_onlysave#>"/>
                                            </td>
                                        </tr>
                                        <tr id="row_nv_restore1" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,19,3)"><#Setting_upload_itemname#></a></th>
                                            <td>
                                                <input name="file_nv" type="file" size="36" />
                                            </td>
                                        </tr>
                                        <tr id="row_nv_restore2" style="display:none">
                                            <th style="border-top: 0 none; padding-top: 0px;"></th>
                                            <td style="border-top: 0 none; padding-top: 0px;">
                                                <input name="upload_nv" class="btn btn-info" style="width: 219px;" onclick="uploadSetting();" type="button" value="<#CTL_upload#>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_Setting_commit_mode#></th>
                                            <td align="left">
                                                <select class="input" name="nvram_manual_fake" onchange="applyRule();">
                                                    <option value="0" <% nvram_match_x("", "nvram_manual", "0", "selected"); %>><#Adm_Setting_commit_item0#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "nvram_manual", "1", "selected"); %>><#Adm_Setting_commit_item1#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_Setting_commit_now#></th>
                                            <td>
                                                <button type="button" name="commit_nvram" id="commit_nvram" class="btn" style="width: 219px; outline: 0"><i class="icon icon-fire"></i>&nbsp;<#CTL_Commit#></button>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_rwfs" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_Setting_rwfs#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_Setting_rwfs_mount#></th>
                                            <td align="left">
                                                <select class="input" name="mtd_rwfs_mount_fake" onchange="applyRule();" >
                                                    <option value="0" <% nvram_match_x("", "mtd_rwfs_mount", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "mtd_rwfs_mount", "1", "selected"); %>>UBIFS</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#Adm_Setting_store#></th>
                                        </tr>
                                        <tr id="row_st_reset" style="display:none">
                                            <th><#Setting_factorydefault_itemname#></th>
                                            <td colspan="2">
                                                <input name="st_action1" class="btn btn-danger" style="width: 219px;" type="button" value="<#CTL_restore#>" onclick="restoreStorage();"/>
                                            </td>
                                        </tr>
                                        <tr id="row_st_backup" style="display:none">
                                            <th><#Adm_Setting_store_backup#></th>
                                            <td>
                                                <input name="st_action2" class="btn btn-info" style="width: 219px;" onclick="saveStorage();" type="button" value="<#CTL_onlysave#>"/>
                                            </td>
                                        </tr>
                                        <tr id="row_st_restore1" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,19,4)"><#Storage_upload_itemname#></a></th>
                                            <td>
                                                <input name="file_st" type="file" size="36" />
                                            </td>
                                        </tr>
                                        <tr id="row_st_restore2" style="display:none">
                                            <th style="border-top: 0 none; padding-top: 0px;"></th>
                                            <td style="border-top: 0 none; padding-top: 0px;">
                                                <input name="upload_st" class="btn btn-info" style="width: 219px;" onclick="uploadStorage();" type="button" value="<#CTL_upload#>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_Setting_store_stats#></th>
                                            <td align="left">
                                                <select class="input" name="rstats_stored_fake" onchange="applyRule();" >
                                                    <option value="0" <% nvram_match_x("", "rstats_stored", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "rstats_stored", "1", "selected"); %>><#Adm_Setting_store_stats_item1#> (*)</option>
                                                    <option value="2" <% nvram_match_x("", "rstats_stored", "2", "selected"); %>><#Adm_Setting_store_stats_item2#></option>
                                                    <option value="3" <% nvram_match_x("", "rstats_stored", "3", "selected"); %>><#Adm_Setting_store_stats_item3#></option>
                                                    <option value="4" <% nvram_match_x("", "rstats_stored", "4", "selected"); %>><#Adm_Setting_store_stats_item4#></option>
                                                    <option value="5" <% nvram_match_x("", "rstats_stored", "5", "selected"); %>><#Adm_Setting_store_stats_item5#></option>
                                                    <option value="6" <% nvram_match_x("", "rstats_stored", "6", "selected"); %>><#Adm_Setting_store_stats_item6#> (!)</option>
                                                    <option value="7" <% nvram_match_x("", "rstats_stored", "7", "selected"); %>><#Adm_Setting_store_stats_item7#> (!)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_Setting_store_stime#></th>
                                            <td align="left">
                                                <select class="input" name="stime_stored_fake" onchange="applyRule();" >
                                                    <option value="1" <% nvram_match_x("", "stime_stored", "1", "selected"); %>><#checkbox_Yes#> (*)</option>
                                                    <option value="0" <% nvram_match_x("", "stime_stored", "0", "selected"); %>><#checkbox_No#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_Setting_store_now#></th>
                                            <td colspan="2">
                                                <button type="button" name="commit_storage" id="commit_storage" class="btn" style="width: 219px; outline: 0"><i class="icon icon-fire"></i>&nbsp;<#CTL_Commit#></button>
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

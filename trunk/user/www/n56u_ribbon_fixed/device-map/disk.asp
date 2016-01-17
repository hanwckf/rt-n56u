<!DOCTYPE html>
<html>
<head>
<title></title>
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
<script>
var $j = jQuery.noConflict();

var diskOrder = parent.getSelectedDiskOrder();
var all_accessable_size = parent.simpleNum2(parent.computeallpools(diskOrder, "size")-parent.computeallpools(diskOrder, "size_in_use"));
var mountedNum = parent.getDiskMountedNum(diskOrder);

<% get_AiDisk_status(); %>

var SMB_status = get_cifs_status();
var FTP_status = get_ftp_status();
var FTP_mode = get_share_management_status("ftp");
var accounts = [<% get_all_accounts("ftp"); %>];

var ddns_enable = '<% nvram_get_x("", "ddns_enable_x"); %>';
var ddns_server = '<% nvram_get_x("", "ddns_server_x"); %>';
var ddns_hostname = '<% nvram_get_x("", "ddns_hostname_x"); %>';

function initial(){
	flash_button();

	if (!found_app_smbd())
		SMB_status = 0;
	if (!found_app_ftpd())
		FTP_status = 0;

	showtext($("disk_model_name"), parent.getDiskModelName(diskOrder));
	showtext($("disk_total_size"), parent.getDiskTotalSize(diskOrder));

	if(mountedNum > 0){
		showtext($("disk_avail_size"), all_accessable_size);
		$("show_remove_button").style.display = "";
		show_disk_link();
	}else{
		$("mounted_item1").style.display = "none";
		$("mounted_item2").style.display = "none";
		
		$("show_removed_string").style.display = "";
		$("unmounted_refresh").style.display = "";
	}

	if(sw_mode == '3')
		$("aidisk_hyperlink").style.display = "none";

	var TotalSize = parent.getDiskTotalSize(diskOrder);
	var alertPercentbar = 'progress-info';
	percentbar = simpleNum2((all_accessable_size)/TotalSize*100);
	percentbar = Math.round(100-percentbar);
	if(percentbar >= 66 && percentbar < 85)
		alertPercentbar = 'progress-warning';
	else if(percentbar >= 85)
		alertPercentbar = 'progress-danger';

	$j('#usb_availablespace').html('<div style="margin-bottom: 2px; width:250px; float: left;" class="progress ' + alertPercentbar + '"><div class="bar" style="width:'+percentbar+'%">'+(percentbar > 10 ? (percentbar + '%') : '')+'</div></div>');
}

function show_disk_link(){
	var labels_show = 0;

	if(sw_mode == '3'){
		if(FTP_status != 0){
			$("ddnslink3").style.display = "";
			labels_show = 1;
		}
	}else{
		if(FTP_status != 0){
			if(ddns_enable == '1' && ddns_server.length > 0 && ddns_hostname.length > 0){
				if(FTP_mode == 1 || FTP_mode == 3){
					$("ddnslink1").style.display = "";
					$("ddnslink1_LAN").style.display = "";
				}else{
					$("ddnslink2").style.display = "";
					$("ddnslink2_LAN").style.display = "";
					
					$("selected_account_link").href = 'ftp://'+accounts[1]+'@<% nvram_get_x("", "ddns_hostname_x"); %>';
					showtext($("selected_account_str"), 'ftp://'+accounts[1]+'@<% nvram_get_x("", "ddns_hostname_x"); %>');
					$("selected_account_link_LAN").href = 'ftp://'+accounts[1]+'@<% nvram_get_x("", "lan_ipaddr_t"); %>';
					showtext($("selected_account_str_LAN"), 'ftp://'+accounts[1]+'@<% nvram_get_x("", "lan_ipaddr_t"); %>');
				}
				$("desc_2").style.display = "";
			}else{
				$("ddnslink3").style.display = "";
				if(ddns_enable != '1'){
					showtext($("noWAN_link"), "<br/><#linktoFTP_no_2#>");
					$("noWAN_link").style.display = "";
				}else if(ddns_hostname.length < 1){
					showtext($("noWAN_link"), "<br/><#linktoFTP_no_3#>");
					$("noWAN_link").style.display = "";
				}
			}
			labels_show = 1;
		}else if(found_app_ftpd()){
			showtext($("noWAN_link"), "<#linktoFTP_no_1#>");
			$("noWAN_link").style.display = "";
			labels_show = 1;
		}
	}

	if(SMB_status != 0){
		$("desc_3").style.display = "";
		labels_show = 1;
	}

	if (!labels_show)
		$("mounted_item2").style.display = "none";
}

function goAiDiskWizard(){
	parent.showLoading();
	parent.location.href = "/aidisk.asp";
}

function remove_disk(){
	var str = "<#Safelyremovedisk_confirm#>";
	
	if(confirm(str)){
		parent.showLoading();
		
		document.diskForm.action = "safely_remove_disk.asp";
		document.diskForm.port.value = parent.getDiskPort(diskOrder);
		document.diskForm.devn.value = parent.getDiskDevice(diskOrder);
		document.diskForm.submit();
	}
}

</script>

<style>
    .table {margin-bottom: 0px;}
    .table th, .table td{vertical-align: middle;}
    .alert-link {margin: 8px 0px 0px 0px; padding: 8px 8px 8px 8px;}
    .progress {
        background-image: -moz-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -ms-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -webkit-gradient(linear, 0 0, 0 100%, from(#f3f3f3), to(#dddddd));
        background-image: -webkit-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -o-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: linear-gradient(top, #f3f3f3, #dddddd);
        filter: progid:dximagetransform.microsoft.gradient(startColorstr='#f3f3f3', endColorstr='#dddddd', GradientType=0);
    }
</style>
</head>

<body class="body_iframe" onload="initial();">

<table width="98%" cellpadding="4" cellspacing="0" class="table">
    <tr>
        <th width="50%" style="border-top: 0 none;"><#Modelname#>:</th>
        <td style="border-top: 0 none;"><span id="disk_model_name"></span></td>
    </tr>
    <tr>
        <th><#Totalspace#>:</th>
        <td><span id="disk_total_size"></span> GB</td>
    </tr>
</table>

<table id="mounted_item1" width="98%" cellpadding="4" cellspacing="0" class="table">
    <tr>
        <th width="50%"><#Availablespace#>:</th>
        <td>
            <span id="disk_avail_size"></span> GB
        </td>
    </tr>
    <tr>
        <th width="50%"><#DiskUsage#>:</th>
        <td>
            <span id="usb_availablespace"></span>
        </td>
    </tr>
    <tr id="aidisk_hyperlink">
        <th><#AiDiskWizard#>:</th>
        <td>
            <input type="button" class="btn span2" onclick="goAiDiskWizard();" value="<#btn_go#>" >
        </td>
    </tr>
</table>

<table width="98%" cellpadding="4" cellspacing="0" class="table">
    <tr>
        <th width="50%"><#Safelyremovedisk_title#>:</th>
        <td>
            <input id="show_remove_button" type="button" class="btn btn-success span2" onclick="remove_disk();" value="<#btn_remove#>" style="display:none;">
            <div id="show_removed_string" style="display:none;"><span class="label label-success"><#Safelyremovedisk#></span></div>
        </td>
    </tr>
</table>

<div id="unmounted_refresh" class="alert alert-info alert-link" style="display:none;">
    <#DiskStatus_refresh1#><a href="/" target="_parent"><#DiskStatus_refresh2#></a><#DiskStatus_refresh3#>
</div>

<div id="mounted_item2">
    <div class="alert alert-info alert-link">
        <span id="ddnslink1" style="display:none;"><#Internet#>&nbsp;<#AiDisk_linktoFTP_fromInternet#>&nbsp;<a href="ftp://<% nvram_get_x("", "ddns_hostname_x"); %>" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank">ftp://<% nvram_get_x("", "ddns_hostname_x"); %></a></span>
        <span id="ddnslink2" style="display:none;"><#Internet#>&nbsp;<#AiDisk_linktoFTP_fromInternet#>&nbsp;<a id="selected_account_link" href="" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank"><span id="selected_account_str"></span></a></span>
        <span id="ddnslink3" style="display:none;"><#AiDisk_linktoFTP_fromInternet#>&nbsp;<a href="ftp://<% nvram_get_x("", "lan_ipaddr_t"); %>" target="_blank">ftp://<% nvram_get_x("", "lan_ipaddr_t"); %></a></span>
        <span id="noWAN_link" style="display:none;"></span>
    </div>
    <div class="alert alert-info alert-link" id="desc_2" style="display:none;">
        <span id="ddnslink1_LAN" style="display:none;"><#linktodisk#><a href="ftp://<% nvram_get_x("", "lan_ipaddr_t"); %>" target="_blank">ftp://<% nvram_get_x("", "lan_ipaddr_t"); %></a></span>
        <span id="ddnslink2_LAN" style="display:none;"><#linktodisk#><a id="selected_account_link_LAN" href="" target="_blank"><span id="selected_account_str_LAN"></span></a></span>
    </div>
    <div class="alert alert-info alert-link" id="desc_3" style="display:none;">
        <span id="ddnslink3_LAN"><#menu5_4_1#>: <a href="file://<% nvram_get_x("", "lan_ipaddr_t"); %>" target="_blank">\\<% nvram_get_x("", "lan_ipaddr_t"); %></a></span>
    </div>
</div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="port" value="">
<input type="hidden" name="devn" value="">
</form>
</body>
</html>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../NM_style.css" rel="stylesheet" type="text/css" />
<link href="../form_style.css" rel="stylesheet" type="text/css" />

<script type="text/javascript" src="/state.js"></script>
<script>
var diskOrder = parent.getSelectedDiskOrder();
var all_accessable_size = parent.simpleNum2(parent.computeallpools(diskOrder, "size")-parent.computeallpools(diskOrder, "size_in_use"));
var mountedNum = parent.getDiskMountedNum(diskOrder);

<% get_AiDisk_status(); %>

var FTP_status = get_ftp_status();  // FTP
var FTP_mode = get_share_management_status("ftp");
var accounts = [<% get_all_accounts(); %>];

var ddns_enable = '<% nvram_get_x("LANHostConfig", "ddns_enable_x"); %>';
var ddns_server = '<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>';
var ddns_hostname = '<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';

function initial(){
	flash_button();
	
	showtext($("disk_model_name"), parent.getDiskModelName(diskOrder));
	showtext($("disk_total_size"), parent.getDiskTotalSize(diskOrder));
	
	if(mountedNum > 0){
		showtext($("disk_avail_size"), all_accessable_size);
		
		$("show_remove_button").style.display = "";
		
		showdisklink();
		DMhint(); //Download Master Hint for user
	}
	else{
		$("mounted_item1").style.display = "none";
		$("mounted_item2").style.display = "none";
		
		$("show_removed_string").style.display = "";
		$("unmounted_refresh").style.display = "";
	}
	if(sw_mode == "3")
		$("aidisk_hyperlink").style.display = "none";
}

function showdisklink(){
	// access the disk from WAN
	if(sw_mode != "3" && FTP_status == 1 && ddns_enable == 1 && ddns_server.length > 0 && ddns_hostname.length > 0){
		if(FTP_mode == 1){
			$("ddnslink1").style.display = "";
			$("desc_2").style.display = "";
			$("ddnslink1_LAN").style.display = "";
		}
		else{
			$("ddnslink2").style.display = "";
			$("desc_2").style.display = "";
			$("ddnslink2_LAN").style.display = "";
			
			$("selected_account_link").href = 'ftp://'+accounts[0]+'@<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';
			showtext($("selected_account_str"), 'ftp://'+accounts[0]+'@<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>');
			$("selected_account_link_LAN").href = 'ftp://'+accounts[0]+'@<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>';
			showtext($("selected_account_str_LAN"), 'ftp://'+accounts[0]+'@<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>');
		}
		if('<% nvram_get_x("", "enable_samba"); %>' == '1' && navigator.appName.indexOf("Microsoft")>=0){
			$("desc_3").style.display = "";
			$("ddnslink3_LAN").style.display = "";
		}	
	}
	else{
		$("noWAN_link").style.display = "";
		$("ddnslink3").style.display = "";
		if('<% nvram_get_x("", "enable_samba"); %>' == '1' &&  navigator.appName.indexOf("Microsoft")>=0){
			$("desc_3").style.display = "";
			$("ddnslink3_LAN").style.display = "";
		}	
	
		if(FTP_status != 1)
			showtext($("noWAN_link"), "<#linktoFTP_no_1#>");
		else if(ddns_enable != 1)
			showtext($("noWAN_link"), "<#linktoFTP_no_2#>");
		else if(ddns_hostname.length <= 0)
			showtext($("noWAN_link"), "<#linktoFTP_no_3#>");
		else
			return false;
			//alert("System error!");
	}
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
		document.diskForm.disk.value = parent.getDiskPort(this.diskOrder);
		setTimeout("document.diskForm.submit();", 1);
	}
}

function DMhint(){
	var size_of_first_partition = 0;
	var format_of_first_partition = "";
	var	mnt_type = '<% nvram_get_x("Storage", "mnt_type"); %>';	
	for(var i = 0; i < parent.pool_names().length; ++i){
		if(parent.per_pane_pool_usage_kilobytes(i, diskOrder)[0] && parent.per_pane_pool_usage_kilobytes(i, diskOrder)[0] > 0){
			size_of_first_partition = parent.simpleNum(parent.per_pane_pool_usage_kilobytes(i, diskOrder)[0]);
			format_of_first_partition = parent.pool_types()[i];
			break;
		}
	}
	//alert(format_of_first_partition);
	/*if(format_of_first_partition != 'vfat'
			&& format_of_first_partition != 'msdos'
			&& format_of_first_partition != 'ext2'
			&& format_of_first_partition != 'ext3'
			&& format_of_first_partition != 'fuseblk'){*/
	/*if(mnt_type == "ntfs"){
		$("DMhint").style.display = "block";
		$("DMFail_reason").innerHTML = "<#DM_reason1#>";
	}
	else*/ if(size_of_first_partition <= 1){	// 0.5 = 512 Mb.
		$("DMhint").style.display = "block";
		$("DMFail_reason").innerHTML = "<#DM_reason2#>";
	}
}
</script>
</head>

<body class="statusbody" onload="initial();">
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#Modelname#>:</th>
    <td><span id="disk_model_name"></span></td>
  </tr>
  <tr>
    <th><#Totalspace#>:</th>
    <td><span id="disk_total_size"></span> GB</td>
  </tr>
</table>

<table id="mounted_item1" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#Availablespace#>:</th>
    <td><span id="disk_avail_size"></span> GB</td>
  </tr>
  <tr id="aidisk_hyperlink">
    <th><#AiDiskWizard#>:</th>
    <td>
	<input type="button" class="button" onclick="goAiDiskWizard();" value="<#btn_go#>" >
	</td>
  </tr>
</table>

<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
		<th width="130px"><#Safelyremovedisk_title#>:</th>
    <td>
	<input id="show_remove_button" type="button" class="button" onclick="remove_disk();" value="<#btn_remove#>" style="display:none;">
    <div id="show_removed_string" style="display:none;"><#Safelyremovedisk#></div>
    </td>
  </tr>
</table>
<div id="unmounted_refresh" style="padding:5px 0px 5px 25px; display:none">
<ul style="font-size:11px; font-family:Arial; padding:0px; margin:0px; list-style:outside; line-height:150%;">
	<li><#DiskStatus_refresh1#><a href="/" target="_parent"><#DiskStatus_refresh2#></a><#DiskStatus_refresh3#></li>
</ul>
</div>
<div id="mounted_item2" style="padding:5px 0px 5px 25px; ">
<ul style="font-size:11px; font-family:Arial; padding:0px; margin:0px; list-style:outside; line-height:150%;">
	<!--li><#linktodisk#>
	  <span id="ie_link" style="display:none;"><a href="\\<% nvram_get_x("Storage", "computer_name"); %>" target="_blank">\\<% nvram_get_x("Storage", "computer_name"); %></a></span>
	  <span id="notie_link" style="display:none;"><a href="ftp://<% nvram_get_x("Storage", "computer_name"); %>/" target="_blank">ftp://<% nvram_get_x("Storage", "computer_name"); %>/</a></span>
	  <span id="noLAN_link" style="display:none;"></span>
	</li-->
	<li>
	  <span id="ddnslink1" style="display:none;"><#Internet#>&nbsp;<#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank">ftp://<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %></a></span>
	  <span id="ddnslink2" style="display:none;"><#Internet#>&nbsp;<#AiDisk_linktoFTP_fromInternet#><a id="selected_account_link" href="" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank"><span id="selected_account_str"></span></a></span>
	  <span id="ddnslink3" style="display:none;"><#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>" target="_blank">ftp://<% nvram_get_x("", "lan_ipaddr_t"); %></a></span>
		<span id="noWAN_link" style="display:none;"></span>
	</li>
	<li id="desc_2" style="display:none;">
		<span id="ddnslink1_LAN" style="display:none;"><#linktodisk#>
	  	<a href="ftp://<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>" target="_blank">ftp://<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %></a>
	  </span>
	  <span id="ddnslink2_LAN" style="display:none;"><#linktodisk#>
	  	<a id="selected_account_link_LAN" href="" target="_blank"><span id="selected_account_str_LAN"></span></a>
	  </span>
	</li>
	<li id="desc_3" style="display:none;">
		<span id="ddnslink3_LAN" style="display:none;"><#menu5_4_1#>:
			<a href="\\<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>" target="_blank">\\<% nvram_get_x("", "lan_ipaddr_t"); %></a>
		</span>		
	</li>
</ul>
<div id="DMhint" class="DMhint">
<#DM_hint1#> <span id="DMFail_reason"></span>
</div>
</div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="disk" value="">
</form>
</body>
</html>

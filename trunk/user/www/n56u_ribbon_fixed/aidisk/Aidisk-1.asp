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
<script type="text/javascript" src="/state.js"></script>
<script>
var dummyShareway = '<% nvram_get_x("", "dummyShareway"); %>';

var FTP_status = parent.get_ftp_status();  // FTP  0=disable 1=enable
var FTP_mode = parent.get_share_management_status("ftp");  // if share by account. 1=no 2=yes
var accounts = [<% get_all_accounts("ftp"); %>];
var ddns_enable = '<% nvram_get_x("", "ddns_enable_x"); %>';
var ddns_server = '<% nvram_get_x("", "ddns_server_x"); %>';
var ddns_hostname = '<% nvram_get_x("", "ddns_hostname_x"); %>';
var format_of_first_partition = parent.pool_types()[0]; //"ntfs";

function initial(){
	parent.hideLoading();
	showdisklink();
	parent.openHint(15, 1);
}

function showdisklink(){
	if(detect_mount_status() == 0){
		$("Nodisk_hint").style.display = 'block';
		$("AiDiskWelcome_desp").style.display = 'none';
		$("linkdiskbox").style.display = 'none';
		$("gotonext").style.display = 'none';
		return;
	}
	else if(dummyShareway != ""){
		$("AiDiskWelcome_desp").style.display = 'none';
		$("linkdiskbox").style.display = 'block';
		$("long_btn_go").innerHTML = "<#AiDiskWelcome_set_again#>";
		
		show_share_link();
	}
	else{
		$("linkdiskbox").style.display = 'none';
	}
}

function show_share_link(){
	if(FTP_status == 1 && ddns_enable == 1 && ddns_server.length > 0 && ddns_hostname.length > 0){
		if(FTP_mode == 1 || FTP_mode == 3 || dummyShareway == 0){
			$("ddnslink1").style.display = ""; 
			$("desc_2").style.display = ""; 
			$("ddnslink1_LAN").style.display = ""; 
		}
		else if(FTP_mode == 2){
			$("ddnslink2").style.display = "";
			$("desc_2").style.display = ""; 
			$("ddnslink2_LAN").style.display = ""; 
		}
	}
	else{
		$("noWAN_link").style.display = "";
		
		if(FTP_status != 1)
			showtext($("noWAN_link"), "<#linktoFTP_no_1#>");
		else if(ddns_enable != 1)
			showtext($("noWAN_link"), "<#linktoFTP_no_2#>");
		else if(ddns_hostname.length <= 0)
			showtext($("noWAN_link"), "<#linktoFTP_no_3#>");
		else
			alert("FTP and ddns exception");
	}
}

function detect_mount_status(){
	var mount_num = 0;

	for(var i = 0; i < parent.foreign_disk_total_mounted_number().length; ++i)
		mount_num += parent.foreign_disk_total_mounted_number()[i];

	return mount_num;
}

function go_next_page(){
	document.redirectForm.action = "/aidisk/Aidisk-2.asp";
	//document.redirectForm.target = "_self";
	document.redirectForm.submit();
}
</script>
</head>

<body class="body_iframe" onload="initial();">
<form method="GET" name="redirectForm" action="">
<input type="hidden" name="flag" value="">
</form>
<table class="table" width="100%" cellpadding="0" cellspacing="0" >
    <tr>
        <td style="border-top: 0 none;"><h3><#AiDiskWelcome_title#></h3></td>
    </tr>
    <tr>
        <td class="textbox">
	        <div id="AiDiskWelcome_desp">
	            <#AiDiskWelcome_desp#>
	  	        <ul>
                    <li><#AiDiskWelcome_desp1#></li>
                    <li><#AiDiskWelcome_desp2#></li>
                </ul>
	        </div>
	
	        <div id="linkdiskbox">
	            <#AiDisk_wizard_text_box_title3#><br/>
	                <ul>
	                    <li>
	                        <span id="ddnslink1" style="display:none;">
	  	                        <#Internet#> <#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("", "ddns_hostname_x"); %>" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank">ftp://<% nvram_get_x("", "ddns_hostname_x"); %></a>
	                        </span>
	                        <span id="ddnslink2" style="display:none;">
	  	                        <#Internet#>&nbsp;<#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("", "acc_username0"); %>@<% nvram_get_x("", "ddns_hostname_x"); %>" onclick="alert('<#AiDiskWelcome_desp1#>');" target="_blank">ftp://<% nvram_get_x("", "acc_username0"); %>@<% nvram_get_x("", "ddns_hostname_x"); %></a>
	                        </span>
	                        <span id="noWAN_link" style="display:none;"></span>
                        </li>
                        <li id="desc_2" style="display:none;">
  	                        <span id="ddnslink1_LAN" style="display:none;">
	  	                        <#t2LAN#>&nbsp;<#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("", "lan_ipaddr"); %>" target="_blank">ftp://<% nvram_get_x("", "lan_ipaddr"); %></a>
	                        </span>
  	                        <span id="ddnslink2_LAN" style="display:none;">
	  	                        <#t2LAN#>&nbsp;<#AiDisk_linktoFTP_fromInternet#><a href="ftp://<% nvram_get_x("", "acc_username0"); %>@<% nvram_get_x("", "lan_ipaddr"); %>" target="_blank">ftp://<% nvram_get_x("", "acc_username0"); %>@<% nvram_get_x("", "lan_ipaddr"); %></a>
	                        </span>
                        </li>
	                </ul>
	        </div>
	        <br />
	        <span id="Nodisk_hint" class="alert alert-danger" style="display: none"><#AiDisk_wizard_text_box_title1#></span>
	    </td>
    </tr>
    <tr align="center">
        <td height="40" style="border-top: 0 none;">
            <div id="gotonext" class="long_btn">
      	        <a href="javascript:go_next_page();" class="btn btn-primary" style="min-width: 219px;" id="long_btn_go"><#btn_go#></a>
            </div>
        </td>
    </tr>
</table>
</body>
</html>

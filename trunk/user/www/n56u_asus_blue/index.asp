<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link href="images/map-iconRouter_iphone.png" rel="apple-touch-icon" />
<title>ASUS Wireless Router <#Web_Title#> - <#menu1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="NM_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<style type="text/css">
.style1 {color: #006633}
.style4 {color: #333333}
.style5 {
	color: #CC0000;
	font-weight: bold;
}
</style>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/alttxt.js"></script>
<script type="text/javascript" src="/aplist.js"></script>
<script>
// for client_function.js
<% login_state_hook(); %>

openHint = null; // disable the openHint().

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

Dev3G = '<% nvram_get_x("General",  "d3g"); %>';
modem_model_name1 = '<% nvram_get_x("", "usb_path1_product"); %>';
modem_model_name2 = '<% nvram_get_x("", "usb_path2_product"); %>';

<% disk_pool_mapping_info(); %>
<% available_disk_names_and_sizes(); %>
<% wanlink(); %>
<% get_printer_info(); %>

var all_disks = foreign_disks().concat(blank_disks());
var all_disk_interface = foreign_disk_interface_names().concat(blank_disk_interface_names());

var flag = '<% get_parameter("flag"); %>';
var disk_number = foreign_disks().length+blank_disks().length;

var leases = [<% dhcp_leases(); %>];	// [[hostname, MAC, ip, lefttime], ...]
var arps = [<% get_arp_table(); %>];		// [[ip, x, x, MAC, x, type], ...]
var arls = [<% get_arl_table(); %>];		// [[MAC, port, x, x], ...]
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var ipmonitor = [<% get_static_client(); %>];	// [[IP, MAC, DeviceName, Type, http, printer, iTune], ...]
var networkmap_fullscan = '<% nvram_match_x("", "networkmap_fullscan", "0", "done"); %>'; //2008.07.24 Add.  1 stands for complete, 0 stands for scanning.;
var usb_path1 = '<% nvram_get_x("", "usb_path1"); %>';
var usb_path2 = '<% nvram_get_x("", "usb_path2"); %>';

var clients = getclients(1);
var $j = jQuery.noConflict();

function initial(){
	show_banner(0);
	show_menu(1, -1, 0);
	show_footer();
	show_device();
	show_middle_status();
	show_client_status(clients.length);
	set_default_choice();
	
	if(sw_mode == "3"){
		showMapWANStatus(3);
		MapUnderAPmode();
	}
	else
		showMapWANStatus(2);

	load_alttxt_enviroment();
	
//	setTimeout('detect_update_info();', 6000);
}

function detect_update_info(){
	var str = $("internetStatus").innerHTML;
	if(str == "<#QKSet_detect_freshbtn#>...")
			refreshpage();
}

function set_default_choice(){
	var icon_name;
	if(flag && flag.length > 0 && wan_route_x != "IP_Bridged"){
		if(flag == "Internet")
			$("statusframe").src = "/device-map/internet.asp";
		else if(flag == "Client")
			$("statusframe").src = "/device-map/clients.asp";
		else if(flag == "Router2g")
			$("statusframe").src = "/device-map/router2g.asp";
		else if(flag == "Router5g")
			$("statusframe").src = "/device-map/router.asp";
		else{
			clickEvent($("iconRouter"));
			return;
		}
		if(flag == "Router2g" || flag == "Router5g" )
			icon_name = "iconRouter";
		else
			icon_name = "icon"+flag;
		clickEvent($(icon_name));
	}
	else
		clickEvent($("iconRouter"));
}

function showMapWANStatus(flag){
	if(sw_mode == "3"){
		showtext($("internetStatus"), "<#WLANConfig11b_x_APMode_itemname#>");
		$("ifconnect").style.display = "none";
	}
	else{
		if(flag == 1){
			showtext($("internetStatus"), "<#Connected#>");
			$("ifconnect").style.display = "none";
		}
		else if(flag == 2){
			showtext($("internetStatus"), "<#QKSet_detect_freshbtn#>...");
			$("ifconnect").style.display = "none";
		}
		else{
			showtext($("internetStatus"), "<#Disconnected#>");
			$("ifconnect").style.display = "block";
		}
	}
}

function show_middle_status(){
	if(ssid2.length > 15){
		ssid2 = ssid2.substring(0,14) + "...";
	}
	
	//$("SSID").value = ssid2;
	
	var auth_mode = document.form.rt_auth_mode.value;
	var wpa_mode = document.form.rt_wpa_mode.value;
	var wl_wep_x = parseInt(document.form.rt_wep_x.value);
	var security_mode;
	
	if(auth_mode == "open")
		security_mode = "Open System";
	else if(auth_mode == "shared")
		security_mode = "Shared Key";
	else if(auth_mode == "psk"){
		if(wpa_mode == "1")
			security_mode = "WPA-Personal";
		else if(wpa_mode == "2")
			security_mode = "WPA2-Personal";
		else if(wpa_mode == "0")
			security_mode = "WPA-Auto-Personal";
		else
			alert("System error for showing auth_mode!");
	}
	else if(auth_mode == "wpa"){
		if(wpa_mode == "3")
			security_mode = "WPA-Enterprise";
		else if(wpa_mode == "4")
			security_mode = "WPA-Auto-Enterprise";
		else
			alert("System error for showing auth_mode!");
	}
	else if(auth_mode == "wpa2")
		security_mode = "WPA2-Enterprise";
	else if(auth_mode == "radius")
		security_mode = "Radius with 802.1x";
	else
		alert("System error for showing auth_mode!");
	$("wl_securitylevel_span").innerHTML = security_mode;
	
	if(auth_mode == "open" && wl_wep_x == 0)
		$("iflock").style.background = 'url(images/unlock_icon.gif) no-repeat';
	else
		$("iflock").style.background = 'url(images/lock_icon.gif) no-repeat';
	
	$("iflock").style.display = "block";
}

function show_client_status(clients_count){
	var client_str = "";
	var wired_num = 0, wireless_num = 0;
	
	if(sw_mode == "1" || sw_mode == "4"){
		client_str += "<#Full_Clients#>: <span>"+clients_count+"</span>";
	}
	else		
		client_str += "<#Noclients#>";
	
	$("clientNumber").innerHTML = client_str;
}

function show_device(){
	var usb_path1 = '<% nvram_get_x("", "usb_path1"); %>';
	var usb_path2 = '<% nvram_get_x("", "usb_path2"); %>';
	
	switch(usb_path1){
		case "storage":
			for(var i = 0; i < all_disks.length; ++i)
				if(foreign_disk_interface_names()[i] == "1"){
					disk_html(0, i);
					break;
				}
			break;
		case "printer":
			printer_html(0, 0);
			break;
		case "modem":
			modem_html(0, 0);
			break;
		case "WIMAX":
			WIMAX_html(0, 0);
			break;
		case "audio":
		case "webcam":
		default:
			no_device_html(0);
	}
	
	// show the upper usb device
	switch(usb_path2){
		case "storage":
			for(var i = 0; i < all_disks.length; ++i)
				if(foreign_disk_interface_names()[i] == "2"){
					disk_html(1, i);
					break;
				}
			break;
		case "printer":
			printer_html(1, 1);
			break;
		case "modem":
			modem_html(1, 0);
			break;
		case "WIMAX":
			WIMAX_html(1, 0);
			break;
		case "audio":
		case "webcam":
		default:
			no_device_html(1);
	}
}

function disk_html(device_order, all_disk_order){
	var device_icon = $("deviceIcon_"+device_order);
	var device_dec = $("deviceDec_"+device_order);
	var icon_html_code = '';
	var dec_html_code = '';
	
	var disk_model_name = "";
	var TotalSize;
	var mount_num = getDiskMountedNum(all_disk_order);
	var all_accessable_size;
	var percentbar = 0;
	
	if(all_disk_order < foreign_disks().length)
		disk_model_name = foreign_disk_model_info()[all_disk_order];
	else
		disk_model_name = blank_disks()[all_disk_order-foreign_disks().length];
	
	icon_html_code += '<a href="device-map/disk.asp" target="statusframe">\n';
	icon_html_code += '    <div id="iconUSBdisk'+all_disk_order+'" class="iconUSBdisk" onclick="setSelectedDiskOrder(this.id);clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	dec_html_code += disk_model_name+'<br>\n';
	
	if(mount_num > 0){
		if(all_disk_order < foreign_disks().length)
			TotalSize = simpleNum(foreign_disk_total_size()[all_disk_order]);
		else
			TotalSize = simpleNum(blank_disk_total_size()[all_disk_order-foreign_disks().length]);
		
		all_accessable_size = simpleNum2(computeallpools(all_disk_order, "size")-computeallpools(all_disk_order, "size_in_use"));
		
		percentbar = simpleNum2((all_accessable_size)/TotalSize*100);
		percentbar = Math.round(100-percentbar);		
		dec_html_code += '<div id="diskquota">\n';
		dec_html_code += '<img src="images/quotabar.gif" width="'+percentbar+'" height="13">';
		dec_html_code += '</div>\n';
		dec_html_code += '<strong><#Totaldisk#></strong>: '+TotalSize+' GB<br>\n';		
		dec_html_code += '<span class="style1"><strong><#Availdisk#></strong>: '+(all_accessable_size)+' GB</span>\n';
	}
	else{
		dec_html_code += '<span class="style1"><strong><#DISK_UNMOUNTED#></strong></span>\n';
	}
	
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

function printer_html(device_seat, printer_order){
	var printer_name = printer_manufacturers()[printer_order]+" "+printer_models()[printer_order];
	var printer_status = "";
	var device_icon = $("deviceIcon_"+device_seat);
	var device_dec = $("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	
	if(printer_pool()[printer_order] != "")
		printer_status = '<#CTL_Enabled#>';
	else
		printer_status = '<#CTL_Disabled#>';
	
	icon_html_code += '<a href="device-map/printer.asp" target="statusframe">\n';
	icon_html_code += '    <div id="iconPrinter'+printer_order+'" class="iconPrinter" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	dec_html_code += printer_name+'<br>\n';
	dec_html_code += '<span class="style5">'+printer_status+'</span>\n';
	
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

var selectedModemOrder = "";

function setSelectedModemOrder(selectedModemId){
	this.selectedModemOrder = parseInt(selectedModemId.substring(selectedModemId.length-1));
}

function getSelectedModemOrder(){
	return this.selectedModemOrder;
}

function modem_html(device_seat, modem_order){
	//var modem_name = Dev3G;  //Viz 2011.09
	var modem_name1 = modem_model_name1;
	var modem_name2 = modem_model_name2;
	var modem_status = "Connected";
	var device_icon = $("deviceIcon_"+device_seat);
	var device_dec = $("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	
	icon_html_code += '<a href="device-map/modem.asp" target="statusframe">\n';
	icon_html_code += '    <div id="iconModem'+device_seat+'" class="iconModem" onclick="setSelectedModemOrder(this.id);clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	//dec_html_code += modem_name+'<br>\n'; //Viz 2011.09
	if(device_seat==0)
		dec_html_code += modem_name1+'<br>\n';
	else	
		dec_html_code += modem_name2+'<br>\n';
	//dec_html_code += '<span class="style1"><strong>'+modem_status+'</strong></span>\n';
	//dec_html_code += '<br>\n';
	//dec_html_code += '<img src="images/signal_'+3+'.gif" align="middle">';
	
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

function WIMAX_html(device_seat, WIMAX_order){
	var WIMAX_status;
	var device_icon = $("deviceIcon_"+device_seat);
	var device_dec = $("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	
	if(wimax_link_status == 2)
		WIMAX_status = "Connected";
	else
		WIMAX_status = "Disconnected";
	
	icon_html_code += '<a href="device-map/wimax.asp" target="statusframe">\n';
	icon_html_code += '    <div id="iconWIMAX'+device_seat+'" class="icon4G" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	dec_html_code += wimax_model_name+'<br>\n';
	dec_html_code += '<span class="style1"><strong id="wimax_icon_status">'+WIMAX_status+'</strong></span>\n';
	dec_html_code += '<br>\n';
	
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

function no_device_html(device_seat){
	var device_icon = $("deviceIcon_"+device_seat);
	var device_dec = $("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	
	icon_html_code += '    <div class="iconNo"></div>';
	
	dec_html_code += '<span class="account style4"><#NoDevice#></span>\n';
	
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

var avoidkey;
var lastClicked;
var lastName;
var clicked_device_order;

function get_clicked_device_order(){
	return clicked_device_order;
}

function clickEvent(obj){
	var icon;
	var ContainerWidth;
	var Containerpadding;
	var stitle;
	var seat;

	clicked_device_order = -1;

	if(obj.id == "iflock"){
		obj = $("iconRouter");
	}	

	if(obj.id.indexOf("Internet") > 0){
		icon = "iconInternet";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_Internet#>";
		$("statusframe").src = "/device-map/internet.asp";
	}
	else if(obj.id.indexOf("Router") > 0){
		icon = "iconRouter";
		ContainerWidth = "320px";
		Containerpadding = "4px";
		stitle = "<#statusTitle_System#>";
	}
	else if(obj.id.indexOf("Client") > 0){
		if(sw_mode != "1" && sw_mode != "4")
			return;
		
		icon = "iconClient";
		ContainerWidth = "396px";
		Containerpadding = "0px";
		stitle = "<#statusTitle_Client#>";
	}
	else if(obj.id.indexOf("USBdisk") > 0){
		icon = "iconUSBdisk";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_USB_Disk#>";
		$("statusframe").src = "/device-map/disk.asp";
	}
	else if(obj.id.indexOf("Modem") > 0){
		icon = "iconModem";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#menu5_4_4#>";
		$("statusframe").src = "/device-map/modem.asp";
	}
	else if(obj.id.indexOf("Printer") > 0){
		seat = obj.id.indexOf("Printer")+7;
		clicked_device_order = parseInt(obj.id.substring(seat, seat+1));
		
		icon = "iconPrinter";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_Printer#>";
	}
	else if(obj.id.indexOf("Remote") > 0){
		icon = "iconRemote";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_AP#>";
		$("statusframe").src = "/device-map/remote.asp";
		//alert($("statusframe").src);
		//alert(obj.id);
	}	
	else if(obj.id.indexOf("No") > 0){
		icon = "iconNo";
	}
	else
		alert("mouse over on wrong place!");
	
	$('statusContainer').style.width = ContainerWidth;
	$('statusContainer').style.paddingRight = Containerpadding;

	if(lastClicked){
		lastClicked.style.background = 'url(images/map-'+lastName+'.gif) no-repeat';
	}
	
	obj.style.background = 'url(images/map-'+icon+'_d.gif) no-repeat';
	$('statusIcon').style.background = "url(images/iframe-"+ icon +".gif) no-repeat";
	
	$('helpname').innerHTML = stitle;
	
	avoidkey = icon;
	lastClicked = obj;
	lastName = icon;
}

function mouseEvent(obj, key){
	var icon;
	
	if(obj.id.indexOf("Internet") > 0)
		icon = "iconInternet";
	else if(obj.id.indexOf("Router") > 0)
		icon = "iconRouter";
	else if(obj.id.indexOf("Client") > 0){
		if(wan_route_x == "IP_Bridged")
			return;
		
		icon = "iconClient";
	}
	else if(obj.id.indexOf("USBdisk") > 0)
		icon = "iconUSBdisk";
	else if(obj.id.indexOf("Printer") > 0)
		icon = "iconPrinter";
	else if(obj.id.indexOf("No") > 0)
		icon = "iconNo";
	else
		alert("mouse over on wrong place!");
	
	if(avoidkey != icon){
		if(key){ //when mouseover
			obj.style.background = 'url("/images/map-'+icon+'_r.gif") no-repeat';
		}
		else {  //when mouseout
			obj.style.background = 'url("/images/map-'+icon+'.gif") no-repeat';
		}
	}
}//end of mouseEvent

function MapUnderAPmode(){// if under AP mode, disable the Internet icon and show hint when mouseover.
	
		//showtext($("internetStatus"), "<#OP_AP_item#>");
		
		$("iconInternet").style.background = "url(images/map-iconRemote.gif) no-repeat";
		$("iconInternet").style.cursor = "default";
		
		$("iconInternet").onmouseover = function(){
			writetxt("<#underAPmode#>");
		}
		$("iconInternet").onmouseout = function(){
			writetxt(0);
		}
		$("iconInternet").onclick = function(){
			return false;
		}
		$("clientStatusLink").href = "javascript:void(0)";
		$("clientStatusLink").style.cursor = "default";	
		$("iconClient").style.background = "url(images/map-iconClient_0.gif) no-repeat";
		$("iconClient").style.cursor = "default";
}
</script>
</head>

<body onunload="return unload_body();">
<noscript>
	<div class="popup_bg" style="visibility:visible; z-index:999;">
		<div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"><#not_support_script#></p></div>
	</div>
</noscript>

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br>
				<br>
		    </div>
		  <div class="drImg"><img src="images/DrsurfImg.gif"></div>
			<div style="height:70px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form name="form">
<input type="hidden" name="current_page" value="index.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="wl_auth_mode" value="<% nvram_get_x("",  "wl_auth_mode"); %>">
<input type="hidden" name="wl_wpa_mode" value="<% nvram_get_x("",  "wl_wpa_mode"); %>">
<input type="hidden" name="wl_wep_x" value="<% nvram_get_x("",  "wl_wep_x"); %>">
<input type="hidden" name="rt_auth_mode" value="<% nvram_get_x("",  "rt_auth_mode"); %>">
<input type="hidden" name="rt_wpa_mode" value="<% nvram_get_x("",  "rt_wpa_mode"); %>">
<input type="hidden" name="rt_wep_x" value="<% nvram_get_x("",  "rt_wep_x"); %>">
</form>

<form name="rt_form">
<input type="hidden" name="rt_ssid" value="">
<input type="hidden" name="rt_wpa_mode" value="">
<input type="hidden" name="rt_key1" value="">
<input type="hidden" name="rt_key2" value="">
<input type="hidden" name="rt_key3" value="">
<input type="hidden" name="rt_key4" value="">
<input type="hidden" name="rt_ssid2" value="">
<input type="hidden" name="rt_key_type" value="">
<input type="hidden" name="rt_auth_mode" value="">
<input type="hidden" name="rt_wep_x" value="">
<input type="hidden" name="rt_key" value="">
<input type="hidden" name="rt_asuskey1" value="">
<input type="hidden" name="rt_crypto" value="">
<input type="hidden" name="rt_wpa_psk" value="">
</form>

<form name="wl_form">
<input type="hidden" name="wl_ssid" value="">
<input type="hidden" name="wl_wpa_mode" value="">
<input type="hidden" name="wl_key1" value="">
<input type="hidden" name="wl_key2" value="">
<input type="hidden" name="wl_key3" value="">
<input type="hidden" name="wl_key4" value="">
<input type="hidden" name="wl_gmode" value="">
<input type="hidden" name="wl_ssid2" value="">
<input type="hidden" name="wl_key_type" value="">
<input type="hidden" name="wl_auth_mode" value="">
<input type="hidden" name="wl_wep_x" value="">
<input type="hidden" name="wl_key" value="">
<input type="hidden" name="wl_asuskey1" value="">
<input type="hidden" name="wl_crypto" value="">
<input type="hidden" name="wl_wpa_psk" value="">
</form>

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" width="23"><div id="Dr_body"></div></td>
		
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="204">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td align="center" valign="top"  class="bgarrow">
		
		<!--=====Beginning of Network Map=====-->
		<div id="tabMenu"></div><br>
		<table width="350" border="0" cellspacing="0" cellpadding="0">
		  <tr align="left">
            <td colspan="2"><table border="0" cellpadding="0" cellspacing="0" style="margin-left:100px; ">
              <tr>
                <td width="95">
				<a href="/device-map/internet.asp" target="statusframe">
					<div id="iconInternet" onclick="clickEvent(this);"></div>
				</a>
				<a href="/device-map/remote.asp" target="statusframe">
					<div id="iconRemote" onclick="clickEvent(this);" style="display='none'"></div>
				</a>
				<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
				</td>
                <td class="NMdesp"><span id="internetStatus"></span></td>
              </tr>
            </table>
			</td>
          </tr>
          <tr align="left" valign="middle" >
            <!--td height="20" colspan="2" style="background:url(images/map-icon-arror.gif) no-repeat 98px;"><div class="ifconnect" id="ifconnect"></div></td-->
			<td id="wan_link_obj" height="20" colspan="2" style="background:url(images/map-icon-arror.gif) repeat-y 98px;">
					<!--object id="wl_radio_obj" style="margin-left:125px; visibility:hidden;"classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0" width="30" height="30">
						<param name="movie" value="/images/radio.png" />
						<param name="wmode" value="transparent">
						<param name="quality" value="high" />
						<embed src="/images/radio.png" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer" type="application/x-shockwave-flash" width="30" height="30" wmode="transparent"></embed>
				  </object-->				
				<div class="ifconnect" id="ifconnect"></div>
			</td>
          </tr>
      <tr>
      <td colspan="2">
			<table class="NMitem" border="0" cellspacing="0" cellpadding="0" style="margin-left:100px; height:77px;">
        <tr>
				<td width="95" align="left">
					<a href="device-map/router2g.asp" target="statusframe"><div id="iconRouter" onclick="clickEvent(this);"></div></a>
				</td>
				<td class="NMdesp">
					<a href="device-map/router2g.asp" target="statusframe"><div id="iflock" onclick="clickEvent(this);"></div></a>
					<strong><#statusTitle_System#></strong>&nbsp;
					<strong><#Security_Level#></strong>:</br>
					<span id="wl_securitylevel_span"></span>
				</td>
        </tr>
      </table>
			</td>
      </tr>
      
			<tr>
            <td colspan="2" style="background:url(images/map-icon-arror1.gif) no-repeat 80px;">&nbsp;</td>
          </tr>
          <tr>
            <td width="130" align="right" valign="top">
				<table width="120" border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td align="center">
							<a id="clientStatusLink" href="device-map/clients.asp" target="statusframe"><!--lock 1226-->
								<div id="iconClient" onclick="clickEvent(this);"></div>
							</a>
						</td>
					</tr>
					<tr>
						<td align="center" class="clients" id="clientNumber"></td><!--lock 1226-->
					</tr>
              </table>
			</td>
			
			<td align="center" class="mapgroup">
				
				<!--div class="hardware_title">USB裝置</div-->
				<table width="95%" border="0" cellpadding="0" cellspacing="0" style="margin-left:5px; ">
					<tr id="device_0">
						<td width="88" height="90">
							<div id="deviceIcon_0"></div>
						</td>
						<td height="90" class="NMdesp">
							<div id="deviceDec_0"></div>
						</td>
					</tr>
					
					<tr id="device_1">
						<td width="88" height="90">
							<div id="deviceIcon_1"></div>
						</td>
						<td height="90" class="NMdesp">
							<div id="deviceDec_1"></div>
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>        
</td>
		<!--=====End of Main Content=====-->
	
	<!--==============Beginning of hint content=============-->
	<td id="statusContainer" width="300" height="540" align="left" valign="top" >
		<div id="statusIcon"></div>
	  <table width="95%" border="0" cellpadding="0" cellspacing="0">
		<tr>
		  <td class="statusTitle">
		  	<!--img id="statusIcon" src="images/iframe-iconRouter.gif"/-->
		  	<!--span id="helpname"><#statusTitle_System#></span-->
		  	<div id="helpname"></div>
		  </td>
		</tr>
		<tr>
		  <td>
			<iframe id="statusframe" name="statusframe" src="/device-map/router2g.asp" frameborder="0" width="100%" height="500"></iframe>
		  </td>
		</tr>
	  </table>
	</td>
	<!--==============Ending of hint content=============-->
  </tr>
</table>
<div id="navtxt" class="navtext" style="position:absolute; top:50px; left:-100px; visibility:hidden; font-family:Arial, Verdana"></div>
<div id="footer"></div>
<script>
if(flag == "Internet" || flag == "Client")
	$("statusframe").src = "";
	initial()
</script>
</body>
</html>

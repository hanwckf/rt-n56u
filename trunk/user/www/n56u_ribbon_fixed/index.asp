<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<!-- <meta name="viewport" content="width=device-width, initial-scale=1.0"> -->
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link href="images/map-iconRouter_iphone.png" rel="apple-touch-icon" />
<title>ASUS Wireless Router RT-N56U - Network Map</title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
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
    $j("#internetStatus").removeClass("badge badge-success badge-warning badge-important");

	if(sw_mode == "3"){
		//showtext($("internetStatus"), "<#WLANConfig11b_x_APMode_itemname#>");
		$j("#internetStatus").addClass("badge badge-success");
		$j("#internetStatus").html('<i class="icon-ok icon-white"></i>');
		//$("ifconnect").style.display = "none";
	}
	else{
		if(flag == 1){
			//showtext($("internetStatus"), "<#Connected#>");
			$j("#internetStatus").addClass("badge badge-success");
			$j("#internetStatus").html('<i class="icon-ok icon-white"></i>');
			//$("ifconnect").style.display = "none";
		}
		else if(flag == 2){
			//showtext($("internetStatus"), "<#QKSet_detect_freshbtn#>...");
			$j("#internetStatus").addClass("badge badge-warning");
			$j("#internetStatus").html('<i class="icon-minus icon-white"></i>');
			//$("ifconnect").style.display = "none";
		}
		else{
			//showtext($("internetStatus"), "<#Disconnected#>");
			$j("#internetStatus").addClass("badge badge-important");
			$j("#internetStatus").html('<i class="icon-remove icon-white"></i>');
			//$("ifconnect").style.display = "block";
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
	//$("wl_securitylevel_span").innerHTML = security_mode;

	if(auth_mode == "open" && wl_wep_x == 0)
	{
	    $j("#wl_securitylevel_span").addClass("badge badge-important");
        $j("#wl_securitylevel_span").html('<i class="icon-exclamation-sign icon-white"></i>');
	}
    else
    {
        $j("#wl_securitylevel_span").addClass("badge badge-success");
        $j("#wl_securitylevel_span").html('<i class="icon-lock icon-white"></i>');
    }

	/*if(auth_mode == "open" && wl_wep_x == 0)
		$("iflock").style.background = 'url(images/unlock_icon.gif) no-repeat';
	else
		$("iflock").style.background = 'url(images/lock_icon.gif) no-repeat';
	
	$("iflock").style.display = "block";*/
}

function show_client_status(clients_count){
	var client_str = "";
	var wired_num = 0, wireless_num = 0;
	
	if(sw_mode == "1" || sw_mode == "4"){
		client_str += "<#Full_Clients#>: <span>"+clients_count+"</span>";
	}
	else
    {
        clients_count = 0;
        client_str += "<#Noclients#>";
    }

	$j("#clientNumber").addClass("badge badge-success");
	if(clients_count < 10)
	    $j("#clientNumber").css({paddingLeft: '6px', paddingRight: '7px'});
	else
	    $j("#clientNumber").css({paddingLeft: '3px', paddingRight: '4px'});

    $j("#clientNumber").html(clients_count);

	//$("clientNumber").innerHTML = client_str;
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
	var alertPercentbar = 'progress-info';
	var progressBarDiv = '';

	if(all_disk_order < foreign_disks().length)
		disk_model_name = foreign_disk_model_info()[all_disk_order];
	else
		disk_model_name = blank_disks()[all_disk_order-foreign_disks().length];
	
	//dec_html_code += disk_model_name+'<br>\n';
	
	if(mount_num > 0){
		if(all_disk_order < foreign_disks().length)
			TotalSize = simpleNum(foreign_disk_total_size()[all_disk_order]);
		else
			TotalSize = simpleNum(blank_disk_total_size()[all_disk_order-foreign_disks().length]);
		
		all_accessable_size = simpleNum2(computeallpools(all_disk_order, "size")-computeallpools(all_disk_order, "size_in_use"));
		
		percentbar = simpleNum2((all_accessable_size)/TotalSize*100);
		percentbar = Math.round(100-percentbar);
		if(percentbar > 50 && percentbar <= 80){
            alertPercentbar = 'progress-warning';
		}
		else if(percentbar > 80) {
		    alertPercentbar = 'progress-danger';
		}

		dec_html_code += '<div id="diskquota">\n';
		//dec_html_code += '<img src="images/quotabar.gif" width="'+percentbar+'" height="13">';
		dec_html_code += progressBarDiv = '<div style="margin-bottom: 10px;" class="progress ' + alertPercentbar + '"><div class="bar" style="width:'+percentbar+'%">'+(percentbar > 10 ? (percentbar + '%') : '')+'</div></div>';
		dec_html_code += '</div>\n';
		dec_html_code += '<strong><#Totaldisk#></strong>: '+TotalSize+' GB<br>\n';		
		dec_html_code += '<span class="style1"><strong><#Availdisk#></strong>: '+(all_accessable_size)+' GB</span>\n';
	}
	else{
		dec_html_code += '<span class="style1"><strong><#DISK_UNMOUNTED#></strong></span>\n';
	}

	icon_html_code += '<a href="device-map/disk.asp" target="statusframe">\n';
    icon_html_code += '    <div rel="rollover_disk" data-original-title="'+disk_model_name+'" data-content="'+(dec_html_code.replace(new RegExp('"', 'g'), "'"))+'" id="iconUSBdisk'+all_disk_order+'" class="big-icons big-icons-usb" onclick="setSelectedDiskOrder(this.id);clickEvent(this);"></div>\n';
    icon_html_code += '</a>\n';

	device_icon.innerHTML = icon_html_code;

	$j(device_dec).addClass("badge badge-success");
	$j(device_dec).css({paddingLeft: '3px'});
    $j(device_dec).html('<i class="icon-share icon-white"></i>');
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
	icon_html_code += '    <div id="iconPrinter'+printer_order+'" class="big-icons big-icons-usb" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	dec_html_code += printer_name+'<br>\n';
	dec_html_code += '<span class="style5">'+printer_status+'</span>\n';
	
	device_icon.innerHTML = icon_html_code;
	//device_dec.innerHTML = dec_html_code;

	$j(device_dec).addClass("badge badge-success");
	$j(device_dec).css({paddingLeft: '3px'});
	$j(device_dec).html('<i class="icon-share"></i>');
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
	icon_html_code += '    <div id="iconModem'+device_seat+'" class="big-icons big-icons-usb" onclick="setSelectedModemOrder(this.id);clickEvent(this);"></div>\n';
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
	//device_dec.innerHTML = dec_html_code;

	$j(device_dec).addClass("badge badge-success");
    $j(device_dec).css({paddingLeft: '3px'});
    $j(device_dec).html('<i class="icon-share"></i>');
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
	icon_html_code += '    <div id="iconWIMAX'+device_seat+'" class="big-icons big-icons-usb" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	
	dec_html_code += wimax_model_name+'<br>\n';
	dec_html_code += '<span class="style1"><strong id="wimax_icon_status">'+WIMAX_status+'</strong></span>\n';
	dec_html_code += '<br>\n';
	
	device_icon.innerHTML = icon_html_code;
	//device_dec.innerHTML = dec_html_code;

	$j(device_dec).addClass("badge badge-success");
    $j(device_dec).css({paddingLeft: '3px'});
    $j(device_dec).html('<i class="icon-share"></i>');
}

function no_device_html(device_seat){
	var device_icon = $("deviceIcon_"+device_seat);
	var device_dec = $("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	
	icon_html_code += '    <div class="iconNo"></div>';
	
	dec_html_code += '<span class="account style4"><#NoDevice#></span>\n';
	
	device_icon.innerHTML = icon_html_code;
	//device_dec.innerHTML = dec_html_code;
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
		//obj = $("iconRouter");
		obj = $("big-icons-router-active");
	}	

	if(obj.id.indexOf("Internet") > 0){
		//icon = "iconInternet";
		icon = "big-icons-globe-active";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_Internet#>";
		$("statusframe").src = "/device-map/internet.asp";
	}
	else if(obj.id.indexOf("Router") > 0){
		icon = "big-icons-router-active";
		ContainerWidth = "320px";
		Containerpadding = "4px";
		stitle = "<#statusTitle_System#>";
	}
	else if(obj.id.indexOf("Client") > 0){
		if(sw_mode != "1" && sw_mode != "4")
			return;

		icon = "big-icons-laptop-active";
		ContainerWidth = "396px";
		Containerpadding = "0px";
		stitle = "<#statusTitle_Client#>";
	}
	else if(obj.id.indexOf("USBdisk") > 0){
		icon = "big-icons-usb-active";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_USB_Disk#>";
		$("statusframe").src = "/device-map/disk.asp";
	}
	else if(obj.id.indexOf("Modem") > 0){
		icon = "big-icons-usb-active";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#menu5_4_4#>";
		$("statusframe").src = "/device-map/modem.asp";
	}
	else if(obj.id.indexOf("Printer") > 0){
		seat = obj.id.indexOf("Printer")+7;
		clicked_device_order = parseInt(obj.id.substring(seat, seat+1));

		icon = "big-icons-usb-active";
		ContainerWidth = "300px";
		Containerpadding = "5px";
		stitle = "<#statusTitle_Printer#>";
	}
	else if(obj.id.indexOf("Remote") > 0){
		//icon = "iconRemote";
		icon = "big-icons-globe-active";
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

	/*if(lastClicked){
		lastClicked.style.background = 'url(images/map-'+lastName+'.gif) no-repeat';
	}*/

	$j(".big-icons").removeClass("big-icons-globe-active big-icons-router-active big-icons-laptop-active big-icons-usb-active");

    $j(obj).addClass(icon);

    // show arrow right icon
    $j(".arrow-right").hide();
    $j(obj).parents('tr').find(".arrow-right").show();
	
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
		
		//$("iconInternet").style.background = "url(images/map-iconRemote.gif) no-repeat";
		$("iconInternet").style.cursor = "default";
		
		/*$("iconInternet").onmouseover = function(){
			writetxt("<#underAPmode#>");
		}
		$("iconInternet").onmouseout = function(){
			writetxt(0);
		}*/

		$("iconInternet").onclick = function(){
			return false;
		}
		$("clientStatusLink").href = "javascript:void(0)";
		$("clientStatusLink").style.cursor = "default";	
		//$("iconClient").style.background = "url(images/map-iconClient_0.gif) no-repeat";
		$("iconClient").style.cursor = "default";
}

$j(document).ready(function(){
    $j('div[rel=rollover_disk]').popover();
});
</script>

<style>
    .arrow-right{
        position: absolute;
        margin-top: -39px;
        margin-left: 69px;
        display: none;
    }

    .badge{
        padding: 2px;
        padding-bottom: 3px;
    }

    .table-big td{
        padding-top: 16px;
        padding-bottom: 16px;
    }

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

<body onunload="return unload_body();">

<div class="wrapper">
    <noscript>
        <div class="popup_bg" style="visibility:visible; z-index:999;">
            <div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"><#not_support_script#></p></div>
        </div>
    </noscript>

    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

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

    <iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no" style="position: absolute;"></iframe>

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
                    <div class="span2">
                        <div class="well" style="height: 570px; padding-left: 18px;">
                            <div id="Dr_body"></div>
                            <div id="tabMenu"></div>

                            <table class="table table-big" style="margin-top: 12px;">
                                <tbody>
                                    <tr>
                                        <td width="30%">
                                            <a href="/device-map/internet.asp" target="statusframe">
                                                <div id="iconInternet" class="big-icons big-icons-globe" onclick="clickEvent(this);"></div>
                                            </a>
                                            <a href="/device-map/remote.asp" target="statusframe">
                                                <div id="iconRemote" class="big-icons big-icons-globe" onclick="clickEvent(this);" style="display: none"></div>
                                            </a>
                                            <div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
                                            <div style="position: absolute; margin-top: -47px; margin-left: 50px;"><div id="internetStatus" style="padding-left: 3px;"></div></div>

                                            <div class="arrow-right" id="arrow-internet"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>
                                        <!--<td>
                                            <div style="margin-top: 15px"><div id="internetStatus"></div></div>
                                        </td>
                                        <td width="5%">
                                            <div class="arrow-right" id="arrow-internet"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td> -->
                                    </tr>

                                    <tr>
                                        <td>
                                            <a href="device-map/router2g.asp" target="statusframe"><div id="iconRouter" class="big-icons big-icons-router" onclick="clickEvent(this);"></div></a>
                                            <div style="position: absolute; margin-top: -47px; margin-left: 50px;"><div id="wl_securitylevel_span" style="padding-right: 3px;"></div></div>

                                            <div class="arrow-right" id="arrow-router"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>
                                        <!--<td>
                                            <div style="margin-top: 15px;"><div class="label label-info"><span id="wl_securitylevel_span"></span></div></div>
                                        </td>
                                        <td width="5%">
                                            <div class="arrow-right" id="arrow-router"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>-->
                                    </tr>

                                    <tr>
                                        <td>
                                            <a id="clientStatusLink" href="device-map/clients.asp" target="statusframe"><div id="iconClient" class="big-icons big-icons-laptop" onclick="clickEvent(this);"></div></a>
                                            <div style="position: absolute; margin-top: -47px; margin-left: 50px;"><b><div id="clientNumber">&nbsp;</div></b></div>

                                            <div class="arrow-right" id="arrow-clients"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>
                                        <!--<td>
                                            <div style="margin-top: 15px;"><div id="clientNumber" class="label label-info">&nbsp;</div></div>
                                        </td>
                                        <td width="5%">
                                            <div class="arrow-right" id="arrow-clients"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td> -->
                                    </tr>

                                    <tr>
                                        <td width="30%">
                                            <div id="deviceIcon_0" class="big-icons big-icons-usb"></div>
                                            <div style="position: absolute; margin-top: -47px; margin-left: 50px;"><div id="deviceDec_0"></div></div>

                                            <div class="arrow-right" id="arrow-usb1"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>
                                        <!--<td>
                                            <div id="deviceDec_0" class="alert alert-info"><div style="margin-top: 15px;"></div></div>
                                        </td>
                                        <td width="5%">
                                            <div class="arrow-right" id="arrow-usb1"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>-->
                                    </tr>
                                    <tr>
                                        <td>
                                            <div id="deviceIcon_1" class="big-icons big-icons-usb"></div>
                                            <div style="position: absolute; margin-top: -47px; margin-left: 50px;"><div id="deviceDec_1"></div></div>

                                            <div class="arrow-right" id="arrow-usb2"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>
                                        <!--<td>
                                            <div id="deviceDec_1" class="alert alert-info"><div style="margin-top: 15px;"></div></div>
                                        </td>
                                        <td width="5%">
                                            <div class="arrow-right" id="arrow-usb2"><img src="/bootstrap/img/arrow-right.png"></div>
                                        </td>-->
                                    </tr>
                                </tbody>
                            </table>

                            <div class="row-fluid">
                                <div class="span12">
                                    <div id="statusContainer" width="0" height="0" align="left" valign="top" ></div>
                                </div>
                            </div>

                        </div>
                    </div>

                    <div class="span10">
                        <div class="box well grad_colour_dark_blue">
                            <div id="statusIcon" style="display: none"></div>
                            <h2 id="helpname" class="box_head round_top"></h2>

                            <div class="round_bottom">
                                <iframe id="statusframe" name="statusframe" src="/device-map/router2g.asp" frameborder="0" width="100%" height="570" ></iframe>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>




    <div id="navtxt" class="navtext" style="position:absolute; top:50px; left:-100px; visibility:hidden; font-family:Arial, Verdana"></div>
    <div id="footer"></div>
    <script>
    if(flag == "Internet" || flag == "Client")
        $("statusframe").src = "";
        initial()
    </script>
</div>
</body>
</html>

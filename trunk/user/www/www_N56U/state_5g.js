//For operation mode;
sw_mode = '<% nvram_get_x("IPConnection",  "sw_mode"); %>';
productid = '<% nvram_get_f("general.log","productid"); %>';

var uptimeStr = "<% uptime(); %>";
var timezone = uptimeStr.substring(26,31);
var boottime = parseInt(uptimeStr.substring(32,38));
var newformat_systime = uptimeStr.substring(8,11) + " " + uptimeStr.substring(5,7) + " " + uptimeStr.substring(17,25) + " " + uptimeStr.substring(12,16);  //Ex format: Jun 23 10:33:31 2008
var systime_millsec = Date.parse(newformat_systime); // millsec from system
var JS_timeObj = new Date(); // 1970.1.1

var test_page = 0;
var testEventID = "";
var dr_surf_time_interval = 5;	// second
var show_hint_time_interval = 1;	// second

var wan_route_x = "";
var wan_nat_x = "";
var wan_proto = "";

// Dr. Surf {
// for detect if the status of the machine is changed. {
var manually_stop_wan = "";

// original status {
var old_ifWANConnect = 0;
var old_qos_ready = 1;
var old_wan_link_str = "";
var old_detect_dhcp_pppoe = "";
var old_wan_status_log = "";
var old_detect_wan_conn = "";
var old_wan_ipaddr_t = "";

var old_disk_status = "";
var old_mount_status = "";
var old_printer_sn = "";
var old_wireless_clients = "";
// original status }

// new status {
var new_ifWANConnect = 0;
var new_wan_link_str = "";
var new_detect_dhcp_pppoe = "";
var new_wan_status_log = "";

var new_disk_status = "";
var new_mount_status = "";
var new_printer_sn = "";
var new_wireless_clients = "";
var new_detect_wan_conn = "";
var new_wan_ipaddr_t = "";
// new status }

var id_of_check_changed_status = 0;

function unload_body(){
	disableCheckChangedStatus();
	no_flash_button();
	
	return true;
}

function enableCheckChangedStatus(flag){
	var seconds = this.dr_surf_time_interval*1000;
	
	disableCheckChangedStatus();
	
	if(old_wan_link_str == ""){
		seconds = 1;
		id_of_check_changed_status = setTimeout("get_changed_status('initial');", seconds);
	}
	else
		id_of_check_changed_status = setTimeout("get_changed_status();", seconds);
}

function disableCheckChangedStatus(){
	clearTimeout(id_of_check_changed_status);
	id_of_check_changed_status = 0;
}

function check_if_support_dr_surf(){
	if($("helpname"))
		return 1;
	else
		return 0;
}

function compareWirelessClient(target1, target2){
	if(target1.length != target2.length)
		return (target2.length-target1.length);
	
	for(var i = 0; i < target1.length; ++i)
		for(var j = 0; j < 3; ++j)
			if(target1[i][j] != target2[i][j])
					return 1;
	
	return 0;
}

function check_changed_status(flag){
	if(this.test_page == 1
			|| wan_route_x == "IP_Bridged")
		return;
	if(flag == "initial"){
		// for the middle of index.asp.
		if(location.pathname == "/" || location.pathname == "/index.asp"){
			if(old_detect_wan_conn == "1")
					showMapWANStatus(1);
			else if(old_detect_wan_conn == "2")
					showMapWANStatus(2);
			else if(old_wan_ipaddr_t == "0.0.0.0")
				showMapWANStatus(0);
			/*else if(old_ifWANConnect == 1 && old_wan_link_str == "Connected")
				showMapWANStatus(1);
			else if(usb_path1 == "HSDPA" && wan0_ipaddr != "")
				showMapWANStatus(1);*/
			else
				showMapWANStatus(0);
		}
		
		// Dr. Surf
		if(old_ifWANConnect == 0) // WAN port is not plugged.
			parent.showDrSurf("1");
		else if(old_qos_ready == 0)
			parent.showDrSurf("40");
		else if(old_wan_link_str == "Disconnected"){
			// PPPoE, PPTP, L2TP
			if(wan_proto != "dhcp" && wan_proto != "static"){
				if(old_wan_status_log.indexOf("Failed to authenticate ourselves to peer") >= 0)
					parent.showDrSurf("2_1");
				else if(old_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else
					parent.showDrSurf("5");
			}
			// dhcp, static
			else
				parent.showDrSurf("5");
		}
		else if(old_detect_wan_conn != "1")
			parent.showDrSurf("2_2"); 
		else
			parent.showDrSurf("0_0"); // connect is ok.

		enableCheckChangedStatus();		
		return;
	}
	
	// for the middle of index.asp.
	if(location.pathname == "/" || location.pathname == "/index.asp"){
		if(new_detect_wan_conn == "1")
			showMapWANStatus(1);
		else if(new_detect_wan_conn == "2")
			showMapWANStatus(2);
		else if(new_wan_ipaddr_t == "0.0.0.0")
			showMapWANStatus(0);
		/*else if(new_ifWANConnect == 1 && new_wan_link_str == "Connected")
			showMapWANStatus(1);
		else if(usb_path1 == "HSDPA" && wan0_ipaddr != "")
			showMapWANStatus(1);*/
		else
			showMapWANStatus(0);
	}
	
	// Dr.Surf.
	
	var diff_number = compareWirelessClient(old_wireless_clients, new_wireless_clients);
	
	if(diff_number != 0){
		old_wireless_clients = new_wireless_clients;
		
		if(diff_number >= 0)
			parent.showDrSurf("11");
		else
			parent.showDrSurf("12");
	}
	else if(old_disk_status != new_disk_status){
		old_disk_status = new_disk_status;
		
		parent.showDrSurf("20");
	}
	else if(parseInt(old_mount_status) < parseInt(new_mount_status)){
		old_mount_status = new_mount_status;
		
		parent.showDrSurf("21");
	} //lock Add 2009.04.01	
	else if(old_printer_sn != new_printer_sn){
		old_printer_sn = new_printer_sn;
	
		parent.showDrSurf("30");
	} //lock modified 2009.04.01
	else if(old_ifWANConnect != new_ifWANConnect){ // if WAN port is plugged.
		old_ifWANConnect = new_ifWANConnect;
		
		if(new_ifWANConnect == 1)
			parent.showDrSurf("0_2");	// not plugged -> plugged
		else
			parent.showDrSurf("1");	// plugged -> not plugged
	}	
	else if(old_wan_link_str != new_wan_link_str){
		old_wan_link_str = new_wan_link_str;
		
		if(new_wan_link_str == "Disconnected"){
			old_detect_dhcp_pppoe = new_detect_dhcp_pppoe;
			
			// PPPoE, PPTP, L2TP
			if(wan_proto != "dhcp" && wan_proto != "static"){
				if(old_wan_status_log != new_wan_status_log){ // PPP serial change!
					old_wan_status_log = new_wan_status_log;
					
					if(new_wan_status_log.length > 0){
						if(new_wan_status_log.indexOf("Failed to authenticate ourselves to peer") >= 0)
							parent.showDrSurf("2_1");
						else
							parent.showDrSurf("2_2");
					}
					else if(new_detect_dhcp_pppoe == "no-respond")
						parent.showDrSurf("2_2");
					else
						parent.showDrSurf("5");
				}
				else if(new_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else
					parent.showDrSurf("3");
			}
			// dhcp, static
			else{
				if(new_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else if(new_detect_dhcp_pppoe == "error")
					parent.showDrSurf("3");
				else
					parent.showDrSurf("5");
			}
		}
		else if(new_detect_wan_conn != "1")
			parent.showDrSurf("2_2"); 
		else
			parent.showDrSurf("0_1"); 
	}
	
	enableCheckChangedStatus();
}

function get_changed_status(flag){
	document.titleForm.action = "/result_of_get_changed_status.asp";
	
	if(flag == "initial")
		document.titleForm.flag.value = flag;
	else
		document.titleForm.flag.value = "";
	
	document.titleForm.submit();
}

function initial_change_status(manually_stop_wan,
															 ifWANConnect,
														   wan_link_str,
														   detect_dhcp_pppoe,
														   wan_status_log,
														   disk_status,
														   mount_status,
														   printer_sn,
														   wireless_clients,
														   qos_ready,
															 detect_wan_conn,
															 wan_ipaddr_t		
														   ){
	this.manually_stop_wan = manually_stop_wan;
	this.old_ifWANConnect = ifWANConnect;
	this.old_wan_link_str = wan_link_str;
	this.old_detect_dhcp_pppoe = detect_dhcp_pppoe;
	this.old_wan_status_log = wan_status_log;
	this.old_printer_sn = printer_sn;
	this.old_wireless_clients = wireless_clients;
	this.old_qos_ready = qos_ready;
	this.old_detect_wan_conn = detect_wan_conn;
	this.old_wan_ipaddr_t = wan_ipaddr_t;
}

function set_changed_status(manually_stop_wan,
														ifWANConnect,
														wan_link_str,
														detect_dhcp_pppoe,
														wan_status_log,
														disk_status,
														mount_status,
														printer_sn,
														wireless_clients,
													  detect_wan_conn,
														wan_ipaddr_t
														){
	this.manually_stop_wan = manually_stop_wan;
	this.new_ifWANConnect = ifWANConnect;
	this.new_wan_link_str = wan_link_str;
	this.new_detect_dhcp_pppoe = detect_dhcp_pppoe;
	this.new_wan_status_log = wan_status_log;
	this.new_printer_sn = printer_sn;
	this.new_wireless_clients = wireless_clients;
	this.new_detect_wan_conn = detect_wan_conn;
	this.new_wan_ipaddr_t = wan_ipaddr_t;
}
// for detect if the status of the machine is changed. }

function set_Dr_work(flag){
	if(flag != "help"){
		$("Dr_body").onclick = function(){
				showDrSurf();
			};
		
		$("Dr_body").onmouseover = function(){
				showDrSurf();
			};
		
		$("Dr_body").onmouseout = function(){
				showDrSurf();
			};
	}
	else{
		$("Dr_body").onclick = function(){
				showDrSurf(null, "help");
			};
		
		$("Dr_body").onmouseover = function(){
				showDrSurf(null, "help");
			};
		
		$("Dr_body").onmouseout = function(){
				showDrSurf(null, "help");
			};
	}
}

var slowHide_ID_start = 0;
var slowHide_ID_mid = 0;

function clearHintTimeout(){
	if(slowHide_ID_start != 0){
		clearTimeout(slowHide_ID_start);
		slowHide_ID_start = 0;
	}
	
	if(slowHide_ID_mid != 0){
		clearTimeout(slowHide_ID_mid);
		slowHide_ID_mid = 0;
	}
}

function showHelpofDrSurf(hint_array_id, hint_show_id){
	var seconds = this.show_hint_time_interval*1000;	
	
	if(!$("eventDescription")){
		setTimeout('showHelpofDrSurf('+hint_array_id+', '+hint_show_id+');', 100);
		return;
	}
	
	disableCheckChangedStatus();
	clearHintTimeout();
	
	if(typeof(hint_show_id) == "number" && hint_show_id > 0){
		if(hint_array_id == "23"){
			var ssid_len = helpcontent[hint_array_id][hint_show_id].length;
			var drsurf_wide = 11;
			
			clicked_help_string = "<span>"+helptitle[hint_array_id][hint_show_id][0] + "</span><br />";
			if(ssid_len < drsurf_wide+1)
				clicked_help_string += "<p><div align='center'>"+decodeURIComponent(helpcontent[hint_array_id][hint_show_id]);
			else{
				var p = 0;
				while(ssid_len > drsurf_wide){
					clicked_help_string += "<p><div align='center'>"+decodeURIComponent(helpcontent[hint_array_id][hint_show_id]).substring(drsurf_wide*p,drsurf_wide*(p+1))+"<br />";
					ssid_len = ssid_len - drsurf_wide;
					p++;
				}
				clicked_help_string += "<p><div>"+ decodeURIComponent(helpcontent[hint_array_id][hint_show_id]).substring(drsurf_wide*p, drsurf_wide*p+ssid_len);
			}
			clicked_help_string += "</div></p>";
		}
		else
			clicked_help_string = "<span>"+helptitle[hint_array_id][hint_show_id][0]+"</span><br />"+helpcontent[hint_array_id][hint_show_id];
	}
	$("eventDescription").innerHTML = clicked_help_string;
	
	set_Dr_work("help");
	$("eventLink").onclick = function(){};
	showtext($("linkDescription"), "");
	
	$("drsword").style.filter = "alpha(opacity=100)";
	$("drsword").style.opacity = 1;	
	$("drsword").style.visibility = "visible";
	
	$("wordarrow").style.filter	= "alpha(opacity=100)";
	$("wordarrow").style.opacity = 1;	
	$("wordarrow").style.visibility = "visible";
	
	slowHide_ID_start = setTimeout("slowHide(100);", seconds);
}

var current_eventID = null;
var now_alert = new Array(3);

var alert_event0_0 = new Array("<#DrSurf_word_connection_ok#>", "", "");
var alert_event0_1 = new Array("<#DrSurf_word_connection_recover#>", "<#DrSurf_refresh_page#>", refreshpage);
var alert_event0_2 = new Array("<#DrSurf_word_connection_WANport_recover#>", "<#DrSurf_refresh_page#>", refreshpage);
var alert_event1 = new Array("<#web_redirect_reason1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event2_1 = new Array("<#web_redirect_reason2_1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event2_2 = new Array("<#web_redirect_reason2_2#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event3 = new Array("<#web_redirect_reason3_1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event4 = new Array("<#web_redirect_reason4#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);  //wan_gateway & lan_ipaddr;
var alert_event5 = new Array("1. <#web_redirect_reason5_1#><br>2. <#web_redirect_reason5_2#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);

var alert_event10 = new Array("<#DrSurf_Alert10#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event11 = new Array("<#DrSurf_Alert11#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event12 = new Array("<#DrSurf_Alert12#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event20 = new Array("<#DrSurf_Alert20#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event21 = new Array("<#DrSurf_Alert21#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event30 = new Array("<#DrSurf_Alert30#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event40 = new Array("<#DrSurf_Alert40#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);

function showDrSurf(eventID, flag){
	var seconds = this.show_hint_time_interval*1000;
	var temp_eventID;
	
	// for test
	if(this.testEventID != "")
		eventID = this.testEventID;
	
	if(eventID){
		this.current_eventID = eventID;
		temp_eventID = eventID;
	}
	else
		temp_eventID = this.current_eventID;
	
	if(!temp_eventID || temp_eventID.length <= 0){
		id_of_check_changed_status = setTimeout("enableCheckChangedStatus();", 1000);
		return;
	}
	
	disableCheckChangedStatus();
	clearHintTimeout();
	
	if(flag != "help"){
		now_alert[0] = eval("alert_event"+temp_eventID+"[0]");
		if(temp_eventID != "5")
			showtext($("eventDescription"), now_alert[0]);
		else if(this.manually_stop_wan == "1")
			showtext($("eventDescription"), "<#web_redirect_reason5_1#>");
		else
			showtext($("eventDescription"), "<#web_redirect_reason5_2#>");
		
		now_alert[1] = eval("alert_event"+temp_eventID+"[1]");
		if(now_alert[1] != ""){
			now_alert[2] = eval("alert_event"+temp_eventID+"[2]");
			
			if ($("eventLink") != null){	//2012.01 Viz {}
				$("eventLink").onclick = function(){
					now_alert[2](temp_eventID);
				};
			}
			
			showtext($("linkDescription"), now_alert[1]);
		}
	}
	
	$("drsword").style.filter = "alpha(opacity=100)";
	$("drsword").style.opacity = 1;	
	$("drsword").style.visibility = "visible";
	
	$("wordarrow").style.filter	= "alpha(opacity=100)";
	$("wordarrow").style.opacity = 1;	
	$("wordarrow").style.visibility = "visible";
	
	slowHide_ID_start = setTimeout("slowHide(100);", seconds);
}

function slowHide(filter){
	clearHintTimeout();
	
	$("drsword").style.filter = "alpha(opacity="+filter+")";
	$("drsword").style.opacity = filter*0.01;
	$("wordarrow").style.filter	= "alpha(opacity="+filter+")";
	$("wordarrow").style.opacity = filter*0.01;
	
	filter -= 5;
	if(filter <= 0){
		hideHint();
		
		enableCheckChangedStatus();
	}
	else
		slowHide_ID_mid = setTimeout("slowHide("+filter+");", 100);
}

function hideHint(){
	if(this.current_eventID){
		now_alert[0] = eval("alert_event"+this.current_eventID+"[0]");
		showtext($("eventDescription"), now_alert[0]);
		
		now_alert[1] = eval("alert_event"+this.current_eventID+"[1]");
		if(now_alert[1] != ""){
			now_alert[2] = eval("alert_event"+this.current_eventID+"[2]");
			
			$("eventLink").onclick = function(){
					now_alert[2](current_eventID);
				};
			
			showtext($("linkDescription"), now_alert[1]);
		}
	}
	
	$("drsword").style.visibility = "hidden";
	$("wordarrow").style.visibility = "hidden";
}

function drdiagnose(eventID){
	if(!check_if_support_dr_surf()){
		alert("Don't yet support Dr. Surf!");
		return;
	}
	
	if($('statusIcon'))
		$('statusIcon').src = "/images/iframe-iconDr.gif";
	
	if(typeof(openHint) == "function")
		openHint(0, 0);
	
	showtext($('helpname'), "<#DrSurf_Diagnose_title#>");
	
	if($("hint_body"))
		$("hint_body").style.display = "none";
	
	$("statusframe").style.display = "block";
	$('statusframe').src = "/device-map/diagnose"+eventID+".asp";
}
// Dr. Surf }

var banner_code, menu_code="", menu1_code="", menu2_code="", tab_code="", footer_code;

function show_banner(L3){// L3 = The third Level of Menu

	var banner_code = "";
		
	// for chang language
	banner_code +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="sid_list" value="LANGUAGE;">\n';
	banner_code +='<input type="hidden" name="action_mode" value=" Apply ">\n';
	banner_code +='<input type="hidden" name="preferred_lang" value="">\n';
	banner_code +='<input type="hidden" name="flag" value="">\n';
	banner_code +='</form>\n';
	
	banner_code +='<div class="banner1" align="center"></div>\n';
	banner_code +='<table width="983" border="0" align="center" cellpadding="0" cellspacing="0">\n';
	banner_code +='<tr>\n';
	banner_code +='<td class="top-logo"><a href="/"><div id="modelName"><#Web_Title#></div></a></td>\n';
	
	banner_code +='<td class="top-message">\n';
	//banner_code +='<span class="top-messagebold"><#Time#>: </span><span class="time" id="systemtime"></span><br/>\n';
	banner_code +='<span class="top-messagebold"><#menu5_1#>: </span><input type="button" class="button2_NW" style="width:55px;" value="2.4GHz" id="elliptic_ssid_2g" onclick="go_setting(2);" readonly=readonly><input type="button" class="button5_NW" style="width:55px;" value="5GHz" id="elliptic_ssid" onclick="go_setting(5);" readonly=readonly><br/>\n';
	//banner_code +='<span class="top-messagebold">SSID: </span><input class="top_ssid" type="text" value="" id="elliptic_ssid" readonly=readonly><br/>\n';
	banner_code +='<span class="top-messagebold"><#General_x_FirmwareVersion_itemname#> </span><a href="/Advanced_FirmwareUpgrade_Content.asp"><span id="firmver" class="time"></span></a><br/>\n';
	banner_code +='<span class="top-messagebold" title="<#OP_desc1#>"><#menu5_6_1_title#>: </span><a href="/Advanced_OperationMode_Content.asp"><span id="sw_mode_span" class="time"></span></a>\n';	
	banner_code +='</td>\n';
	
	banner_code +='<td class="top-message"width="150">\n';
	banner_code +='<span class="top-messagebold"><#PASS_LANG#></span><br>\n';
	banner_code +='<select name="select_lang" id="select_lang" class="top-input" onchange="change_language();">\n';
	banner_code +='<% shown_language_option(); %>';
	banner_code +='</select>\n';
	banner_code +='<input type="button" id="change_lang_btn" class="button" value="<#CTL_ok#>" onclick="submit_language();" style="float:right; margin:5px 10px 0 0;" disabled=disabled>\n';
	
	banner_code +='</td>\n';
	banner_code +='<td class="top-message" width="120">\n';
	banner_code +='<div id="logout_btn" class="buttonquit"><a style="text-decoration:none;" href="javascript:;" onclick="logout();"><#t1Logout#></a></div>\n';
	banner_code +='<div id="reboto_btn" class="buttonquit"><a style="text-decoration:none;" href="javascript:;" onclick="reboot();"><#BTN_REBOOT#></a></div>\n';
	banner_code +='</td>\n';
	
// Dr. Surf {
	banner_code += '<td id="Dr_body" class="top-message" width="40">\n';
	
	banner_code += '<div id="dr" class="dr"></div>\n';
	banner_code += '<div id="drsword" class="drsword">\n';
	banner_code += '<span id="eventDescription"></span>\n';
	banner_code += '<br>\n';
	banner_code += '<a id="eventLink" href="javascript:void(0);"><span id="linkDescription"></span></a>\n';
	banner_code += '</div>\n';
	banner_code += '<div id="wordarrow" class="wordarrow"><img src="/images/wordarrow.png"></div>\n';
	
	banner_code += '&nbsp;</td>\n';
// Dr. Surf }
	
	banner_code +='<td width="11"><img src="images/top-03.gif" width="11" height="78" /></td>\n';
	banner_code +='</td></tr></table>\n';
	
	if(L3 == 0) 		// IF Without Level 3 menu, banner style will use top.gif.
		banner_code +='<div id="banner3" align="center"><img src="images/top.gif" width="983" height="19" /></div>\n';
	else
		banner_code +='<div id="banner3" align="center"><img src="images/top-advance.gif" width="983" height="19" /></div>\n';

	$("TopBanner").innerHTML = banner_code;
	
	show_loading_obj();
	
	if(location.pathname == "/" || location.pathname == "/index.asp"){
		if(wan_route_x != "IP_Bridged")
			id_of_check_changed_status = setTimeout('hideLoading();', 3000);
	}
	else{
		id_of_check_changed_status = setTimeout('hideLoading();', 1);
	}
	//show_time();
	show_top_status();
	set_Dr_work();
}

var tabtitle = new Array(7);
tabtitle[0] = new Array("", "<#menu5_1_1#>", "<#menu5_1_2#>", "<#menu5_1_3#>", "<#menu5_1_4#>", "<#menu5_1_5#>", "<#menu5_1_6#>");
tabtitle[1] = new Array("", "<#menu5_2_1#>", "<#menu5_2_2#>", "<#menu5_2_3#>", "<#menu5_2_4#>", "<#menu5_2_5#>");
tabtitle[2] = new Array("", "<#menu5_3_1#>", "<#menu5_3_3#>", "<#menu5_3_4#>", "<#menu5_3_5#>", "<#menu5_3_6#>", "<#NAT_passthrough_itemname#>");
tabtitle[3] = new Array("", "<#menu5_4_1#>", "<#menu5_4_2#>", "<#menu5_4_3#>", "<#menu5_4_4#>", "<#menu5_4_5#>");
tabtitle[4] = new Array("", "<#menu5_5_1#>", "<#menu5_5_2#>", "<#menu5_5_3#>", "<#menu5_5_4#>");
tabtitle[5] = new Array("", "<#menu5_6_1#>", "<#menu5_6_2#>", "<#menu5_6_3#>", "<#menu5_6_4#>");
tabtitle[6] = new Array("", "<#menu5_7_2#>", "<#menu5_7_3#>", "<#menu5_7_4#>", "<#menu5_7_5#>", "<#menu5_7_6#>");

//Level 3 Tab title
var tablink = new Array(7);
tablink[0] = new Array("", "Advanced_Wireless_Content.asp", "Advanced_WWPS_Content.asp", "Advanced_WMode_Content.asp", "Advanced_ACL_Content.asp", "Advanced_WSecurity_Content.asp", "Advanced_WAdvanced_Content.asp");
tablink[1] = new Array("", "Advanced_LAN_Content.asp", "Advanced_DHCP_Content.asp", "Advanced_GWStaticRoute_Content.asp", "Advanced_IPTV_Content.asp", "Advanced_Switch_Content.asp");
tablink[2] = new Array("", "Advanced_WAN_Content.asp", "Advanced_PortTrigger_Content.asp", "Advanced_VirtualServer_Content.asp", "Advanced_Exposed_Content.asp", "Advanced_ASUSDDNS_Content.asp", "Advanced_NATPassThrough_Content.asp");
tablink[3] = new Array("", "Advanced_AiDisk_samba.asp", "Advanced_AiDisk_ftp.asp", "Advanced_AiDisk_others.asp", "Advanced_Modem_others.asp", "Advanced_Printer_others.asp");
tablink[4] = new Array("", "Advanced_BasicFirewall_Content.asp", "Advanced_URLFilter_Content.asp", "Advanced_MACFilter_Content.asp", "Advanced_Firewall_Content.asp");
tablink[5] = new Array("", "Advanced_OperationMode_Content.asp", "Advanced_System_Content.asp", "Advanced_FirmwareUpgrade_Content.asp", "Advanced_SettingBackup_Content.asp");
tablink[6] = new Array("", "Main_LogStatus_Content.asp", "Main_DHCPStatus_Content.asp", "Main_WStatus_Content.asp", "Main_IPTStatus_Content.asp", "Main_RouteStatus_Content.asp");

//Level 2 Menu
menuL2_title = new Array("", "<#menu5_1#>", "<#menu5_2#>", "<#menu5_3#>", "<#menu5_4#>", "<#menu5_5#>", "<#menu5_6#>", "<#menu5_7#>");
menuL2_link  = new Array("", tablink[0][1], tablink[1][1], tablink[2][1], tablink[3][1], tablink[4][1], tablink[5][1], tablink[6][1]);	

//Level 1 Menu in Gateway, Router mode
menuL1_title = new Array("", "<#menu1#>", "<#menu3#>", "<#menu2#>", "<#menu4#>", "<#menu5#>");
menuL1_link = new Array("", "index.asp", "aidisk.asp", "poptop.asp", "Main_TrafficMonitor_realtime.asp", "as.asp");

function show_menu(L1, L2, L3){

	//tabtitle[3].splice(4,1);//HSDPA
	//tablink[3].splice(4,1);//HSDPA
	
	if(sw_mode == '4'){
		tablink[2].splice(6,1);
		tabtitle[2].splice(6,1);
		tablink[2].splice(2,3);
		tabtitle[2].splice(2,3);
	}
	if(sw_mode == '2' || sw_mode == '3'){

		if(sw_mode == "2"){
			menuL2_link[1] = "";
			menuL2_title[1] = "";
			menuL2_link[2] = "";
			menuL2_title[2] = "";
			
			tablink[1].splice(1,3);
			tabtitle[1].splice(1,3);			
			tablink[0].splice(1,7);
			tabtitle[0].splice(1,7);

		}else if(sw_mode == "3"){
			tabtitle[0].splice(2,1);//WPS
			tablink[0].splice(2,1);//WPS
		}

		tabtitle[1].splice(2,3);//LAN
		tabtitle[2].splice(1,6);//WAN
		tabtitle[3].splice(4,1);//WAN
		tabtitle[4].splice(1,4);//firewall
		tabtitle[6].splice(2,1);//log
		tabtitle[6].splice(3,2);//log

		tablink[1].splice(2,3);
		tablink[1][1] = "Advanced_APLAN_Content.asp";
		menuL2_link[2] = "Advanced_APLAN_Content.asp";
		tablink[2].splice(1,6);
		tablink[3].splice(4,1);		
		tablink[4].splice(1,4);
		tablink[6].splice(2,1);
		tablink[6].splice(3,2);
		
		menuL2_link[5] = "";
		menuL2_link[3] = "";
		menuL2_title[5] = "";
		menuL2_title[3] = "";
		
		menuL1_link[4] = "";  //remove EzQoS;
		menuL1_title[4] = "";
		menuL1_link[3] = "";  //remove VPN Server;
		menuL1_title[3] = "";
		menuL1_link[2] = "";  //remove AiDisk;
		menuL1_title[2] = "";		
		
		menuL2_link[1] = tablink[0][1];
		menuL2_link[6] = tablink[5][1];
		
	}
	
	for(i = 1; i <= menuL1_title.length-1; i++){
		if(menuL1_title[i] == "")
			continue;
		else if(L1 == i && L2 <= 0){
		  menu1_code += '<div class="m'+i+'_r" id="option'+i+'">'+menuL1_title[i]+'</div>\n';
		}
		else{
		  menu1_code += '<div class="menu" id="option'+i+'"><a href="'+menuL1_link[i]+'" title="'+menuL1_link[i]+'">'+menuL1_title[i]+'</a></div>\n';	
		}
	}
	
	$("mainMenu").innerHTML = menu1_code;
	
//	if(L2 != -1){
		for(var i = 1; i <= menuL2_title.length-1; ++i){
			if(menuL2_title[i] == "")
				continue;
			else if(L2 == i)
				menu2_code += '<div class="thissubmenu">'+menuL2_title[i]+'</div>\n';
			else
				menu2_code += '<div class="submenu"><a href="'+menuL2_link[i]+'">'+menuL2_title[i]+'</a></div>\n';
		}
//	}
	menu2_code += '<div><img src="images/m-button-07end.gif" width="187" height="47" /></div>\n';
	$("subMenu").innerHTML = menu2_code;
	
	if(L3){
		tab_code = '<table border="0" cellspacing="0" cellpadding="0"><tr>\n';
		for(var i = 1; i < tabtitle[L2-1].length; ++i){
			if(tabtitle[L2-1][i] == "")
				continue;
			else if(L3 == i)
				tab_code += '<td class=\"b1\">'+ tabtitle[L2-1][i] +'</td>\n';
			else
				tab_code += '<td class=\"b2\"><a href="' +tablink[L2-1][i]+ '">'+ tabtitle[L2-1][i] +'</a></td>\n';
		}
		tab_code += '</tr></table>\n';
		
		$("tabMenu").innerHTML = tab_code;
	}
	else
		$("tabMenu").innerHTML = "";//*/
}

function show_footer(){
	footer_code = '<div align="center" class="bottom-image"></div>\n';
	footer_code +='<div align="center" class="copyright"><#footer_copyright_desc#></div>\n';
	
	$("footer").innerHTML = footer_code;
	
	if($("helpname"))
		showtext($("helpname"), "<#CTL_help#>");
	if($("hint_body"))
		showtext($("hint_body"), "<#Help_init_word1#> <a class=\"hintstyle\" style=\"background-color:#7aa3bd\"><#Help_init_word2#></a> <#Help_init_word3#>");
	flash_button();
		
	$("elliptic_ssid").onmouseover = function(){
		parent.showHelpofDrSurf(23, 1);
	};
	
	$("elliptic_ssid_2g").onmouseover = function(){
		parent.showHelpofDrSurf(23, 2);
	};
}

var ssid2 = "";
var ssid2_2g = "";

/*function show_top_status(){
	// show SSID in the top-middle block		
	ssid2 = "<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>"
	ssid2_2g = "<% nvram_char_to_ascii("WLANConfig11b", "rt_ssid"); %>";	

	if(ssid2.length > 21){
		ssid2 = ssid2.substring(0,20) + "...";
		//$("elliptic_ssid").title = decodeURIComponent(document.form.wl_ssid2.value);
			$("elliptic_ssid").title = ssid2_5g + " / " + ssid2;	
	}	
	
	$("elliptic_ssid").value = ssid2 + " / " + ssid2_2g;	
	showtext($("firmver"), document.form.firmver.value);

	
	if(sw_mode == "1")  // Show operation mode in banner, Lock add at 2009/02/19
		$("sw_mode_span").innerHTML = "IP Sharing";
	else if(sw_mode == "2")
		$("sw_mode_span").innerHTML = "Router";
	else if(sw_mode == "3")
		$("sw_mode_span").innerHTML = "AP";	
}*/

function show_top_status(){
	//Viz modify for "1.0.1.4j" showtext($("firmver"), document.form.firmver.value);
	showtext($("firmver"), '<% nvram_get_x("",  "firmver_sub"); %>');
	
	if(sw_mode == "1")  // Show operation mode in banner, Lock add at 2009/02/19
		$("sw_mode_span").innerHTML = "IP Sharing";
	else if(sw_mode == "3")
		$("sw_mode_span").innerHTML = "AP";	
	else
		$("sw_mode_span").innerHTML = "Router";
}

function go_setting(band){
	if(band == "2")
		location.href = "Advanced_Wireless2g_Content.asp";
	else
		location.href = "Advanced_Wireless_Content.asp";
}

function show_time(){	
	JS_timeObj.setTime(systime_millsec); // Add millsec to it.	
	JS_timeObj3 = JS_timeObj.toString();	
	JS_timeObj3 = checkTime(JS_timeObj.getHours()) + ":" +
				  			checkTime(JS_timeObj.getMinutes()) + ":" +
				  			checkTime(JS_timeObj.getSeconds());
	$('systemtime').innerHTML ="<a href='/Advanced_System_Content.asp'>" + JS_timeObj3 + "</a>";
	systime_millsec += 1000;		
	
	stime_ID = setTimeout("show_time();", 1000);
}

function checkTime(i)
{
if (i<10) 
  {i="0" + i}
  return i
}

function show_loading_obj(){
	var obj = $("Loading");
	var code = "";
	
	code +='<table cellpadding="5" cellspacing="0" id="loadingBlock" class="loadingBlock" align="center">\n';
	code +='<tr>\n';
	code +='<td width="20%" height="80" align="center"><img src="/images/loading.gif"></td>\n';
	code +='<td><span id="proceeding_main_txt"><#Main_alert_proceeding_desc4#></span><span id="proceeding_txt" style="color:#FFFFCC;"></span></td>\n';
	code +='</tr>\n';
	code +='</table>\n';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->\n';
	
	obj.innerHTML = code;
}

var nav;

if(navigator.appName == 'Netscape')
	nav = true;
else{
	nav = false;
	document.onkeydown = MicrosoftEventHandler_KeyDown;
}

function MicrosoftEventHandler_KeyDown(){
	return true;
}

function submit_language(){
	if($("select_lang").value != $("preferred_lang").value){
		showLoading();
		
		with(document.titleForm){
			action = "/start_apply.htm";
			
			if(location.pathname == "/")
				current_page.value = "/index.asp";
			else
				current_page.value = location.pathname;
			
			preferred_lang.value = $("select_lang").value;
			flag.value = "set_language";
			
			submit();
		}
	}
	else
		alert("No change LANGUAGE!");
}

function change_language(){
	if($("select_lang").value != $("preferred_lang").value)
		$("change_lang_btn").disabled = false;
	else
		$("change_lang_btn").disabled = true;
}

function logout(){
	if(confirm('<#JS_logout#>')){
		setTimeout('location = "Logout.asp";', 1);
	}
}

function reboot(){
	if(confirm("<#Main_content_Login_Item7#>")){
 		 if(window.frames["statusframe"] && window.frames["statusframe"].stopFlag == 0){
  		 window.frames["statusframe"].stopFlag = 1;
  		 //alert(window.frames["statusframe"].stopFlag);
 		 }
		showLoading(36);
		setTimeout("location.href = '/index.asp';", 36000);
		$("hidden_frame").src = "Reboot.asp";
	}
}

function kb_to_gb(kilobytes){
	if(typeof(kilobytes) == "string" && kilobytes.length == 0)
		return 0;
	
	return (kilobytes*1024)/(1024*1024*1024);
}

function simpleNum(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(kb_to_gb(num)*1000)/1000;
}

function simpleNum2(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num*1000)/1000;
}

function simpleNum3(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num)/1024;
}

function $(){
	var elements = new Array();
	
	for(var i = 0; i < arguments.length; ++i){
		var element = arguments[i];
	if(typeof element == 'string')
		element = document.getElementById(element);
		
		if(arguments.length == 1)
			return element;
		
		elements.push(element);
	}
	
	return elements;
}

function getElementsByName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		objsName = tagObjs[i].getAttribute("name");
		
		if(objsName && objsName.indexOf(name) == 0){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function getElementsByClassName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		if(navigator.appName == 'Netscape')
			objsName = tagObjs[i].getAttribute("class");
		else
			objsName = tagObjs[i].getAttribute("className");
		
		if(objsName == name){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;//*/
}

function showhtmlspace(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf(" ")) >= 0){
		str += head.substring(0, tail_num);
		str += "&nbsp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function showhtmland(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf("&")) >= 0){
		str += head.substring(0, tail_num);
		str += "&amp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

// A dummy function which just returns its argument. This was needed for localization purpose
function translate(str){
	return str;
}

function trim(val){
	val = val+'';
	for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
	for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
	return val.substring(startIndex,endIndex+1);
}

function IEKey(){
	return event.keyCode;
}

function NSKey(){
	return 0;
}

function is_string(o){
	if(!nav)
		keyPressed = IEKey();
	else
		keyPressed = NSKey();
	
	if(keyPressed == 0)
		return true;
	else if(keyPressed >= 0 && keyPressed <= 126)
		return true;
	
	alert('<#JS_validchar#>');
	return false;
}

function validate_string(string_obj, flag){
	if(string_obj.value.charAt(0) == '"'){
		if(flag != "noalert")
			alert('<#JS_validstr1#> ["]');
		
		string_obj.value = "";
		string_obj.focus();
		
		return false;
	}
	else{
		invalid_char = "";
		
		for(var i = 0; i < string_obj.value.length; ++i){
			if(string_obj.value.charAt(i) < ' ' || string_obj.value.charAt(i) > '~'){
				invalid_char = invalid_char+string_obj.value.charAt(i);
			}
		}
		
		if(invalid_char != ""){
			if(flag != "noalert")
				alert("<#JS_validstr2#> '"+invalid_char+"' !");
			string_obj.value = "";
			string_obj.focus();
			
			return false;
		}
	}
	
	return true;
}

function validate_hex(obj){
	var obj_value = obj.value
	var re = new RegExp("[^a-fA-F0-9]+","gi");
	
	if(re.test(obj_value))
		return false;
	else
		return true;
}

function validate_psk(psk_obj){
	var psk_length = psk_obj.value.length;
	
	if(psk_length < 8){
		alert("<#JS_passzero#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length > 64){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = psk_obj.value.substring(0, 64);
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length >= 8 && psk_length <= 63 && !validate_string(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length == 64 && !validate_hex(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	return true;
}

function validate_wlkey(key_obj){
	var wep_type = document.form.wl_wep_x.value;
	var iscurrect = true;
	var str = "<#JS_wepkey#>";
	
	if(wep_type == "0")
		iscurrect = true;	// do nothing
	else if(wep_type == "1"){
		if(key_obj.value.length == 5 && validate_string(key_obj)){
			document.form.wl_key_type.value = 1; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else if(key_obj.value.length == 10 && validate_hex(key_obj)){
			document.form.wl_key_type.value = 0; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else{
			str += "(<#WLANConfig11b_WEPKey_itemtype1#>)";
			
			iscurrect = false;
		}
	}
	else if(wep_type == "2"){
		if(key_obj.value.length == 13 && validate_string(key_obj)){
			document.form.wl_key_type.value = 1; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else if(key_obj.value.length == 26 && validate_hex(key_obj)){
			document.form.wl_key_type.value = 0; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else{
			str += "(<#WLANConfig11b_WEPKey_itemtype2#>)";
			
			iscurrect = false;
		}
	}
	else{
		alert("System error!");
		iscurrect = false;
	}
	
	if(iscurrect == false){
		alert(str);
		
		key_obj.focus();
		key_obj.select();
	}
	
	return iscurrect;
}

function checkDuplicateName(newname, targetArray){
	var existing_string = targetArray.join(',');
	existing_string = ","+existing_string+",";
	var newstr = ","+trim(newname)+",";
	
	var re = new RegExp(newstr, "gi");
	var matchArray = existing_string.match(re);
	
	if(matchArray != null)
		return true;
	else
		return false;
}

function alert_error_msg(error_msg){
	alert(error_msg);
	refreshpage();
}

function refreshpage(seconds){
	if(typeof(seconds) == "number")
		setTimeout("refreshpage()", seconds*1000);
	else
		location.href = location.href;
}

function hideLinkTag(){
	if(document.all){
		var tagObjs = document.all.tags("a");
		
		for(var i = 0; i < tagObjs.length; ++i)
			tagObjs(i).outerHTML = tagObjs(i).outerHTML.replace(">"," hidefocus=true>");
	}
}

function buttonOver(o){	//Lockchou 1206 modified
	o.style.color = "#FFFFFF";
	o.style.background = "url(/images/bgaibutton.gif) #ACCCE1";
	o.style.cursor = "hand";
}

function buttonOut(o){	//Lockchou 1206 modified
	o.style.color = "#000000";
	o.style.background = "url(/images/bgaibutton0.gif) #ACCCE1";
}

function flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = function(){
				buttonOver(this);
			};
		
		btnObj[i].onmouseout = function(){
				buttonOut(this);
			};
	}
}

function no_flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = "";
		
		btnObj[i].onmouseout = "";
	}
}

function gotoprev(formObj){
	var prev_page = formObj.prev_page.value;
	
	if(prev_page == "/")
		prev_page = "/index.asp";
	
	if(prev_page.indexOf('QIS') < 0){
		formObj.action = prev_page;
		formObj.target = "_parent";
		formObj.submit();
	}
	else{
		formObj.action = prev_page;
		formObj.target = "";
		formObj.submit();
	}
}

function add_option(selectObj, str, value, selected){
	var tail = selectObj.options.length;
	
	if(typeof(str) != "undefined")
		selectObj.options[tail] = new Option(str);
	else
		selectObj.options[tail] = new Option();
	
	if(typeof(value) != "undefined")
		selectObj.options[tail].value = value;
	else
		selectObj.options[tail].value = "";
	
	if(selected == 1)
		selectObj.options[tail].selected = selected;
}
/*
function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = 0; i < selectObj.options.length; ++i){
		selectObj.options[0].value = null;
		selectObj.options[0] = null;
	}
}*/

function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = selectObj.options.length-1; i >= 0; --i){
  		selectObj.options[i].value = null;
		selectObj.options[i] = null;
	}
}

function blocking(obj_id, show){
	var state = show?'block':'none';
	
	if(document.getElementById)
		$(obj_id).style.display = state;
	else if(document.layers)
		document.layers[obj_id].display = state;
	else if(document.all)
		document.all[obj_id].style.display = state;
}

function inputCtrl(obj, flag){
	if(flag == 0){
		obj.disabled = true;
		obj.style.backgroundColor = "#CCCCCC";		
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#C0DAE4";
	}
	else{
		obj.disabled = false;
		obj.style.backgroundColor = "#FFF";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#C0DAE4";
	}
}

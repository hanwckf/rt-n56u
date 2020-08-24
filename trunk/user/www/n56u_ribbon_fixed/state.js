var sw_mode = '<% nvram_get_x("", "sw_mode"); %>';
var wan_route_x = '<% nvram_get_x("", "wan_route_x"); %>';
var wan_proto = '<% nvram_get_x("", "wan_proto"); %>';
var lan_proto = '<% nvram_get_x("", "lan_proto_x"); %>';
var log_float = '<% nvram_get_x("", "log_float_ui"); %>';
var reboot_schedule_support = '<% nvram_get_x("", "reboot_schedule_enable"); %>';
var ss_schedule_support = '<% nvram_get_x("", "ss_schedule_enable"); %>';
var log_stamp = 0;
var sysinfo = <% json_system_status(); %>;
var uptimeStr = "<% uptime(); %>";
var timezone = uptimeStr.substring(26,31);
var newformat_systime = uptimeStr.substring(8,11) + " " + uptimeStr.substring(5,7) + " " + uptimeStr.substring(17,25) + " " + uptimeStr.substring(12,16);  //Ex format: Jun 23 10:33:31 2008
var systime_millsec = Date.parse(newformat_systime); // millsec from system
var JS_timeObj = new Date(); // 1970.1.1
var cookie_pref = 'n56u_cookie_';

var uagent = navigator.userAgent.toLowerCase();
var is_ie11p = (/trident\/7\./).test(uagent);
var is_mobile = (/iphone|ipod|ipad|iemobile|android|blackberry|fennec/).test(uagent);

var new_wan_internet = '<% nvram_get_x("", "link_internet"); %>';
var id_check_status = 0;
var id_system_info = 0;

var cookie = {
	set: function(key, value, days) {
		document.cookie = cookie_pref + key + '=' + value + '; expires=' +
			(new Date(new Date().getTime() + ((days ? days : 14) * 86400000))).toUTCString() + '; path=/';
	},
	get: function(key) {
		var r = ('; ' + document.cookie + ';').match('; ' + cookie_pref + key + '=(.*?);');
		return r ? r[1] : null;
	},
	unset: function(key) {
		document.cookie = cookie_pref + key + '=; expires=' + (new Date(1)).toUTCString() + '; path=/';
	}
};

<% firmware_caps_hook(); %>

function get_ap_mode(){
	return (wan_route_x == 'IP_Bridged' || sw_mode == '3') ? true : false;
}

function unload_body(){
	disableCheckChangedStatus();
	no_flash_button();
	return true;
}

function enableCheckChangedStatus(flag){
	var tm_int_sec = 1;

	disableCheckChangedStatus();

	if (new_wan_internet == '0')
		tm_int_sec = 2;
	else if (new_wan_internet == '1')
		tm_int_sec = 5;

	id_check_status = setTimeout("get_changed_status();", tm_int_sec * 1000);
}

function disableCheckChangedStatus(){
	clearTimeout(id_check_status);
}

function update_internet_status(){
	if (new_wan_internet == '1')
		showMapWANStatus(1);
	else if(new_wan_internet == '2')
		showMapWANStatus(2);
	else
		showMapWANStatus(0);
}

function notify_status_internet(wan_internet){
	this.new_wan_internet = wan_internet;
	if((location.pathname == "/" || location.pathname == "/index.asp") && (typeof(update_internet_status) === 'function'))
		update_internet_status();
}

function notify_status_vpn_client(vpnc_state){
	if((location.pathname == "/vpncli.asp") && (typeof(update_vpnc_status) === 'function'))
		update_vpnc_status(vpnc_state);
}

function get_changed_status(){
	var $j = jQuery.noConflict();
	$j.ajax({
		type: 'get',
		url: '/status_internet.asp',
		dataType: 'script',
		cache: true,
		error: function(xhr) {
			;
		},
		success: function(response) {
			notify_status_internet(now_wan_internet);
			notify_status_vpn_client(now_vpnc_state);
			enableCheckChangedStatus();
		}
	});
}

function get_system_info(){
	var $j = jQuery.noConflict();
	clearTimeout(id_system_info);
	$j.ajax({
		type: 'get',
		url: '/system_status_data.asp',
		dataType: 'script',
		cache: true,
		error: function(xhr) {
			id_system_info = setTimeout('get_system_info()', 2000);
		},
		success: function(response) {
			id_system_info = setTimeout('get_system_info()', 2000);
			setSystemInfo(response);
		}
	});
}

function bytesToSize(bytes, precision){
	var absval = Math.abs(bytes);
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;

	if (absval < kilobyte)
		return bytes + ' B';
	else if (absval < megabyte)
		return (bytes / kilobyte).toFixed(precision) + ' KB';
	else if (absval < gigabyte)
		return (bytes / megabyte).toFixed(precision) + ' MB';
	else if (absval < terabyte)
		return (bytes / gigabyte).toFixed(precision) + ' GB';
	else
		return (bytes / terabyte).toFixed(precision) + ' TB';
}

function getLALabelStatus(num){
	var la = parseFloat(num);
	return la > 0.9 ? 'danger' : (la > 0.5 ? 'warning' : 'info');
}

function setSystemInfo(response){
	if(typeof(si_new) !== 'object')
		return;

	var cpu_now = {};
	var cpu_total = (si_new.cpu.total - sysinfo.cpu.total);
	if (!cpu_total)
		cpu_total = 1;

	cpu_now.busy = parseInt((si_new.cpu.busy - sysinfo.cpu.busy) * 100 / cpu_total);
	cpu_now.user = parseInt((si_new.cpu.user - sysinfo.cpu.user) * 100 / cpu_total);
	cpu_now.nice = parseInt((si_new.cpu.nice - sysinfo.cpu.nice) * 100 / cpu_total);
	cpu_now.system = parseInt((si_new.cpu.system - sysinfo.cpu.system) * 100 / cpu_total);
	cpu_now.idle = parseInt((si_new.cpu.idle - sysinfo.cpu.idle) * 100 / cpu_total);
	cpu_now.iowait = parseInt((si_new.cpu.iowait - sysinfo.cpu.iowait) * 100 / cpu_total);
	cpu_now.irq = parseInt((si_new.cpu.irq - sysinfo.cpu.irq) * 100 / cpu_total);
	cpu_now.sirq = parseInt((si_new.cpu.sirq - sysinfo.cpu.sirq) * 100 / cpu_total);

	sysinfo = si_new;

	showSystemInfo(cpu_now,1);
}

function showSystemInfo(cpu_now,force){
	var $j = jQuery.noConflict();
	var arrLA = sysinfo.lavg.split(' ');
	var h = sysinfo.uptime.hours < 10 ? ('0'+sysinfo.uptime.hours) : sysinfo.uptime.hours;
	var m = sysinfo.uptime.minutes < 10 ? ('0'+sysinfo.uptime.minutes) : sysinfo.uptime.minutes;

	$j("#la_info").html('<span class="label label-'+getLALabelStatus(arrLA[0])+'">'+arrLA[0]+'</span>&nbsp;<span class="label label-'+getLALabelStatus(arrLA[1])+'">'+arrLA[1]+'</span>&nbsp;<span class="label label-'+getLALabelStatus(arrLA[2])+'">'+arrLA[2]+'</span>');
	$j("#cpu_info").html(cpu_now.busy + '%');
	$j("#mem_info").html(bytesToSize(sysinfo.ram.free*1024, 2) + " / " + bytesToSize(sysinfo.ram.total*1024, 2));
	$j("#uptime_info").html(sysinfo.uptime.days + "<#Day#>".substring(0,1) + " " + h+"<#Hour#>".substring(0,1) + " " + m+"<#Minute#>".substring(0,1));

	$j("#cpu_usage tr:nth-child(1) td:first").html('busy: '+cpu_now.busy+'%');
	$j("#cpu_usage tr:nth-child(2) td:first").html('user: '+cpu_now.user+'%');
	$j("#cpu_usage tr:nth-child(2) td:last").html('system: '+cpu_now.system+'%');
	$j("#cpu_usage tr:nth-child(3) td:first").html('sirq: '+cpu_now.sirq+'%');
	$j("#cpu_usage tr:nth-child(3) td:last").html('irq: '+cpu_now.irq+'%');
	$j("#cpu_usage tr:nth-child(4) td:first").html('idle: '+cpu_now.idle+'%');
	$j("#cpu_usage tr:nth-child(4) td:last").html('nice: '+cpu_now.nice+'%');

	$j("#mem_usage tr:nth-child(1) td:first").html('total: '+bytesToSize(sysinfo.ram.total*1024, 2));
	$j("#mem_usage tr:nth-child(2) td:first").html('free: '+bytesToSize(sysinfo.ram.free*1024, 2));
	$j("#mem_usage tr:nth-child(2) td:last").html('used: '+bytesToSize(sysinfo.ram.used*1024, 2));
	$j("#mem_usage tr:nth-child(3) td:first").html('cached: '+bytesToSize(sysinfo.ram.cached*1024, 2));
	$j("#mem_usage tr:nth-child(3) td:last").html('buffers: '+bytesToSize(sysinfo.ram.buffers*1024, 2));
	$j("#mem_usage tr:nth-child(4) td:first").html('swap: '+bytesToSize(sysinfo.swap.total*1024, 2));
	$j("#mem_usage tr:nth-child(4) td:last").html('swap used: '+bytesToSize(sysinfo.swap.used*1024, 2));

	if(parseInt(sysinfo.wifi2.state) > 0)
		$j('#wifi2_b').addClass('btn-info');
	else
		$j('#wifi2_b').removeClass('btn-info');

	if(parseInt(sysinfo.wifi5.state) > 0)
		$j('#wifi5_b').addClass('btn-info');
	else
		$j('#wifi5_b').removeClass('btn-info');

	if(parseInt(sysinfo.wifi2.guest) > 0)
		$j('#wifi2_b_g').addClass('btn-info');
	else
		$j('#wifi2_b_g').removeClass('btn-info');

	if(parseInt(sysinfo.wifi5.guest) > 0)
		$j('#wifi5_b_g').addClass('btn-info');
	else
		$j('#wifi5_b_g').removeClass('btn-info');

	setLogStamp(sysinfo.logmt);

	if(force && typeof(parent.getSystemJsonData) === 'function')
		getSystemJsonData(cpu_now, sysinfo.ram);
}

var menu_code="", menu1_code="", menu2_code="", tab_code="", footer_code;
var enabled2Gclass = '<% nvram_match_x("","rt_radio_x", "1", "btn-info"); %>';
var enabled5Gclass = '<% nvram_match_x("","wl_radio_x", "1", "btn-info"); %>';
var enabledGuest2Gclass = '<% nvram_match_x("","rt_guest_enable", "1", "btn-info"); %>';
var enabledGuest5Gclass = '<% nvram_match_x("","wl_guest_enable", "1", "btn-info"); %>';
var enabledBtnCommit = '<% nvram_match_x("","nvram_manual", "0", "display:none;"); %>';

// L3 = The third Level of Menu
function show_banner(L3){
	var bc = '';
	var style_2g = 'width:55px;';
	var style_5g = 'width:55px;';
	if (!support_5g_radio()) {
		style_2g = 'width:114px;';
		style_5g = 'width:21px;display:none;';
	}
	var title_2g = '"2.4G"'
	if (!support_2g_radio()) {
		title_2g = '"N/A" disabled';
	}

	// log panel
	if (!is_mobile && log_float != '0'){
		bc += '<div class="syslog_panel">\n';
		bc += '<button id="syslog_panel_button" class="handle" href="/"><span class="log_text">Log</span></button>\n';
		bc += '<table class="" style="margin-top: 0px; margin-bottom: 5px" width="100%" border="0">\n';
		bc += '  <tr>\n';
		bc += '    <td width="60%" style="text-align: left"><b><#General_x_SystemTime_itemname#>:</b><span class="alert alert-info" style="margin-left: 10px; padding-top: 4px; padding-bottom: 4px;" id="system_time_log_area"></span></td>\n';
		bc += '    <td style="text-align: lift"><input type="hidden" id="scrATop" value=""></td>\n';
		bc += '    <td style="text-align: right"><button type="button" id="clearlog_btn" class="btn btn-info" style="min-width: 170px;" onclick="clearlog();"><#CTL_clear#></button></td>\n';
		bc += '  </tr>\n';
		bc += '</table>\n';
		bc += '<span><textarea rows="28" wrap="off" class="span12" readonly="readonly" id="log_area"></textarea></span>\n';
		bc += '</div>\n';
	}

	// for chang language
	bc +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	bc +='<input type="hidden" name="current_page" value="">\n';
	bc +='<input type="hidden" name="sid_list" value="LANGUAGE;">\n';
	bc +='<input type="hidden" name="action_mode" value=" Apply ">\n';
	bc +='<input type="hidden" name="preferred_lang" value="">\n';
	bc +='<input type="hidden" name="flag" value="">\n';
	bc +='</form>\n';

	// --->
	bc += '<div class="container-fluid" style="padding-left: 0px; margin-left: -6px;">\n';
	bc += '<div class="row-fluid">\n';

	// block system info
	bc += '<div class="span6">\n';
	bc += '<div class="well" style="margin-bottom: 0px; height: 109px; padding: 7px 6px 6px 6px;">\n';
	bc += '<div class="row-fluid">\n';

	bc += '<div id="main_info">\n';
	bc += '<table class="table table-condensed" width="100%" style="margin-bottom: 0px;">\n';
	bc += '  <tr>\n';
	bc += '    <td width="43%" style="border: 0 none;"><#SI_LoadAvg#></td>\n';
	bc += '    <td style="border: 0 none; min-width: 115px;"><div id="la_info"> -- -- -- </div></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td style="height: 20px;"><a class="adv_info" href="javascript:void(0)" onclick="click_info_cpu();"><#SI_LoadCPU#></a></td>\n';
	bc += '    <td><span id="cpu_info"> -- % </span></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td><a class="adv_info" href="javascript:void(0)" onclick="click_info_mem();"><#SI_FreeMem#></a></td>\n';
	bc += '    <td><span id="mem_info"> -- MB / -- MB </span></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td><#SI_Uptime#></td>\n';
	bc += '    <td><span id="uptime_info">&nbsp;</span></td>\n';
	bc += '  </tr>\n';
	bc += '</table>\n';
	bc += '</div>\n';

	bc += '<div id="cpu_usage" style="display: none;">\n';
	bc += '<table class="table table-condensed" width="100%" style="margin-bottom: 0px;">\n';
	bc += '  <tr>\n';
	bc += '    <td width="43%" style="text-align:left; border: 0 none;"></td>\n';
	bc += '    <td style="border: 0 none; text-align:right;"><a class="label" href="javascript:void(0)" onclick="hide_adv_info();">hide</a></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td style="height: 20px;"></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '</table>\n';
	bc += '</div>\n';

	bc += '<div id="mem_usage" style="display: none;">\n';
	bc += '<table class="table table-condensed" width="100%" style="margin-bottom: 0px;">\n';
	bc += '  <tr>\n';
	bc += '    <td width="43%" style="text-align:left; border: 0 none;"></td>\n';
	bc += '    <td style="border: 0 none; text-align:right;"><a class="label" href="javascript:void(0)" onclick="hide_adv_info();">hide</a></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td style="height: 20px;"></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td></td>\n';
	bc += '    <td></td>\n';
	bc += '  </tr>\n';
	bc += '</table>\n';
	bc += '</div>\n';

	bc += '</div>\n';
	bc += '</div>\n';
	bc += '</div>\n';

	// block firmware version
	bc += '<div class="span6">\n';
	bc += '<div class="well" style="margin-bottom: 0px; height: 109px; padding: 5px 6px 8px 6px;">\n';
	bc += '<div class="row-fluid">\n';
	bc += '<table class="table table-condensed" style="margin-bottom: 0px">\n';
	bc += '  <tr>\n';
	bc += '    <td width="50%" style="border: 0 none;"><#menu5_1#>:</td>\n';
	bc += '    <td style="border: 0 none; min-width: 115px;"><div class="form-inline"><input type="button" id="wifi2_b" class="btn btn-mini '+enabled2Gclass+'" style="'+style_2g+'" value='+title_2g+' onclick="go_setting(2);">&nbsp;<input type="button" id="wifi5_b" style="'+style_5g+'" class="btn btn-mini '+enabled5Gclass+'" value="5G" onclick="go_setting(5);"></div></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td><#menu5_1_2#>:</td>\n';
	bc += '    <td><div class="form-inline"><input type="button" id="wifi2_b_g" class="btn btn-mini '+enabledGuest2Gclass+'" style="'+style_2g+'" value='+title_2g+' onclick="go_wguest(2);">&nbsp;<input type="button" id="wifi5_b_g" style="'+style_5g+'" class="btn btn-mini '+enabledGuest5Gclass+'" value="5G" onclick="go_wguest(5);"></div></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td><#General_x_FirmwareVersion_itemname#></td>\n';
	bc += '    <td><a href="/Advanced_FirmwareUpgrade_Content.asp"><span id="firmver" class="time"></span></a></td>\n';
	bc += '  </tr>\n';
	bc += '  <tr>\n';
	bc += '    <td><button type="button" id="commit_btn" class="btn btn-mini" style="width: 114px; height: 21px; outline:0; '+enabledBtnCommit+'" onclick="commit();"><i class="icon icon-fire"></i>&nbsp;<#CTL_Commit#></button></td>\n';
	bc += '    <td><button type="button" id="logout_btn" class="btn btn-mini" style="height: 21px; outline:0;" title="<#t1Logout#>" onclick="logout();"><i class="icon icon-user"></i></button> <button type="button" id="reboto_btn" class="btn btn-mini" style="height: 21px; outline:0;" title="<#BTN_REBOOT#>" onclick="reboot();"><i class="icon icon-repeat"></i></button> <button type="button" id="shutdown_btn" class="btn btn-mini" style="height: 21px; outline:0;" title="<#BTN_SHUTDOWN#>" onclick="shutdown();"><i class="icon icon-off"></i></button></td>\n';
	bc += '  </tr>\n';
	bc += '</table>\n';
	bc += '</div>\n';
	bc += '</div>\n';
	bc += '</div>\n';

	bc += '</div>\n';
	bc += '</div>\n';

	bc +='</td></tr></table>\n';

	$("TopBanner").innerHTML = bc;

	show_loading_obj();
	show_top_status();
}

var tabtitle = new Array(21);
var tablink = new Array(21);
tabtitle[0] = new Array("", "<#menu5_1_1#>", "<#menu5_1_2#>", "<#menu5_1_3#>", "<#menu5_1_4#>", "<#menu5_1_5#>", "<#menu5_1_6#>");
tabtitle[1] = new Array("", "<#menu5_1_1#>", "<#menu5_1_2#>", "<#menu5_1_3#>", "<#menu5_1_4#>", "<#menu5_1_5#>", "<#menu5_1_6#>");
tabtitle[2] = new Array("", "<#menu5_2_1#>", "<#menu5_2_2#>", "<#menu5_2_3#>", "<#menu5_2_4#>", "<#menu5_2_5#>", "<#menu5_2_6#>");
tabtitle[3] = new Array("", "<#menu5_3_1#>", "<#menu5_3_3#>", "<#menu5_3_4#>", "<#menu5_3_5#>", "<#menu5_3_6#>");
tabtitle[4] = new Array("", "<#menu5_5_1#>", "<#menu5_5_5#>", "<#menu5_5_2#>", "<#menu5_5_3#>", "<#menu5_5_4#>");
tabtitle[5] = new Array("", "<#menu5_4_3#>", "<#menu5_4_1#>", "<#menu5_4_2#>", "<#menu5_4_4#>", "<#menu5_4_5#>");
tabtitle[6] = new Array("", "<#menu5_6_2#>", "<#menu5_6_5#>", "<#menu5_6_1#>", "<#menu5_6_3#>", "<#menu5_6_4#>", "<#menu5_6_6#>");
tabtitle[7] = new Array("", "<#menu5_10_1#>", "<#menu5_10_2#>", "<#menu5_10_3#>" , "<#menu5_10_4#>");
tabtitle[8] = new Array("", "<#menu5_11#>", "<#menu5_12#>", "WAN", "", "", "", "", "", "", "");
tabtitle[9] = new Array("", "<#menu5_7_2#>", "<#menu5_7_3#>", "<#menu5_7_5#>", "<#menu5_7_6#>", "<#menu5_7_8#>");
if (found_app_scutclient()){
	tabtitle[10] = new Array("", "<#menu5_1_1#>","<#menu5_13_log#>");
}
if (found_app_dnsforwarder()){
	tabtitle[11] = new Array("", "<#menu5_1_1#>");
}
if (found_app_shadowsocks()){
	tabtitle[12] = new Array("", "<#menu5_1_1#>","<#menu5_16_20#>");
}
if (found_app_mentohust()){
	tabtitle[13] = new Array("", "<#menu5_1_1#>","<#menu5_13_log#>");
}
if (found_app_adbyby()){
	tabtitle[14] = new Array("", "<#menu5_20_1#>");
}
if (found_app_koolproxy()){
	if (found_app_adbyby()){
		tabtitle[14].push("<#menu5_26_1#>");
	}else{
	tabtitle[14] = new Array("", "<#menu5_26_1#>");
	}
}
if (found_app_smartdns()){
	tabtitle[15] = new Array("", "<#menu5_29#>");	
}else{
if (found_app_adguardhome()){
	tabtitle[15] = new Array("", "<#menu5_29#>");
}
}
if (found_app_aliddns()){
	tabtitle[16] = new Array("", "<#menu5_30#>");
}else{
if (found_app_zerotier()){
	tabtitle[16] = new Array("", "<#menu5_32#>");
}
}
if (found_app_frp()){
	tabtitle[17] = new Array("", "<#menu5_25_1#>");
}
if (found_app_caddy()){
	tabtitle[18] = new Array("", "<#menu5_27_1#>");
}
if (found_app_wyy()){
	tabtitle[18] = new Array("", "<#menu5_31_1#>");
}
//Level 3 Tab title

tablink[0] = new Array("", "Advanced_Wireless2g_Content.asp", "Advanced_WGuest2g_Content.asp", "Advanced_WMode2g_Content.asp", "Advanced_ACL2g_Content.asp", "Advanced_WSecurity2g_Content.asp", "Advanced_WAdvanced2g_Content.asp");
tablink[1] = new Array("", "Advanced_Wireless_Content.asp", "Advanced_WGuest_Content.asp", "Advanced_WMode_Content.asp", "Advanced_ACL_Content.asp", "Advanced_WSecurity_Content.asp", "Advanced_WAdvanced_Content.asp");
tablink[2] = new Array("", "Advanced_LAN_Content.asp", "Advanced_DHCP_Content.asp", "Advanced_GWStaticRoute_Content.asp", "Advanced_IPTV_Content.asp", "Advanced_Switch_Content.asp", "Advanced_WOL_Content.asp");
tablink[3] = new Array("", "Advanced_WAN_Content.asp", "Advanced_IPv6_Content.asp", "Advanced_VirtualServer_Content.asp", "Advanced_Exposed_Content.asp", "Advanced_DDNS_Content.asp");
tablink[4] = new Array("", "Advanced_BasicFirewall_Content.asp", "Advanced_Netfilter_Content.asp", "Advanced_URLFilter_Content.asp", "Advanced_MACFilter_Content.asp", "Advanced_Firewall_Content.asp");
tablink[5] = new Array("", "Advanced_AiDisk_others.asp", "Advanced_AiDisk_samba.asp", "Advanced_AiDisk_ftp.asp", "Advanced_Modem_others.asp", "Advanced_Printer_others.asp");
tablink[6] = new Array("", "Advanced_System_Content.asp", "Advanced_Services_Content.asp", "Advanced_OperationMode_Content.asp", "Advanced_FirmwareUpgrade_Content.asp", "Advanced_SettingBackup_Content.asp", "Advanced_Console_Content.asp");
tablink[7] = new Array("", "Advanced_Tweaks_Content.asp", "Advanced_Scripts_Content.asp", "Advanced_InetDetect_Content.asp" ,"Advanced_web.asp");
tablink[8] = new Array("", "Main_WStatus2g_Content.asp", "Main_WStatus_Content.asp", "", "", "", "", "", "", "", "");
tablink[9] = new Array("", "Main_LogStatus_Content.asp", "Main_DHCPStatus_Content.asp", "Main_IPTStatus_Content.asp", "Main_RouteStatus_Content.asp", "Main_CTStatus_Content.asp");
if (found_app_scutclient()){
	scutclient_array = new Array("", "scutclient.asp", "scutclient_log.asp");
	tablink[10] = (scutclient_array);
}
if (found_app_dnsforwarder()){
	dns_forwarder_array = new Array("", "dns-forwarder.asp");
	tablink[11] = (dns_forwarder_array);
}
if (found_app_shadowsocks()){
	shadowsocks_array = new Array("","Shadowsocks.asp","Shadowsocks_log.asp");
	tablink[12] = (shadowsocks_array);
}
if (found_app_mentohust()){
	mentohust_array = new Array("","mentohust.asp","mentohust_log.asp");
	tablink[13] = (mentohust_array);
}
if (found_app_adbyby()){
	ad_array = new Array("","Advanced_adbyby.asp");
	tablink[14] = (ad_array);
}else if (found_app_koolproxy()){
	kp_array = new Array("","Advanced_koolproxy.asp");
	tablink[14] = (kp_array);
}
if (found_app_smartdns()){
	smartdns_array = new Array("","Advanced_smartdns.asp");
	tablink[15] = (smartdns_array);
}else if (found_app_adguardhome()){
	adg_array = new Array("","Advanced_adguardhome.asp");
	tablink[15] = (adg_array);
}
if (found_app_aliddns()){
	aliddns_array = new Array("","Advanced_aliddns.asp");
	tablink[16] = (aliddns_array);
}else if (found_app_zerotier()){
	zerotier_array = new Array("","Advanced_zerotier.asp");
	tablink[16] = (zerotier_array);
}
if (found_app_frp()){
	frp_array = new Array("","Advanced_frp.asp");
	tablink[17] = (frp_array);
}
if (found_app_caddy()){
	caddy_array = new Array("","Advanced_caddy.asp");
	tablink[18] = (caddy_array);
}
if (found_app_wyy()){
	wyy_array = new Array("","Advanced_wyy.asp");
	tablink[19] = (wyy_array);
}

//Level 2 Menu
menuL2_title = new Array(21)
menuL2_title = new Array("", "<#menu5_11#>", "<#menu5_12#>", "<#menu5_2#>", "<#menu5_3#>", "<#menu5_5#>", "<#menu5_4#>", "<#menu5_6#>", "<#menu5_10#>", "<#menu5_9#>", "<#menu5_7#>");
if (found_app_scutclient()){
	menuL2_title.push("<#menu5_13#>");
} else menuL2_title.push("");

if (found_app_dnsforwarder()){
	menuL2_title.push("<#menu5_15#>");
} else menuL2_title.push("");

if (found_app_shadowsocks()){
	menuL2_title.push("<#menu5_16#>");
} else menuL2_title.push("");

if (found_app_mentohust()){
	menuL2_title.push("mentohust");
} else menuL2_title.push("");

if (found_app_koolproxy()){
	menuL2_title.push("<#menu5_20#>");
}else if (found_app_adbyby()){
	menuL2_title.push("<#menu5_20#>");
} else menuL2_title.push("");

if (found_app_smartdns()){
	menuL2_title.push("<#menu5_29#>");
} else if (found_app_adguardhome()){
	menuL2_title.push("<#menu5_29#>");
} else menuL2_title.push("");

if (found_app_aliddns()){
	menuL2_title.push("<#menu5_30#>");
} else if (found_app_zerotier()){
	menuL2_title.push("<#menu5_30#>");
} else menuL2_title.push("");

if (found_app_frp()){
	menuL2_title.push("<#menu5_25#>");
} else menuL2_title.push("");

if (found_app_caddy()){
	menuL2_title.push("<#menu5_27#>");
} else menuL2_title.push("");

if (found_app_wyy()){
	menuL2_title.push("<#menu5_31#>");
} else menuL2_title.push("");

menuL2_link  = new Array("", tablink[0][1], tablink[1][1], tablink[2][1], tablink[3][1], tablink[4][1], tablink[5][1], tablink[6][1], tablink[7][1], tablink[8][1], tablink[9][1]);
if (found_app_scutclient()){
	menuL2_link.push(scutclient_array[1]);
} else menuL2_link.push("");

if (found_app_dnsforwarder()){
	menuL2_link.push(dns_forwarder_array[1]);
} else menuL2_link.push("");

if (found_app_shadowsocks()){
	menuL2_link.push(shadowsocks_array[1]);
} else menuL2_link.push("");

if (found_app_mentohust()){
	menuL2_link.push(mentohust_array[1]);
} else menuL2_link.push("");
if (found_app_adbyby()){
	menuL2_link.push(ad_array[1]);
} else if (found_app_koolproxy()){
	menuL2_link.push(kp_array[1]);
} else menuL2_link.push("");
if (found_app_smartdns()){
	menuL2_link.push(smartdns_array[1]);
} else if (found_app_adguardhome()){
	menuL2_link.push(adg_array[1]);
} else menuL2_link.push("");
if (found_app_aliddns()){
	menuL2_link.push(aliddns_array[1]);
} else if (found_app_zerotier()){
	menuL2_link.push(zerotier_array[1]);
} else menuL2_link.push("");
if (found_app_frp()){
	menuL2_link.push(frp_array[1]);
} else menuL2_link.push("");
if (found_app_caddy()){
	menuL2_link.push(caddy_array[1]);
} else menuL2_link.push("");
if (found_app_wyy()){
	menuL2_link.push(wyy_array[1]);
} else menuL2_link.push("");

//Level 1 Menu in Gateway, Router mode
menuL1_title = new Array("", "<#menu1#>", "", "<#menu2#>", "<#menu6#>", "<#menu4#>", "<#menu5_8#>", "<#menu5#>");
menuL1_link = new Array("", "index.asp", "", "vpnsrv.asp", "vpncli.asp", "Main_TrafficMonitor_realtime.asp", "Advanced_System_Info.asp", "as.asp");
menuL1_icon = new Array("", "icon-home", "icon-hdd", "icon-retweet", "icon-globe", "icon-tasks", "icon-random", "icon-wrench");

function show_menu(L1, L2, L3){
	var i;
	var num_ephy = support_num_ephy();
	if (num_ephy < 2)
		num_ephy = 2;
	if (num_ephy > 8)
		num_ephy = 8;
	if(sw_mode == '3'){
		tabtitle[2].splice(3,1);//LAN
		tablink[2].splice(3,1);
		tabtitle[3].splice(1,5);//WAN
		tablink[3].splice(1,5);
		tabtitle[4].splice(1,5);//firewall
		tablink[4].splice(1,5);
		tabtitle[5].splice(4,1);//USB
		tablink[5].splice(4,1);
		tabtitle[9].splice(2,4);//log
		tablink[9].splice(2,4);
		tablink[2][1] = "Advanced_APLAN_Content.asp";
		menuL2_link[3] = tablink[2][1];
		menuL2_link[4] = "";  //remove WAN
		menuL2_title[4] = "";
		menuL2_link[5] = "";  //remove Firewall
		menuL2_title[5] = "";
		menuL1_link[3] = "";  //remove VPN svr
		menuL1_title[3] = "";
		menuL1_link[4] = "";  //remove VPN cli
		menuL1_title[4] = "";
		
		if (lan_proto == '1'){
			tabtitle[2].splice(2,1);
			tablink[2].splice(2,1);
		}
	}else{
		if(sw_mode == '4'){
			tablink[3].splice(3,2);
			tabtitle[3].splice(3,2);
		}
		if(!support_ipv6()){
			tablink[3].splice(2,1);
			tabtitle[3].splice(2,1);
		}
	}

	for (i=0;i<num_ephy;i++){
		tablink[8][i+3] = "Main_EStatus_Content.asp#"+i.toString();
		if (i>0)
			tabtitle[8][i+3] = "LAN"+i.toString();
	}

	if(num_ephy<8){
		tabtitle[8].splice(3+num_ephy,8-num_ephy);
		tablink[8].splice(3+num_ephy,8-num_ephy);
	}

	if(!support_2g_radio()){
		menuL2_link[1] = "";  //remove 2G
		menuL2_title[1] = "";
		tabtitle[0].splice(1,6);
		tablink[0].splice(1,6);
	}

	if(!support_5g_radio()){
		menuL2_link[2] = "";  //remove 5GHz
		menuL2_title[2] = "";
		tabtitle[1].splice(1,6);
		tablink[1].splice(1,6);
		tabtitle[8].splice(2,1);
		tablink[8].splice(2,1);
	}

	if(!support_storage()){
		tabtitle[5].splice(1,5);
		tablink[5].splice(1,5);
		menuL2_link[6] = "";  //remove USB
		menuL2_title[6] = "";
	}else{
		if(!support_usb()){
			tabtitle[5].splice(4,2);
			tablink[5].splice(4,2);
		}
		if(!found_app_smbd() && !found_app_ftpd()){
			tabtitle[5].splice(2,2);
			tablink[5].splice(2,2);
		}
		else if(!found_app_smbd()){
			tabtitle[5].splice(2,1);
			tablink[5].splice(2,1);
		}
		else if(!found_app_ftpd()){
			tabtitle[5].splice(3,1);
			tablink[5].splice(3,1);
		}
	}

	for(i = 1; i <= menuL1_title.length-1; i++){
		if(menuL1_title[i] == "")
			continue;
		else if(L1 == i && L2 <= 0)
			menu1_code += '<li class="active" id="option'+i+'"><a href="javascript:;"><i class="'+menuL1_icon[i]+'"></i>&nbsp;&nbsp;'+menuL1_title[i]+'</a></li>\n';
		else
			menu1_code += '<li id="option'+i+'"><a href="'+menuL1_link[i]+'" title="'+menuL1_link[i]+'"><i class="'+menuL1_icon[i]+'"></i>&nbsp;&nbsp;'+menuL1_title[i]+'</a></li>\n';
	}

	$("mainMenu").innerHTML = menu1_code;

	for(var i = 1; i <= menuL2_title.length-1; ++i){
		if(menuL2_title[i] == "")
			continue;
		else if(L2 == i)
			menu2_code += '<a href="javascript: void(0)" style="color: #005580; font-weight: bold"><i class="icon-minus"></i>&nbsp;&nbsp;'+menuL2_title[i]+'</a>\n';
		else
			menu2_code += '<a href="'+menuL2_link[i]+'"><i class="icon-minus"></i>&nbsp;&nbsp;'+menuL2_title[i]+'</a>\n';
	}
	$("subMenu").innerHTML = menu2_code;

	if(L3){
		tab_code = '<ul class="nav nav-tabs" style="margin-bottom: 0px;">\n';
		for(var i = 1; i < tabtitle[L2-1].length; ++i){
			if(tabtitle[L2-1][i] == "")
				continue;
			if(L3 == i){
				tab_ref = "javascript: void(0)";
				if (L2==9 && i>0 && tablink[L2-1][i].indexOf("#")>0)
					tab_ref = tablink[L2-1][i];
				tab_code += '<li class="active"><a href="' +tab_ref+ '">'+tabtitle[L2-1][i]+'</a></li>\n';
			}else
				tab_code += '<li><a href="' +tablink[L2-1][i]+ '">'+tabtitle[L2-1][i]+'</a></li>\n';
		}
		tab_code += '</ul>\n';
		$("tabMenu").innerHTML = tab_code;
	}
	else
		$("tabMenu").innerHTML = "";
}

function show_footer(){
	footer_code = '<div align="center" class="bottom-image"></div>\n';
	footer_code +='<div align="center" class="copyright"><#footer_copyright_desc#></div>\n';
	footer_code +='<div align="center">\n';
	footer_code +='  <span>Highcharts by Torstein Hønsi & <a href="http://www.highcharts.com">Highsoft</a></span></br>\n';
	footer_code +='  <span>Big icons designed by <a href="http://www.freepik.com">Freepik</a></br></span>\n';
	footer_code +='  <span>Non-Commercial Use Only</span></br>\n';
	footer_code +='</div>\n';

	$("footer").innerHTML = footer_code;

	flash_button();
}

function show_top_status(){
	var $j = jQuery.noConflict();

	$j("#cpu_info").click(function(){
		$j("#main_info").hide();
		$j("#cpu_usage").show();
	});
	$j("#mem_info").click(function(){
		$j("#main_info").hide();
		$j("#mem_usage").show();
	});

	id_system_info = setTimeout('get_system_info()', 2000);
	showSystemInfo({busy: 0, user: 0, nice: 0, system: 0, idle: 0, iowait: 0, irq: 0, sirq: 0}, 0);

	showtext($("firmver"), '<% nvram_get_x("",  "firmver_sub"); %>');

	/*if(sw_mode == "3")
		$("sw_mode_span").innerHTML = "AP";
	else
		$("sw_mode_span").innerHTML = "Router";*/
}

function go_setting(band){
	if(band == "2")
		location.href = "Advanced_Wireless2g_Content.asp";
	else
		location.href = "Advanced_Wireless_Content.asp";
}

function go_wguest(band){
	if(band == "2")
		location.href = "Advanced_WGuest2g_Content.asp";
	else
		location.href = "Advanced_WGuest_Content.asp";
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

function checkTime(i){
	if (i<10)
		{i="0" + i}
	return i;
}

function show_loading_obj(){
	var code = '';

	code += '<center><div id="loadingBlock" class="loadingBlock">';
	code += '<div class="container-fluid">';
	code += '<div class="well" style="background-color: #212121; width: 60%;">';
	code += '<div class="progress progress-striped active" style="width: 50%; text-align: left;"><div class="bar" id="proceeding_bar" style="width: 0%;"><span id="proceeding_txt"></span></div></div>';
	code += '<span id="proceeding_main_txt"><#Main_alert_proceeding_desc4#></span></span>';
	code += '</div>';
	code += '</div>';
	code += '</div></center>';

	$("Loading").innerHTML = code;
	id_check_status = setTimeout('hideLoading();', 3000);
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
}

function logout(){
	if(!confirm('<#JS_logout#>'))
		return;
	setTimeout('location = "Logout.asp";', 1);
}

function reboot(){
	if(!confirm('<#Main_content_Login_Item7#>'))
		return;
	showLoading(board_boot_time());
	var $j = jQuery.noConflict();
	$j.post('/apply.cgi',
	{
		'action_mode': ' Reboot ',
	});
}

function shutdown(){
	if(!confirm('<#JS_shutdown#>'))
		return;
	var $j = jQuery.noConflict();
	$j.post('/apply.cgi',
	{
		'action_mode': ' Shutdown ',
		'current_page': 'Main_LogStatus_Content.asp'
	});
}

function click_info_cpu(){
	location.href="/Advanced_System_Info.asp#CPU";
}

function click_info_mem(){
	location.href="/Advanced_System_Info.asp#MEM";
}

function hide_adv_info(){
	var $j = jQuery.noConflict();
	$j("#cpu_usage").hide();
	$j("#mem_usage").hide();
	$j("#main_info").show();
}

function reset_btn_commit(btn_id){
	var $j = jQuery.noConflict();
	var $btn=$j('#'+btn_id);
	$btn.removeClass('alert-error').removeClass('alert-success');
}

function commit(){
	if(!confirm('<#Commit_confirm#>'))
		return;
	var $j = jQuery.noConflict();
	var $btn = $j('#commit_btn');
	$j.ajax({
		type: "post",
		url: "/apply.cgi",
		data: {
			action_mode: " CommitFlash ",
			nvram_action: "commit_nvram"
		},
		dataType: "json",
		error: function(xhr) {
			$btn.addClass('alert-error');
			setTimeout("reset_btn_commit('#commit_btn')", 1500);
		},
		success: function(response) {
			var sys_result = (response != null && typeof response === 'object' && "sys_result" in response)
				? response.sys_result : -1;
			if(sys_result == 0)
				$btn.addClass('alert-success');
			else
				$btn.addClass('alert-error');
			setTimeout("reset_btn_commit('commit_btn')", 1500);
		}
	});
}

function clearlog(){
	var $j = jQuery.noConflict();
	$j.post('/apply.cgi',
	{
		'action_mode': ' ClearLog ',
		'current_page': 'Main_LogStatus_Content.asp'
	});
	setLogData();
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

function E(e) {
	return (typeof(e) === 'string') ? document.getElementById(e) : e;
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

function trim(str){
	return str.replace(/^\s+|\s+$/g, '');
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

	formObj.action = prev_page;
	formObj.target = "_parent";
	formObj.submit();
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
	obj.disabled = (flag == 0);
}

// add eagle23
jQuery(document).ready(function(){
    var $j = jQuery.noConflict();

    $j("#logo").click(function(){
        location.href = '/';
    });

    // tabindex navigation
    $j(function(){
        var tabindex = 1;
        $j('input,select').each(function() {
            if (this.type != "hidden"  && this.type != 'radio') {
                var $input = $j(this);
                $input.attr("tabindex", tabindex);
                tabindex++;
            }
        });
    });

    var idFindSyslogPanel = setInterval(function(){
        if(is_mobile || log_float == '0'){
            clearInterval(idFindSyslogPanel);
        }else if($j('.syslog_panel').size() > 0){
            clearInterval(idFindSyslogPanel);

            var offsetLeft = $j('.wrapper').offset().left;
            $j('.syslog_panel').css({opacity: 1});
            $j('.syslog_panel').tabSlideOut({
                tabHandle: '.handle',
                imageHeight: '20px',
                imageWidth: '62px',
                tabLocation: 'top',
                speed: 300,
                action: 'click',
                topPos: '400px',
                leftPos: (offsetLeft+5)+'px',
                fixedPosition: true
            });

            setLogData();
            showClockLogArea();
        }
    }, 100);
});

// fix for ie
String.prototype.nl2br = function(){
    return this.replace(/\n/g, "\n\r");
}

function setLogStamp(mt){
    if(is_mobile || log_float == '0')
        return;

    var $j = jQuery.noConflict();

    if(isLocalStorageAvailable())
        log_stamp = localStorage.getItem('syslog_stamp');
    if(log_stamp == null)
        log_stamp = 0;

    if (log_stamp != mt){
        setToLocalStorage('syslog_stamp', mt);
        if (log_stamp != 0){
            setLogData();
            if(!$j('.syslog_panel').hasClass('open')){
                var tabText = 'Log <span class="label label-important">!</span>';
                $j(".log_text").html(tabText);
            }
        }
        log_stamp = mt;
    }
}

function setLogData(){
    var $j = jQuery.noConflict();
    $j.get('/log_content.asp', function(data){
        // fix for ie
        if($j.browser.msie && !is_ie11p)
            data = data.nl2br();
        if($j("#log_area").val() == ''){
            $j("#log_area").text(data);
            $j("#log_area").prop('scrollTop', $j("#log_area").prop('scrollHeight'));
            $j("#scrATop").val($j("#log_area").prop('scrollTop'));
        }else{
            var scrMaxTop = $j("#log_area").prop('scrollHeight')
            var scrTop = $j("#log_area").prop('scrollTop');
            $j("#log_area").text(data);
            var scrITop = scrMaxTop - scrTop;
            if($j("#scrATop").val() == scrTop || scrITop < 629){
                $j("#log_area").prop('scrollTop', scrMaxTop);
                $j("#scrATop").val($j("#log_area").prop('scrollTop'));
            }else{
                $j("#log_area").prop('scrollTop', scrTop);
            }
        }
    });
}

function showClockLogArea(){
    if(jQuery('#system_time').size() == 0){
        JS_timeObj.setTime(systime_millsec);
        systime_millsec += 1000;

        JS_timeObj2 = JS_timeObj.toString();
        JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
        JS_timeObj2.substring(4,10) + "  " +
        checkTime(JS_timeObj.getHours()) + ":" +
        checkTime(JS_timeObj.getMinutes()) + ":" +
        checkTime(JS_timeObj.getSeconds()) + "  " +
        JS_timeObj.getFullYear() + " GMT" + timezone;
    }
    jQuery("#system_time_log_area").html(JS_timeObj2);
    setTimeout("showClockLogArea()", 1000);
}

function onCompleteSlideOutLogArea(){
    var idTimeout = setTimeout(function(){
        clearTimeout(idTimeout);
        jQuery(".log_text").html('Log');
    }, 1500);
}

function passwordShowHide(id){
    var obj = $j('#'+id);
    var changeTo = (obj.attr('type') == 'password') ? 'text' : 'password';
    if ($j.browser.msie && parseInt($j.browser.version, 10) < 9){
        var marker = $j('<span />').insertBefore('#'+id);
        obj.detach().attr('type', changeTo).insertAfter(marker);
        marker.remove();
    }else{
        document.getElementById(id).type = changeTo;
    }
}

/**
 * Local Storage HTML5 Standart
 * http://www.w3.org/TR/webstorage/
 */
/**
 * ckeck if localStorage available
 * @return void
 */
function isLocalStorageAvailable(){
    try {
        return 'localStorage' in window && window['localStorage'] !== null;
    } catch (e) {
        return false;
    }
}

/**
 * set value to localStorage
 * @param name string
 * @param value mixed
 */
function setToLocalStorage(name, value){
    if(isLocalStorageAvailable()){
        try {
            localStorage.setItem(name, value);
        } catch (e) {
            if (e == QUOTA_EXCEEDED_ERR) {
            }
        }
    }
}

/**
 * get from localStorage
 * @param name
 * @return mixed
 */
function getFromLocalStorage(name){
    if(isLocalStorageAvailable()){
        return localStorage.getItem(name);
    }
}

/**
 * remove from localStorage
 * @param name
 * @return void
 */
function removeFromLocalStorage(name){
    if(isLocalStorageAvailable()){
        localStorage.removeItem(name);
    }
}
//WEB自定义菜单
var w_ai = '<% nvram_get_x("", "w_ai"); %>';
var w_vpn_s = '<% nvram_get_x("", "w_vpn_s"); %>';
var w_vpn_c = '<% nvram_get_x("", "w_vpn_c"); %>';
var w_wnet = '<% nvram_get_x("", "w_wnet"); %>';
var w_sys = '<% nvram_get_x("", "w_sys"); %>';
var w_usb = '<% nvram_get_x("", "w_usb"); %>';
var w_net = '<% nvram_get_x("", "w_net"); %>';
var w_log = '<% nvram_get_x("", "w_log"); %>';
var w_scu = '<% nvram_get_x("", "w_scu"); %>';
var w_dnsf = '<% nvram_get_x("", "w_dnsf"); %>';
var w_ss = '<% nvram_get_x("", "w_ss"); %>';
var w_men = '<% nvram_get_x("", "w_men"); %>';
var w_adbyby = '<% nvram_get_x("", "w_adbyby"); %>';

if (w_ai==0){
	menuL1_link[2] = "";
	menuL1_title[2] = "";
}
if (w_vpn_s==0){
	menuL1_link[3] = "";
	menuL1_title[3] = "";
}
if (w_vpn_c==0){
	menuL1_link[4] = "";
	menuL1_title[4] = "";
}
if (w_wnet==0){
	menuL1_link[5] = "";
	menuL1_title[5] = "";
}
if (w_sys==0){
	menuL1_link[6] = "";
	menuL1_title[6] = "";
}
if (w_usb==0){
	menuL2_link[6] = "";
	menuL2_title[6] = "";
}
if (w_net==0){
	menuL2_link[9] = "";
	menuL2_title[9] = "";
}
if (w_log==0){
	menuL2_link[10] = "";
	menuL2_title[10] = "";
}
if (w_scu==0){
	menuL2_link[11] = "";
	menuL2_title[11] = "";
}
if (w_dnsf==0){
	menuL2_link[12] = "";
	menuL2_title[12] = "";
}
if (w_ss==0){
	menuL2_link[13] = "";
	menuL2_title[13] = "";
}
if (w_men==0){
	menuL2_link[14] = "";
	menuL2_title[14] = "";
}
if (w_adbyby==0){
	menuL2_link[15] = "";
	menuL2_title[15] = "";
}
(function($){
    var $j = $.noConflict();
    $j.fn.tabSlideOut = function(callerSettings){
        var settings =  $j.extend({
            tabHandle: '.handle',
            speed: 300,
            action: 'click',
            tabLocation: 'left',
            topPos: '200px',
            leftPos: '20px',
            fixedPosition: false,
            positioning: 'absolute',
            pathToTabImage: null,
            imageHeight: null,
            imageWidth: null
        }, callerSettings||{});

        settings.tabHandle =  $j(settings.tabHandle);
        var obj = this;
        if (settings.fixedPosition === true) {
            settings.positioning = 'fixed';
        } else {
            settings.positioning = 'absolute';
        }

        //ie6 doesn't do well with the fixed option
        if (document.all && !window.opera && !window.XMLHttpRequest) {
            settings.positioning = 'absolute';
        }

        //set initial tabHandle css
        settings.tabHandle.css({
            'display': 'block',
            'width' : settings.imageWidth,
            'height': settings.imageHeight,
            //'textIndent' : '-99999px',
            //'background' : 'url('+settings.pathToTabImage+') no-repeat',
            'outline' : 'none',
            'position' : 'absolute',
            'border-radius': '0px 0px 4px 4px',
            'background-color': '#f5f5f5',
            'border-left': '1px solid #ddd',
            'border-right': '1px solid #ddd',
            'border-bottom': '1px solid #ddd'
        });

        obj.css({
            'line-height' : '1',
            'position' : settings.positioning
        });

        var properties = {
            containerWidth: parseInt(obj.outerWidth(), 10) + 'px',
            containerHeight: parseInt(obj.outerHeight(), 10) + 'px',
            tabWidth: parseInt(settings.tabHandle.outerWidth(), 10) + 'px',
            tabHeight: parseInt(settings.tabHandle.outerHeight(), 10) + 'px'
        };

        //set calculated css
        if(settings.tabLocation === 'top' || settings.tabLocation === 'bottom') {
            obj.css({'left' : settings.leftPos});
            settings.tabHandle.css({'right' : -1});
        }

        if(settings.tabLocation === 'top') {
            obj.css({'top' : '-' + properties.containerHeight});
            settings.tabHandle.css({'bottom' : '-' + properties.tabHeight});
        }

        if(settings.tabLocation === 'bottom') {
            obj.css({'bottom' : '-' + properties.containerHeight, 'position' : 'fixed'});
            settings.tabHandle.css({'top' : '-' + properties.tabHeight});

        }

        if(settings.tabLocation === 'left' || settings.tabLocation === 'right') {
            obj.css({
                'height' : properties.containerHeight,
                'top' : settings.topPos
            });

            settings.tabHandle.css({'top' : 0});
        }

        if(settings.tabLocation === 'left') {
            obj.css({ 'left': '-' + properties.containerWidth});
            settings.tabHandle.css({'right' : '-' + properties.tabWidth});
        }

        if(settings.tabLocation === 'right') {
            obj.css({ 'right': '-' + properties.containerWidth});
            settings.tabHandle.css({'left' : '-' + properties.tabWidth});

            $j('html').css('overflow-x', 'hidden');
        }

        //functions for animation events

        settings.tabHandle.click(function(event){
            event.preventDefault();
        });

        var slideIn = function() {

            if (settings.tabLocation === 'top') {
                obj.animate({top:'-' + properties.containerHeight}, settings.speed).removeClass('open');
            } else if (settings.tabLocation === 'left') {
                obj.animate({left: '-' + properties.containerWidth}, settings.speed).removeClass('open');
            } else if (settings.tabLocation === 'right') {
                obj.animate({right: '-' + properties.containerWidth}, settings.speed).removeClass('open');
            } else if (settings.tabLocation === 'bottom') {
                obj.animate({bottom: '-' + properties.containerHeight}, settings.speed).removeClass('open');
            }

        };

        var slideOut = function() {

            if (settings.tabLocation == 'top') {
                obj.animate({top:'-3px'},  settings.speed, onCompleteSlideOutLogArea()).addClass('open');
            } else if (settings.tabLocation == 'left') {
                obj.animate({left:'-3px'},  settings.speed, onCompleteSlideOutLogArea()).addClass('open');
            } else if (settings.tabLocation == 'right') {
                obj.animate({right:'-3px'},  settings.speed, onCompleteSlideOutLogArea()).addClass('open');
            } else if (settings.tabLocation == 'bottom') {
                obj.animate({bottom:'-3px'},  settings.speed, onCompleteSlideOutLogArea()).addClass('open');
            }
        };

        var clickScreenToClose = function() {
            obj.click(function(event){
                event.stopPropagation();
            });

            $j(document).click(function(){
                slideIn();
            });
        };

        var clickAction = function(){
            settings.tabHandle.click(function(event){
                if (obj.hasClass('open')) {
                    slideIn();
                } else {
                    slideOut();
                }
            });

            clickScreenToClose();
        };

        var hoverAction = function(){
            obj.hover(
                function(){
                    slideOut();
                },

                function(){
                    slideIn();
                });

            settings.tabHandle.click(function(event){
                if (obj.hasClass('open')) {
                    slideIn();
                }
            });
            clickScreenToClose();

        };

        //choose which type of action to bind
        if (settings.action === 'click') {
            clickAction();
        }

        if (settings.action === 'hover') {
            hoverAction();
        }
    };
})(jQuery);

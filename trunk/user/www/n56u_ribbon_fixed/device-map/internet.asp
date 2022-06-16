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
<script type="text/javascript" src="formcontrol.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>

<% wanlink(); %>

var $j = jQuery.noConflict();
var id_update_wanip = 0;
var last_bytes_rx = 0;
var last_bytes_tx = 0;
var last_time = 0;

window.performance = window.performance || {};
performance.now = (function() {
	return performance.now ||
	performance.mozNow ||
	performance.msNow ||
	performance.oNow ||
	performance.webkitNow ||
	function() { return new Date().getTime(); };
})();

function initial(){
	flash_button();

	if(!support_usb())
		$j("#domore")[0].remove(6);

	if(sw_mode == '4'){
		$j("#domore")[0].remove(4);
		$j("#domore")[0].remove(3);
	}

	if(!support_ipv6())
		$j("#domore")[0].remove(2);

	if(typeof parent.modem_devnum === 'function'){
		if(parent.modem_devnum().length > 0)
			$("row_modem_prio").style.display = "";
	}

	fill_info();

	id_update_wanip = setTimeout("update_wanip();", 2500);
}

function bytesToIEC(bytes, precision){
	var absval = Math.abs(bytes);
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;
	var petabyte = terabyte * 1024;

	if (absval < kilobyte)
		return bytes + ' B';
	else if (absval < megabyte)
		return (bytes / kilobyte).toFixed(precision) + ' KiB';
	else if (absval < gigabyte)
		return (bytes / megabyte).toFixed(precision) + ' MiB';
	else if (absval < terabyte)
		return (bytes / gigabyte).toFixed(precision) + ' GiB';
	else if (absval < petabyte)
		return (bytes / terabyte).toFixed(precision) + ' TiB';
	else
		return (bytes / petabyte).toFixed(precision) + ' PiB';
}

function kbitsToRate(kbits, precision){
	var absval = Math.abs(kbits);
	var megabit = 1000;
	var gigabit = megabit * 1000;

	if (absval < megabit)
		return kbits + ' Kbps';
	else if (absval < gigabit)
		return (kbits / megabit).toFixed(precision) + ' Mbps';
	else
		return (kbits / gigabit).toFixed(precision) + ' Gbps';
}

function secondsToDHM(seconds){
	var days, hours, minutes;

	days = Math.floor(seconds / 86400);
	minutes = Math.floor(seconds / 60);
	hours = (Math.floor(minutes / 60)) % 24;
	minutes = minutes % 60;

	hours = hours < 10 ? ('0'+hours) : hours;
	minutes = minutes < 10 ? ('0'+minutes) : minutes;

	return days+"<#Day#>".substring(0,1)+" "+hours+"<#Hour#>".substring(0,1)+" "+minutes+"<#Minute#>".substring(0,1);
}

function fill_status(scode,wtype){
	var stext = "Unknown";
	if (scode == 0)
		stext = "<#InetState0#>";
	else if (scode == 1)
		stext = "<#InetState1#>";
	else if (scode == 2)
		stext = "<#InetState2#>";
	else if (scode == 3)
		stext = "<#InetState3#>";
	else if (scode == 4)
		stext = "<#InetState4#>";
	else if (scode == 5)
		stext = "<#InetState5#>";
	else if (scode == 6)
		stext = "<#InetState6#>";
	else if (scode == 7)
		stext = "<#InetState7#>";
	else if (scode == 8)
		stext = "<#InetState8#>";
	else if (scode == 9)
		stext = "<#InetState9#>";
	$("wan_status").innerHTML = '<span class="label label-' + (scode != 0 ? 'warning' : 'success') + '">' + stext + '</span>';

	var wtext = wtype;
	if(wtype == 'Automatic IP')
		wtext = 'IPoE: <#BOP_ctype_title1#>';
	else if(wtype == 'Static IP')
		wtext = 'IPoE: <#BOP_ctype_title5#>';
	$("WANType").innerHTML = wtext;
}

function fill_uptime(uptime,dltime){
	if (uptime > 0){
		$("WANTime").innerHTML = secondsToDHM(uptime);
		$("row_uptime").style.display = "";
		if (dltime > 0){
			$("WANLease").innerHTML = secondsToDHM(dltime);
			$("row_dltime").style.display = "";
		}else
			$("row_dltime").style.display = "none";
	}else{
		$("row_dltime").style.display = "none";
		$("row_uptime").style.display = "none";
	}
}

function fill_phylink(ether_link,apcli_link){
	$("WANEther").innerHTML = ether_link;
	$("WANAPCli").innerHTML = apcli_link;
	if (ether_link != '')
		$("row_link_ether").style.display = "";
	else
		$("row_link_ether").style.display = "none";
	if (apcli_link != '')
		$("row_link_apcli").style.display = "";
	else
		$("row_link_apcli").style.display = "none";
}

function fill_man_addr4(ip,gw){
	if (ip != ''){
		$("MANIP4").innerHTML = ip;
		$("MANGW4").innerHTML = gw;
		$("row_man_ip4").style.display = "";
		$("row_man_gw4").style.display = "";
	}else{
		$("row_man_ip4").style.display = "none";
		$("row_man_gw4").style.display = "none";
	}
}

function fill_wan_addr6(wan_ip,lan_ip){
	if (wan_ip != ''){
		$("WANIP6").innerHTML = wan_ip;
		$("row_wan_ip6").style.display = "";
	}else
		$("row_wan_ip6").style.display = "none";
	if (lan_ip != ''){
		$("LANIP6").innerHTML = lan_ip;
		$("row_lan_ip6").style.display = "";
	}else
		$("row_lan_ip6").style.display = "none";
}

function fill_wan_bytes(rx,tx,now){
	if (rx > 0 || tx > 0){
		var diff_rx = 0;
		var diff_tx = 0;
		if (last_time > 0 && now > last_time){
			var diff_time = (now - last_time);
			if (last_bytes_rx > 0){
				if (rx < last_bytes_rx){
					if (last_bytes_rx <= 0xFFFFFFFF && last_bytes_rx > 0xE0000000)
						diff_rx = (0xFFFFFFFF - last_bytes_rx) + rx;
				}else
					diff_rx = rx - last_bytes_rx;
				diff_rx = Math.floor(diff_rx * 8 / diff_time);
			}
			if (last_bytes_tx > 0){
				if (tx < last_bytes_tx){
					if (last_bytes_tx <= 0xFFFFFFFF && last_bytes_tx > 0xE0000000)
						diff_tx = (0xFFFFFFFF - last_bytes_tx) + tx;
				}else
					diff_tx = tx - last_bytes_tx;
				diff_tx = Math.floor(diff_tx * 8 / diff_time);
			}
		}
		last_bytes_rx = rx;
		last_bytes_tx = tx;
		last_time = now;
		
		$("WANBytesRX").innerHTML = '<i class="icon-arrow-down"></i>'+bytesToIEC(rx,2);
		$("WANBytesTX").innerHTML = '<i class="icon-arrow-up"></i>'+bytesToIEC(tx,2);
		$("WANBRateRX").innerHTML = '<i class="icon-arrow-down"></i>'+kbitsToRate(diff_rx,2);
		$("WANBRateTX").innerHTML = '<i class="icon-arrow-up"></i>'+kbitsToRate(diff_tx,2);
		$("row_bytes").style.display = "";
		$("row_brate").style.display = "";
	}else{
		$("row_bytes").style.display = "none";
		$("row_brate").style.display = "none";
	}
}

function fill_info(){
	var now = performance.now();
	fill_status(wanlink_status(), wanlink_type());
	fill_uptime(wanlink_uptime(), wanlink_dltime());
	fill_phylink(wanlink_etherlink(), wanlink_apclilink());
	fill_man_addr4(wanlink_ip4_man(), wanlink_gw4_man());
	fill_wan_addr6(wanlink_ip6_wan(), wanlink_ip6_lan());
	fill_wan_bytes(wanlink_bytes_rx(), wanlink_bytes_tx(), now);
	$("WANIP4").innerHTML = wanlink_ip4_wan();
	$("WANGW4").innerHTML = wanlink_gw4_wan();
	$("WANDNS").innerHTML = wanlink_dns();
	$("WANMAC").innerHTML = wanlink_mac();
}

function update_wanip(){
	clearTimeout(id_update_wanip);
	$j.ajax({
		url: '/status_wanlink.asp',
		dataType: 'script',
		cache: true,
		error: function(xhr){
			;
		},
		success: function(response){
			fill_info();
			id_update_wanip = setTimeout("update_wanip();", 2500);
		}
	});
}

function submitInternet(v){
	parent.showLoading();
	document.internetForm.action = "wan_action.asp";
	document.internetForm.wan_action.value = v;
	document.internetForm.modem_prio.value = $("modem_prio").value;
	document.internetForm.submit();
}

</script>
</head>

<body class="body_iframe" onload="initial();">
<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_info">
  <tr>
    <th width="50%" style="border-top: 0 none;"><#InetControl#></th>
    <td style="border-top: 0 none;" colspan="3">
      <input type="button" id="btn_connect_1" class="btn btn-info" value="<#Connect#>" onclick="submitInternet('Connect');">
      <input type="button" id="btn_connect_0" class="btn btn-danger" value="<#Disconnect#>" onclick="submitInternet('Disconnect');">
    </td>
  </tr>
  <tr id="row_modem_prio" style="display:none">
    <th><#ModemPrio#></th>
    <td colspan="3">
        <select id="modem_prio" class="input" style="width: 260px;" onchange="submitInternet('ModemPrio');">
            <option value="0" <% nvram_match_x("", "modem_prio", "0", "selected"); %>><#ModemPrioItem0#></option>
            <option value="1" <% nvram_match_x("", "modem_prio", "1", "selected"); %>><#ModemPrioItem1#></option>
            <option value="2" <% nvram_match_x("", "modem_prio", "2", "selected"); %>><#ModemPrioItem2#></option>
        </select>
    </td>
  </tr>
  <tr id="row_link_ether" style="display:none">
    <th><#SwitchState#></th>
    <td colspan="3"><span id="WANEther"></span></td>
  </tr>
  <tr id="row_link_apcli" style="display:none">
    <th><#InetStateWISP#></th>
    <td colspan="2"><span id="WANAPCli"></span></td>
    <td width="40px" style="text-align: right; padding: 6px 8px"><button type="button" class="btn btn-mini" style="height: 21px; outline:0;" title="<#Connect#>" onclick="submitInternet('WispReassoc');"><i class="icon icon-refresh"></i></button></td>
  </tr>
  <tr>
    <th><#ConnectionStatus#></th>
    <td id="wan_status" colspan="3"></td>
  </tr>
  <tr>
    <th><#Connectiontype#>:</th>
    <td colspan="3"><span id="WANType"></span></td>
  </tr>
  <tr id="row_uptime" style="display:none">
    <th><#WAN_Uptime#></th>
    <td colspan="3"><span id="WANTime"></span></td>
  </tr>
  <tr id="row_dltime" style="display:none">
    <th><#WAN_Lease#></th>
    <td colspan="3"><span id="WANLease"></span></td>
  </tr>
  <tr id="row_bytes" style="display:none">
    <th><#WAN_Bytes#></th>
    <td width="90px"><span id="WANBytesRX"></span></td>
    <td colspan="2"><span id="WANBytesTX"></span></td>
  </tr>
  <tr id="row_brate" style="display:none">
    <th><#WAN_BRate#></th>
    <td width="90px"><span id="WANBRateRX"></span></td>
    <td colspan="2"><span id="WANBRateTX"></span></td>
  </tr>
  <tr>
    <th><#IP4_Addr#> WAN:</th>
    <td colspan="3"><span id="WANIP4"></span></td>
  </tr>
  <tr id="row_man_ip4" style="display:none">
    <th><#IP4_Addr#> MAN:</th>
    <td colspan="3"><span id="MANIP4"></span></td>
  </tr>
  <tr id="row_wan_ip6" style="display:none">
    <th><#IP6_Addr#> WAN:</th>
    <td colspan="3"><span id="WANIP6"></span></td>
  </tr>
  <tr id="row_lan_ip6" style="display:none">
    <th><#IP6_Addr#> LAN:</th>
    <td colspan="3"><span id="LANIP6"></span></td>
  </tr>
  <tr>
    <th><#Gateway#> WAN:</th>
    <td colspan="3"><span id="WANGW4"></span></td>
  </tr>
  <tr id="row_man_gw4" style="display:none">
    <th><#Gateway#> MAN:</th>
    <td colspan="3"><span id="MANGW4"></span></td>
  </tr>
  <tr>
    <th>DNS:</th>
    <td colspan="3"><span id="WANDNS"></span></td>
  </tr>
  <tr>
    <th><#MAC_Address#></th>
    <td colspan="3"><span id="WANMAC"></span></td>
  </tr>
  <tr id="row_more_links">
    <td style="padding-bottom: 0px;">&nbsp;</td>
    <td style="padding-bottom: 0px;" colspan="3">
        <select id="domore" class="domore" style="width: 260px;" onchange="domore_link(this);">
          <option selected="selected"><#MoreConfig#>...</option>
          <option value="../Advanced_WAN_Content.asp"><#menu5_3_1#></option>
          <option value="../Advanced_IPv6_Content.asp"><#menu5_3_3#></option>
          <option value="../Advanced_VirtualServer_Content.asp"><#menu5_3_4#></option>
          <option value="../Advanced_Exposed_Content.asp"><#menu5_3_5#></option>
          <option value="../Advanced_DDNS_Content.asp"><#menu5_3_6#></option>
          <option value="../Advanced_Modem_others.asp"><#menu5_4_4#></option>
          <option value="../vpnsrv.asp"><#menu2#></option>
          <option value="../vpncli.asp"><#menu6#></option>
        </select>
    </td>
  </tr>
</table>

<form method="post" name="internetForm" action="">
<input type="hidden" name="wan_action" value="">
<input type="hidden" name="modem_prio" value="">
</form>

</body>
</html>

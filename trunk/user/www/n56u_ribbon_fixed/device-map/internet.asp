<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title></title>
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

function initial(){
	flash_button();

	if(sw_mode == '4'){
		$j("#domore")[0].remove(4);
		$j("#domore")[0].remove(3);
	}

	if(!support_ipv6())
		$j("#domore")[0].remove(2);

	if(parent.modem_devnum().length > 0)
		$("row_modem_prio").style.display = "";

	fill_info();
	id_update_wanip = setTimeout("update_wanip();", 3000);
}

function bytesToIEC(bytes, precision){
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;
	var petabyte = terabyte * 1024;

	if ((bytes >= 0) && (bytes < kilobyte))
		return bytes + ' B';
	else if ((bytes >= kilobyte) && (bytes < megabyte))
		return (bytes / kilobyte).toFixed(precision) + ' KiB';
	else if ((bytes >= megabyte) && (bytes < gigabyte))
		return (bytes / megabyte).toFixed(precision) + ' MiB';
	else if ((bytes >= gigabyte) && (bytes < terabyte))
		return (bytes / gigabyte).toFixed(precision) + ' GiB';
	else if ((bytes >= terabyte) && (bytes < petabyte))
		return (bytes / terabyte).toFixed(precision) + ' TiB';
	else if (bytes >= petabyte)
		return (bytes / petabyte).toFixed(precision) + ' PiB';
	else
		return bytes + ' B';
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

function fill_wan_bytes(rx,tx){
	if (rx > 0 || tx > 0){
		$("WANBytes").innerHTML = '<i class="icon-arrow-down"></i>'+bytesToIEC(rx,2)+'&nbsp;&nbsp;<i class="icon-arrow-up"></i>'+bytesToIEC(tx,2);
		$("row_bytes").style.display = "";
	}else
		$("row_bytes").style.display = "none";
}

function fill_info(){
	fill_status(wanlink_status(), wanlink_type());
	fill_uptime(wanlink_uptime(), wanlink_dltime());
	fill_phylink(wanlink_etherlink(), wanlink_apclilink());
	fill_man_addr4(wanlink_ip4_man(), wanlink_gw4_man());
	fill_wan_addr6(wanlink_ip6_wan(), wanlink_ip6_lan());
	fill_wan_bytes(wanlink_bytes_rx(), wanlink_bytes_tx());
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
			id_update_wanip = setTimeout("update_wanip();", 3000);
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
<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%" style="border-top: 0 none;"><#InetControl#></th>
    <td style="border-top: 0 none;">
    <div style="display:none"></div>
      <input type="button" id="btn_connect_1" class="btn btn-info" value="<#Connect#>" onclick="submitInternet('Connect');">
      <input type="button" id="btn_connect_0" class="btn btn-danger" value="<#Disconnect#>" onclick="submitInternet('Disconnect');">
    </td>
  </tr>
  <tr id="row_modem_prio" style="display:none">
    <th><#ModemPrio#></th>
    <td>
        <select id="modem_prio" class="input" style="width: 260px;" onchange="submitInternet('ModemPrio');">
            <option value="0" <% nvram_match_x("", "modem_prio", "0", "selected"); %>><#ModemPrioItem0#></option>
            <option value="1" <% nvram_match_x("", "modem_prio", "1", "selected"); %>><#ModemPrioItem1#></option>
            <option value="2" <% nvram_match_x("", "modem_prio", "2", "selected"); %>><#ModemPrioItem2#></option>
        </select>
    </td>
  </tr>
  <tr id="row_link_ether" style="display:none">
    <th><#SwitchState#></th>
    <td><span id="WANEther"></span></td>
  </tr>
  <tr id="row_link_apcli" style="display:none">
    <th><#InetStateWISP#></th>
    <td><span id="WANAPCli"></span></td>
  </tr>
  <tr>
    <th><#ConnectionStatus#></th>
    <td id="wan_status"></td>
  </tr>
  <tr id="row_uptime" style="display:none">
    <th><#WAN_Uptime#></th>
    <td><span id="WANTime"></span></td>
  </tr>
  <tr id="row_dltime" style="display:none">
    <th><#WAN_Lease#></th>
    <td><span id="WANLease"></span></td>
  </tr>
  <tr id="row_bytes" style="display:none">
    <th><#WAN_Bytes#></th>
    <td><span id="WANBytes"></span></td>
  </tr>
  <tr>
    <th><#Connectiontype#>:</th>
    <td><span id="WANType"></span></td>
  </tr>
  <tr>
    <th><#IP4_Addr#> WAN:</th>
    <td><span id="WANIP4"></span></span></td>
  </tr>
  <tr id="row_man_ip4" style="display:none">
    <th><#IP4_Addr#> MAN:</th>
    <td><span id="MANIP4"></span></td>
  </tr>
  <tr id="row_wan_ip6" style="display:none">
    <th><#IP6_Addr#> WAN:</th>
    <td><span id="WANIP6"></span></td>
  </tr>
  <tr id="row_lan_ip6" style="display:none">
    <th><#IP6_Addr#> LAN:</th>
    <td><span id="LANIP6"></span></td>
  </tr>
  <tr>
    <th><#Gateway#> WAN:</th>
    <td><span id="WANGW4"></span></td>
  </tr>
  <tr id="row_man_gw4" style="display:none">
    <th><#Gateway#> MAN:</th>
    <td><span id="MANGW4"></span></td>
  </tr>
  <tr>
    <th>DNS:</th>
    <td><span id="WANDNS"></span></td>
  </tr>
  <tr>
    <th><#MAC_Address#></th>
    <td><span id="WANMAC"></span></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td>
        <select id="domore" class="domore" style="width: 260px;" onchange="domore_link(this);">
          <option selected="selected"><#MoreConfig#>...</option>
          <option value="../Advanced_WAN_Content.asp"><#menu5_3_1#></option>
          <option value="../Advanced_IPv6_Content.asp"><#menu5_3_3#></option>
          <option value="../Advanced_VirtualServer_Content.asp"><#menu5_3_4#></option>
          <option value="../Advanced_Exposed_Content.asp"><#menu5_3_5#></option>
          <option value="../Advanced_DDNS_Content.asp"><#menu5_3_6#></option>
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

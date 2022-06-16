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
var id_update_lanip = 0;

function initial(){
	flash_button();

	fill_info();
	id_update_lanip = setTimeout("update_lanip();", 3000);
}

function fill_status(scode,ltype){
	var stext = "Unknown";
	if (scode == 0)
		stext = "<#InetState0#>";
	else if (scode == 1)
		stext = "<#InetState1#>";
	else if (scode == 4)
		stext = "<#InetState4#>";
	else if (scode == 5)
		stext = "<#InetState5#>";
	else if (scode == 8)
		stext = "<#InetState8#>";
	$("lan_status").innerHTML = '<span class="label label-' + (scode != 0 ? 'warning' : 'success') + '">' + stext + '</span>';

	var ltext = ltype;
	if(ltype == 'Automatic IP')
		ltext = '<#BOP_ctype_title1#>';
	else if(ltype == 'Static IP')
		ltext = '<#BOP_ctype_title5#>';
	$("LANType").innerHTML = ltext;
}

function fill_info(){
	fill_status(wanlink_status(), wanlink_type());
	$("LANIP4").innerHTML = wanlink_ip4_wan();
	$("LANGW4").innerHTML = wanlink_gw4_wan();
	$("LANDNS").innerHTML = wanlink_dns();
	$("LANMAC").innerHTML = wanlink_mac();
}

function update_lanip(){
	clearTimeout(id_update_lanip);
	$j.ajax({
		url: '/status_wanlink.asp',
		dataType: 'script',
		cache: true,
		error: function(xhr){
			;
		},
		success: function(response){
			fill_info();
			id_update_lanip = setTimeout("update_lanip();", 3000);
		}
	});
}

</script>
</head>

<body class="body_iframe" onload="initial();">
<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th style="border-top: 0 none;"><#ConnectionStatus#></th>
    <td style="border-top: 0 none;" id="lan_status"></td>
  </tr>
  <tr>
    <th><#Connectiontype#>:</th>
    <td><span id="LANType"></span></td>
  </tr>
  <tr>
    <th><#IP4_Addr#> LAN:</th>
    <td><span id="LANIP4"></span></td>
  </tr>
  <tr>
    <th><#Gateway#> LAN:</th>
    <td><span id="LANGW4"></span></td>
  </tr>
  <tr>
    <th>DNS:</th>
    <td><span id="LANDNS"></span></td>
  </tr>
  <tr>
    <th><#MAC_Address#></th>
    <td><span id="LANMAC"></span></td>
  </tr>
  <tr>
    <td width="50%">&nbsp;</td>
    <td>
        <select id="domore" class="domore" onchange="domore_link(this);">
          <option selected="selected"><#MoreConfig#>...</option>
          <option value="../Advanced_APLAN_Content.asp"><#menu5_2_1#></option>
          <option value="../Advanced_Switch_Content.asp"><#menu5_2_5#></option>
          <option value="../Advanced_WOL_Content.asp"><#menu5_2_6#></option>
        </select>
    </td>
  </tr>
</table>

</body>
</html>

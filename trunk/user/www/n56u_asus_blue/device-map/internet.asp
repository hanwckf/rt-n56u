<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Untitled Document</title>
<link rel="stylesheet" type="text/css" href="../NM_style.css">
<link rel="stylesheet" type="text/css" href="../form_style.css">

<script type="text/javascript" src="formcontrol.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/detectWAN.js"></script>
<script>
<% wanlink(); %>

function initial(){
	flash_button();
	detectWANstatus();
	setTimeout("update_wanip();",2000);
	
	if(sw_mode == "4"){
		$("#domore")[0].remove(4);
		$("#domore")[0].remove(3);
		$("#domore")[0].remove(2);
	}
	
	var wantime = wanlink_time();
	if (wantime > 0){
		var updays, uphours, upminutes;
		updays = Math.floor(wantime / 86400);
		upminutes = Math.floor(wantime / 60);
		uphours = (Math.floor(upminutes / 60)) % 24;
		upminutes = upminutes % 60;
		uphours = uphours < 10 ? ('0'+uphours) : uphours;
		upminutes = upminutes < 10 ? ('0'+upminutes) : upminutes;
		$("WANTime").innerHTML = updays + "<#Day#>".substring(0,1) + " " + uphours+"<#Hour#>".substring(0,1) + " " + upminutes+"<#Minute#>".substring(0,1);
		$("row_ppp_time").style.display = "";
	}
	
	var wantype = wanlink_type();
	if(wantype == 'Automatic IP')
		$("WANType").innerHTML = 'IPoE: <#BOP_ctype_title1#>';
	else if(wantype == 'Static IP')
		$("WANType").innerHTML = 'IPoE: <#BOP_ctype_title5#>';
	else
		$("WANType").innerHTML = wantype;
	
	showtext($j("#wan_status")[0], wanlink_statusstr());
	
	$("WANEther").innerHTML = wanlink_etherlink();
	$("WANIP4").innerHTML   = wanlink_ip4_wan();
	$("WANGW4").innerHTML   = wanlink_gw4_wan();
	$("WANDNS").innerHTML   = wanlink_dns();
	
	if (wanlink_ip4_man() != ''){
		$("MANIP4").innerHTML = wanlink_ip4_man();
		$("MANGW4").innerHTML = wanlink_gw4_man();
		$("row_man_ip4").style.display = "";
		$("row_man_gw4").style.display = "";
	}
}

function update_wanip(e) {
  $.ajax({
    url: '/status.asp',
    dataType: 'script', 
	
    error: function(xhr) {
      ;
    },
    success: function(response) {
			var old_wan_status = $("#wan_status")[0].innerHTML;
	    if(wanlink_statusstr() != old_wan_status)
				refreshpage();
			else
				setTimeout("update_wanip();", 2000);
    }
  });
}

function getWANStatus(){
	if("<% detect_if_wan(); %>" == "1" && wanlink_statusstr() == "Connected")
		return 1;
	else
		return 0;
}

function submitWANAction(status){
	//var status = getWANStatus();

	switch(status){
		case 0:
			parent.showLoading();
			setTimeout('location.href = "/device-map/wan_action.asp?wanaction=Connect";', 1);
			break;
		case 1:
			parent.showLoading();
			setTimeout('location.href = "/device-map/wan_action.asp?wanaction=Disconnect";', 1);
			break;
		default:
			alert("No change!");
	}
}

function goQIS(){
	parent.showLoading();
	parent.location.href = '/QIS_wizard.htm';
}

function sbtnOver(o){
	o.style.color = "#FFFFFF";		
	o.style.background = "url(/images/sbtn.gif) #FFCC66";
	o.style.cursor = "pointer";
}

function sbtnOut(o){
	o.style.color = "#000000";
	o.style.background = "url(/images/sbtn0.gif) #FFCC66";
}
</script>
</head>

<body class="statusbody" onload="initial();">
<form method="post" name="internetForm">
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="120"><#ConnectionStatus#>:</th>
    <td width="150">
      <div style="display:none"><span id="connectstatus"></span></div>
      <input type="button" id="connectbutton_link" class="button" value="<#Connect#>" onclick="submitWANAction(0);">
      <input type="button" id="connectbutton_nolink" style="margin-top:3px" class="button" value="<#Disconnect#>" onclick="submitWANAction(1);">
    </td>
  </tr>
  <tr>
    <th><#SwitchState#></th>
    <td><span id="WANEther"></span></td>
  </tr>
  <tr>
    <th><#Connectiontype#>:</th>
    <td><span id="WANType"></span></td>
  </tr>
  <tr id="row_ppp_time" style="display:none">
    <th><#PPP_Uptime#></th>
    <td><span id="WANTime"></span></td>
  </tr>
  <tr>
    <th><#IP4_Addr#> WAN:</th>
    <td><span id="WANIP4"></span><span id="wan_status" style="display:none"></span></td>
  </tr>
  <tr id="row_man_ip4" style="display:none">
    <th><#IP4_Addr#> MAN:</th>
    <td><span id="MANIP4"></span></td>
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
    <th><#QIS#>:</th>
    <td><input type="button" class="sbtn" value="<#btn_go#>" onclick="javascript:goQIS();" onmouseover="sbtnOver(this);" onmouseout="sbtnOut(this);"></td>
  </tr>
</table>
</form>
<select id="domore" class="domore" onchange="domore_link(this);">
  <option selected="selected"><#MoreConfig#>...</option>
  <option value="../Advanced_WAN_Content.asp"><#menu5_3_1#></option>
  <option value="../Advanced_VirtualServer_Content.asp"><#menu5_3_4#></option>
  <option value="../Advanced_Exposed_Content.asp"><#menu5_3_5#></option>  
  <option value="../Advanced_ASUSDDNS_Content.asp"><#menu5_3_6#></option>
  <!--option value="../Main_IPTStatus_Content.asp"><#menu5_7_5#></option-->
</select>
</body>
</html>

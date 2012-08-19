<!DOCTYPE html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Untitled Document</title>
<!--
<link rel="stylesheet" type="text/css" href="../NM_style.css">
<link rel="stylesheet" type="text/css" href="../form_style.css"> -->
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="formcontrol.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/detectWAN.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script>
<% wanlink(); %>

var $j = jQuery.noConflict();

function initial(){
	flash_button();
	detectWANstatus();
	setTimeout("update_wanip();",2000);
	
	if(sw_mode == "4"){
		$j("#domore")[0].remove(4);
		$j("#domore")[0].remove(3);
		$j("#domore")[0].remove(2);
	}
	
	showtext($j("#WANEther")[0], wanlink_etherlink());
	showtext($j("#WANIP")[0], wanlink_ipaddr());
	showtext($j("#wan_status")[0], wanlink_statusstr());

	var dnsArray = wanlink_dns().split(" ");
	if(dnsArray[0])
		showtext($j("#DNS1")[0], dnsArray[0]);
	if(dnsArray[1])
		showtext($j("#DNS2")[0], dnsArray[1]);
	
	var wantype = wanlink_type();
	if(wantype=='Automatic IP')
		showtext($j("#connectionType")[0], '<#BOP_ctype_title1#>');
	else	
		showtext($j("#connectionType")[0], wantype);
	
	showtext($j("#gateway")[0], wanlink_gateway());
}

function update_wanip(e) {
  $j.ajax({
    url: '/status.asp',
    dataType: 'script', 
	
    error: function(xhr) {
      ;
    },
    success: function(response) {
        var old_wan_status = $j("#wan_status")[0].innerHTML;
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

/*
function sbtnOver(o){
	o.style.color = "#FFFFFF";		
	o.style.background = "url(/images/sbtn.gif) #FFCC66";
	o.style.cursor = "pointer";
}

function sbtnOut(o){
	o.style.color = "#000000";
	o.style.background = "url(/images/sbtn0.gif) #FFCC66";
}
*/
</script>
</head>

<body class="body_iframe" onload="initial();">
<br>
<form method="post" name="internetForm">
<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%" style="border-top: 0 none;"><#ConnectionStatus#>:</th>
    <td style="border-top: 0 none;">
    <div style="display:none"><span id="connectstatus"></span></div>
    <input type="button" id="connectbutton_link" class="btn btn-info" value="<#Connect#>" onclick="submitWANAction(0);">
    <input type="button" id="connectbutton_nolink" class="btn btn-danger" class="button" value="<#Disconnect#>" onclick="submitWANAction(1);">
    </td>
  </tr>
  <tr>
    <th><#SwitchState#></th>
    <td><span id="WANEther"></span></td>
  </tr>
  <tr>
    <th><#WAN_IP#>:</th>
    <td><span id="WANIP"></span><span id="wan_status" style="display:none"></span></td>
  </tr>
  <tr>
    <th>DNS:</th>
    <td><span id="DNS1"></span><br><span id="DNS2"></span></td>
  </tr>
  <tr>
    <th><#Connectiontype#>:</th>
    <td><span id="connectionType"></span></td>
  </tr>
  <tr>
    <th><#Gateway#>:</th>
    <td><span id="gateway"></span></td>
  </tr>
</table>
</form>

<table width="100%" align="center" class="table">
    <tr>
        <td width="50%">&nbsp;</td>
        <td>
            <select id="domore" class="domore" onchange="domore_link(this);">
              <option selected="selected"><#MoreConfig#>...</option>
              <option value="../Advanced_WAN_Content.asp"><#menu5_3_1#></option>
              <option value="../Advanced_PortTrigger_Content.asp"><#menu5_3_3#></option>
              <option value="../Advanced_VirtualServer_Content.asp"><#menu5_3_4#></option>
              <option value="../Advanced_Exposed_Content.asp"><#menu5_3_5#></option>
              <option value="../Advanced_ASUSDDNS_Content.asp"><#menu5_3_6#></option>
              <!--option value="../Main_IPTStatus_Content.asp"><#menu5_7_5#></option-->
            </select>
        </td>
    </tr>
</table>
</body>
</html>

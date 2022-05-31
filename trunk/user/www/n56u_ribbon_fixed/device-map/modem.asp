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

var modem_ports_array = parent.modem_ports();
var modem_devnum_array = parent.modem_devnum();
var modem_types_array = parent.modem_types();
var modem_models_array = parent.modem_models();
var modem_vendors_array = parent.modem_manufacts();

var modem_order = parent.get_clicked_device_order();

var modem_active = '<% nvram_get_x("", "wan0_modem_dev"); %>';

function initial(){
	if(modem_models_array.length > 0 ) {
		showtext($("modem_name"), modem_models_array[modem_order]);
		showtext($("modem_vend"), modem_vendors_array[modem_order]);
		showtext($("modem_type"), modem_types_array[modem_order]);
	}
	
	if (modem_active == modem_devnum_array[modem_order]) {
		$("remove_status").style.display = "none";
		$("remove_button").style.display = "";
	}
}

function go_modem_config(){
	parent.location.href = "/Advanced_Modem_others.asp";
}

function remove_modem(){
	var str = "<#USB_Modem_remove_confirm#>";
	
	if(confirm(str)){
		parent.showLoading();
		
		document.diskForm.action = "safely_remove_disk.asp";
		document.diskForm.port.value = modem_ports_array[modem_order];
		document.diskForm.devn.value = modem_devnum_array[modem_order];
		document.diskForm.submit();
	}
}
</script>
</head>

<body class="body_iframe" onload="initial();">

<table width="100%" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%" style="border-top: 0 none;"><#Modelname#>:</th>
    <td style="border-top: 0 none;"><span id="modem_name"></span></td>
  </tr>
  <tr>
    <th><#Manufacturer#>:</th>
    <td><span id="modem_vend"></span></td>
  </tr>
  <tr>
    <th width="50%"><#ModemType#></th>
    <td <span id="modem_type"></span></td>
  </tr>
  <tr>
    <th width="50%"><#GO_HSDPA_SETTING#>:</th>
    <td>
        <input type="button" class="btn span2" onclick="go_modem_config();" value="<#btn_go#>">
    </td>
  </tr>
  <tr id="remove_button" style="display:none;">
    <th width="50%"><#HSDPAConfig_safely_remove#>:</th>
    <td>
        <input type="button" class="btn btn-success span2" onclick="remove_modem();" value="<#btn_remove#>">
    </td>
  </tr>
  <tr id="remove_status">
    <th width="50%"><#HSDPAConfig_safely_remove#>:</th>
    <td>
        <span class="label label-success"><#USB_Modem_unused#></span>
    </td>
  </tr>
</table>

<form method="post" name="diskForm" action="">
<input type="hidden" name="port" value="">
<input type="hidden" name="devn" value="">
</form>
</body>
</html>

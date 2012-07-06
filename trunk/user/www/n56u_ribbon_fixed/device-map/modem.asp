<!DOCTYPE html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/state.js"></script>
<script>
var modem_running = '<% nvram_get_x("", "modem_running"); %>';
var d3g = "";
var modemOrder = parent.getSelectedModemOrder();
if(modemOrder == 1)
	d3g = '<% nvram_get_x("", "usb_path2_product"); %>';
else
	d3g = '<% nvram_get_x("", "usb_path1_product"); %>';

function initial(){
	showtext($("disk_model_name"), d3g);
	
	if(modem_running == '1')
		$("remove_table").style.display = "";
}

function goHspdaWizard(){
	parent.location.href = "/Advanced_Modem_others.asp";
}

function remove_d3g(){
	var str = "Do you really want to remove this USB dongle?";
	
	if(confirm(str)){
		parent.showLoading();
		
		document.diskForm.action = "safely_remove_disk.asp";
		document.diskForm.disk.value = modemOrder+1;
		setTimeout("document.diskForm.submit();", 1);
	}
}
</script>
</head>

<body class="body_iframe" onload="initial();">

<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%" style="border-top: 0 none;"><#Modelname#></th>
    <td><span id="disk_model_name"></span></td>
  </tr>
</table>

<table id="mounted_item1" width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%"><#GO_HSDPA_SETTING#></th>
    <td><input type="button" class="btn btn-primary" style="width: 170px" onclick="goHspdaWizard();" value="<#btn_go#>" ></td>
  </tr>
</table>

<table id="remove_table" style="display:none;" width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%"><#HSDPAConfig_safely_remove#></th>
    <td>
        <input id="show_remove_button" type="button" class="btn btn-primary" style="width: 170px" onclick="remove_d3g();" value="<#btn_remove#>">
        <div id="show_removed_string" style="display:none;"><#Safelyremovedisk#></div>
    </td>
  </tr>
</table>

<div id="mounted_item2" class="alert alert-info"></div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="disk" value="">
</form>
</body>
</html>

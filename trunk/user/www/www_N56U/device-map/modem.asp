<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../NM_style.css" rel="stylesheet" type="text/css" />
<link href="../form_style.css" rel="stylesheet" type="text/css" />

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

<body class="statusbody" onload="initial();">
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#Modelname#></th>
    <td><span id="disk_model_name"></span></td>
  </tr>
</table>

<table id="mounted_item1" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#GO_HSDPA_SETTING#></th>
    <td><input type="button" class="button" onclick="goHspdaWizard();" value="<#btn_go#>" ></td>
  </tr>
</table>

<table id="remove_table" style="display:none;" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
		<th width="130px"><#HSDPAConfig_safely_remove#></th>
    <td>
	<input id="show_remove_button" type="button" class="button" onclick="remove_d3g();" value="<#btn_remove#>">
    <div id="show_removed_string" style="display:none;"><#Safelyremovedisk#></div>
    </td>
  </tr>
</table>

<div id="mounted_item2" style="padding:5px 0px 5px 25px; ">
</div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="disk" value="">
</form>
</body>
</html>

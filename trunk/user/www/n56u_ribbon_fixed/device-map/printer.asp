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
var printer_manufacts_array = parent.printer_manufacts();
var printer_models_array = parent.printer_models();
var printer_order = parent.get_clicked_device_order();

function initial(){
	if(printer_models_array.length > 0 ) {
		showtext($("printerModel"), printer_models_array[printer_order]);
		showtext($("printerStatus"), '<#CTL_Enabled#>');
	}
}

function u2ec_monopolize(){
	parent.showLoading(2);
	
	document.form.action_mode.value = "Update";
	document.form.action_script.value = "mfp_monopolize";
	document.form.current_page.value = "";
	document.form.next_page.value = "device-map/printer.asp";
	document.form.submit();
}
</script>
</head>

<body class="body_iframe" onload="initial();">

<form method="post" name="form" action="/start_apply.htm">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">

<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
  <tr>
    <th width="50%"><span class="top-messgae"><#PrinterStatus_x_PrinterModel_itemname#></span></th>
    <td><span id="printerModel"></span></td>
  </tr>
  <tr>
    <th><span class="top-messgae"><#Printing_status#></span></th>
    <td><span id="printerStatus"></span></td>
  </tr>
  <tr>
    <th><#Printing_button_item#></th>
    <td><input type="button" class="btn btn-info span2" value="<#btn_Enable#>" onclick="u2ec_monopolize();"></td>
  </tr>
</table>

<div class="alert alert-info">
    <ul>
        <li><#PrinterStatus_x_Monopoly_itemdesc#></li>
    </ul>
</div>

</form>
</body>
</html>

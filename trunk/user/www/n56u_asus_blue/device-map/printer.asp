<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Untitled Document</title>
<link href="../NM_style.css" rel="stylesheet" type="text/css">
<link rel="stylesheet" type="text/css" href="../form_style.css">
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script>
var printer_manufacturer_array = parent.printer_manufacturers();
var printer_model_array = parent.printer_models(); 
var printer_pool_array = parent.printer_pool();
var printer_order = parent.get_clicked_device_order();

function initial(){
	if(printer_model_array.length > 0 ) {
		showtext($("printerModel"), printer_manufacturer_array[printer_order]+" "+printer_model_array[printer_order]);
		
		if(printer_pool_array[printer_order] != ""){
			showtext($("printerStatus"), '<#CTL_Enabled#>');
			$("printer_button").style.display = "";
			$("button_descrition").style.display = "";
		}
		else{
			showtext($("printerStatus"), '<#CTL_Disabled#>');
			$("printer_button").style.display = "none";
			$("button_descrition").style.display = "none";
		}
	}
	else
		showtext($("printerStatus"), '<% translate_x("System_Internet_Details_Item5_desc2"); %>');
}

function cleanTask(){
	parent.showLoading(3);
	
	document.form.action_mode.value = "Update";
	document.form.action_script.value = "mfp_monopolize";
	document.form.current_page.value = "";
	document.form.next_page.value = "device-map/printer.asp";
	
	document.form.submit();
}
</script>
</head>

<body class="statusbody" onload="initial();">

<form method="post" name="form" action="/start_apply.htm">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="current_page" value="Main_GStatus_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">

<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
	<tr>
		<th width="50%"><span class="top-messgae"><#PrinterStatus_x_PrinterModel_itemname#></span></th>
		<td width="130"><span id="printerModel"></span></td>
	</tr>
	<tr>
		<th><span class="top-messgae"><#Printing_status#></span></th>
		<td><span id="printerStatus"></span></td>
	</tr>
	<tr id="printer_button" style="display:none;">
		<th><#Printing_button_item#></th>
		<td><input type="button" class="button" value="<#btn_Enable#>" onclick="cleanTask();"></td>
	</tr>
</table>

<div id="button_descrition" style="display:none;padding:5px 0px 5px 25px;">
<ul style="font-size:11px; font-family:Arial; padding:0px; margin:0px; list-style:outside; line-height:150%;">
	<li><#PrinterStatus_x_Monopoly_itemdesc#></li>
</ul>
</div>
</form>
</body>
</html>

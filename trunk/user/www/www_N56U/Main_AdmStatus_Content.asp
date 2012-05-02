<html>
<head>
<title><#ZVMODELVZ#> Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script language="javascript">
function onSubmitCtrl(o, s) {
	document.form.action_mode.value = s;
	return true;
}
</script>
</head>  

<body onLoad="document.form.SystemCmd.focus();" >
<form method="GET" name="form" action="/apply.cgi"> 
<input type="hidden" name="current_page" value="Main_AdmStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_AdmStatus_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" value="<% nvram_get_x("","preferred_lang"); %>">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
	<thead>
	<tr>
		<td colspan="2" height="30">System Command</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td>
			<input class="input" style="font-size:12pt;" type="text" maxlength="255" size="52" name="SystemCmd" onkeydown="onSubmitCtrl(this, ' Refresh ')" value="">
			
			<input class="button" onClick="onSubmitCtrl(this, ' Refresh ')" type="submit" value="<#CTL_refresh#>" name="action">
		</td>
	</tr>
	<tr>
		<td>
			<textarea style="font-size:12pt;" cols="63" rows="20" wrap="off" readonly="1"><% nvram_dump("syscmd.log","syscmd.sh"); %></textarea>
		</td>
	</tr>
	</tbody>
</table>	

</form>
</body>
</html>

<html>
<head>
<title><#ZVMODELVZ#> Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
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

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span12">
            System Command
        </div>
    </div>

    <div class="row-fluid">
        <div class="span10">
            <input type="text" class="span12" name="SystemCmd" onkeydown="onSubmitCtrl(this, ' Refresh ')" value="">
        </div>
        <div class="span2">
            <input class="btn btn-primary" style="width: 219px;" onClick="onSubmitCtrl(this, ' Refresh ')" type="submit" value="<#CTL_refresh#>" name="action">
        </div>
    </div>

    <div class="row-fluid">
        <div class="span12">
            <textarea class="span12" style="font-size:12pt;" rows="20" wrap="off" readonly="1"><% nvram_dump("syscmd.log","syscmd.sh"); %></textarea>
        </div>
    </div>


</div>

</form>
</body>
</html>

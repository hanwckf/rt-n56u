<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script>
var protocol = '<% get_parameter("protocol"); %>';
var flag = '<% get_parameter("flag"); %>';

function set_account_permission_error(error_msg){
	if(flag == "aidisk_wizard")
		parent.alert_error_msg("<#AiDisk_Wizard_failedreson1#>");
	else
		parent.alert_error_msg(error_msg);
}

function set_account_permission_success(){
	parent.submitChangePermission(protocol);
}
</script>
</head>

<body>

<% set_account_permission(); %>

</body>
</html>

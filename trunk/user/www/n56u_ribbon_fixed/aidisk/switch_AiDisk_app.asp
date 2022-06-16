<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<script>
function set_AiDisk_status_error(error_msg){
	alert(error_msg);
	parent.resultOfSwitchAppStatus(error_msg);
}

function set_AiDisk_status_success(){
	parent.resultOfSwitchAppStatus();
}
</script>
</head>

<body onload="set_AiDisk_status_success();">

<% set_AiDisk_status(); %>

</body>
</html>

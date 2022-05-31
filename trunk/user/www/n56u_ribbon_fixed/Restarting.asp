<!DOCTYPE html>
<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script>
var action_mode = '<% get_parameter("action_mode"); %>';
var boot_time = parent.board_boot_time();

function redirect(){
	setTimeout("redirect1();", (boot_time+2)*1000);
}

function redirect1(){
	if(action_mode == " RestoreNVRAM ")
		parent.location.href = "http://192.168.2.1/";
	else
		parent.location.href = "/";
}
</script>
</head>
<body onLoad="redirect();">
<script>
	parent.hideLoading();
	parent.showLoading(boot_time, "waiting");
</script>
</body>
</html>

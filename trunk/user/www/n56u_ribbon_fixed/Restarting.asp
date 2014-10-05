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

function redirect(){
	setTimeout("redirect1();", 40000);
}

function redirect1(){
	setTimeout("parent.hideLoading()", 1000);
	if(action_mode == " RestoreNVRAM "){
		parent.location.href = "http://192.168.1.1/QIS_wizard.htm?flag=detect";
	}else{
		parent.location.href = "/";
	}
}
</script>
</head>
<body onLoad="redirect();">
<script>
	parent.hideLoading();
	parent.showLoading(40, "waiting");
</script>
</body>
</html>

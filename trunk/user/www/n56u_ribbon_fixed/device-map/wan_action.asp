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

var restart_time = 3;

function restart_needed_time(second){
	restart_time = second;
}

function Callback(){
	parent.showLoading(restart_time);
	setTimeout("document.redirectForm.submit();", restart_time*1000);
}

</script>
</head>

<body onLoad="Callback();">
<% wan_action(); %>

<form method="post" name="redirectForm" action="/" target="_parent">
<input type="hidden" name="flag" value="Internet">
</form>
</body>
</html>

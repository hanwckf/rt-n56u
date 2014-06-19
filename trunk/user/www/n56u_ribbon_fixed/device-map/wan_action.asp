<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<script type="text/javascript">

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

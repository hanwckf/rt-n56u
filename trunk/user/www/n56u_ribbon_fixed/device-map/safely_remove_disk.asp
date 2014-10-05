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

function safely_remove_disk_error(error_msg){
	parent.alert_error_msg(error_msg);
}

function safely_remove_disk_success(){
	parent.showLoading(1);
	parent.refreshpage(1);
}

</script>
</head>

<body>
<% safely_remove_disk(); %>

</body>
</html>

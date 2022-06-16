<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script>
function delete_account_error(error_msg){
	parent.alert_error_msg(error_msg);
}

function delete_account_success(){
	parent.refreshpage();
}
</script>
</head>

<body>

<% delete_account(); %>

</body>
</html>

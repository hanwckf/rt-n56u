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
function create_account_error(error_msg){
	parent.alert_error_msg(error_msg);
}

function create_account_success(){
	parent.resultOfCreateAccount();
}
</script>
</head>

<body>

<% create_account(); %>

</body>
</html>

<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script type="text/javascript">
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

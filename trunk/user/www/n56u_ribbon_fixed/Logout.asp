<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<script src="jquery.js"></script>
<script src="state.js"></script>
<style>
.alertX{
  padding: 10px;
  margin:30px 10px 10px 10px;
  border: 1px solid #bce8f1;
  border-radius: 4px;
  max-width: 400px;
  text-align:left;
  background:#FEFEFE;
}
</style>
<script>
function initial(){
	var xmlhttp=(window.XMLHttpRequest)?new XMLHttpRequest():new ActiveXObject("Microsoft.XMLHTTP");
	xmlhttp.open("HEAD","logout",true,"logout","");
	xmlhttp.send(null);
}
</script>
</head>
<body onload="initial()">
	<div id="logo"></div>
	<div class="alertX">
                <h2><#logoutmessage#></h2>
                <p><#Not_authpage_login_again#></p>
	</div>
	</div>
</body>
</html>

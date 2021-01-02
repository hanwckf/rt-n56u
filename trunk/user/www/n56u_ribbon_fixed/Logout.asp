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

<script>
function initial(){
	var xmlhttp;
	try{
		if (window.XMLHttpRequest)
			xmlhttp=new XMLHttpRequest();
		else
			xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}catch (e){
		xmlhttp=null;
	}
	if (xmlhttp != null){
		xmlhttp.open("HEAD","logout",true,"logout","");
		xmlhttp.send(null);
	}
}
</script>
</head>
<body onload="initial()">
    <div style="max-width:300px;margin:150px auto 0 auto;">
        <center>
            <div class="well">
                <h2><#logoutmessage#></h2>

                <div><#Not_authpage_login_again#></div>
            </div>
        </center>
    </div>
</body>
</html>

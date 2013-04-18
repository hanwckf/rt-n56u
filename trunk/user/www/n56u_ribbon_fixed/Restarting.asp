<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router Web Manager</title>
<link rel="stylesheet" type="text/css" href="other.css">

<script>
function redirect(){
	setTimeout("redirect1();", 40000)
}

function redirect1(){
	if(parent.lan_ipaddr == "192.168.1.1"){
		//if(navigator.appName.indexOf("Microsoft") >= 0){
			parent.parent.location.href = "http://192.168.1.1/QIS_wizard.htm?flag=detect";
	}else{
		parent.$('drword').innerHTML = "<#Setting_factorydefault_iphint#><br/>";
		setTimeout("parent.hideLoading()",1000);
		setTimeout("parent.dr_advise();",1000);
		parent.parent.location.href = "http://192.168.1.1/QIS_wizard.htm?flag=detect";
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
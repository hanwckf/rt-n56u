<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../other.css" rel="stylesheet" type="text/css">

<script>
function pppoeLink(){
	parent.showLoading();
	
	with(document.redirectForm){
		action = "/QIS_wizard.htm";
		target = "_parent";
		
		flag.value = "pppoe";
		
		submit();
	}
}

function detectLink(){
	parent.showLoading();
	
	with(document.redirectForm){
		action = "/QIS_wizard.htm";
		target = "_parent";
		
		flag.value = "detect";
		
		submit();
	}
}
</script>
</head>

<body class="diagnosebody">
<form method="post" name="redirectForm">
<input type="hidden" name="flag" value="">
</form>

<span class="diagnose_title"><#web_redirect_fail_reason0#>:</span>
<div id="failReason" class="diagnose_suggest2" style="color:#CC0000"><#web_redirect_reason2_1#></div>
<br/>

<span class="diagnose_title"><#web_redirect_suggestion0#>:</span>
<div class="diagnose_suggest">
<ul>
  <li>
	  <a href="javascript:pppoeLink();"><#web_redirect_suggestion2_1#></a>
	  <span><#web_redirect_suggestion2_1_desc#></span>
  </li>
  <li>
	  <a href="javascript:detectLink();"><#web_redirect_suggestion2_2#></a>
	  <span><#web_redirect_suggestion2_2_desc#></span>
  </li>
</ul>  
</div>
</body>
</html>

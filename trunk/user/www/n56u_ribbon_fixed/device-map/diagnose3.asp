<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../other.css"  rel="stylesheet" type="text/css">

<script>
function wanLink(){
	parent.showLoading();
	parent.location.href = "/Advanced_WAN_Content.asp";
	with(document.redirectForm){
		action = "/Advanced_WAN_Content.asp";
		target = "_parent";
		
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
<div id="failReason" class="diagnose_suggest2" style="color:#CC0000"><#web_redirect_reason3_1#></div>
<br/>
<span class="diagnose_title"><#web_redirect_suggestion0#>:</span>
<div class="diagnose_suggest">
<ul>
<li>
  <span><#web_redirect_suggestion3#></span>
</li>
<li>
  <span><#web_redirect_suggestion_final#>
  <a href="javascript:wanLink();"><#web_redirect_suggestion_etc#></a>
  <#web_redirect_suggestion_etc_desc#></span>
</li>
</ul>
</div>
</body>
</html>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../other.css"  rel="stylesheet" type="text/css">
<script>
function indexLink(){
	parent.showLoading();
	
	with(document.redirectForm){
		action = "/";
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

<span class="diagnose_title"><#DrSurf_Alert_detect#>:</span>
<div id="failReason" class="diagnose_suggest2" style="color:#CC0000"><#DrSurf_Alert21#></div>
<br/>
<span class="diagnose_title"><#web_redirect_suggestion0#>:</span>
<div class="diagnose_suggest2">
	<a href="javascript:indexLink();">
	<span><#DrSurf_suggestion10#></span>
	</a>
</div>
</body>
</html>

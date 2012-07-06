<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title></title>
<link type="text/css" rel="stylesheet" href="../other.css">

<script type="text/javascript" src="/state.js"></script>
<script>
function initial(){
	var html_code = '';
	
	showtext($("desc_str"), "<#Web_Title#> don't get the IP address yet! Please wait a moment or check the system log.");
	
	html_code += '<ul>\n';
	html_code += '<li><a href="javascript:syslogLink();">Watch the current dial states...</a></li>\n';
	html_code += '<li>\n';
	html_code += '<span id="suggestion_final"><#web_redirect_suggestion_final#></span>\n';
	html_code += '<a id="selfsetting_link" href="javascript:ModemLink();"><#web_redirect_suggestion_etc#></a>\n';
	html_code += '<span><#web_redirect_suggestion_etc_desc#></span>\n';
	html_code += '</li>\n';
	html_code += '</ul>\n';
	
	$("sug_code").innerHTML = html_code;
}

function syslogLink(){
	location.href = "/Main_LogStatus_Content.asp";
}

function ModemLink(){
	location.href = "/Advanced_Modem_others.asp";
}
</script>
</head>
<body onload="initial();" class="diagnosebody">

<span class="diagnose_title"><#web_redirect_fail_reason0#>:</span>
<div id="failReason" class="diagnose_suggest2" style="color:#CC0000">		
<ul>
<li>
  <span id="desc_str" style="color:#CC0000"></span>
</li>
</ul>
</div>
<br/>

<span class="diagnose_title"><#web_redirect_suggestion0#>:</span>

<div id="sug_code" class="diagnose_suggest"></div>

</body>
</html>

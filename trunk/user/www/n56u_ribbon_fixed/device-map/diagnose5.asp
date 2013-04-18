<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title></title>
<link type="text/css" rel="stylesheet" href="../other.css">

<script type="text/javascript" src="/state.js"></script>
<script>
var manually_stop_wan = '<% nvram_get_x("", "manually_disconnect_wan"); %>';

function initial(){
	var html_code = '';
	
	if(manually_stop_wan == "1")
		showtext($("desc_str"), "<#web_redirect_reason5_1#>");
	else
		showtext($("desc_str"), "<#web_redirect_reason5_2#>");
	
	html_code += '<ul>\n';
	if(manually_stop_wan == "1")
		html_code += '<li><a href="javascript:manually_start_wan_Link();"><#web_redirect_suggestion_manually_start_wan#></a></li>\n';
	else
		html_code += '<li><span><#web_redirect_suggestion5_2#></span></li>\n';
	
	html_code += '<li><span><#web_redirect_suggestion_final#><a href="javascript:wanLink();"><#web_redirect_suggestion_etc#></a><#web_redirect_suggestion_etc_desc#></span></li>\n';
	html_code += '</ul>\n';
	
	$("sug_code").innerHTML = html_code;
}

function wanLink(){
	parent.location.href = "/Advanced_WAN_Content.asp";
}

function manually_start_wan_Link(){
	parent.location.href = "/index.asp?flag=Internet";
}
</script>
</head>
<body onload="initial();" class="diagnosebody">

<span class="diagnose_title"><#web_redirect_fail_reason0#>:</span>
<div id="failReason" class="diagnose_suggest">		
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

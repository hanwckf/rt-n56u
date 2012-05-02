<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_7_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

function showclock(){
	JS_timeObj.setTime(systime_millsec);
	systime_millsec += 1000;
	
	JS_timeObj2 = JS_timeObj.toString();	
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  JS_timeObj.getFullYear() + " GMT" +
				  timezone;
	$("system_time").value = JS_timeObj2;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "698";
    //$("banner3").style.height = "13";
}

function showbootTime(){
	Days = Math.floor(boottime / (60*60*24));	
	Hours = Math.floor((boottime / 3600) % 24);
	Minutes = Math.floor(boottime % 3600 / 60);
	Seconds = Math.floor(boottime % 60);
	
	$("boot_days").innerHTML = Days;
	$("boot_hours").innerHTML = Hours;
	$("boot_minutes").innerHTML = Minutes;
	$("boot_seconds").innerHTML = Seconds;
	boottime += 1;
	setTimeout("showbootTime()", 1000);
}
function clearLog(){
	document.form1.target = "hidden_frame";
	document.form1.action_mode.value = " Clear ";
	document.form1.submit();
	location.href = location.href;
}
</script>
</head>

<body onload="show_banner(2); show_menu(5,7,1); show_footer();load_body();showclock(); showbootTime();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" >
<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_LogStatus_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
    	<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div><br/>
			
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top" >
					<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle">
						<thead>
						<tr>
							<td><#menu5_7#> - <#menu5_7_2#></td>
						</tr>
						</thead>
						<tbody>
						<tr>
							<td bgcolor="#FFFFFF">
								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
									<tr>
										<th width="30%"><#General_x_SystemTime_itemname#>:</th>
										<td>
											<input type="text" id="system_time" name="system_time" size="40" class="devicepin" value="" readonly="1" style="font-size:12px;">											
										</td>										
									</tr>
									<tr>
										<th><#General_x_SystemUpTime_itemname#></th>
										<td><span id="boot_days"></span> <#Day#> <span id="boot_hours"></span> <#Hour#> <span id="boot_minutes"></span> <#Minute#> <span id="boot_seconds"></span> <#Second#></td>
									</tr>
								</table>
							</td>
						</tr>
						<tr>
							<td align="center" bgcolor="#FFFFFF">
								<textarea cols="63" rows="20" wrap="off" readonly="readonly" id="textarea" style="width:99%; font-family:'Courier New', Courier, mono; font-size:11px;" ><% nvram_dump("syslog.log","syslog.sh"); %></textarea>
							</td>
						</tr>
						</tbody>
					</table>
			<table width="300" border="0" align="right" cellpadding="2" cellspacing="0">
				<tr align="center">
					<td>
<form method="post" name="form1" action="apply.cgi">
<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
<input type="hidden" name="action_mode" value=" Clear ">
<input type="hidden" name="next_host" value="">
<input type="submit" onClick="document.form1.next_host.value = location.host; onSubmitCtrl(this, ' Clear ')" value="<#CTL_clear#>" class="button">
<!--input type="button" onClick="clearLog();" value="<#CTL_clear#>" class="button"-->
</form>
		</td>
		
		<td>
<form method="post" name="form2" action="syslog.cgi">
<input type="hidden" name="next_host" value="">
<input type="submit" onClick="document.form2.next_host.value = location.host; onSubmitCtrl(this, ' Save ')" value="<#CTL_onlysave#>" class="button">
</form>
					</td>
					
					<td>
<form method="post" name="form3" action="apply.cgi">
<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
<input type="hidden" name="action_mode" value=" Refresh ">
<input type="hidden" name="next_host" value="">
<!--input type="submit" onClick="document.form3.next_host.value = location.host; onSubmitCtrl(this, ' Refresh ')" value="<#CTL_refresh#>" class="button"-->
<input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="button">

</form>
					</td>
				</tr>
			</table>
		</form>
		</td>
		<!--td id="help_td" style="width:15px;" valign="top">
			<form name="hint_form"></form>
			<div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
			<img src="images/help.gif">
		</div>
	  
		<div id="hintofPM" style="display:none;">
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:closeHint();"><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tbody>
		  <tr>
			<td valign="top">
			  <div id="hint_body" class="hint_body2"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
		  </tbody>
		</table>
		</div>
		</td-->
	</tr>
</table>
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>
<div id="footer"></div>
</body>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript">
	jQuery.noConflict();
	(function($){
		var textArea = document.getElementById('textarea');
		textArea.scrollTop = textArea.scrollHeight;
	})(jQuery);
</script>
</html>
